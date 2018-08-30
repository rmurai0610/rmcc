#include "rmcc.h"

static SymbolTable *root_table;
static SymbolTable *current_table;

void symbol_table_init(void) {
    root_table = malloc(sizeof(SymbolTable));
    root_table->parent = NULL;
    root_table->childrens = vector_init();
    root_table->symbols = map_init();
    root_table->variable_offset = 1;
    root_table->param_offset = 1;

    current_table = root_table;
}

void symbol_table_open_scope(void) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->parent = current_table;
    table->childrens = vector_init();
    table->symbols = map_init();
    table->variable_offset = 1;
    table->param_offset = 1;
    vector_add(current_table->childrens, table);

    current_table = table;
}

void symbol_table_close_scope(void) { current_table = current_table->parent; }

Symbol *symbol_table_get_symbol(char *key) {
    SymbolTable *table = current_table;
    for (;;) {
        if (table == NULL) {
            return NULL;
        }
        Symbol *symbol = map_get(table->symbols, key);
        if (symbol != NULL) {
            return symbol;
        }
        table = table->parent;
    }
}

Symbol *symbol_table_get_symbol_from_table(SymbolTable *table, char *key) {
    for (;;) {
        if (table == NULL) {
            return NULL;
        }
        Symbol *symbol = map_get(table->symbols, key);
        if (symbol != NULL) {
            return symbol;
        }
        table = table->parent;
    }
}

void symbol_table_add_variable(char *key) {
    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->type = SYMBOL_VARIABLE;
    symbol->offset = current_table->variable_offset++;
    map_add(current_table->symbols, key, symbol);
}

void symbol_table_add_param(char *key) {
    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->type = SYMBOL_PARAM;
    symbol->offset = current_table->param_offset++;
    map_add(current_table->symbols, key, symbol);
}

bool symbol_table_check_symbol(char *key) { return symbol_table_get_symbol(key) != NULL; }

SymbolTable *symbol_table_current(void) { return current_table; }

void symbol_table_dump(SymbolTable *table) {
    Vector *data = table->symbols->data;
    for (int i = 0; i < data->count; ++i) {
        KeyValue *kv = vector_get(data, i);
        printf("Key %s\n", kv->key);
    }
}
