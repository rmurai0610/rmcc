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
    FUNC(TOKEN_IF)           \
    FUNC(TOKEN_FOR)          \
    /* operators */          \
    FUNC(TOKEN_ADD)          \
    FUNC(TOKEN_SUB)          \
    FUNC(TOKEN_MUL)          \
    FUNC(TOKEN_DIV)          \
    FUNC(TOKEN_EQU)          \
    FUNC(TOKEN_DOUBLE_EQU)   \
    FUNC(TOKEN_NOT)          \
    FUNC(TOKEN_NOT_EQU)      \
    FUNC(TOKEN_LT)           \
    FUNC(TOKEN_GT)           \
    FUNC(TOKEN_LTE)          \
    FUNC(TOKEN_GTE)          \
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
    FUNC(AST_IF)         \
    FUNC(AST_FOR)        \
    FUNC(AST_LHS)        \
    FUNC(AST_ASSIGN)     \
    FUNC(AST_FUNC_CALL)  \
    FUNC(AST_STAT_LIST)  \
    FUNC(AST_ARG)        \
    FUNC(AST_ARG_LIST)   \
    FUNC(AST_PARAM)      \
    FUNC(AST_PARAM_LIST) \
    FUNC(AST_FUNC)       \
    FUNC(AST_PROGRAM)

#define ALL_BIN_OP(FUNC) \
    FUNC(BIN_ADD)        \
    FUNC(BIN_SUB)        \
    FUNC(BIN_MUL)        \
    FUNC(BIN_DIV)        \
    FUNC(BIN_DOUBLE_EQU) \
    FUNC(BIN_GT)         \
    FUNC(BIN_GTE)        \
    FUNC(BIN_LT)         \
    FUNC(BIN_LTE)

#define ALL_SYMBOL_TYPE(FUNC) \
    FUNC(SYMBOL_VARIABLE)     \
    FUNC(SYMBOL_PARAM)

/* Tokens */
enum { ALL_TOKENS(TO_ENUM) } typedef TokenKind;
extern const char *token_kind_string[];
struct Token {
    TokenKind token_kind;
    char *token_val;
    int line_no;
    int col_no;
} typedef Token;

/* AST */
enum { ALL_AST(TO_ENUM) } typedef AstType;
enum { ALL_BIN_OP(TO_ENUM) } typedef BinOp;
extern const char *ast_type_string[];
typedef struct Ast {
    struct SymbolTable *symbol_table;
    AstType type;
    union {
        /* int literal */
        int int_val;
        /* string literal */
        char *str_val;
        /* binary operator */
        struct {
            BinOp bin_op;
            struct Ast *bin_left;
            struct Ast *bin_right;
        };
        /* assignment */
        struct {
            struct Ast *assign_lhs;
            struct Ast *assign_rhs;
        };
        /* lhs */
        struct {
            struct Ast *lhs_type;
            struct Ast *lhs_ident;
        };
        /*if */
        struct {
            struct Ast *if_cond;
            struct Ast *if_branch;
        };
        /* for */
        struct {
            struct Ast *for_init;
            struct Ast *for_cond;
            struct Ast *for_update;
            struct Ast *for_body;
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
        /* arg */
        struct {
            struct Ast *arg_type;
            struct Ast *arg_name;
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
        /* param */
        struct {
            struct Ast *param_type;
            struct Ast *param_name;
        };
        /* param list */
        struct {
            struct Vector *param_list;
        };
        /* function */
        struct {
            struct Ast *func_name;
            struct Ast *func_return_type;
            struct Ast *func_param_list;
            struct Ast *func_stat_list;
        };
        /* program */
        struct {
            struct Vector *program;
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

/* debug */
void lex_print_tokens(Vector *token_vec);
void print_ast(Ast *ast);

/* symbol table */
enum { ALL_SYMBOL_TYPE(TO_ENUM) } typedef SymbolType;
struct Symbol {
    SymbolType type;
    int offset;
} typedef Symbol;

struct SymbolTable {
    struct Map *symbols;
    struct SymbolTable *parent;
    struct Vector *childrens;
    int variable_offset;
    int param_offset;
} typedef SymbolTable;

void symbol_table_init(void);
void symbol_table_open_scope(void);
void symbol_table_close_scope(void);
bool symbol_table_check_symbol(char *key);
Symbol *symbol_table_get_symbol(char *key);
Symbol *symbol_table_get_symbol_from_table(SymbolTable *table, char *key);
void symbol_table_add_variable(char *key);
void symbol_table_add_param(char *key);
SymbolTable *symbol_table_current(void);
void symbol_table_dump(SymbolTable *table);

/* lexer */
Vector *lex_init(void);
void lex_scan(Vector *vec);

/* parser */
Ast *parse(Vector *token_vec);

/* codegen */
void compile(Ast *ast);

#endif
