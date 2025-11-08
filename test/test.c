#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define strncpy_s(dest, dest_size, src, count) strncpy_s(dest, dest_size, src, count)
#else
#define strncpy_s(dest, dest_size, src, count) strncpy(dest, src, count); (dest)[(dest_size)-1] = '\0';
#endif

int tests_run = 0;
int tests_passed = 0;

const char* GREEN = "";
const char* RED = "";
const char* RESET = "";

void setup_console() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (SetConsoleMode(hOut, dwMode)) {
                GREEN = "\033[0;32m";
                RED = "\033[0;31m";
                RESET = "\033[0m";
            }
        }
    }
#else
    GREEN = "\033[0;32m";
    RED = "\033[0;31m";
    RESET = "\033[0m";
#endif
}

#define TEST(name) \
    do { \
        tests_run++; \
        printf("running test: %s...", #name); \
        int passed = 1; \
        do

#define END_TEST \
        while (0); \
        if (passed) { \
            tests_passed++; \
            printf("%s  PASSED%s\n", GREEN, RESET); \
        } else { \
            printf("%s  FAILED%s\n", RED, RESET); \
        } \
    } while (0)

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("  assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), (long)(actual)); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_PTR_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_PTR_NOT_EQ(expected, actual) \
do { \
    if ((expected) == (actual)) { \
        printf("  assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
        passed = 0; \
    } \
} while (0)

#define ASSERT_PTR_NULL(actual) \
    do { \
        if (NULL != (actual)) { \
            printf("  assertion failed at %s:%d: Expected NULL, got %p\n", __FILE__, __LINE__, (actual)); \
            passed = 0; \
        } \
    } while (0)

#define ASSERT_PTR_NOT_NULL(actual) \
    do { \
        if (NULL == (actual)) { \
            printf("  assertion failed at %s:%d: Expected non-NULL, got NULL\n", __FILE__, __LINE__); \
            passed = 0; \
        } \
    } while (0)

/* forward-declare opaque dict and public API implemented in func.c */
typedef struct dict dict;
dict *func_parse_to_dict(const char *json);
char *func_parse_to_string(const dict *d);
void func_free_dict(dict *d);
bool func_json_equal(const char *a, const char *b);

void test_func_parse_to_dict() {
    TEST(test_func_parse_to_dict) {
         
        const char* filename = "test/test.json";
        FILE* fp = fopen(filename, "r");
        ASSERT_PTR_NOT_NULL(fp);

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* json = calloc(1, size + 1);
        ASSERT_PTR_NOT_NULL(json);

        fread(json, 1, size, fp);
        json[size] = '\0';
        fclose(fp);

        /* parse into internal dict */
        dict *d = func_parse_to_dict(json);
        ASSERT_PTR_NOT_NULL(d);

        /* render dict back to string */
        /* request preserving input order */
        char *out = func_parse_to_string(d);
        ASSERT_PTR_NOT_NULL(out);

        /* compare structurally (order-insensitive) */
        ASSERT(func_json_equal(json, out));

        /* cleanup */
        free(out);
        func_free_dict(d);
        free(json);
    } END_TEST;
}
