#include "self-c.h"
const char *token_kind_string[] = {ALL_TOKENS(TO_STRING)};
const char *ast_type_string[] = {ALL_AST(TO_STRING)};
const char *bin_op_string[] = {ALL_BIN_OP(TO_STRING)};

static const char *bin_op2str(BinOp op) { return bin_op_string[op]; }
/*static const char *ast_type2str(AstType type) { return ast_type_string[type]; }*/

void lex_print_tokens(Vector *token_vec) {
    for (int i = 0; i < token_vec->count; ++i) {
        Token *token = vector_get(token_vec, i);
        printf("%03d %s\n", i, token_kind_string[token->token_kind]);
    }
}

void print_ast(Ast *ast) {
    switch (ast->type) {
        case AST_TYPE:
            printf("%s", ast->str_val);
            break;
        case AST_INT:
            printf("%d", ast->int_val);
            break;
        case AST_IDENT:
            printf("%s", ast->str_val);
            break;
        case AST_BIN_OP:
            printf("(");
            printf("%s ", bin_op2str(ast->bin_op));
            print_ast(ast->bin_left);
            printf(" ");
            print_ast(ast->bin_right);
            printf(")");
            break;
        case AST_FUNC_CALL:
            printf("CALL ");
            print_ast(ast->func_call_name);
            print_ast(ast->func_call_arg_list);
            break;
        case AST_LHS:
            if (ast->lhs_type) {
                print_ast(ast->lhs_type);
                printf(" ");
            }
            print_ast(ast->lhs_ident);
            break;
        case AST_RETURN:
            printf("RETURN ");
            print_ast(ast->stat_rhs);
            break;
        case AST_IF:
            printf("IF ");
            print_ast(ast->if_cond);
            printf("\n");
            print_ast(ast->if_branch);
            break;
        case AST_ASSIGN:
            print_ast(ast->stat_lhs);
            printf(" = ");
            print_ast(ast->stat_rhs);
            break;
        case AST_STAT_LIST:
            for (int i = 0; i < ast->stat_list->count; ++i) {
                print_ast(vector_get(ast->stat_list, i));
                printf("\n");
            }
            break;
        case AST_PARAM:
            print_ast(ast->param_type);
            printf(" ");
            print_ast(ast->param_name);
            break;
        case AST_PARAM_LIST:
            for (int i = 0; i < ast->param_list->count; ++i) {
                print_ast(vector_get(ast->param_list, i));
                if (i != ast->param_list->count - 1) {
                    printf(", ");
                }
            }
            printf("\n");
            break;
        case AST_FUNC:
            printf("FUNC: ");
            print_ast(ast->func_name);
            printf("\n");
            printf("PARAMS: ");
            print_ast(ast->func_param_list);
            print_ast(ast->func_stat_list);
            printf("\n");
            break;
        case AST_PROGRAM:
            printf("PROGRAM:\n");
            for (int i = 0; i < ast->program->count; ++i) {
                print_ast(vector_get(ast->program, i));
            }
            break;
        default:
            break;
    }
}
