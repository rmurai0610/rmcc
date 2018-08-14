#include "self-c.h"
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

void compile(Ast *ast) {
    printf(
        ".intel_syntax noprefix\n"
        ".text\n"
        "\t.global _mymain\n"
        "_mymain:\n");
    emit_expr(ast);
    printf("ret\n");
}
