#ifndef SET_H
#define SET_H

typedef struct Set {
    char *v;
    struct Set *next;
} Set;

Set *set_add(Set *s, char *v);
bool set_has(Set *s, char *v);
Set *set_union(Set *a, Set *b);
Set *set_intersection(Set *a, Set *b);

#endif
