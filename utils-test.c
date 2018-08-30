#include "rmcc.h"
#define ASSERT_EQUAL_INT(actual, expected)                         \
    if (!assert_equal_int(actual, expected, __func__, __LINE__)) { \
        FAILED_TEST();                                             \
        return;                                                    \
    }

#define ASSERT_EQUAL_PTR(actual, expected)                         \
    if (!assert_equal_ptr(actual, expected, __func__, __LINE__)) { \
        FAILED_TEST();                                             \
        return;                                                    \
    }
#define PASSED_TEST() (printf("%s: PASS\n", __func__))
#define FAILED_TEST() (printf("%s: FAIL\n", __func__))

static bool assert_equal_int(int actual, int expected, const char *func, int line) {
    if (actual != expected) {
        printf("%s Line %d: Expected %d but received %d\n", func, line, expected, actual);
        return false;
    }
    return true;
}

static bool assert_equal_ptr(void *actual, void *expected, const char *func, int line) {
    if (actual != expected) {
        printf("%s Line %d: Expected %p but received %p\n", func, line, expected, actual);
        return false;
    }
    return true;
}

static void int_vector_print(Vector *vec) {
    for (int i = 0; i < vec->count; ++i) {
        printf("%d ", *(int *)vec->data[i]);
    }
    printf("\n");
}

static void test_vector() {
    Vector *vec = vector_init();
    ASSERT_EQUAL_INT(vec->size, 1);
    ASSERT_EQUAL_INT(vec->count, 0);
    int *i = malloc(sizeof(int));
    *i = 1;
    vector_add(vec, i);
    ASSERT_EQUAL_INT(vec->size, 1);
    ASSERT_EQUAL_INT(vec->count, 1);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 0), *i);
    int *j = malloc(sizeof(int));
    *j = 2;
    vector_add(vec, j);
    ASSERT_EQUAL_INT(vec->size, 2);
    ASSERT_EQUAL_INT(vec->count, 2);
    vector_add(vec, j);
    ASSERT_EQUAL_INT(vec->size, 4);
    ASSERT_EQUAL_INT(vec->count, 3);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 2), 2);

    vector_delete(vec, 2);
    ASSERT_EQUAL_INT(vec->size, 4);
    ASSERT_EQUAL_INT(vec->count, 2);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 0), 1);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 1), 2);

    int *k = malloc(sizeof(int));
    *k = 3;
    vector_add(vec, k);
    ASSERT_EQUAL_INT(vec->size, 4);
    ASSERT_EQUAL_INT(vec->count, 3);
    vector_delete(vec, 1);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 0), 1);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 1), 3);
    vector_delete(vec, 0);
    ASSERT_EQUAL_INT(*(int *)vector_get(vec, 0), 3);

    PASSED_TEST();
}

static void test_map() {
    Map *map = map_init();
    int *i = malloc(sizeof(int));
    *i = 1;
    map_add(map, "a", i);
    ASSERT_EQUAL_INT(*(int *)map_get(map, "a"), 1);
    int *j = malloc(sizeof(int));
    *j = 2;
    map_add(map, "b", j);
    ASSERT_EQUAL_INT(*(int *)map_get(map, "a"), 1);
    ASSERT_EQUAL_INT(*(int *)map_get(map, "b"), 2);
    map_delete(map, "a");
    ASSERT_EQUAL_PTR(map_get(map, "a"), NULL);
    PASSED_TEST();
}

int main(int argc, char const *argv[]) {
    test_vector();
    test_map();
    return 0;
}
