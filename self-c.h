#ifndef SELF_C
#define SELF_C
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDENT_BUF_LEN 512

#define TO_ENUM(ENUM) ENUM,
#define TO_STRING(ENUM) #ENUM,

#define ALL_TOKENS(FUNC)     \
    /* punctuations */       \
    FUNC(TOKEN_LPARAN)       \
    FUNC(TOKEN_RPARAN)       \
    FUNC(TOKEN_LPARAN_CURLY) \
    FUNC(TOKEN_RPARAN_CURLY) \
    FUNC(TOKEN_COMMA)        \
    FUNC(TOKEN_SEMICOLON)    \
    /* reserved keywords */  \
    FUNC(TOKEN_RETURN)       \
    /* operators */          \
    FUNC(TOKEN_ADD)          \
    FUNC(TOKEN_SUB)          \
    FUNC(TOKEN_MUL)          \
    FUNC(TOKEN_DIV)          \
    FUNC(TOKEN_EQU)          \
    /* identifier */         \
    FUNC(TOKEN_IDENT)        \
    /* types */              \
    FUNC(TOKEN_TYPE)         \
    /* literals */           \
    FUNC(TOKEN_INT_LIT)

#define ALL_AST(FUNC) \
    FUNC(AST_IDENT)   \
    FUNC(AST_TYPE)    \
    FUNC(AST_INT)     \
    FUNC(AST_RETURN)  \
    FUNC(AST_FUNC)

/* Tokens */
enum { ALL_TOKENS(TO_ENUM) } typedef TokenKind;
extern const char *token_kind_string[];
struct Token {
    TokenKind token_kind;
    char *token_val;
} typedef Token;

/* AST */
enum { ALL_AST(TO_ENUM) } typedef AstType;
extern const char *ast_type_string[];
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
        /* stat */
        struct {
            struct Ast *stat_expr;
        };
        /* stat list */
        struct {
            struct Vector *stat_list;
        };
        /* param */
        struct {
            struct Ast *param_type;
            char *param_name;
        };
        /* param list */
        struct {
            struct Vector *param_list;
        };
        /* function call */
        struct {
            char *func_call_name;
            int num_args;
            struct Ast *arg_list;
        };
        /* function */
        struct {
            struct Ast *func_name;
            struct Ast *func_return_type;
            int num_params;
            struct Ast *func_param_list;
            struct Ast *func_stat_list;
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
void error_unexpected(char *func_name, char *unexpected);
void error_unexpected_token(const char *func_name, char unexpected);
void error_buffer_overflow(const char *func_name, int max_size);
void error_token_mismatch(const char *func_name, TokenKind token_actual, TokenKind token_excepted);
void error_token_mismatch_group(const char *func_name, TokenKind token_actual, char *group);

/* lexer */
Vector *lex_init();
void lex_scan(Vector *vec);
void lex_print_tokens(Vector *token_vec);

/* parser */
Ast *parse(Vector *token_vec);
void print_ast(Ast *ast);

/* codegen */
void compile(Ast *ast);

#endif
