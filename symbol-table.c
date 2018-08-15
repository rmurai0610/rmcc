#include "self-c.h"

static SymbolTable *root_table;

void init_symbol_tables(void) {
    root_table = malloc(sizeof(SymbolTable));
    root_table->offset_map = map_init();
    root_table->offset = 1;
}

bool symbol_table_check_symbol(char *key) { return map_get(root_table->offset_map, key) != NULL; }

int symbol_table_get_offset(char *key) {
    int *offset = map_get(root_table->offset_map, key);
    return *offset;
}

void symbol_table_add_offset(char *key) {
    int *value = malloc(sizeof(int));
    *value = root_table->offset++;
    map_add(root_table->offset_map, key, value);
}
