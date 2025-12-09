/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   December 9, 2025 at 11:48:26 AM GMT+3
 *
 */
/*
    Copyright (C) 2022-2047 default-writer
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MACROS_H
#define MACROS_H

#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef JSON_EXPORT
#if defined(_MSC_VER) && defined(JSON_C_DLL)
#define JSON_EXPORT __declspec(dllexport)
#else
#define JSON_EXPORT extern
#endif
#endif

#define MAX_BUFFER_SIZE 0x100

#ifdef _WIN32
#include <windows.h>
/* Provide a safe wrapper around fopen on Windows to avoid deprecation warnings.
 * The wrapper uses fopen_s internally and returns the FILE* pointer.
 * Existing code using fopen stays unchanged.
 */
static inline FILE *safe_fopen(const char *filename, const char *mode) {
  FILE *fp = NULL;
  errno_t err = fopen_s(&fp, filename, mode);
  if (err != 0) {
    return NULL;
  }
  return fp;
}
/* Redirect calls to fopen to the safe wrapper. */
#define fopen(filename, mode) safe_fopen(filename, mode)
#define fprintf(stream, format, ...)            \
  do {                                          \
    fprintf_s((stream), (format), __VA_ARGS__); \
  } while (0)
#endif

#define DICTIONARY_SIZE 16
#define JSON_VALUE_POOL_SIZE 0xFFFF

#define STATE_INITIAL 1
#define STATE_ESCAPE_START 2
#define STATE_ESCAPE_UNICODE_BYTE1 3
#define STATE_ESCAPE_UNICODE_BYTE2 4
#define STATE_ESCAPE_UNICODE_BYTE3 5
#define STATE_ESCAPE_UNICODE_BYTE4 6
#define TEXT_SIZE(name) sizeof(name) - 1
#define TOKEN(value) value, TEXT_SIZE(value)
#define NEXT_TOKEN(s)                     \
  do {                                    \
    while (**(s) != '\0') {               \
      if (!isspace((unsigned char)**(s))) \
        break;                            \
      (*s)++;                             \
    }                                     \
  } while (0)

#endif /* JSON_H */