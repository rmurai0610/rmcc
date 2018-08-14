#include "self-c.h"

static void skip_space(void) {
    int c;
    while ((c = getc(stdin)) != EOF) {
        if (isspace(c)) continue;
        ungetc(c, stdin);
        return;
    }
}

static void make_operator(Token *token, char c) {
    token->token_val = NULL;
    switch (c) {
        case '+':
            token->token_kind = TOKEN_ADD;
            break;
        case '-':
            token->token_kind = TOKEN_SUB;
            break;
        case '*':
            token->token_kind = TOKEN_MUL;
            break;
        case '/':
            token->token_kind = TOKEN_DIV;
            break;
        case '=':
            token->token_kind = TOKEN_EQU;
            break;
        default:
            error("%s: Unexpected token %c\n", __func__, c);
    }
}

static void make_punctuation(Token *token, char c) {
    switch (c) {
        case '{':
            token->token_kind = TOKEN_LPARAN_CURLY;
            break;
        case '}':
            token->token_kind = TOKEN_RPARAN_CURLY;
            break;
        case '(':
            token->token_kind = TOKEN_LPARAN;
            break;
        case ')':
            token->token_kind = TOKEN_RPARAN;
            break;
        case ',':
            token->token_kind = TOKEN_COMMA;
            break;
        case ';':
            token->token_kind = TOKEN_COMMA;
            break;
        default:
            error("%s: Unexpected token %c\n", __func__, c);
    }
    token->token_val = NULL;
}

static void make_number(Token *token, char c) {
    token->token_kind = TOKEN_INT_LIT;
    char *buf = (char *)malloc(IDENT_BUF_LEN);
    buf[0] = c;
    for (int i = 1; i < IDENT_BUF_LEN; ++i) {
        c = getc(stdin);
        if (!isdigit(c)) {
            ungetc(c, stdin);
            buf[i] = '\0';
            token->token_val = buf;
            return;
        }
        buf[i] = c;
    }
    error("%s: Buffer overflow\n", __func__);
}

static void make_ident(Token *token, char c) {
    token->token_kind = TOKEN_IDENT;
    char *buf = (char *)malloc(IDENT_BUF_LEN);
    buf[0] = c;
    for (int i = 1; i < IDENT_BUF_LEN; ++i) {
        c = getc(stdin);
        if (!(c == '_' || isalnum(c))) {
            ungetc(c, stdin);
            buf[i] = '\0';
            token->token_val = buf;
            return;
        }
        buf[i] = c;
    }
    error("%s: Buffer overflow\n", __func__);
}

Vector *lex_init() {
    Vector *token_vec = (Vector *)vector_init();
    return token_vec;
}

void lex_scan(Vector *token_vec) {
    for (;;) {
        skip_space();
        char c = getc(stdin);
        if (c == EOF) {
            return;
        }
        Token *token = (Token *)malloc(sizeof(Token));
        switch (c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '=':
                make_operator(token, c);
                break;
            case '{':
            case '}':
            case '(':
            case ')':
            case ',':
            case ';':
                make_punctuation(token, c);
                break;
            case '0' ... '9':
                make_number(token, c);
                break;
            case 'A' ... 'Z':
            case 'a' ... 'z':
            case '_':
                make_ident(token, c);
                break;
            default:
                error("%s: Unexpected token %c", __func__, c);
        }
        vector_add(token_vec, token);
    }
}

/* Debug utils */
void lex_print_tokens(Vector *token_vec) {
    for (int i = 0; i < token_vec->count; ++i) {
        Token *token = vector_get(token_vec, i);
        switch (token->token_kind) {
            case TOKEN_LPARAN:
                printf("TOKEN_LPARAN\n");
                break;
            case TOKEN_RPARAN:
                printf("TOKEN_RPARAN\n");
                break;
            case TOKEN_LPARAN_CURLY:
                printf("TOKEN_LPARAN_CURLY\n");
                break;
            case TOKEN_RPARAN_CURLY:
                printf("TOKEN_RPARAN_CURLY\n");
                break;
            case TOKEN_COMMA:
                printf("TOKEN_COMMA\n");
                break;
            case TOKEN_SEMICOLON:
                printf("TOKEN_SEMICOLON\n");
                break;
            case TOKEN_RETURN:
                printf("TOKEN_RETURN\n");
                break;
            case TOKEN_ADD:
                printf("TOKEN_ADD\n");
                break;
            case TOKEN_SUB:
                printf("TOKEN_SUB\n");
                break;
            case TOKEN_MUL:
                printf("TOKEN_MUL\n");
                break;
            case TOKEN_DIV:
                printf("TOKEN_DIV\n");
                break;
            case TOKEN_EQU:
                printf("TOKEN_EQU\n");
                break;
            case TOKEN_IDENT:
                printf("TOKEN_IDENT %s\n", token->token_val);
                break;
            case TOKEN_INT_LIT:
                printf("TOKEN_INT_LIT %s\n", token->token_val);
                break;
        }
    }
}
