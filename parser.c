#include "self-c.h"

const char *ast_type_string[] = {ALL_AST(TO_STRING)};

static Vector *token_vec;
static int token_index;

static int char2int(char c) { return c - '0'; }

static bool is_token(Token *token, TokenKind kind) { return token->token_kind == kind; }

struct Parser {
    Vector *tokens;
    int token_index;
} typedef Parser;

static void match(Token *token, TokenKind kind) {
    if (!is_token(token, kind)) {
        printf("%d\n", token_index);
        error_token_mismatch(__func__, token->token_kind, kind);
    }
    token_index++;
}

static Ast *make_ast(void) {
    Ast *ast = malloc(sizeof(Ast));
    ast->symbol_table = symbol_table_current();
    return ast;
}

static Ast *make_ast_int(int n) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_INT;
    ast->int_val = n;
    return ast;
}

static Ast *make_ast_ident(char *ident) {
    Ast *ast = make_ast();
    ast->type = AST_IDENT;
    ast->str_val = ident;
    return ast;
}

static Ast *make_ast_type(char *type) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_TYPE;
    ast->str_val = type;
    return ast;
}

static Ast *make_ast_op(TokenKind op, Ast *left, Ast *right) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_BIN_OP;
    switch (op) {
        case TOKEN_ADD:
            ast->op = '+';
            break;
        case TOKEN_SUB:
            ast->op = '-';
            break;
        case TOKEN_MUL:
            ast->op = '*';
            break;
        case TOKEN_DIV:
            ast->op = '/';
            break;
        default:
            error_token_mismatch_group(__func__, op, "binary operators");
    }
    ast->left = left;
    ast->right = right;
    return ast;
}

static Ast *read_int_lit(char *val) {
    int n = 0;
    while (*val != '\0') {
        n = n * 10 + char2int(*val);
        val++;
    }
    return make_ast_int(n);
}

static Ast *read_factor() {
    Token *token = vector_get(token_vec, token_index);
    if (is_token(token, TOKEN_IDENT)) {
        if (!symbol_table_check_symbol(token->token_val)) {
            error_identifier_not_found(__func__, token->token_val);
        }
        token_index++;
        return make_ast_ident(token->token_val);
    }
    match(token, TOKEN_INT_LIT);
    return read_int_lit(token->token_val);
}

static Ast *read_term_tail(Ast *left) {
    if (token_vec->count == token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, token_index);
    if (!(token->token_kind == TOKEN_MUL || token->token_kind == TOKEN_DIV)) {
        return left;
    }
    token_index++;
    Ast *right = read_factor();
    return read_term_tail(make_ast_op(token->token_kind, left, right));
}

static Ast *read_term() { return read_term_tail(read_factor()); }

static Ast *read_expr_tail(Ast *left) {
    if (token_vec->count == token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, token_index);
    if (!(token->token_kind == TOKEN_ADD || token->token_kind == TOKEN_SUB)) {
        return left;
    }
    token_index++;
    Ast *right = read_term();
    return read_expr_tail(make_ast_op(token->token_kind, left, right));
}

static Ast *read_expr() {
    /*if (is_token(vector_get(token_vec, token_index), TOKEN_IDENT) &&*/
    /*is_token(vector_get(token_vec, token_index + 1), TOKEN_LPARAN)) {*/
    /*return read_func_call();*/
    /*}*/
    Ast *left = read_term();
    return read_expr_tail(left);
}

static Ast *read_arg_list(TokenKind end_kind) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_ARG_LIST;
    ast->arg_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->arg_list, read_expr());
        token = vector_get(token_vec, token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        match(token, TOKEN_COMMA);
    }
}

static Ast *read_func_call() {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_FUNC_CALL;
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_IDENT);
    ast->func_call_name = make_ast_ident(token->token_val);
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_LPARAN);
    ast->func_call_arg_list = read_arg_list(TOKEN_RPARAN);
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_RPARAN);
    return ast;
}

static Ast *read_rhs() {
    if (is_token(vector_get(token_vec, token_index), TOKEN_IDENT) &&
        is_token(vector_get(token_vec, token_index + 1), TOKEN_LPARAN)) {
        return read_func_call();
    }
    return read_expr();
}

static Ast *read_stat_return() {
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_RETURN);
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_RETURN;
    ast->stat_rhs = read_rhs();
    return ast;
}

static Ast *read_stat_assignment() {
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_TYPE);
    Ast *ast = malloc(sizeof(Ast));

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_IDENT);
    ast->stat_lhs = make_ast_ident(token->token_val);
    symbol_table_add_variable(token->token_val);
    ast->symbol_table = symbol_table_current();

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_EQU);

    ast->type = AST_ASSIGN;
    ast->stat_rhs = read_rhs();

    return ast;
}

static Ast *read_stat() {
    Token *token = vector_get(token_vec, token_index);
    if (is_token(token, TOKEN_RETURN)) {
        return read_stat_return();
    }
    if (is_token(token, TOKEN_TYPE)) {
        return read_stat_assignment();
    }
    if (is_token(token, TOKEN_IDENT)) {
        return read_expr();
    }
    error_token_mismatch_group(__func__, token->token_kind, "statement");
}

static Ast *read_stat_list(TokenKind end_kind) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_STAT_LIST;
    ast->stat_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->stat_list, read_stat());
        token = vector_get(token_vec, token_index);
        match(token, TOKEN_SEMICOLON);
    }
}

static Ast *read_param() {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_PARAM;
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_TYPE);
    ast->param_type = make_ast_type(token->token_val);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_IDENT);
    ast->param_name = token->token_val;

    symbol_table_add_param(token->token_val);
    ast->symbol_table = symbol_table_current();

    return ast;
}

static Ast *read_param_list(TokenKind end_kind) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_PARAM_LIST;
    ast->param_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->param_list, read_param());
        token = vector_get(token_vec, token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        match(token, TOKEN_COMMA);
    }
}

static Ast *read_function() {
    symbol_table_open_scope();
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_FUNC;
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_TYPE);
    ast->func_return_type = make_ast_type(token->token_val);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_IDENT);
    ast->func_name = make_ast_ident(token->token_val);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_LPARAN);

    ast->func_param_list = read_param_list(TOKEN_RPARAN);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_RPARAN);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_LPARAN_CURLY);

    ast->func_stat_list = read_stat_list(TOKEN_RPARAN_CURLY);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_RPARAN_CURLY);
    symbol_table_close_scope();
    return ast;
}

static Ast *read_program() {
    Ast *ast = malloc(sizeof(Ast));
    ast->program = vector_init();
    while (token_index != token_vec->count) {
        Ast *function = read_function();
        vector_add(ast->program, function);
    }
    return ast;
}

Ast *parse(Vector *vec) {
    token_index = 0;
    token_vec = vec;
    return read_program();
}

/* Debug utils */
void print_ast(Ast *ast) {
    /*switch (ast->type) {*/
    /*case AST_INT:*/
    /*printf("%d", ast->int_val);*/
    /*break;*/
    /*default:*/
    /*printf("(%c", ast->type);*/
    /*print_ast(ast->left);*/
    /*printf(" ");*/
    /*print_ast(ast->right);*/
    /*printf(")");*/
    /*break;*/
    /*}*/
}
