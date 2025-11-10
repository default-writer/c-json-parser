#ifndef JSON
#define JSON

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
#define fprintf(stream, format, ...)                                           \
  do {                                                                         \
    fprintf_s((stream), (format), __VA_ARGS__);                                \
  } while (0)
#define strncpy_s(dest, dest_size, src, count)                                 \
  strncpy_s(dest, dest_size, src, count)
#else
#define strncpy_s(dest, dest_size, src, count)                                 \
  strncpy(dest, src, count);                                                   \
  (dest)[(dest_size) - 1] = '\0';
#endif

/* Simple JSON structures (a dictionary + value types) */

/* Forward declarations */
typedef struct json_value json_value;

typedef enum {
  J_NULL,
  J_BOOLEAN,
  J_NUMBER,
  J_STRING,
  J_ARRAY,
  J_OBJECT
} json_type;

typedef struct {
  const char *ptr; /* start of this value in source JSON */
  size_t len;
} reference;

typedef struct json_object *json_object_ptr;
typedef struct json_value *json_value_ptr;

typedef struct json_value {
  json_type type;
  union {
    reference string;
    reference boolean;
    reference number;
    struct {
      json_value_ptr items;
      size_t count;
      size_t cap;
    } array;
    struct {
      json_object_ptr items;
      size_t count;
      size_t cap;
    } object;
  } u;
} *json_value_ptr;

typedef struct json_object {
  const char *ptr;
  size_t len;
  json_value_ptr value;
} json_object;


#endif 