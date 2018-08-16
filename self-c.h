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

#define ALL_AST(FUNC)    \
    FUNC(AST_IDENT)      \
    FUNC(AST_TYPE)       \
    FUNC(AST_INT)        \
    FUNC(AST_BIN_OP)     \
    FUNC(AST_RETURN)     \
    FUNC(AST_ASSIGN)     \
    FUNC(AST_FUNC_CALL)  \
    FUNC(AST_STAT_LIST)  \
    FUNC(AST_ARG)        \
    FUNC(AST_ARG_LIST)   \
    FUNC(AST_PARAM)      \
    FUNC(AST_PARAM_LIST) \
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
    AstType type;
    union {
        /* int literal */
        int int_val;
        /* string literal */
        char *str_val;
        /* binary operator */
        struct {
            char op;
            struct Ast *left;
            struct Ast *right;
        };
        /* stat */
        struct {
            struct Ast *stat_type;
            struct Ast *stat_lhs;
            struct Ast *stat_rhs;
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
        /* arg */
        struct {
            struct Ast *arg_type;
            char *arg_name;
        };
        /* arg list */
        struct {
            struct Vector *arg_list;
        };
        /* function call */
        struct {
            struct Ast *func_call_name;
            struct Ast *func_call_arg_list;
        };
        /* function */
        struct {
            struct Ast *func_name;
            struct Ast *func_return_type;
            struct Ast *func_param_list;
            struct Ast *func_stat_list;
        };
    };
} Ast;

/* vector */
struct Vector {
    int size;
    int count;
    void **data;
} typedef Vector;

Vector *vector_init(void);
void *vector_get(Vector *vec, int index);
void vector_add(Vector *vec, void *data);
void vector_delete(Vector *vec, int index);
void vector_resize(Vector *vec, int size);

/* map */
struct KeyValue {
    char *key;
    void *value;
} typedef KeyValue;

struct Map {
    Vector *data;
} typedef Map;

Map *map_init(void);
Map *map_add(Map *map, char *key, void *value);
Map *map_delete(Map *map, char *key);
void *map_get(Map *map, char *key);

/* error-util */
void error(char *fmt, ...) __attribute__((noreturn));
void error_unexpected(char *func_name, char *unexpected) __attribute__((noreturn));
void error_unexpected_token(const char *func_name, char unexpected) __attribute__((noreturn));
void error_buffer_overflow(const char *func_name, int max_size) __attribute__((noreturn));
void error_token_mismatch(const char *func_name, TokenKind token_actual, TokenKind token_excepted)
    __attribute__((noreturn));
void error_token_mismatch_group(const char *func_name, TokenKind token_actual, char *group) __attribute__((noreturn));
void error_identifier_not_found(const char *func_name, char *ident) __attribute__((noreturn));

/* symbol table */
struct SymbolTable {
    Map *offset_map;
    int offset;
} typedef SymbolTable;

void init_symbol_tables(void);
bool symbol_table_check_symbol(char *key);
int symbol_table_get_offset(char *key);
void symbol_table_add_offset(char *key);

/* lexer */
Vector *lex_init(void);
void lex_scan(Vector *vec);
void lex_print_tokens(Vector *token_vec);

/* parser */
Ast *parse(Vector *token_vec);
void print_ast(Ast *ast);

/* codegen */
void compile(Ast *ast);

#endif
