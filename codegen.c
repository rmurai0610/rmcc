#include "self-c.h"
static void emit_expr(Ast *ast);
static void emit_binop(Ast *ast) {
    char *op;
    switch (ast->op) {
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
    printf("\tpush rax\n");
    emit_expr(ast->left);
    printf("\tpop rbx\n");
    if (ast->op == '/') {
        printf("\tcdq\n");
    }
    printf("\t%s rax, rbx\n", op);
}

static void emit_expr(Ast *ast) {
    if (ast->type == AST_INT) {
        printf("\tmov rax, %d\n", ast->int_val);
        return;
    }
    if (ast->type == AST_IDENT) {
        int offset = symbol_table_get_symbol_from_table(ast->symbol_table, ast->str_val)->offset * 8;
        printf("\tmov rax, [rbp-%d]\n", offset);
        return;
    }
    emit_binop(ast);
}

static void emit_stat(Ast *ast) {
    if (ast->type == AST_RETURN) {
        emit_expr(ast->stat_rhs);
        printf("pop rsp\n");
        printf("pop rbp\n");
        printf("\tret\n");
    }
    if (ast->type == AST_ASSIGN) {
        emit_expr(ast->stat_rhs);
        int offset = symbol_table_get_symbol_from_table(ast->symbol_table, ast->stat_lhs->str_val)->offset * 8;
        printf("\tmov [rbp-%d], rax\n", offset);
    }
}

static void emit_stat_list(Ast *ast) {
    for (int i = 0; i < ast->stat_list->count; ++i) {
        emit_stat(vector_get(ast->stat_list, i));
    }
}

static void emit_func(Ast *ast) {
    printf("%s:\n", ast->func_name->str_val);
    printf("\tpush rbp\n");
    printf("\tpush rsp\n");
    emit_stat_list(ast->func_stat_list);
    printf("\tpop rsp\n");
    printf("\tpop rbp\n");
    printf("\tret\n");
}

static void emit_program(Ast *ast) {
    for (int i = 0; i < ast->program->count; ++i) {
        Ast *a = vector_get(ast->program, i);
        if (a->type == AST_FUNC) {
            emit_func(a);
        }
    }
}

void compile(Ast *ast) {
    printf(
        ".intel_syntax noprefix\n"
        ".text\n"
        "\t.global _mymain\n"
        "_mymain:\n"
        "\tcall main\n"
        "\tret\n");
    emit_program(ast);
}
