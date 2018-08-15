#include "self-c.h"
const char *token_kind_string[] = {ALL_TOKENS(TO_STRING)};

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
            error_unexpected_token(__func__, c);
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
            token->token_kind = TOKEN_SEMICOLON;
            break;
        default:
            error_unexpected_token(__func__, c);
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
    error_buffer_overflow(__func__, IDENT_BUF_LEN);
}

static void make_ident_or_type(Token *token, char c) {
    token->token_kind = TOKEN_IDENT;
    char *buf = (char *)malloc(IDENT_BUF_LEN);
    buf[0] = c;
    for (int i = 1; i < IDENT_BUF_LEN; ++i) {
        c = getc(stdin);
        if (!(c == '_' || isalnum(c))) {
            ungetc(c, stdin);
            buf[i] = '\0';
            if (!strcmp(buf, "return")) {
                token->token_kind = TOKEN_RETURN;
            }
            if (!strcmp(buf, "int")) {
                token->token_kind = TOKEN_TYPE;
            }
            token->token_val = buf;
            return;
        }
        buf[i] = c;
    }
    error_buffer_overflow(__func__, IDENT_BUF_LEN);
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
                make_ident_or_type(token, c);
                break;
            default:
                error_unexpected_token(__func__, c);
        }
        vector_add(token_vec, token);
    }
}

/* Debug utils */
void lex_print_tokens(Vector *token_vec) {
    for (int i = 0; i < token_vec->count; ++i) {
        Token *token = vector_get(token_vec, i);
        printf("%s\n", token_kind_string[token->token_kind]);
    }
}
