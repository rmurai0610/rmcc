#include "rmcc.h"

static int map_lookup_index(Map *map, char *key) {
    for (int i = 0; i < map->data->count; ++i) {
        KeyValue *kv = vector_get(map->data, i);
        if (!strcmp(kv->key, key)) {
            return i;
        }
    }
    return -1;
}

Map *map_init(void) {
    Map *map = malloc(sizeof(Map));
    map->data = vector_init();
    return map;
}

Map *map_add(Map *map, char *key, void *value) {
    KeyValue *kv = malloc(sizeof(KeyValue));
    kv->key = key;
    kv->value = value;
    vector_add(map->data, kv);
    return map;
}

Map *map_delete(Map *map, char *key) {
    int index = map_lookup_index(map, key);
    if (index == -1) {
        return map;
    }
    vector_delete(map->data, index);
    return map;
}

void *map_get(Map *map, char *key) {
    int index = map_lookup_index(map, key);
    if (index == -1) {
        return NULL;
    }
    KeyValue *kv = vector_get(map->data, index);
    return kv->value;
}
