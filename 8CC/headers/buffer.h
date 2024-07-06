#ifndef BUFFER_H
#define BUFFER_H

#include <stdarg.h>
#include <stdint.h>

typedef struct {
    char *body;
    int nalloc;
    int len;
} Buffer;

Buffer *to_utf16(char *p, int len);
Buffer *to_utf32(char *p, int len);
void write_utf8(Buffer *b, uint32_t rune);

Buffer *make_buffer(void);
char *buf_body(Buffer *b);
int buf_len(Buffer *b);
void buf_write(Buffer *b, char c);
void buf_append(Buffer *b, char *s, int len);
void buf_printf(Buffer *b, char *fmt, ...);
char *vformat(char *fmt, va_list ap);
char *format(char *fmt, ...);
char *quote_cstring(char *p);
char *quote_cstring_len(char *p, int len);
char *quote_char(char c);

#endif
