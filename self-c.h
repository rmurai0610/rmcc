#ifndef SELF_C
#define SELF_C
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IDENT_BUF_LEN 512

/* Tokens */
enum {
    /* punctuations */
    TOKEN_LPARAN,
    TOKEN_RPARAN,
    TOKEN_LPARAN_CURLY,
    TOKEN_RPARAN_CURLY,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    /* reserved keywords */
    TOKEN_RETURN,
    /* operators */
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_EQU,
    /* identifier */
    TOKEN_IDENT,
    /* literals */
    TOKEN_INT_LIT,
};
struct Token {
    int token_kind;
    char *token_val;
} typedef Token;

/* AST */
enum { AST_IDENT, AST_INT, AST_FUNC };
typedef struct Ast {
    char type;
    union {
        /* int literal */
        int int_val;
        /* string literal */
        char *str_val;
        /* binary operator */
        struct {
            struct Ast *left;
            struct Ast *right;
        };
        /* function call */
        struct {
            char *func_call_name;
            int num_args;
            struct Ast *arg_list;
        };
        /* function */
        struct {
            char *func_name;
            int num_params;
            struct Ast *param_list;
        };
    };
} Ast;

/* Vector */
struct Vector {
    int size;
    int count;
    void **data;
} typedef Vector;

Vector *vector_init();
void *vector_get(Vector *vec, int index);
void vector_add(Vector *vec, void *data);
void vector_delete(Vector *vec, int index);
void vector_resize(Vector *vec, int size);

/* error-util */
void error(char *fmt, ...) __attribute__((noreturn));

/* lexer */
Vector *lex_init();
void lex_scan(Vector *vec);
void lex_print_tokens(Vector *token_vec);

/* self-c */
#endif