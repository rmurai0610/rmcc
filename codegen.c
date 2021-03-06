#include "rmcc.h"
static void emit_expr(Ast *ast);
static void emit_stat_list(Ast *ast);

static int if_jump = 0;
static int for_jump = 0;

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

static void push(char *reg) {
    if (reg[0] == 'r') {
        printf("\tpush %s\n", reg);
        return;
    }
    if (reg[0] == 'e') {
        printf("\tsub esp, 4\n");
        printf("\tmov [esp], %s\n", reg);
        return;
    }
}

static void pop(char *reg) {
    if (reg[0] == 'r') {
        printf("\tpop %s\n", reg);
        return;
    }
    if (reg[0] == 'e') {
        printf("\tmov %s, [esp]\n", reg);
        printf("\tadd esp, 4\n");
        return;
    }
}

static void emit_binop(Ast *ast) {
    char *op;
    switch (ast->bin_op) {
        case BIN_ADD:
            op = "add";
            break;
        case BIN_SUB:
            op = "sub";
            break;
        case BIN_MUL:
            op = "imul";
            break;
        case BIN_DIV:
            op = "idiv";
            break;
        case BIN_DOUBLE_EQU:
            op = "sete";
            break;
        case BIN_GT:
            op = "setg";
            break;
        case BIN_GTE:
            op = "setge";
            break;
        case BIN_LT:
            op = "setl";
            break;
        case BIN_LTE:
            op = "setle";
            break;
    }
    emit_expr(ast->bin_right);
    push("rax");
    emit_expr(ast->bin_left);
    pop("rbx");
    if (ast->bin_op == BIN_DOUBLE_EQU || ast->bin_op == BIN_GT || ast->bin_op == BIN_GTE || ast->bin_op == BIN_LT ||
        ast->bin_op == BIN_LTE) {
        printf("\tcmp rax, rbx\n");
        printf("\t%s al\n", op);
        printf("\tmovzx rax, al\n");
        return;
    }
    if (ast->bin_op == BIN_DIV) {
        printf("\tcdq\n");
    }
    printf("\t%s rax, rbx\n", op);
}

static void emit_unary_op(Ast *ast) {
    if (ast->unary_op == UNARY_ADDR && ast->unary_operand->type == AST_IDENT) {
        int offset = get_ident_offset(ast->unary_operand);
        if (offset >= 0) {
            printf("\tlea rax, [rbp-%d]\n", offset);
        } else {
            printf("\tlea rax, [rbp+%d]\n", -offset);
        }
    }
    if (ast->unary_op == UNARY_DREF) {
        emit_expr(ast->unary_operand);
        printf("\tmov rax, [rax]\n");
    }
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
            push("rax");
        }
        printf("\tcall %s\n", ast->func_name->str_val);
        printf("\tadd rsp, %d\n", 8 * arguments->count);
    }
    if (ast->type == AST_BIN_OP) {
        emit_binop(ast);
    }
    if (ast->type == AST_UNARY_OP) {
        emit_unary_op(ast);
    }
    if (ast->type == AST_ASSIGN) {
        emit_expr(ast->assign_rhs);
        int offset = get_ident_offset(ast->assign_lhs->lhs_ident);
        if (offset >= 0) {
            printf("\tmov [rbp-%d], rax\n", offset);
        } else {
            printf("\tmov [rbp+%d], rax\n", -offset);
        }
    }
}

static void emit_stat(Ast *ast) {
    if (ast->type == AST_RETURN) {
        emit_expr(ast->stat_rhs);
        printf("\tmov rsp, rbp\n");
        pop("rbp");
        printf("\tret\n");
        return;
    }
    if (ast->type == AST_IF) {
        emit_expr(ast->if_cond);
        printf("\tcmp rax, 0\n");
        printf("\tje if_end_%d\n", if_jump);
        emit_stat_list(ast->if_branch);
        printf("if_end_%d:\n", if_jump++);
        return;
    }
    if (ast->type == AST_FOR) {
        emit_expr(ast->for_init);
        printf("for_begin_%d:\n", for_jump);
        emit_expr(ast->for_cond);
        printf("\tcmp rax, 0\n");
        printf("\tje for_end_%d\n", if_jump);
        emit_expr(ast->for_update);
        emit_stat_list(ast->for_body);
        printf("\tjmp for_begin_%d\n", for_jump);
        printf("for_end_%d:\n", for_jump++);
        return;
    }
    emit_expr(ast);
}

static void emit_stat_list(Ast *ast) {
    for (int i = 0; i < ast->stat_list->count; ++i) {
        emit_stat(vector_get(ast->stat_list, i));
    }
}

static void emit_func(Ast *ast) {
    printf("%s:\n", ast->func_name->str_val);
    push("rbp");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", (ast->symbol_table->variable_offset - 1) * 8);
    emit_stat_list(ast->func_stat_list);
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
