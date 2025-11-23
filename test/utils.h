#ifndef UTILS_H
#define UTILS_H

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

#include "../src/json.h"

long long utils_get_time(void);
void utils_print_time_diff(long long start_ns, long long end_ns);
char *utils_get_test_json_data(const char *filename);
bool utils_test_json_equal(const char *a, const char *b);
void utils_output(const char *s);

#define NEXT_TOKEN(s)                      \
  do {                                     \
    if (!json_next_token((s))) \
      return false;                        \
  } while (0)

#endif // UTILS_H
