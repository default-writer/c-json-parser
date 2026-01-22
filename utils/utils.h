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
#include "../src/headers.h"

#ifdef __cplusplus
extern "C" {
#endif

void utils_initialize();
char *utils_get_test_json_data(const char *filename);
bool utils_test_json_equal(const char *a, const char *b);
void utils_output(const char *s);
void utils_itoa(int n, char s[]);
long long utils_get_time(void);
void utils_print_time_diff(long long start_ns, long long end_ns);

#ifdef __cplusplus
}
#endif

#endif
