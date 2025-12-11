#ifndef TEST_H
#define TEST_H

#include "../utils/utils.h"

#ifndef LIBRARY_C_JSON_PARSER_EXPORT
#if defined(_MSC_VER) && defined(JSON_C_DLL)
#define LIBRARY_C_JSON_PARSER_EXPORT __declspec(dllexport)
#else
#define LIBRARY_C_JSON_PARSER_EXPORT extern
#endif
#endif

#ifdef LONG_TEST
#define TEST_COUNT 1000000UL
#else
#define TEST_COUNT 100000UL
#endif


#define TEST_SETUP()                              \
  LIBRARY_C_JSON_PARSER_EXPORT int tests_run;     \
  LIBRARY_C_JSON_PARSER_EXPORT int tests_passed;  \
  LIBRARY_C_JSON_PARSER_EXPORT const char *GREEN; \
  LIBRARY_C_JSON_PARSER_EXPORT const char *RED;   \
  LIBRARY_C_JSON_PARSER_EXPORT const char *RESET; \
  LIBRARY_C_JSON_PARSER_EXPORT void utils_initialize(void)

#define TEST_DEFINITION(name) \
  LIBRARY_C_JSON_PARSER_EXPORT void(name)(void)

#define TEST_INITIALIZE \
  do {                  \
    utils_initialize(); \
    tests_run = 0;      \
    tests_passed = 0;   \
  } while (0)

#define TEST_SUITE(name)                                                                         \
  do {                                                                                           \
    printf("===============================================================================\n"); \
    printf("running %s\n", name);                                                                \
    printf("===============================================================================\n"); \
  } while (0)

#define TEST_FINALIZE                                                                            \
  do {                                                                                           \
    printf("===============================================================================\n"); \
    printf("tests run: %d\n", tests_run);                                                        \
    printf("tests passed: %d\n", tests_passed);                                                  \
    printf("===============================================================================\n"); \
    if (tests_run == tests_passed) {                                                             \
      printf("all tests %sPASSED%s\n", GREEN, RESET);                                            \
      return 0;                                                                                  \
    } else {                                                                                     \
      printf("some tests %sFAILED%s\n", RED, RESET);                                             \
      return 1;                                                                                  \
    }                                                                                            \
  } while (0)

#define TEST(name, ...)                          \
  void name() {                                  \
    __VA_ARGS__;                                 \
    do {                                         \
      tests_run++;                               \
      int passed = 1;                            \
      const char *test_name = #name;             \
      printf("running test: %65s\n", test_name); \
      do

#define END_TEST                          \
  }                                       \
  while (0)                               \
    ;                                     \
  if (passed) {                           \
    tests_passed++;                       \
    printf("status: %65s", "");           \
    printf("%sPASSED%s\n", GREEN, RESET); \
  } else {                                \
    printf("status: %65s", "");           \
    printf("%sFAILED%s\n", RED, RESET);   \
  }                                       \
  }                                       \
  while (0)

#define ASSERT_TRUE(actual)                                                           \
  do {                                                                                \
    if (!((actual) == true)) {                                                        \
      printf("assertion failed at %s:%d: %s == true\n", __FILE__, __LINE__, #actual); \
      passed = 0;                                                                     \
    }                                                                                 \
  } while (0)

#define ASSERT(condition)                                                        \
  do {                                                                           \
    if (!(condition)) {                                                          \
      printf("assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
      passed = 0;                                                                \
    }                                                                            \
  } while (0)

#define ASSERT_EQ(expected, actual)                                                                      \
  do {                                                                                                   \
    if ((expected) != (actual)) {                                                                        \
      printf("assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), \
             (long)(actual));                                                                            \
      passed = 0;                                                                                        \
    }                                                                                                    \
  } while (0)

#define ASSERT_NOT_EQ(expected, actual)                                                                  \
  do {                                                                                                   \
    if ((expected) == (actual)) {                                                                        \
      printf("assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), \
             (long)(actual));                                                                            \
      passed = 0;                                                                                        \
    }                                                                                                    \
  } while (0)

#define ASSERT_PTR_EQ(expected, actual)                                                                     \
  do {                                                                                                      \
    if ((expected) != (actual)) {                                                                           \
      printf("assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
      passed = 0;                                                                                           \
    }                                                                                                       \
  } while (0)

#define ASSERT_PTR_NOT_EQ(expected, actual)                                                                 \
  do {                                                                                                      \
    if ((expected) == (actual)) {                                                                           \
      printf("assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
      passed = 0;                                                                                           \
    }                                                                                                       \
  } while (0)

#define ASSERT_PTR_NULL(actual)                                                                   \
  do {                                                                                            \
    if (NULL != (actual)) {                                                                       \
      printf("assertion failed at %s:%d: Expected NULL, got %p\n", __FILE__, __LINE__, (actual)); \
      passed = 0;                                                                                 \
    }                                                                                             \
  } while (0)

#define ASSERT_PTR_NOT_NULL(actual)                                                           \
  do {                                                                                        \
    if (NULL == (actual)) {                                                                   \
      printf("assertion failed at %s:%d: Expected non-NULL, got NULL\n", __FILE__, __LINE__); \
      passed = 0;                                                                             \
    }                                                                                         \
  } while (0)

TEST_SETUP();

#endif
