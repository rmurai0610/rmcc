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

static void emit_expr(Ast *ast);
static void emit_binop(Ast *ast) {
    char *op;
    switch (ast->type) {
        case '+':
            op = "add";
            break;
        case '-':
            op = "sub";
            break;
        case '*':
            op = "imul";
            break;
        case '/':
            op = "idiv";
            break;
    }
    emit_expr(ast->right);
    printf("\tPUSH rax\n");
    emit_expr(ast->left);
    printf("\tPOP rbx\n");
    if (ast->type == '/') {
        printf("\tcdq\n");
    }
    printf("\t%s rax, rbx\n", op);
}

static void emit_expr(Ast *ast) {
    if (ast->type == AST_INT) {
        printf("\tmov rax, %d\n", ast->int_val);
    } else {
        emit_binop(ast);
    }
}

static void emit_func(Ast *ast) { /** **/
}

static void emit_program(Ast *ast) {
    if (ast->type == AST_FUNC) {
        emit_func(ast);
    }
}

static Ast *parser_to_ast(Vector *token_vec) {
    Ast *ast = NULL;
    int token_index = 0;
    return read_expr(ast, token_vec, &token_index);
}

static void compile(Ast *ast) {
    printf(
        ".intel_syntax noprefix\n"
        ".text\n"
        "\t.global _mymain\n"
        "_mymain:\n");
    emit_expr(ast);
    printf("ret\n");
}

static void print_ast(Ast *ast) {
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

int main(int argc, char const *argv[]) {
    Vector *token_vec = lex_init();
    lex_scan(token_vec);
    /*Ast *ast = read_expr();*/
    if (argc == 2) {
        if (!strcmp(argv[1], "-a")) {
            Ast *ast = parser_to_ast(token_vec);
            print_ast(ast);
            printf("\n");
            return 0;
        }
        if (!strcmp(argv[1], "-t")) {
            lex_print_tokens(token_vec);
            printf("\n");
            return 0;
        }
    } else {
        Ast *ast = parser_to_ast(token_vec);
        compile(ast);
    }
    return 0;
}
