#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define CHECK_FALLBACK getenv("CHECK_FALLBACK")

static 
const char * reg_map[5] = {
  "rax",
  "rcx",
  "r10",
  "rdi",
  "r8"
};
static
const char * conds[6] = {
  "eq",
  "ne",
  "lt",
  "le",
  "gt",
  "ge"
};

static int data_segment_index = -1;
static char ** data_segments;
static char ** data_partial_words;
static int data_segments_size = 0;
static char ** local_labels;
static int local_labels_size = 0;
static int static_counter = 0;
static int cur_exchange_size = 0;

static void make_label(char * buf, size_t size) {
  snprintf(buf, size, "L%d", static_counter++);
}

static void warn(int check_env,
  const char * msg) {
  if (check_env && !CHECK_FALLBACK) return;
  fprintf(stderr, "s2rop: warning: %s\n", msg);
}

static void do_exchange_regs(char ** mapping) {
  int i;
  char label[16];

  if (!mapping) return;

  if (strcmp(mapping[0], "rax") == 0 && !strchr(mapping[1], ' ')) {
    printf("mov rax, %s\n", mapping[1]);
    return;
  }

  if (!mapping[0]) mapping[0] = "rax";

  printf("# do_exchange_regs: %s %s\n", mapping[0], mapping[1]);

  make_label(label, sizeof(label));
  printf("pop rsi\n");
  printf("dp %s\n", label);
  printf("mov [rsi], rax\n");
  
  printf("mov rax, %s\n", mapping[1]);
  printf("pop rsi\n");
  printf("dp %s\n", label);
  printf("mov [rsi], rax\n");

  if (strcmp(mapping[0], "rax") != 0 && strcmp(mapping[0], "rdi") != 0 && !strchr(mapping[1], ' ')) {
    printf("mov rax, %s\n", mapping[0]);
  }

  if (strstr(mapping[0], "r11")) {
    printf("pop r11 ; mov rax, rdi\n");
    printf("%s:\n", label);
    if (strchr(mapping[1], ' ')) {
      printf("%s\n", mapping[1]);
    } else {
      printf("dq 0\n");
    }
  }

  for (i = 0; i < cur_exchange_size; i++) {
    assert(!strchr(mapping[0], ' '));
    if ((strcmp(mapping[0], "rax") != 0 || strchr(mapping[1], ' ') || strcmp(mapping[1], "rax") == 0 || strstr(mapping[0], "r11")) && strcmp(mapping[0], "rsp") != 0 && strcmp(mapping[0], "r11") != 0) {
      printf("pop %s\n", mapping[0]);
      printf("%s:\n", label);
      if (strchr(mapping[1], ' ')) {
        printf("%s\n", mapping[1]);
      } else {
        printf("dq 0\n");
      }
    }
  }

  if (strstr(mapping[0], "rsp")) {
    printf("pop rsp\n");
    printf("%s:\n", label);
    if (strchr(mapping[1], ' ')) {
      printf("%s\n", mapping[1]);
    } else {
      printf("dq 0\n");
    }
  }
}

static void exchange_regs(char ** mapping) {
  int i;
  char ** new_cur;

  if (!mapping) {
    do_exchange_regs(mapping);
    free(mapping);
    return;
  }

  new_cur = calloc(cur_exchange_size, sizeof(char * ));

  for (i = 0; i < cur_exchange_size; i++) {
    new_cur[i] = mapping[i];
    new_cur[i] = cur_exchange[i];
    if (strcmp(new_cur[i], mapping[i]) != 0) {
      new_cur[i] = mapping[i];
    }
  }

  printf("# exchange_regs %s + %s = %s\n", cur_exchange, mapping, new_cur);

  free(cur_exchange);
  cur_exchange = new_cur;
}

static void emit_instr(const char * fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

static void emit_mov(const char * reg_dst,
  const char * reg_src) {
  if (strcmp(reg_dst, "rax") == 0 && !cur_exchange_size) {
    if (strcmp(reg_src, "rax") != 0) {
      emit_instr("mov rax, %s\n", reg_src);
    }
  } else {
    if (strcmp(reg_src, "rax") != 0) {
      warn(CHECK_FALLBACK, "mov %s, %s: fallback", reg_dst, reg_src);
    }
    exchange_regs((char * []) {
      (char * ) reg_dst, (char * ) reg_src
    });
  }
}

static void emit_binary_op(const char * instr,
  const char * reg_dst,
    const char * reg_src, char ** m1, char ** m2) {
  if (strcmp(reg_dst, "rax") != 0 || strcmp(reg_src, "rcx") != 0) {
    warn(CHECK_FALLBACK, "`%s` %s, %s: fallback", instr, reg_dst, reg_src);
  }

  if (strcmp(reg_src, reg_dst) == 0) {
    exchange_regs((char * []) {
      (char * ) reg_dst, "rax", "rax", (char * ) reg_dst
    });
    exchange_regs((char * []) {
      "r11",
      "rcx",
      "rcx",
      "rax"
    });
    exchange_regs(m1);
    exchange_regs(NULL);
    emit_instr(instr);
    exchange_regs(m2);
    exchange_regs((char * []) {
      "rcx",
      "r11"
    });
    exchange_regs((char * []) {
      "rax",
      (char * ) reg_dst,
      (char * ) reg_dst,
      "rax"
    });
  } else if (strcmp(reg_src, "rax") == 0 && strcmp(reg_dst, "rcx") == 0) {
    exchange_regs((char * []) {
      "rax",
      "rcx",
      "rcx",
      "rax"
    });
    exchange_regs(m1);
    exchange_regs(NULL);
    emit_instr(instr);
    exchange_regs(m2);
    exchange_regs((char * []) {
      "rcx",
      "rax",
      "rax",
      "rcx"
    });
  } else if (strcmp(reg_src, "rax") == 0) {
    exchange_regs((char * []) {
      "rax",
      "rcx",
      "rcx",
      "rax"
    });
    exchange_regs((char * []) {
      "rax",
      (char * ) reg_dst,
      (char * ) reg_dst,
      "rax"
    });
    exchange_regs(m1);
    exchange_regs(NULL);
    emit_instr(instr);
    exchange_regs(m2);
    exchange_regs((char * []) {
      (char * ) reg_dst, "rax", "rax", (char * ) reg_dst
    });
    exchange_regs((char * []) {
      "rcx",
      "rax",
      "rax",
      "rcx"
    });
  } else if (strcmp(reg_dst, "rcx") == 0) {
    exchange_regs((char * []) {
      "rax",
      "rcx",
      "rcx",
      "rax"
    });
    exchange_regs((char * []) {
      "rcx",
      (char * ) reg_src,
      (char * ) reg_src,
      "rcx"
    });
    exchange_regs(m1);
    exchange_regs(NULL);
    emit_instr(instr);
    exchange_regs(m2);
    exchange_regs((char * []) {
      (char * ) reg_src, "rcx", "rcx", (char * ) reg_src
    });
    exchange_regs((char * []) {
      "rax",
      "rcx",
      "rcx",
      "rax"
    });
  } else {
    exchange_regs((char * []) {
      "rax",
      (char * ) reg_dst,
      (char * ) reg_dst,
      "rax"
    });
    exchange_regs((char * []) {
      "rcx",
      (char * ) reg_src,
      (char * ) reg_src,
      "rcx"
    });
    exchange_regs(m1);
    exchange_regs(NULL);
    emit_instr(instr);
    exchange_regs(m2);
    exchange_regs((char * []) {
      (char * ) reg_src, "rcx", "rcx", (char * ) reg_src
    });
    exchange_regs((char * []) {
      (char * ) reg_dst, "rax", "rax", (char * ) reg_dst
    });
  }
}

static void emit_unary_op(const char * instr,
  const char * reg) {
  exchange_regs((char * []) {
    "rax",
    (char * ) reg,
    (char * ) reg,
    "rax"
  });
  exchange_regs(NULL);
  emit_instr(instr);
  exchange_regs((char * []) {
    (char * ) reg, "rax", "rax", (char * ) reg
  });
}

static void emit_load_imm(const char * reg,
  const char * imm) {
  exchange_regs((char * []) {
    (char * ) reg, (char * ) imm
  });
}