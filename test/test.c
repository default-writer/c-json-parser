#include <stdio.h>
#include <stdlib.h>

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


void test_rc_gc_free_one_block() {
    TEST(test_rc_gc_multiple_blocks) {
         
        const char* filename = "test/test.json";
        FILE* fp = fopen(filename, "r");
        ASSERT_PTR_NOT_NULL(fp);
        // if (!fp) {
        //     perror("fopen");
        //     return 1;
        // }

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* json = malloc(size + 1);
        ASSERT_PTR_NOT_NULL(json);
        // if (!json) {
        //     fprintf(stderr, "malloc failed\n");
        //     fclose(fp);
        //     return 1;
        // }

        // TODO: code to to call function func_parse_to_dict and loads data, then printing it to the another file, and comparing that byte-to byte.

        fread(json, 1, size, fp);
        json[size] = '\0';
        fclose(fp);


        free(json);
    } END_TEST;
}
