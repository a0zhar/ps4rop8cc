#ifndef DICT_H
#define DICT_H

#include "vector.h"

typedef struct {
    struct Map *map;
    Vector *key;
} Dict;

Dict *make_dict(void);
void *dict_get(Dict *dict, char *key);
void dict_put(Dict *dict, char *key, void *val);
Vector *dict_keys(Dict *dict);

#endif
