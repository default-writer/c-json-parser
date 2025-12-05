/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   December 5, 2025 at 10:46:42 PM GMT+3
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

#ifndef JSON_H
#define JSON_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef JSON_EXPORT
#if defined(_MSC_VER) && defined(JSON_C_DLL)
#define JSON_EXPORT __declspec(dllexport)
#else
#define JSON_EXPORT extern
#endif
#endif

#define MAX_BUFFER_SIZE 0xFFFF

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

/**
 * @brief Enumeration of JSON value types.
 */
typedef enum {
  J_NULL = 1,    
  J_BOOLEAN = 2, 
  J_NUMBER = 3,  
  J_STRING = 4,  
  J_ARRAY = 5,   
  J_OBJECT = 6   
} json_token;

/**
 * @brief Represents a reference to a part of the original JSON string.
 * This is used to avoid allocating new memory for strings, numbers, and booleans.
 */
typedef struct {
  const char *ptr; 
  size_t len;      
} reference;

/* Forward declarations */
typedef struct json_value json_value_decl;
typedef struct json_object json_object_decl;
typedef struct json_array_node json_array_node_decl;
typedef struct json_object_node json_object_node_decl;

/**
 * @brief Represents a JSON value.
 * The type of the value is determined by the `type` field.
 */
typedef struct json_value {
  json_token type; 
  union {
    reference string;  
    reference boolean; 
    reference number;  
    struct {
      json_array_node_decl *last;
      json_array_node_decl *items;
    } array;                  
    struct {
      json_object_node_decl *last;
      json_object_node_decl *items;
    } object;                  
  } u;
} json_value;

/**
 * @brief Represents a key-value pair in a JSON object.
 */
typedef struct json_object {
  reference key;     
  json_value_decl value;
} json_object;

/**
 * @brief Represents a node in a linked list of JSON values.
 */
typedef struct json_object_node {
  json_object_decl item;
  json_object_node_decl *next;
} json_object_node;

typedef struct json_array_node {
  json_value_decl item;
  json_array_node_decl *next;
} json_array_node;

/**
 * @brief Returns a pointer to the original JSON source string.
 * @param v The JSON value object.
 * @return A pointer to the original JSON source string, or NULL on error.
 */
const char *json_source(const json_value *v);

/**
 * @brief Parses a JSON string and returns a tree of json_value objects.
 * The caller is responsible for freeing the returned structure by calling json_free().
 * @param json The JSON string to parse.
 * @return A pointer to the root json_value, or NULL on error.
 */
bool json_parse(const char *json, json_value *root);

/**
 * @brief Compares two JSON strings for structural equality.
 * @param a The first JSON value.
 * @param b The second JSON value.
 * @return true if the JSON structures are equivalent, false otherwise.
 */
bool json_equal(const json_value *a, const json_value *b);

/**
 * @brief Converts a json_value tree to a pretty-printed JSON string.
 * The caller is responsible for freeing the returned string.
 * @param v The json_value to stringify.
 * @return A newly allocated string containing the JSON, or NULL on error.
 */
char *json_stringify(const json_value *v);

/**
 * @brief Frees a json_value and all its children.
 * @param v The json_value to free.
 */
void json_free(json_value *v);

/**
 * @brief Prints a json_value tree to a standard output.
 * @param v The json_value to print.
 * @param out The standard output FILE handle.
 */
void json_print(const json_value *v, FILE *out);

/**
 * @brief Returns next token from input string,
 * @param s Null-terminated input string.
 * @return true if next token can be read from input string, of false otherwise.
 */
bool json_next_token(const char **s);

#ifdef __cplusplus
}
#endif

#endif /* JSON_H */