#include "self-c.h"

void error(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

void error_unexpected_token(const char *func_name, char unexpected) {
    error("%s: Unexcepted token %c\n", func_name, unexpected);
}

void error_buffer_overflow(const char *func_name, int max_size) {
    error("%s: Buffer overflow, buffer limit %d exceeded\n", func_name, max_size);
}

void error_token_mismatch(const char *func_name, TokenKind token_actual, TokenKind token_excepted) {
    error("%s: Token mismatch, expected %s but got %s\n", func_name, token_kind_string[token_excepted],
          token_kind_string[token_actual]);
}

void error_token_mismatch_group(const char *func_name, TokenKind token_actual, char *group) {
    error("%s: Token mismatch, expected group %s but got %s\n", func_name, group, token_kind_string[token_actual]);
}
void error_identifier_not_found(const char *func_name, char *ident) {
    error("%s: Identifier %s not found\n", func_name, ident);
}
