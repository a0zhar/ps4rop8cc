 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_LINE_LENGTH 1024

const char *reg_map[][2] = {
    {"A", "rax"},
    {"B", "rcx"},
    {"C", "rdx"},
    {"SP", "rsp"},
    {"BP", "rbp"}
};

const char *conds[][2] = {
    {"eq", "e"},
    {"ne", "ne"},
    {"lt", "l"},
    {"gt", "g"},
    {"le", "le"},
    {"ge", "ge"}
};

const char* get_reg(const char *reg) {
    for (int i = 0; i < sizeof(reg_map) / sizeof(reg_map[0]); i++) {
        if (strcmp(reg_map[i][0], reg) == 0) {
            return reg_map[i][1];
        }
    }
    return reg;
}

const char* get_cond(const char *cond) {
    for (int i = 0; i < sizeof(conds) / sizeof(conds[0]); i++) {
        if (strcmp(conds[i][0], cond) == 0) {
            return conds[i][1];
        }
    }
    return NULL;
}

void print_instruction(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void handle_command(char *cmd, char *args[]) {
    if (strcmp(cmd, "mov") == 0 || strcmp(cmd, "add") == 0 || strcmp(cmd, "sub") == 0 ||
        strcmp(cmd, "mul") == 0 || strcmp(cmd, "and") == 0 || strcmp(cmd, "or") == 0 || strcmp(cmd, "xor") == 0) {
        if (strcmp(cmd, "mul") == 0) cmd = "imul";
        print_instruction("%s %s, %s", cmd, get_reg(args[0]), get_reg(args[1]));
    } else if (strcmp(cmd, "not") == 0) {
        print_instruction("not %s", get_reg(args[0]));
    } else if (strcmp(cmd, "jmp") == 0) {
        print_instruction("jmp %s", get_reg(args[0]));
    } else if (strcmp(cmd, "shl") == 0 || strcmp(cmd, "shr") == 0 || strcmp(cmd, "sar") == 0) {
        assert(strcmp(args[1], "B") == 0);
        print_instruction("%s %s, cl", cmd, get_reg(args[0]));
    } else if (strncmp(cmd, "crop", 4) == 0 || strncmp(cmd, "icrop", 5) == 0) {
        if (strcmp(cmd, "crop64") == 0 || strcmp(cmd, "icrop64") == 0) return;
        assert(strcmp(args[0], "SP") != 0 && strcmp(args[0], "BP") != 0);
        const char *reg0 = get_reg(args[0]);
        const char *reg = reg0;
        int sz = atoi(cmd + (cmd[0] == 'i' ? 5 : 4));
        const char *asmcmd = cmd[0] == 'i' ? "movsx" : "movzx";
        if (sz == 8) reg = reg[1] == 'a' ? "al" : (reg[1] == 'b' ? "bl" : (reg[1] == 'c' ? "cl" : "dl"));
        else if (sz == 16) reg = reg[1] == 'a' ? "ax" : (reg[1] == 'b' ? "bx" : (reg[1] == 'c' ? "cx" : "dx"));
        else if (sz == 32) {
            reg = reg[1] == 'a' ? "eax" : (reg[1] == 'b' ? "ebx" : (reg[1] == 'c' ? "ecx" : "edx"));
            if (cmd[0] == 'i') asmcmd = "movsxd";
        }
        print_instruction("%s %s, %s", asmcmd, reg0, reg);
    } else if (strcmp(cmd, "div") == 0 || strcmp(cmd, "mod") == 0 || strcmp(cmd, "idiv") == 0 || strcmp(cmd, "imod") == 0) {
        print_instruction("push rax");
        print_instruction("push rax");
        print_instruction("push rdx");
        print_instruction("push rcx");
        if (strcmp(args[0], "A") != 0 || strcmp(args[1], "B") != 0) {
            print_instruction("push %s", get_reg(args[0]));
            print_instruction("push qword %s", get_reg(args[1]));
            print_instruction("pop rcx");
            print_instruction("pop rax");
        }
        if (cmd[0] == 'i') {
            print_instruction("cqo");
            print_instruction("idiv rcx");
        } else {
            print_instruction("xor rdx, rdx");
            print_instruction("div rcx");
        }
        print_instruction("mov [rsp+24], %s", strcmp(cmd, "mod") == 0 ? "rdx" : "rax");
        print_instruction("pop rcx");
        print_instruction("pop rdx");
        print_instruction("pop rax");
        print_instruction("pop %s", get_reg(args[0]));
    } else if (strncmp(cmd, "store", 5) == 0) {
        assert((strcmp(args[0], "SP") != 0 && strcmp(args[0], "BP") != 0) || strcmp(cmd, "store64") == 0);
        const char *reg_src = get_reg(args[0]);
        if (strcmp(cmd, "store32") == 0) reg_src = "e" + reg_src[1];
        else if (strcmp(cmd, "store16") == 0) reg_src = reg_src + 1;
        else if (strcmp(cmd, "store8") == 0) reg_src = reg_src[1] == 'a' ? "al" : (reg_src[1] == 'b' ? "bl" : (reg_src[1] == 'c' ? "cl" : "dl"));
        print_instruction("mov [%s], %s", get_reg(args[1]), reg_src);
    } else if (strncmp(cmd, "load", 4) == 0) {
        const char *reg_dst = get_reg(args[0]);
        const char *xcmd = "movsx";
        const char *xsz = "qword ";
        if (strcmp(cmd, "load32") == 0) {
            xcmd = "movsxd";
            xsz = "dword ";
        } else if (strcmp(cmd, "load16") == 0) {
            xsz = "word ";
        } else if (strcmp(cmd, "load8") == 0) {
            xsz = "byte ";
        } else {
            xsz = "";
            xcmd = "mov";
        }
        print_instruction("%s %s, %s[%s]", xcmd, reg_dst, xsz, get_reg(args[1]));
    } else if (get_cond(cmd) != NULL) {
        assert(strcmp(args[0], "SP") != 0 && strcmp(args[0], "BP") != 0);
        const char *reg_dst = get_reg(args[0]);
        print_instruction("cmp %s, %s", reg_dst, get_reg(args[1]));
        print_instruction("set%s %sl", get_cond(cmd), reg_dst[1] == 'a' ? "al" : (reg_dst[1] == 'b' ? "bl" : (reg_dst[1] == 'c' ? "cl" : "dl")));
        print_instruction("movzx %s, %sl", reg_dst, reg_dst[1] == 'a' ? "al" : (reg_dst[1] == 'b' ? "bl" : (reg_dst[1] == 'c' ? "cl" : "dl")));
    } else if (cmd[0] == 'j' && get_cond(cmd + 1) != NULL) {
        print_instruction("cmp %s, %s", get_reg(args[1]), get_reg(args[2]));
        print_instruction("j%s %s", get_cond(cmd + 1), args[0]);
    } else if (strcmp(cmd, ".byte") == 0) {
        print_instruction("db %s", args[0]);
    } else if (strcmp(cmd, ".short") == 0) {
        print_instruction("dw %s", args[0]);
    } else if (strcmp(cmd, ".int") == 0) {
        print_instruction("dd %s", args[0]);
    } else if (strcmp(cmd, ".long") == 0) {
        print_instruction("dq %s", args[0]);
    } else if (strcmp(cmd, ".ptr") == 0) {
        print_instruction("dq %s", args[0]);
    } else if (strcmp(cmd, ".gadget_addr") == 0) {
     
        assert(strncmp(args[1], "dq ", 3) == 0);
        print_instruction("mov %s, %s", get_reg(args[0]), args[1] + 3);
    } else {
        assert(0 && "Unknown command");
    }
}
