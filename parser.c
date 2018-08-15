#include "self-c.h"

const char *ast_type_string[] = {ALL_AST(TO_STRING)};

static int char2int(char c) { return c - '0'; }

static void match(Token *token, TokenKind kind, int *token_index) {
    if (token->token_kind != kind) {
        error_token_mismatch(__func__, token->token_kind, kind);
    }
    *token_index = *token_index + 1;
}

static Ast *make_ast_int(int n) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_INT;
    ast->int_val = n;
    return ast;
}

static Ast *make_ast_ident(char *ident) {
    Ast *ast = malloc(sizeof(Ast));
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
    switch (op) {
        case TOKEN_ADD:
            ast->type = '+';
            break;
        case TOKEN_SUB:
            ast->type = '-';
            break;
        case TOKEN_MUL:
            ast->type = '*';
            break;
        case TOKEN_DIV:
            ast->type = '/';
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

static Ast *read_factor(Vector *token_vec, int *token_index) {
    Token *token = vector_get(token_vec, *token_index);
    if (token->token_kind != TOKEN_INT_LIT) {
        error_token_mismatch_group(__func__, token->token_kind, "digits");
    }
    *token_index = *token_index + 1;
    return read_int_lit(token->token_val);
}

static Ast *read_term_tail(Vector *token_vec, int *token_index, Ast *left) {
    if (token_vec->count == *token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, *token_index);
    if (!(token->token_kind == TOKEN_MUL || token->token_kind == TOKEN_DIV)) {
        return left;
    }
    *token_index = *token_index + 1;
    Ast *right = read_factor(token_vec, token_index);
    return read_term_tail(token_vec, token_index, make_ast_op(token->token_kind, left, right));
}

static Ast *read_term(Vector *token_vec, int *token_index) {
    Ast *left = read_factor(token_vec, token_index);
    return read_term_tail(token_vec, token_index, left);
}

static Ast *read_expr_tail(Vector *token_vec, int *token_index, Ast *left) {
    if (token_vec->count == *token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, *token_index);
    if (!(token->token_kind == TOKEN_ADD || token->token_kind == TOKEN_SUB)) {
        return left;
    }
    *token_index = *token_index + 1;
    Ast *right = read_term(token_vec, token_index);
    return read_expr_tail(token_vec, token_index, make_ast_op(token->token_kind, left, right));
}

static Ast *read_expr(Vector *token_vec, int *token_index) {
    Ast *left = read_term(token_vec, token_index);
    return read_expr_tail(token_vec, token_index, left);
}

static Ast *read_stat(Vector *token_vec, int *token_index) {
    Ast *ast = malloc(sizeof(Ast));
    Token *token = vector_get(token_vec, *token_index);
    match(token, TOKEN_RETURN, token_index);
    ast->type = AST_RETURN;

    ast->stat_expr = read_expr(token_vec, token_index);
    return ast;
}

static Ast *read_stat_list(Vector *token_vec, int *token_index, TokenKind end_kind) {
    Ast *ast = malloc(sizeof(Ast));
    ast->stat_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, *token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->stat_list, read_stat(token_vec, token_index));
        token = vector_get(token_vec, *token_index);
        match(token, TOKEN_SEMICOLON, token_index);
    }
    return ast;
}

static Ast *read_param(Vector *token_vec, int *token_index) {
    Ast *ast = malloc(sizeof(Ast));
    Token *token = vector_get(token_vec, *token_index);
    match(token, TOKEN_TYPE, token_index);
    ast->param_type = make_ast_type(token->token_val);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_IDENT, token_index);
    ast->param_name = token->token_val;
    return ast;
}

static Ast *read_params_list(Vector *token_vec, int *token_index, TokenKind end_kind) {
    Ast *ast = malloc(sizeof(Ast));
    ast->param_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, *token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->param_list, read_param(token_vec, token_index));
        token = vector_get(token_vec, *token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        match(token, TOKEN_COMMA, token_index);
    }
    return ast;
}

static Ast *read_function(Vector *token_vec, int *token_index) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_FUNC;
    Token *token = vector_get(token_vec, *token_index);
    match(token, TOKEN_TYPE, token_index);
    ast->func_return_type = make_ast_type(token->token_val);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_IDENT, token_index);
    ast->func_name = make_ast_ident(token->token_val);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_LPARAN, token_index);

    ast->func_param_list = read_params_list(token_vec, token_index, TOKEN_RPARAN);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_RPARAN, token_index);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_LPARAN_CURLY, token_index);

    ast->func_stat_list = read_stat_list(token_vec, token_index, TOKEN_RPARAN_CURLY);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_RPARAN_CURLY, token_index);

    return ast;
}

static Ast *read_program(Vector *token_vec, int *token_index) { return read_function(token_vec, token_index); }

Ast *parse(Vector *token_vec) {
    int token_index = 0;
    return read_program(token_vec, &token_index);
}

/* Debug utils */
void print_ast(Ast *ast) {
    switch (ast->type) {
        case AST_INT:
            printf("%d", ast->int_val);
            break;
        default:
            printf("(%c", ast->type);
            print_ast(ast->left);
            printf(" ");
            print_ast(ast->right);
            printf(")");
            break;
    }
}
