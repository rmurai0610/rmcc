#include "self-c.h"
static void emit_expr(Ast *ast);

static int get_ident_offset(Ast *ast) {
    char *ident = ast->str_val;
    Symbol *symbol = symbol_table_get_symbol_from_table(ast->symbol_table, ident);
    if (symbol->type == SYMBOL_VARIABLE) {
        return symbol->offset * 8;
    }
    if (symbol->type == SYMBOL_PARAM) {
        return -(symbol->offset * 8 + 8);
    }
    return 0;
}

static void emit_binop(Ast *ast) {
    char *op;
    switch (ast->bin_op) {
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
    emit_expr(ast->bin_right);
    printf("\tpush rax\n");
    emit_expr(ast->bin_left);
    printf("\tpop rbx\n");
    if (ast->bin_op == '/') {
        printf("\tcdq\n");
    }
    printf("\t%s rax, rbx\n", op);
}

static void emit_expr(Ast *ast) {
    if (ast->type == AST_INT) {
        printf("\tmov rax, %d\n", ast->int_val);
    }
    if (ast->type == AST_IDENT) {
        int offset = get_ident_offset(ast);
        if (offset >= 0) {
            printf("\tmov rax, [rbp-%d]\n", offset);
        } else {
            printf("\tmov rax, [rbp+%d]\n", -offset);
        }
    }
    if (ast->type == AST_FUNC_CALL) {
        Vector *arguments = ast->func_call_arg_list->arg_list;
        for (int i = arguments->count - 1; i >= 0; --i) {
            Ast *arg = vector_get(arguments, i);
            emit_expr(arg);
            printf("\tpush rax\n");
        }
        printf("\tcall %s\n", ast->func_name->str_val);
    }
    if (ast->type == AST_BIN_OP) {
        emit_binop(ast);
    }
}

static void emit_stat(Ast *ast) {
    if (ast->type == AST_RETURN) {
        emit_expr(ast->stat_rhs);
        printf("\tmov rsp, rbp\n");
        printf("\tpop rbp\n");
        printf("\tret\n");
    }
    if (ast->type == AST_ASSIGN) {
        emit_expr(ast->stat_rhs);
        int offset = get_ident_offset(ast->stat_lhs->lhs_ident);
        if (offset >= 0) {
            printf("\tmov [rbp-%d], rax\n", offset);
        } else {
            printf("\tmov [rbp+%d], rax\n", -offset);
        }
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
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", (ast->symbol_table->variable_offset - 1) * 8);
    emit_stat_list(ast->func_stat_list);
    /*printf("\tpop rsp\n");*/
    /*printf("\tpop rbp\n");*/
    /*printf("\tret\n");*/
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
