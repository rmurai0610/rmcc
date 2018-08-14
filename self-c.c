#include "self-c.h"

int main(int argc, char const *argv[]) {
    Vector *token_vec = lex_init();
    lex_scan(token_vec);
    /*Ast *ast = read_expr();*/
    if (argc == 2) {
        if (!strcmp(argv[1], "-a")) {
            Ast *ast = parse(token_vec);
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
        Ast *ast = parse(token_vec);
        compile(ast);
    }
    return 0;
}
