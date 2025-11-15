#ifndef TEST_H
#define TEST_H

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_SETUP()                  \
  extern void test_initialize(void);  \
  extern int tests_run;               \
  extern int tests_passed;            \
  extern const char *GREEN;           \
  extern const char *RED;             \
  extern const char *RESET;           \
  extern void test_initialize(void);  \
  extern long long get_time_ns(void); \
  extern void print_time_diff(const char *test, long long start_ns, long long end_ns);

#define TEST_DEFINITION(name) \
  extern void(name)(void);

#define TEST_INITIALIZE() \
  do {                    \
    test_initialize();    \
    tests_run = 0;        \
    tests_passed = 0;     \
  } while (0)

#define TEST_SUITE(name)                                                                         \
  do {                                                                                           \
    printf("===============================================================================\n"); \
    printf("running %s\n", name);                                                                \
    printf("===============================================================================\n"); \
  } while (0)

#define TEST_FINALIZE()                                                                            \
  do {                                                                                             \
    if (tests_run == tests_passed) {                                                               \
      printf("===============================================================================\n"); \
      printf("tests run: %d\n", tests_run);                                                        \
      printf("tests passed: %d\n", tests_passed);                                                  \
      printf("===============================================================================\n"); \
      printf("all tests %sPASSED%s\n", GREEN, RESET);                                              \
      return 0;                                                                                    \
    } else {                                                                                       \
      printf("===============================================================================\n"); \
      printf("tests run: %d\n", tests_run);                                                        \
      printf("tests passed: %d\n", tests_passed);                                                  \
      printf("===============================================================================\n"); \
      printf("some tests %sFAILED%s\n", RED, RESET);                                               \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)

#define TEST(name)             \
  void name() {                \
    do {                       \
      tests_run++;             \
      int passed = 1;          \
      char *test_name = #name; \
      do

#define END_TEST                              \
  }                                           \
  while (0)                                   \
    ;                                         \
  if (passed) {                               \
    tests_passed++;                           \
    printf("running test: %55s", test_name);  \
    printf(" ...%sPASSED%s\n", GREEN, RESET); \
  } else {                                    \
    printf("running test: %55s", test_name);  \
    printf(" ...%sFAILED%s\n", RED, RESET);   \
  }                                           \
  }                                           \
  while (0)

#define ASSERT(condition)                                                          \
  do {                                                                             \
    if (!(condition)) {                                                            \
      printf("  assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
      passed = 0;                                                                  \
    }                                                                              \
  } while (0)

#define ASSERT_EQ(expected, actual)                                                                        \
  do {                                                                                                     \
    if ((expected) != (actual)) {                                                                          \
      printf("  assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), \
             (long)(actual));                                                                              \
      passed = 0;                                                                                          \
    }                                                                                                      \
  } while (0)

#define ASSERT_NOT_EQ(expected, actual)                                                                    \
  do {                                                                                                     \
    if ((expected) == (actual)) {                                                                          \
      printf("  assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), \
             (long)(actual));                                                                              \
      passed = 0;                                                                                          \
    }                                                                                                      \
  } while (0)

#define ASSERT_PTR_EQ(expected, actual)                                                                       \
  do {                                                                                                        \
    if ((expected) != (actual)) {                                                                             \
      printf("  assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
      passed = 0;                                                                                             \
    }                                                                                                         \
  } while (0)

#define ASSERT_PTR_NOT_EQ(expected, actual)                                                                   \
  do {                                                                                                        \
    if ((expected) == (actual)) {                                                                             \
      printf("  assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
      passed = 0;                                                                                             \
    }                                                                                                         \
  } while (0)

#define ASSERT_PTR_NULL(actual)                                                                     \
  do {                                                                                              \
    if (NULL != (actual)) {                                                                         \
      printf("  assertion failed at %s:%d: Expected NULL, got %p\n", __FILE__, __LINE__, (actual)); \
      passed = 0;                                                                                   \
    }                                                                                               \
  } while (0)

#define ASSERT_PTR_NOT_NULL(actual)                                                             \
  do {                                                                                          \
    if (NULL == (actual)) {                                                                     \
      printf("  assertion failed at %s:%d: Expected non-NULL, got NULL\n", __FILE__, __LINE__); \
      passed = 0;                                                                               \
    }                                                                                           \
  } while (0)

#endif
