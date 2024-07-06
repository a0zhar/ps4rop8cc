#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <time.h>

typedef struct {
    FILE *file;  // stream backed by FILE *
    char *p;     // stream backed by string
    char *name;
    int line;
    int column;
    int ntok;     // token counter
    int last;     // the last character read from file
    int buf[3];   // push-back buffer for unread operations
    int buflen;   // push-back buffer size
    time_t mtime; // last modified time. 0 if string-backed file
} File;

File *make_file(FILE *file, char *name);
File *make_file_string(char *s);
int readc(void);
void unreadc(int c);
File *current_file(void);
void stream_push(File *file);
int stream_depth(void);
char *input_position(void);
void stream_stash(File *f);
void stream_unstash(void);

#endif
