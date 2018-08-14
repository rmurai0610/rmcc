#include "self-c.h"

static int char2int(char c) { return c - '0'; }

static void skip_space(void) {
    int c;
    while ((c = getc(stdin)) != EOF) {
        if (isspace(c)) continue;
        ungetc(c, stdin);
        return;
    }
}

static Ast *make_ast_ident(char *ident) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->type = AST_IDENT;
    ast->str_val = ident;
    return ast;
}

static Ast *make_ast_int(int n) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->type = AST_INT;
    ast->int_val = n;
    return ast;
}

static Ast *make_ast_op(char op, Ast *left, Ast *right) {
    Ast *ast = (Ast *)malloc(sizeof(Ast));
    ast->type = op;
    ast->left = left;
    ast->right = right;
    return ast;
}

static Ast *read_ident() {
    char *buf = (char *)malloc(IDENT_BUF_LEN);
    for (int i = 0; i < IDENT_BUF_LEN; ++i) {
        char c = getc(stdin);
        if (!isalnum(c)) {
            ungetc(c, stdin);
            return make_ast_ident(buf);
        }
        if (i == 0 && isdigit(c)) {
            error("%s: Expected alphabet but received %c\n", __func__, c);
        }
        buf[i] = c;
    }
    error("%s: Identifier exceeded %d char limit\n", __func__, IDENT_BUF_LEN);
}

static Ast *read_number() {
    int n = 0;
    for (;;) {
        char c = getc(stdin);
        if (!isdigit(c)) {
            ungetc(c, stdin);
            return make_ast_int(n);
        }
        n = n * 10 + char2int(c);
    }
}

static Ast *read_factor() {
    skip_space();
    char c = getc(stdin);
    if (isdigit(c)) {
        ungetc(c, stdin);
        return read_number();
    }
    error("%s: Expected digit but received %c", __func__, c);
}

static Ast *read_term_tail(Ast *left) {
    skip_space();
    char c = getc(stdin);
    if (c == EOF) {
        return left;
    }
    if (c != '*' && c != '/') {
        ungetc(c, stdin);
        return left;
    }
    Ast *right = read_factor();
    return read_term_tail(make_ast_op(c, left, right));
}

static Ast *read_term() {
    skip_space();
    Ast *left = read_factor();
    return read_term_tail(left);
}

static Ast *read_expr_tail(Ast *left) {
    skip_space();
    char c = getc(stdin);
    if (c == EOF) {
        return left;
    }
    if (c != '+' && c != '-') {
        ungetc(c, stdin);
        return left;
    }
    Ast *right = read_term();
    return read_expr_tail(make_ast_op(c, left, right));
}

static Ast *read_expr(void) {
    skip_space();
    Ast *left = read_term();
    return read_expr_tail(left);
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

static void emit_func(Ast *ast) {}

static void emit_program(Ast *ast) {
    if (ast->type == AST_FUNC) {
        emit_func(ast);
    }
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
    lex_print_tokens(token_vec);
    /*Ast *ast = read_expr();*/
    /*if (argc == 2 && !strcmp(argv[1], "-a")) {*/
    /*print_ast(ast);*/
    /*printf("\n");*/
    /*} else {*/
    /*compile(ast);*/
    /*}*/
    return 0;
}
