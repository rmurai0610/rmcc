#include "self-c.h"

static int char2int(char c) { return c - '0'; }

static void match(Token *token, TokenKind kind) {
    if (token->token_kind != kind) {
        // TODO better error msg
        error("Could not match token %d != %d\n", token->token_kind, kind);
    }
}

static Ast *make_ast_int(int n) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->type = AST_INT;
    ast->int_val = n;
    return ast;
}

static Ast *make_ast_ident(char *ident) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->type = AST_IDENT;
    ast->str_val = ident;
    return ast;
}

static Ast *make_ast_op(TokenKind op, Ast *left, Ast *right) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
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
            error("%s: Unexpected token");
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
        error("%s: Expected digit! %s", __func__, token->token_val);
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
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    Token *token = vector_get(token_vec, *token_index);
    match(token, TOKEN_RETURN);
    *token_index = *token_index + 1;
    ast->type = AST_RETURN;

    ast->stat_expr = read_expr(token_vec, token_index);
    return ast;
}

static Ast *read_stat_list(Vector *token_vec, int *token_index, TokenKind end_kind) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->stat_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, *token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->stat_list, read_stat(token_vec, token_index));
        token = vector_get(token_vec, *token_index);
        match(token, TOKEN_SEMICOLON);
        *token_index = *token_index + 1;
    }
    return ast;
}

static Ast *read_function(Vector *token_vec, int *token_index) {
    Ast *ast = malloc(sizeof(Ast));
    ast->type = AST_FUNC;
    Token *token = vector_get(token_vec, *token_index);
    match(token, TOKEN_IDENT);
    ast->func_return_type = make_ast_ident(token->token_val);
    *token_index = *token_index + 1;

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_IDENT);
    ast->func_name = make_ast_ident(token->token_val);
    *token_index = *token_index + 1;

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_LPARAN);
    *token_index = *token_index + 1;

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_RPARAN);
    *token_index = *token_index + 1;

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_LPARAN_CURLY);
    *token_index = *token_index + 1;

    ast->func_stat_list = read_stat_list(token_vec, token_index, TOKEN_RPARAN_CURLY);

    token = vector_get(token_vec, *token_index);
    match(token, TOKEN_RPARAN_CURLY);
    *token_index = *token_index + 1;

    return ast;
}

static Ast *read_program(Vector *token_vec, int *token_index) { return read_function(token_vec, token_index); }

Ast *parse(Vector *token_vec) {
    int token_index = 0;
    return read_program(token_vec, &token_index);
}

/* Debug utils */
void print_ast(Ast *ast) {
    if (ast->type == AST_INT) {
        printf("%d", ast->int_val);
    } else {
        printf("(%c", ast->type);
        print_ast(ast->left);
        printf(" ");
        print_ast(ast->right);
        printf(")");
    }
}
