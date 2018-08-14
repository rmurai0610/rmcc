#include "self-c.h"

static int char2int(char c) { return c - '0'; }

static Ast *make_ast_int(int n) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->type = AST_INT;
    ast->int_val = n;
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
        error("%s: Expected digit", __func__);
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

static Ast *read_expr(Ast *ast, Vector *token_vec, int *token_index) {
    Ast *left = read_term(token_vec, token_index);
    return read_expr_tail(token_vec, token_index, left);
}

Ast *parse(Vector *token_vec) {
    Ast *ast = NULL;
    int token_index = 0;
    return read_expr(ast, token_vec, &token_index);
}

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
