#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "file.h"
#include "set.h"
#include "dict.h"
#include "vector.h"

typedef struct {
    int kind;
    File *file;
    int line;
    int column;
    bool space;   // true if the token has a leading space
    bool bol;     // true if the token is at the beginning of a line
    int count;    // token number in a file, counting from 0.
    Set *hideset; // used by the preprocessor for macro expansion
    union {
        // TKEYWORD
        int id;
        // TSTRING or TCHAR
        struct {
            char *sval;
            int slen;
            int c;
            int enc;
        };
        // TMACRO_PARAM
        struct {
            bool is_vararg;
            int position;
        };
    };
} Token;

typedef struct {
    int kind;
    int size;
    int align;
    bool usig; // true if unsigned
    bool isstatic;
    // pointer or array
    struct Type *ptr;
    // array length
    int len;
    // struct
    Dict *fields;
    int offset;
    bool is_struct; // true if struct, false if union
    // bitfield
    int bitoff;
    int bitsize;
    // function
    struct Type *rettype;
    Vector *params;
    bool hasva;
    bool oldstyle;
} Type;

typedef struct {
    char *file;
    int line;
} SourceLoc;

typedef struct Node {
    int kind;
    Type *ty;
    SourceLoc *sourceLoc;
    union {
        // Char, int, or long
        long ival;
        // Float or double
        struct {
            double fval;
            char *flabel;
        };
        // String
        struct {
            char *sval;
            char *slabel;
        };
        // Local/global variable
        struct {
            char *varname;
            // local
            int loff;
            Vector *lvarinit;
            // global
            char *glabel;
        };
        // Binary operator
        struct {
            struct Node *left;
            struct Node *right;
        };
        // Unary operator
        struct {
            struct Node *operand;
        };
        // Function call or function declaration
        struct {
            char *fname;
            // Function call
            Vector *args;
            struct Type *ftype;
            // Function pointer or function designator
            struct Node *fptr;
            // Function declaration
            Vector *params;
            Vector *localvars;
            struct Node *body;
        };
        // Declaration
        struct {
            struct Node *declvar;
            Vector *declinit;
        };
        // Initializer
        struct {
            struct Node *initval;
            int initoff;
            Type *totype;
        };
        // If statement or ternary operator
        struct {
            struct Node *cond;
            struct Node *then;
            struct Node *els;
        };
        // Goto and label
        struct {
            char *label;
            char *newlabel;
        };
        // Return statement
        struct Node *retval;
        // Compound statement
        Vector *stmts;
        // Struct reference
        struct {
            struct Node *struc;
            char *field;
            Type *fieldtype;
        };
    };
} Node;

extern Type *type_void;
extern Type *type_bool;
extern Type *type_char;
extern Type *type_short;
extern Type *type_int;
extern Type *type_long;
extern Type *type_llong;
extern Type *type_uchar;
extern Type *type_ushort;
extern Type *type_uint;
extern Type *type_ulong;
extern Type *type_ullong;
extern Type *type_float;
extern Type *type_double;
extern Type *type_ldouble;

#define EMPTY_MAP ((Map){})
#define EMPTY_VECTOR ((Vector){})

#endif
