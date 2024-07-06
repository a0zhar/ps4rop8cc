#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES       1000
#define MAX_LINE_LENGTH 256

char lines[MAX_LINES][MAX_LINE_LENGTH];
char lines2[MAX_LINES][MAX_LINE_LENGTH];

int lines_count = 0;
int lines2_count = 0;

int is_label(const char *line) {
    size_t len = strlen(line);
    return len > 0 && line[len - 1] == ':' && line[0] != '.';
}

void add_line(char lines[][MAX_LINE_LENGTH], int *count, const char *line) {
    strncpy(lines[*count], line, MAX_LINE_LENGTH);
    lines[*count][MAX_LINE_LENGTH - 1] = '\0';
    (*count)++;
}

void process_lines() {
    int i, j, ii;
    char ncalls[MAX_LINES][MAX_LINE_LENGTH];
    int ncalls_count = 0;

    for (i = 0; i < lines_count; i++) {
        const char *l = lines[i];
        if (is_label(l)) {
            ncalls_count = 0;
            for (ii = 0; ii < lines2_count; ii++) {
                char *ll = lines2[ii];
                if (strncmp(ll, "jmp ", 4) == 0 && ll[4] != '.' && ll[4] != '\0') {
                    if (ll[4] != '_') {
                        fprintf(stderr, "Assertion failed: %s\n", ll);
                        exit(EXIT_FAILURE);
                    }
                    snprintf(ncalls[ncalls_count], MAX_LINE_LENGTH, "%s", ll + 5);
                    ncalls[ncalls_count][MAX_LINE_LENGTH - 1] = '\0';
                    ncalls_count++;
                    snprintf(ll, MAX_LINE_LENGTH, "jmp ._native_%s", ll + 5);
                }
            }
            for (ii = 0; ii < ncalls_count; ii++) {
                char nativecall[MAX_LINE_LENGTH+1]={0};
                snprintf(nativecall, sizeof(nativecall)-1, "nativecall ._native_%s", ncalls[ii]);
                add_line(lines2, &lines2_count, nativecall);
            }
        }
        add_line(lines2, &lines2_count, l);
    }
}

int main() {
    char buffer[MAX_LINE_LENGTH];

    // Read input lines
    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
        add_line(lines, &lines_count, buffer);
    }
    add_line(lines, &lines_count, ":");

    // Process lines
    process_lines();

    // Print the processed lines
    for (int i = 0; i < lines2_count - 1; i++) {
        printf("%s\n", lines2[i]);
    }

    return 0;
}
