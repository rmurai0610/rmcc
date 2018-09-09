#include "rmcc.h"

static Ast *read_func_call();
static Ast *read_stat_list(TokenKind end_kind);
static Ast *read_expr();
static Ast *read_stat();

static Vector *token_vec;
static int token_index;

static int char2int(char c) { return c - '0'; }

static bool is_token(Token *token, TokenKind kind) { return token->token_kind == kind; }
static bool is_token2(Token *token1, Token *token2, TokenKind kind1, TokenKind kind2) {
    return token1->token_kind == kind1 && token2->token_kind == kind2;
}

static void match(Token *token, TokenKind kind) {
    if (!is_token(token, kind)) {
        printf("error on token: %d\n", token_index);
        error_token_mismatch(__func__, token->token_kind, kind);
    }
    token_index++;
}

static void match_curr(TokenKind kind) {
    Token *token = vector_get(token_vec, token_index);
    match(token, kind);
}

static Ast *make_ast() {
    Ast *ast = malloc(sizeof(Ast));
    ast->symbol_table = symbol_table_current();
    return ast;
}

static Ast *make_ast_int(int n) {
    Ast *ast = make_ast();
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
    Ast *ast = make_ast();
    ast->type = AST_TYPE;
    ast->str_val = type;
    return ast;
}

static Ast *make_ast_op(TokenKind op, Ast *left, Ast *right) {
    Ast *ast = make_ast();
    ast->type = AST_BIN_OP;
    switch (op) {
        case TOKEN_ADD:
            ast->bin_op = BIN_ADD;
            break;
        case TOKEN_SUB:
            ast->bin_op = BIN_SUB;
            break;
        case TOKEN_MUL:
            ast->bin_op = BIN_MUL;
            break;
        case TOKEN_DIV:
            ast->bin_op = BIN_DIV;
            break;
        case TOKEN_DOUBLE_EQU:
            ast->bin_op = BIN_DOUBLE_EQU;
            break;
        case TOKEN_GT:
            ast->bin_op = BIN_GT;
            break;
        case TOKEN_GTE:
            ast->bin_op = BIN_GTE;
            break;
        case TOKEN_LT:
            ast->bin_op = BIN_LT;
            break;
        case TOKEN_LTE:
            ast->bin_op = BIN_LTE;
            break;
        default:
            error_token_mismatch_group(__func__, op, "binary operators");
    }
    ast->bin_left = left;
    ast->bin_right = right;
    return ast;
}

static Ast *make_unary_op(TokenKind op, Ast *operand) {
    Ast *ast = make_ast();
    ast->type = AST_UNARY_OP;
    switch (op) {
        case TOKEN_MUL:
            ast->unary_op = UNARY_DREF;
            break;
        case TOKEN_AND:
            ast->unary_op = UNARY_ADDR;
            break;
        default:
            error_token_mismatch_group(__func__, op, "unary operators");
    }
    ast->unary_operand = operand;
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

static Ast *read_primary_expr() {
    Token *token = vector_get(token_vec, token_index);
    if (is_token(vector_get(token_vec, token_index), TOKEN_IDENT) &&
        is_token(vector_get(token_vec, token_index + 1), TOKEN_LPARAN)) {
        return read_func_call();
    }
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

static Ast *read_unary_expr() {
    Token *token = vector_get(token_vec, token_index);
    if (!(is_token(token, TOKEN_MUL) || is_token(token, TOKEN_AND))) {
        return read_primary_expr();
    }
    token_index++;
    return make_unary_op(token->token_kind, read_primary_expr());
}

static Ast *read_multiplicative_expr_tail(Ast *left) {
    if (token_vec->count == token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, token_index);
    if (!(token->token_kind == TOKEN_MUL || token->token_kind == TOKEN_DIV)) {
        return left;
    }
    token_index++;
    Ast *right = read_unary_expr();
    return read_multiplicative_expr_tail(make_ast_op(token->token_kind, left, right));
}

static Ast *read_multiplicative_expr() { return read_multiplicative_expr_tail(read_unary_expr()); }

static Ast *read_additive_expr_tail(Ast *left) {
    if (token_vec->count == token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, token_index);
    if (!(token->token_kind == TOKEN_ADD || token->token_kind == TOKEN_SUB)) {
        return left;
    }
    token_index++;
    Ast *right = read_multiplicative_expr();
    return read_additive_expr_tail(make_ast_op(token->token_kind, left, right));
}

static Ast *read_additive_expr() { return read_additive_expr_tail(read_multiplicative_expr()); }

static Ast *read_relational_expr_tail(Ast *left) {
    if (token_vec->count == token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, token_index);
    if (!(token->token_kind == TOKEN_GT || token->token_kind == TOKEN_GTE || token->token_kind == TOKEN_LT ||
          token->token_kind == TOKEN_LTE)) {
        return left;
    }
    token_index++;
    Ast *right = read_additive_expr();
    return read_relational_expr_tail(make_ast_op(token->token_kind, left, right));
}

static Ast *read_relational_expr() { return read_relational_expr_tail(read_additive_expr()); }

static Ast *read_equality_expr_tail(Ast *left) {
    if (token_vec->count == token_index) {
        return left;
    }
    Token *token = vector_get(token_vec, token_index);
    if (!(token->token_kind == TOKEN_DOUBLE_EQU)) {
        return left;
    }
    token_index++;
    Ast *right = read_relational_expr();
    return read_equality_expr_tail(make_ast_op(token->token_kind, left, right));
}

static Ast *read_equality_expr() { return read_equality_expr_tail(read_relational_expr()); }

static Ast *read_rhs() { return read_expr(); }

static Ast *read_lhs() {
    Ast *ast = make_ast();
    ast->type = AST_LHS;
    ast->lhs_type = NULL;
    Token *token = vector_get(token_vec, token_index);
    if (is_token(token, TOKEN_TYPE)) {
        match(token, TOKEN_TYPE);
        token = vector_get(token_vec, token_index);
        if (is_token(token, TOKEN_MUL)) {
            // TODO make proper pointer type
            match(token, TOKEN_MUL);
        }
        ast->lhs_type = make_ast_type(token->token_val);
    }
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_IDENT);
    ast->lhs_ident = make_ast_ident(token->token_val);
    if (ast->lhs_type != NULL) {
        symbol_table_add_variable(token->token_val);
    }
    return ast;
}

static Ast *read_assignment_expr() {
    Ast *ast = make_ast();
    ast->type = AST_ASSIGN;
    ast->assign_lhs = read_lhs();
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_EQU);
    ast->assign_rhs = read_rhs();
    return ast;
}

static bool try_read_assignment_expr(Ast **ast) {
    Token *token1 = vector_get(token_vec, token_index);
    Token *token2 = vector_get(token_vec, token_index + 1);
    if (!(is_token(token1, TOKEN_TYPE) || is_token2(token1, token2, TOKEN_MUL, TOKEN_TYPE) ||
          is_token2(token1, token2, TOKEN_IDENT, TOKEN_EQU))) {
        return false;
    }
    *ast = read_assignment_expr();
    return true;
}

static Ast *read_expr() {
    Ast *ast = NULL;
    if (try_read_assignment_expr(&ast)) {
        return ast;
    }
    return read_equality_expr();
}

static Ast *read_arg_list(TokenKind end_kind) {
    Ast *ast = make_ast();
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
    Ast *ast = make_ast();
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

static Ast *read_expr_stat() {
    Ast *ast = read_expr();
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_SEMICOLON);
    return ast;
}

static Ast *read_return_stat() {
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_RETURN);
    Ast *ast = make_ast();
    ast->type = AST_RETURN;
    ast->stat_rhs = read_rhs();
    return ast;
}

static Ast *read_for_stat() {
    match_curr(TOKEN_FOR);
    Ast *ast = make_ast();
    ast->type = AST_FOR;

    symbol_table_open_scope();
    match_curr(TOKEN_LPARAN);
    ast->for_init = read_expr_stat();
    ast->for_cond = read_expr_stat();
    ast->for_update = read_expr();
    match_curr(TOKEN_RPARAN);

    match_curr(TOKEN_LPARAN_CURLY);
    ast->for_body = read_stat_list(TOKEN_RPARAN_CURLY);
    match_curr(TOKEN_RPARAN_CURLY);
    symbol_table_close_scope();

    return ast;
}

static Ast *read_if_stat() {
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_IF);
    Ast *ast = make_ast();
    ast->type = AST_IF;
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_LPARAN);
    ast->if_cond = read_expr();
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_RPARAN);
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_LPARAN_CURLY);
    symbol_table_open_scope();
    ast->if_branch = read_stat_list(TOKEN_RPARAN_CURLY);
    symbol_table_close_scope();
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_RPARAN_CURLY);
    return ast;
}

static bool try_read_if_stat(Ast **ast) {
    Token *token = vector_get(token_vec, token_index);
    if (!is_token(token, TOKEN_IF)) {
        return false;
    }
    *ast = read_if_stat();
    return true;
}

static bool try_read_for_stat(Ast **ast) {
    Token *token = vector_get(token_vec, token_index);
    if (!is_token(token, TOKEN_FOR)) {
        return false;
    }
    *ast = read_for_stat();
    return true;
}

static bool try_read_jump_stat(Ast **ast) {
    Token *token = vector_get(token_vec, token_index);
    if (!is_token(token, TOKEN_RETURN)) {
        return false;
    }
    *ast = read_return_stat();
    token = vector_get(token_vec, token_index);
    match(token, TOKEN_SEMICOLON);
    return true;
}

static Ast *read_stat() {
    Ast *ast = NULL;
    if (try_read_jump_stat(&ast)) {
        return ast;
    }
    if (try_read_if_stat(&ast)) {
        return ast;
    }
    if (try_read_for_stat(&ast)) {
        return ast;
    }
    return read_expr_stat();
}

static Ast *read_stat_list(TokenKind end_kind) {
    Ast *ast = make_ast();
    ast->type = AST_STAT_LIST;
    ast->stat_list = vector_init();
    for (;;) {
        Token *token = vector_get(token_vec, token_index);
        if (token->token_kind == end_kind) {
            return ast;
        }
        vector_add(ast->stat_list, read_stat());
        token = vector_get(token_vec, token_index);
    }
}

static Ast *read_param() {
    Ast *ast = make_ast();
    ast->type = AST_PARAM;
    Token *token = vector_get(token_vec, token_index);
    match(token, TOKEN_TYPE);
    ast->param_type = make_ast_type(token->token_val);

    token = vector_get(token_vec, token_index);
    match(token, TOKEN_IDENT);
    ast->param_name = make_ast_ident(token->token_val);

    symbol_table_add_param(token->token_val);

    return ast;
}

static Ast *read_param_list(TokenKind end_kind) {
    Ast *ast = make_ast();
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
    Ast *ast = make_ast();
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

static Ast *read_top_level() { return read_function(); }

static Ast *read_program() {
    Ast *ast = make_ast();
    ast->type = AST_PROGRAM;
    ast->program = vector_init();
    while (token_index != token_vec->count) {
        Ast *top_level = read_top_level();
        vector_add(ast->program, top_level);
    }
    return ast;
}

Ast *parse(Vector *vec) {
    token_index = 0;
    token_vec = vec;
    return read_program();
}
