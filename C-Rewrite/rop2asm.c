#include <stdio.h>
#include <string.h>

int main() {
    printf("section .text\n");
    printf("use64\n");
    printf("global main\n");
    printf("main:\n");
    printf("mov rsp, rop_start\n");
    printf("ret\n");
    printf("extern exit\n");
    printf("section .data\n");
    printf("align 8\n");
    printf("rop_start:\n");

    char line[256];
    int gid = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *comment = strchr(line, '#');
        if (comment) *comment = '\0';
        char *l = strtok(line, "\n");

        if (l == NULL || strlen(l) == 0) continue;
        
        if (strcmp(l, "$pivot_addr") == 0) {
            l = "mov rsp, [rdi+0x38] ; pop rdi";
        } else if (strcmp(l, "$jop_frame_addr") == 0) {
            l = "push rbp ; mov rbp, rsp ; mov rax, [rdi] ; call [rax]";
        }

        if (l[0] == '$') {
            if (strstr(l, "_addr") == l + strlen(l) - 5) {
                printf("extern %.*s\n", (int)(strlen(l) - 6), l + 1);
                printf("dq %.*s\n", (int)(strlen(l) - 6), l + 1);
            } else {
                printf("%s\n", l + 1);
            }
        } else if (l[strlen(l) - 1] == ':') {
            printf("global %.*s\n", (int)(strlen(l) - 1), l);
            printf("%s\n", l);
            printf("section .text\n");
            printf("global text_%.*s\n", (int)(strlen(l) - 1), l);
            printf("text_%s\n", l);
            printf("section .data\n");
        } else if (strncmp(l, "db ", 3) == 0 || strncmp(l, "dq ", 3) == 0) {
            printf("%s\n", l);
        } else if (strncmp(l, "dp ", 3) == 0) {
            printf("dq %s\n", l + 3);
        } else {
            printf("dq gadget_%d\n", gid);
            printf("section .text\n");
            printf("gadget_%d:\n", gid);
            char *inst = strtok(l, " ; ");
            while (inst != NULL) {
                printf("%s\n", inst);
                inst = strtok(NULL, " ; ");
            }
            printf("ret\n");
            printf("section .data\n");
            gid++;
        }
    }

    return 0;
}
