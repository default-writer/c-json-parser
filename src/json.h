#ifndef JSON_H
#define JSON_H

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 0xffff

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
#define fprintf(stream, format, ...)                                           \
  do {                                                                         \
    fprintf_s((stream), (format), __VA_ARGS__);                                \
  } while (0)
#endif

/**
 * @brief Enumeration of JSON value types.
 */
typedef enum {
  J_NULL,    /**< null value */
  J_BOOLEAN, /**< boolean value (true or false) */
  J_NUMBER,  /**< number value (integer or floating-point) */
  J_STRING,  /**< string value */
  J_ARRAY,   /**< array value */
  J_OBJECT   /**< object value */
} json_type;

/**
 * @brief Represents a reference to a part of the original JSON string.
 * This is used to avoid allocating new memory for strings, numbers, and booleans.
 */
typedef struct {
  const char *ptr; /**< Pointer to the start of the value in the source JSON string. */
  size_t len;      /**< Length of the value. */
} reference;

/* Forward declarations */
typedef struct json_value json_value;
typedef struct json_object json_object;

/**
 * @brief Represents a JSON value.
 * The type of the value is determined by the `type` field.
 */
typedef struct json_value {
  json_type type; /**< The type of the JSON value. */
  union {
    reference string;  /**< Used when type is J_STRING. */
    reference boolean; /**< Used when type is J_BOOLEAN. */
    reference number;  /**< Used when type is J_NUMBER. */
    struct {
      json_value *items; /**< Array of JSON values. */
      size_t count;      /**< Number of items in the array. */
      size_t capacity;   /**< Allocated capacity of the array. */
    } array;             /**< Used when type is J_ARRAY. */
    struct {
      json_object *items; /**< Array of key-value pairs. */
      size_t count;       /**< Number of items in the object. */
      size_t capacity;    /**< Allocated capacity of the object. */
    } object;             /**< Used when type is J_OBJECT. */
  } u;
} json_value;

/**
 * @brief Represents a key-value pair in a JSON object.
 */
typedef struct json_object {
  const char *ptr;    /**< Pointer to the key in the source JSON string. */
  size_t len;         /**< Length of the key. */
  json_value *value; /**< Pointer to the JSON value. */
} json_object;

/**
 * @brief Returns a JSON string of json_value objects.
 * @param json The JSON value oject.
 * @return A pointer to the json_value origin, or NULL on error.
 */
const char* json_source(const json_value *v);

/**
 * @brief Parses a JSON string and returns a tree of json_value objects.
 * The caller is responsible for freeing the returned structure by calling json_free().
 * @param json The JSON string to parse.
 * @return A pointer to the root json_value, or NULL on error.
 */
const json_value *json_parse(const char *json);

/**
 * @brief Compares two JSON strings for structural equality.
 * @param a The first JSON string.
 * @param b The second JSON string.
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
void json_free(const json_value *v);

/**
 * @brief Prints a json_value tree to a standard output.
 * @param v The json_value to print.
 * @param out The standatd output FILE handle.
 */
void json_print(const json_value *v, FILE *out);

#endif /* JSON_H */