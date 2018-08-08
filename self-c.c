#include <ctype.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

enum { AST_INT };

typedef struct Ast {
    char type;
    union {
        int int_val;
        struct {
            struct Ast *left;
            struct Ast *right;
        };
    };
} Ast;

static void error(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

static int char2int(char c) { return c - '0'; }
static void skip_space(void) {
    int c;
    while ((c = getc(stdin)) != EOF) {
        if (isspace(c)) continue;
        ungetc(c, stdin);
        return;
    }
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

static Ast *read_number(int n) {
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
        return read_number(c - '0');
    }
    error("factor: cannot handle %c", c);
    return 0;
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
    emit_expr(ast->left);
    printf("\tmov ebx, eax\n");
    emit_expr(ast->right);
    if (ast->type == '/') {
        printf("\tmov ecx, eax\n");
        printf("\tmov eax, ebx\n");
        printf("\tcdq\n");
        printf("\t%s ecx\n", op);
    } else {
        printf("\t%s ebx, eax\n", op);
        printf("\tmov eax, ebx\n");
    }
}

static void emit_expr(Ast *ast) {
    if (ast->type == AST_INT) {
        printf("\tmov eax, %d\n", ast->int_val);
    } else {
        emit_binop(ast);
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
    Ast *ast = read_expr();
    /*print_ast(ast);*/
    /*printf("\n");*/
    compile(ast);
    return 0;
}
