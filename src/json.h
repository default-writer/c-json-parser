/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   February 1, 2026 at 5:07:11 PM UTC
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

/* Memory pool and buffer size constants */
#define DICTIONARY_SIZE 16          /* Size of lookup dictionary for parsing optimization */
#define MAX_BUFFER_SIZE 0x100       /* Maximum buffer size for temporary string operations (256 bytes) */
#define JSON_VALUE_POOL_SIZE 0xFFFF /* Maximum number of json_value objects that can be allocated (65535) */
#define JSON_STACK_SIZE 0xFFFF      /* Maximum stack depth for recursive parsing (65535 levels) */
#define LOOKUP_TABLE_SIZE 256       /* Size of character lookup tables for whitespace/parsing (256 for all byte values) */

#include "headers.h"

#if defined(__has_attribute)
#if __has_attribute(always_inline)
#define INLINE __inline__
#define INLINE_ATTRIBUTE __attribute__((always_inline))
#else
#define INLINE __inline__
#define INLINE_ATTRIBUTE
#endif
#else
#define INLINE inline
#define INLINE_ATTRIBUTE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of JSON parsing error codes.
 *
 * Error codes indicate specific parsing failures during JSON processing.
 * Values 0-15 are primary error types, while values 16-31 are
 * extended error codes with E_NULL bit set for null pointer scenarios.
 */
typedef enum : uint8_t {
  E_NO_ERROR = 0,                                         /* No error occurred */
  E_NO_DATA = 1,                                          /* No data provided or empty input */
  E_INVALID_JSON = 2,                                     /* Invalid JSON structure */
  E_INVALID_JSON_DATA = 3,                                /* Invalid data within JSON structure */
  E_STACK_OVERFLOW_OBJECT = 4,                            /* Stack overflow while parsing object */
  E_STACK_OVERFLOW_ARRAY = 5,                             /* Stack overflow while parsing array */
  E_OBJECT_KEY = 6,                                       /* Invalid object key format */
  E_OBJECT_VALUE = 7,                                     /* Invalid object value */
  E_EXPECTED_OBJECT = 8,                                  /* Expected object but found different type */
  E_EXPECTED_ARRAY = 9,                                   /* Expected array but found different type */
  E_EXPECTED_STRING = 10,                                 /* Expected string but found different type */
  E_EXPECTED_BOOLEAN = 11,                                /* Expected boolean but found different type */
  E_EXPECTED_NULL = 12,                                   /* Expected null but found different type */
  E_INVALID_DATA = 13,                                    /* Invalid data format */
  E_MAILFORMED_JSON = 14,                                 /* Malformed JSON structure (typo: should be E_MALFORMED_JSON) */
  E_UNKNOWN_ERROR = 15,                                   /* Unknown or unexpected error */
  E_NULL = 0x10,                                          /* Null pointer encountered flag */
  E_EXPECTED_OBJECT_KEY = E_OBJECT_KEY | E_NULL,          /* Object key with null pointer */
  E_EXPECTED_OBJECT_VALUE = E_OBJECT_VALUE | E_NULL,      /* Object value with null pointer */
  E_EXPECTED_ARRAY_ELEMENT = E_EXPECTED_ARRAY | E_NULL,   /* Array element with null pointer */
  E_EXPECTED_OBJECT_ELEMENT = E_EXPECTED_OBJECT | E_NULL, /* Object element with null pointer */
} json_error;

/**
 * @brief Enumeration of JSON value types.
 *
 * Each type corresponds to a valid JSON data type.
 * The type field in json_value determines which union member is active.
 */
typedef enum {
  J_NULL = 1,    /* JSON null value */
  J_BOOLEAN = 2, /* JSON boolean (true/false) */
  J_NUMBER = 3,  /* JSON numeric value (integer or float) */
  J_STRING = 4,  /* JSON string value */
  J_ARRAY = 5,   /* JSON array (ordered list) */
  J_OBJECT = 6   /* JSON object (key-value pairs) */
} json_token;

/**
 * @brief Represents a reference to a substring within the original JSON input.
 *
 * This structure enables zero-copy parsing by storing pointers to the original
 * input string instead of allocating new memory for strings, numbers, and booleans.
 * The referenced memory must remain valid for the lifetime of the json_value.
 */
typedef struct reference {
  const char *ptr; /* Pointer to the start of the substring in original input */
  size_t len;      /* Length of the referenced substring in bytes */
} reference;

/* Forward declarations */
typedef struct json_value json_value_type;
typedef struct json_object json_object_type;
typedef struct json_array_node json_array_node_type;
typedef struct json_object_node json_object_node_type;

/**
 * @brief Represents a generic JSON value that can hold any JSON data type.
 *
 * This structure uses a tagged union pattern where the `type` field determines
 * which member of the `u` union is valid and should be accessed.
 * For simple types (string, boolean, number), references to the original
 * input are stored to avoid memory allocation.
 */
typedef struct json_value {
  json_token type; /* Type discriminator determining active union member */
  union {
    reference string;  /* String value (valid when type == J_STRING) */
    reference boolean; /* Boolean value (valid when type == J_BOOLEAN) */
    reference number;  /* Number value (valid when type == J_NUMBER) */
    struct {
      json_array_node_type *last;  /* Pointer to last array element (for O(1) append) */
      json_array_node_type *items; /* Pointer to first array element (head of linked list) */
    } array;                       /* Array value (valid when type == J_ARRAY) */
    struct {
      json_object_node_type *last;  /* Pointer to last object element (for O(1) append) */
      json_object_node_type *items; /* Pointer to first object element (head of linked list) */
    } object;                       /* Object value (valid when type == J_OBJECT) */
  } u;                              /* Union holding value data based on type */
} json_value;

/**
 * @brief Represents a key-value pair in a JSON object.
 *
 * This structure stores object entries with the key as a reference to the original
 * input string and the value as a nested json_value structure.
 */
typedef struct json_object {
  reference key;         /* Object key as reference to original input string */
  json_value_type value; /* Object value (can be any JSON type) */
} json_object;

/**
 * @brief Node in a singly-linked list of JSON object key-value pairs.
 *
 * JSON objects are implemented as linked lists of these nodes for
 * memory efficiency and easy insertion/deletion.
 */
typedef struct json_object_node {
  json_object_type item;       /* Key-value pair stored in this node */
  json_object_node_type *next; /* Pointer to next node in the list (NULL for last) */
} json_object_node;

/**
 * @brief Node in a singly-linked list of JSON array elements.
 *
 * JSON arrays are implemented as linked lists of these nodes for
 * memory efficiency and easy insertion/deletion.
 */
typedef struct json_array_node {
  json_value_type item;       /* JSON value stored in this array element */
  json_array_node_type *next; /* Pointer to next node in the list (NULL for last) */
} json_array_node;

/**
 * @brief Parses a JSON string and creates a tree of `json_value` objects.
 *
 * This function parses JSON using a recursive descent approach and constructs
 * an in-memory representation using internal memory pools.
 * The parsed JSON tree structure is stored in the provided root parameter.
 *
 * @param s The JSON string to parse (must be null-terminated)
 * @param len The length of JSON string in bytes
 * @param root A pointer to root `json_value` where parsed JSON will be stored
 * @return `true` if JSON was successfully parsed, `false` otherwise
 */
bool json_parse(const char *s, size_t len, json_value *root);

/**
 * @brief Parses a JSON string iteratively and creates a tree of `json_value` objects.
 *
 * This function uses an iterative parsing approach with explicit stack management
 * instead of recursion, making it more memory-efficient and stack-safe for deeply
 * nested JSON structures. Suitable for performance-critical applications.
 *
 * @param s The JSON string to parse (must be null-terminated)
 * @param len The length of JSON string in bytes
 * @param root A pointer to root `json_value` where parsed JSON will be stored
 * @return `true` if JSON was successfully parsed, `false` otherwise
 */
bool json_parse_iterative(const char *s, size_t len, json_value *root);

/**
 * @brief Validates a JSON string without allocating memory for parsed tree.
 *
 * This function performs syntax validation only, checking if the JSON is well-formed
 * without constructing the full in-memory representation. It's optimized for
 * scenarios where only validation is needed, not parsing.
 *
 * @param s A pointer to const char* pointer representing JSON string to validate.
 *            The pointer may be advanced during validation (pass by pointer to pointer).
 * @param len The length of JSON string in bytes
 * @return E_NO_ERROR if string is valid JSON, non-zero error code otherwise.
 *         The error code indicates the type of validation failure.
 */
json_error json_validate(const char **s, size_t len);

/**
 * @brief Compares two JSON values for structural and value equality.
 *
 * This function performs deep comparison of JSON structures, checking that
 * both the structure and all contained values are equivalent. For arrays,
 * element order matters. For objects, key-value pairs are compared regardless
 * of order (object property order is not significant in JSON).
 *
 * @param a The first JSON value to compare (can be NULL)
 * @param b The second JSON value to compare (can be NULL)
 * @return true if JSON structures are equivalent, false otherwise
 */
bool json_equal(const json_value *a, const json_value *b);

/**
 * @brief Converts a `json_value` tree to a pretty-printed JSON string.
 *
 * This function serializes a parsed JSON tree back to a string format with
 * proper indentation and formatting for human readability. The resulting
 * string is allocated with malloc() and must be freed by the caller.
 *
 * @param v The json_value tree to serialize (must not be NULL)
 * @return A newly allocated string containing formatted JSON, or NULL on error
 *         (e.g., if v is NULL or memory allocation fails)
 */
char *json_stringify(const json_value *v);

/**
 * @brief Resets internal memory pool indexes for `json_value` allocation.
 *
 * This function resets the internal memory pool allocation pointers to their
 * initial state, effectively marking all previously allocated json_value structures
 * as available for reuse. This is more efficient than individual free operations
 * and is useful for parsing multiple JSON documents in sequence.
 */
void json_reset(void);

/**
 * @brief Clears all internal memory pools by filling them with zeroes.
 *
 * This function completely resets the internal memory management system,
 * clearing all allocated JSON structures. Unlike json_reset(), this actually
 * clears the memory contents, not just the allocation pointers.
 * Use this when you want to ensure all previously parsed data is inaccessible.
 */
void json_cleanup(void);

/**
 * @brief Frees a `json_value` and all its children recursively.
 *
 * @deprecated This function is kept for backward compatibility only.
 *             Use json_reset() or json_cleanup() for better performance
 *             with the memory pool system. Individual freeing of nodes
 *             is inefficient with the pool-based allocator.
 *
 * @param v The json_value to free (can be NULL)
 */
void json_free(json_value *v);

/**
 * @brief Prints a `json_value` tree to a file stream in compact JSON format.
 *
 * This function serializes a JSON tree to the specified FILE* stream
 * using compact formatting (no extra whitespace or indentation).
 * Useful for debugging, logging, or direct output to files/streams.
 *
 * @param v The json_value tree to print (must not be NULL)
 * @param out The FILE* stream to write output to (must not be NULL)
 *             Typical values: stdout, stderr, or an opened file handle
 */
void json_print(const json_value *v, FILE *out);

/**
 * @brief Returns a human-readable string description for a JSON error code.
 *
 * This function converts json_error enumeration values to descriptive
 * strings that explain the specific error that occurred during JSON parsing.
 * Useful for debugging and error reporting.
 *
 * @param error The json_error code to get description for
 * @return A constant string containing the error description, never NULL
 */
const char *json_error_string(json_error error);

#ifdef __cplusplus
}
#endif

#endif /* JSON_H */