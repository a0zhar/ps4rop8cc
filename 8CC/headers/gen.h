#ifndef GEN_H
#define GEN_H

#include "../structs.h"


void set_output_file(FILE *fp);
void close_output_file(void);
void emit_toplevel(Node *v);

#endif
