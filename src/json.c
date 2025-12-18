/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   December 18, 2025 at 7:18:17 PM GMT+3
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

#include "json.h"

#define JSON_NULL_LEN 4
#define JSON_TRUE_LEN 4
#define JSON_FALSE_LEN 5
#define HEX_OFFSET 10
#define HIGH_SURROGATE_START 0xD800
#define HIGH_SURROGATE_END 0xDBFF
#define LOW_SURROGATE_START 0xDC00
#define LOW_SURROGATE_END 0xDFFF
#define SPACE_CHAR 0x20
#define MASK_CHAR 0x80

typedef struct {
  char *buf;
  int cap;
  int pos;
} buffer;

#ifndef USE_ALLOC
static json_array_node json_array_node_pool[JSON_VALUE_POOL_SIZE];
static size_t json_array_node_free_count = JSON_VALUE_POOL_SIZE;

static json_object_node json_object_node_pool[JSON_VALUE_POOL_SIZE];
static size_t json_object_node_free_count = JSON_VALUE_POOL_SIZE;
#endif

static json_value *json_object_get(const json_value *obj, const char *key, size_t len);

static void skip_whitespace(const char **s);
static bool parse_number(const char **s, json_value *v);
static bool parse_string(const char **s, json_value *v);
static bool parse_array_value(const char **s, json_value *v);
static bool parse_object_value(const char **s, json_value *v);
static bool parse_value_build(const char **s, json_value *v);

static void print_string_escaped(FILE *out, const char *s, size_t len);
static void print_indent(FILE *out, int indent);
static void print_array_compact(const json_value *v, FILE *out);
static void print_object_compact(const json_value *v, FILE *out);
static void print_value_compact(const json_value *v, FILE *out);
static void print_value(const json_value *v, int indent, FILE *out);

static int buffer_write_indent(buffer *b, int indent);
static int buffer_write_string(buffer *b, const char *s, size_t len);
static int buffer_write_array(buffer *b, const json_value *v);
static int buffer_write_object_indent(buffer *b, const json_value *v, int indent);
static int buffer_write_object(buffer *b, const json_value *v);
static int buffer_write_value_indent(buffer *b, const json_value *v, int indent);
static int buffer_write_value(buffer *b, const json_value *v);

static int buffer_write(buffer *b, const char *data, int len);
static int buffer_putc(buffer *b, char c);

static bool json_array_equal(const json_value *a, const json_value *b);
static bool json_object_equal(const json_value *a, const json_value *b);

static json_array_node *new_array_node();
static json_object_node *new_object_node();
static bool free_array_node(json_array_node *array_node);
static bool free_object_node(json_object_node *object_node);

static json_value *json_object_get(const json_value *obj, const char *key, size_t len) {
  if (!obj || obj->type != J_OBJECT || !key)
    return NULL;
  json_object_node *object_items = obj->u.object.items;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (object_items->item.key.ptr && key && object_items->item.key.len == len && strncmp(object_items->item.key.ptr, key, len) == 0)
      return &object_items->item.value;
    object_items = next;
  }
  return NULL;
}

static INLINE json_array_node *INLINE_ATTRIBUTE new_array_node() {
#ifdef USE_ALLOC
  return (json_array_node *)calloc(1, sizeof(json_array_node));
#else
  if (json_array_node_free_count == 0)
    return NULL;
  json_array_node *node = &json_array_node_pool[JSON_VALUE_POOL_SIZE - json_array_node_free_count];
  json_array_node_free_count--;
  return node;
#endif
}

static INLINE bool INLINE_ATTRIBUTE free_array_node(json_array_node *array_node) {
#ifdef USE_ALLOC
  free(array_node);
  return true;
#else
  if (json_array_node_free_count < JSON_VALUE_POOL_SIZE) {
    memset(array_node, 0, sizeof(json_array_node));
    json_array_node_free_count++;
    return true;
  }
  return false;
#endif
}

static INLINE json_object_node *INLINE_ATTRIBUTE new_object_node() {
#ifdef USE_ALLOC
  return (json_object_node *)calloc(1, sizeof(json_object_node));
#else
  if (json_object_node_free_count == 0)
    return NULL;
  json_object_node *node = &json_object_node_pool[JSON_VALUE_POOL_SIZE - json_object_node_free_count];
  json_object_node_free_count--;
  return node;
#endif
}

static INLINE bool INLINE_ATTRIBUTE free_object_node(json_object_node *object_node) {
#ifdef USE_ALLOC
  free(object_node);
  return true;
#else
  if (json_object_node_free_count < JSON_VALUE_POOL_SIZE) {
    memset(object_node, 0, sizeof(json_object_node));
    json_object_node_free_count++;
    return true;
  }
  return false;
#endif
}

static INLINE void INLINE_ATTRIBUTE skip_whitespace(const char **s) {
  while (**s && isspace((unsigned char)**s)) {
    (*s)++;
  }
}

static INLINE bool INLINE_ATTRIBUTE parse_number(const char **s, json_value *v) {
  const char *start_p = *s;
  const char *p = *s;
  if (*p == '-') {
    p++;
  }
  if (*p == '0') {
    p++;
    if (*p >= '0' && *p <= '9') {
      return false;
    }
  } else if (*p >= '1' && *p <= '9') {
    p++;
    while (*p >= '0' && *p <= '9') {
      p++;
    }
  } else {
    return false;
  }
  if (*p == '.') {
    p++;
    if (*p >= '0' && *p <= '9') {
      p++;
      while (*p >= '0' && *p <= '9') {
        p++;
      }
    } else {
      return false;
    }
  }
  if (*p == 'e' || *p == 'E') {
    p++;
    if (*p == '+' || *p == '-') {
      p++;
    }
    if (*p >= '0' && *p <= '9') {
      p++;
      while (*p >= '0' && *p <= '9') {
        p++;
      }
    } else {
      return false;
    }
  }
  v->u.number.ptr = start_p;
  v->u.number.len = p - start_p;
  *s = p;
  return true;
}

#ifdef STRING_VALIDATION
static INLINE bool validate_string_chunk(const char *s, size_t len) {
  size_t i = 0;
#if defined(__SSE2__)
  const __m128i limit = _mm_set1_epi8(0x20);
  const __m128i high_bit = _mm_set1_epi8(0x80);
  const __m128i limit_shifted = _mm_xor_si128(limit, high_bit);
  for (; i + 15 < len; i += 16) {
    __m128i chunk = _mm_loadu_si128((const __m128i *)(s + i));
    __m128i chunk_shifted = _mm_xor_si128(chunk, high_bit);
    __m128i result_mask = _mm_cmplt_epi8(chunk_shifted, limit_shifted);
    if (_mm_movemask_epi8(result_mask) != 0) {
      return false;
    }
  }
#endif
  for (; i < len; i++) {
    if ((unsigned char)s[i] < 0x20) {
      return false;
    }
  }
  return true;
}
#endif

static bool parse_hex4(const char **s, uint16_t *result) {
  *result = 0;
  for (int i = 0; i < 4; ++i) {
    char c = (*s)[i];
    if (c == '\0')
      return false;
    if (!isxdigit((unsigned char)c))
      return false;
    *result <<= 4;
    if (c >= '0' && c <= '9') {
      *result |= c - '0';
    } else if (c >= 'a' && c <= 'f') {
      *result |= c - 'a' + HEX_OFFSET;
    } else {
      *result |= c - 'A' + HEX_OFFSET;
    }
  }
  *s += 4;
  return true;
}

static INLINE bool INLINE_ATTRIBUTE parse_string(const char **s, json_value *v) {
  const char *p = *s + 1;
  v->u.string.ptr = p;
  const char *end = p;
  while (true) {
    size_t span = strcspn(end, "\"\\");
#ifdef STRING_VALIDATION
    if (!validate_string_chunk(end, span)) {
      return false;
    }
#endif
    end += span;
    if (*end == '"') {
      v->u.string.len = end - p;
      *s = end + 1;
      return true;
    } else if (*end == '\\') {
      end++;
      if (*end == '\0')
        return false;
      switch (*end) {
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        end++;
        if (*end == '\0')
          return false;
        break;
      case 'u':
        end++;
        if (*end == '\0')
          return false;
        uint16_t codepoint;
        if (!parse_hex4(&end, &codepoint)) {
          return false;
        }
        if (codepoint >= HIGH_SURROGATE_START && codepoint <= HIGH_SURROGATE_END) { // High surrogate
          if (end[0] != '\\' || end[1] != 'u') {
            return false; // Expected low surrogate
          }
          end += 2;
          uint16_t low_surrogate;
          if (!parse_hex4(&end, &low_surrogate)) {
            return false;
          }
          if (low_surrogate < LOW_SURROGATE_START || low_surrogate > LOW_SURROGATE_END) {
            return false; // Invalid low surrogate
          }
        } else if (codepoint >= LOW_SURROGATE_START && codepoint <= LOW_SURROGATE_END) {
          return false; // Low surrogate without high surrogate
        }
        break;
      default:
        return false;
      }
    } else {
      return false;
    }
  }
}

static bool parse_array_value(const char **s, json_value *v) {
  while (true) {
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    json_array_node *array_node = new_array_node();
    if (array_node == NULL) {
      return false;
    }
    do {
      if (v->u.array.items == NULL) {
        v->u.array.items = array_node;
        break;
      }
      if (v->u.array.last == NULL) {
        v->u.array.last = array_node;
        v->u.array.items->next = array_node;
        break;
      }
      json_array_node *last = v->u.array.last;
      v->u.array.last = array_node;
      last->next = array_node;
    } while (0);
    if (!parse_value_build(s, &array_node->item)) {
      free_array_node(array_node);
      v->u.array.items = NULL;
      return false;
    }
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    if (**s == ',') {
      (*s)++;
      continue;
    }
    if (**s == ']') {
      (*s)++;
      return true;
    }
    return false;
  }
}

static bool parse_object_value(const char **s, json_value *v) {
  while (true) {
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    if (**s != '"') {
      return false;
    }
    json_value key;
    key.type = J_STRING;
    if (!parse_string(s, &key)) {
      return false;
    }
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    if (**s != ':') {
      return false;
    }
    (*s)++;
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    json_object_node *object_node = NULL;
    json_object_node *object_items = v->u.object.items;
    while (object_items) {
      json_object_node *next = object_items->next;
      if (object_items->item.key.ptr && object_items->item.key.len == key.u.string.len && strncmp(object_items->item.key.ptr, key.u.string.ptr, key.u.string.len) == 0) {
        break;
      }
      object_items = next;
    }
    if (object_items == NULL) {
      object_node = new_object_node();
      if (object_node == NULL) {
        return false;
      }
      object_node->item.key.ptr = key.u.string.ptr;
      object_node->item.key.len = key.u.string.len;
      do {
        if (v->u.object.items == NULL) {
          v->u.object.items = object_node;
          break;
        }
        if (v->u.object.last == NULL) {
          v->u.object.last = object_node;
          v->u.object.items->next = object_node;
          break;
        }
        json_object_node *last = v->u.object.last;
        v->u.object.last = object_node;
        last->next = object_node;
      } while (0);
    } else {
      object_node = object_items;
    }
    if (!parse_value_build(s, &object_node->item.value)) {
      free_object_node(object_node);
      v->u.object.items = NULL;
      return false;
    }
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    if (**s == ',') {
      (*s)++;
      continue;
    }
    if (**s == '}') {
      (*s)++;
      return true;
    }
    return false;
  }
}

static bool parse_value_build(const char **s, json_value *v) {
  if (**s == '{') {
    v->type = J_OBJECT;
    v->u.object.items = NULL;
    v->u.object.last = NULL;
    (*s)++;
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    if (**s == '}') {
      (*s)++;
      return true;
    }
    return parse_object_value(s, v);
  }
  if (**s == '[') {
    v->type = J_ARRAY;
    v->u.array.items = NULL;
    v->u.array.last = NULL;
    (*s)++;
    skip_whitespace(s);
    if (**s == '\0')
      return false;
    if (**s == ']') {
      (*s)++;
      return true;
    }
    return parse_array_value(s, v);
  }
  if (**s == '"') {
    v->type = J_STRING;
    return parse_string(s, v);
  }
  if (**s == 'n' && *(*s + 1) == 'u' && *(*s + 2) == 'l' && *(*s + 3) == 'l') {
    v->type = J_NULL;
    v->u.string.ptr = *s;
    v->u.string.len = 4;
    *s += 4;
    return true;
  }
  if (**s == 't' && *(*s + 1) == 'r' && *(*s + 2) == 'u' && *(*s + 3) == 'e') {
    v->type = J_BOOLEAN;
    v->u.boolean.ptr = *s;
    v->u.boolean.len = JSON_TRUE_LEN;
    *s += JSON_TRUE_LEN;
    return true;
  }
  if (**s == 'f' && *(*s + 1) == 'a' && *(*s + 2) == 'l' && *(*s + 3) == 's' && *(*s + 4) == 'e') {
    v->type = J_BOOLEAN;
    v->u.boolean.ptr = *s;
    v->u.boolean.len = JSON_FALSE_LEN;
    *s += JSON_FALSE_LEN;
    return true;
  }
  if (parse_number(s, v)) {
    v->type = J_NUMBER;
    return true;
  }
  return false;
}

/* --- pretty-print helpers --- */

static void print_string_escaped(FILE *out, const char *s, size_t len) {
  fputc('"', out);
  size_t i = 0;
  const unsigned char *p;
  for (p = (const unsigned char *)s; *p && i < len; ++i, ++p) {
    unsigned char c = *p;
    fputc(c, out);
  }
  fputc('"', out);
}

static void print_indent(FILE *out, int indent) {
  int i;
  for (i = 0; i < indent; ++i)
    fputs("    ", out);
}

static void print_array_compact(const json_value *v, FILE *out) {
  if (!v || v->type != J_ARRAY) {
    fputs("[]", out);
    return;
  }
  fputc('[', out);
  json_array_node *array_items = v->u.array.items;
  while (array_items) {
    json_array_node *next = array_items->next;
    print_value_compact(&array_items->item, out);
    if (next)
      fputs(", ", out);
    array_items = next;
  }
  fputc(']', out);
}

static void print_object_compact(const json_value *v, FILE *out) {
  if (!v || v->type != J_OBJECT) {
    fputs("{}", out);
    return;
  }
  fputc('{', out);
  json_object_node *object_items = v->u.object.items;
  while (object_items) {
    json_object_node *next = object_items->next;
    print_string_escaped(out, object_items->item.key.ptr, object_items->item.key.len);
    fputs(": ", out);
    print_value_compact(&object_items->item.value, out);
    if (next)
      fputs(", ", out);
    object_items = next;
  }
  fputc('}', out);
}

static void print_value_compact(const json_value *v, FILE *out) {
  if (!v) {
    fputs("null", out);
    return;
  }
  switch (v->type) {
  case J_NULL:
    fputs("null", out);
    break;
  case J_BOOLEAN:
    fprintf(out, "%.*s", (int)v->u.boolean.len, v->u.boolean.ptr);
    break;
  case J_NUMBER:
    fprintf(out, "%.*s", (int)v->u.number.len, v->u.number.ptr);
    break;
  case J_STRING:
    print_string_escaped(out, v->u.string.ptr, v->u.string.len);
    break;
  case J_ARRAY:
    print_array_compact(v, out);
    break;
  case J_OBJECT:
    print_object_compact(v, out);
    break;
  }
}

static void print_value(const json_value *v, int indent, FILE *out) {
  if (!v) {
    fputs("null", out);
    return;
  }
  switch (v->type) {
  case J_NULL:
    fputs("null", out);
    break;
  case J_BOOLEAN:
    fprintf(out, "%.*s", (int)v->u.boolean.len, v->u.boolean.ptr);
    break;
  case J_NUMBER:
    fprintf(out, "%.*s", (int)v->u.number.len, v->u.number.ptr);
    break;
  case J_STRING:
    print_string_escaped(out, v->u.string.ptr, v->u.string.len);
    break;
  case J_ARRAY:
    print_array_compact(v, out);
    break;
  case J_OBJECT:
    fputs("{\n", out);
    json_object_node *object_items = v->u.object.items;
    while (object_items) {
      json_object_node *next = object_items->next;
      print_indent(out, indent + 1);
      print_string_escaped(out, object_items->item.key.ptr, object_items->item.key.len);
      fputs(": ", out);
      print_value(&object_items->item.value, indent + 1, out);
      if (next)
        fputs(", ", out);
      object_items = next;
    }
    fputc('\n', out);
    print_indent(out, indent);
    fputc('}', out);
    break;
  }
}

static int buffer_write_indent(buffer *b, int indent) {
  int i;
  for (i = 0; i < indent; ++i) {
    if (buffer_write(b, "    ", 4) < 0)
      return -1;
  }
  return 0;
}

static int buffer_write_string(buffer *b, const char *s, size_t len) {
  if (buffer_putc(b, '"') < 0)
    return -1;
  size_t i = 0;
  const unsigned char *p;
  for (p = (const unsigned char *)s; *p && i < len; ++i, ++p) {
    unsigned char c = *p;
    if (buffer_putc(b, c) < 0)
      return -1;
  }
  if (buffer_putc(b, '"') < 0)
    return -1;
  return 0;
}

static int buffer_write_array(buffer *b, const json_value *v) {
  if (buffer_putc(b, '[') < 0)
    return -1;
  json_array_node *array_items = v->u.array.items;
  while (array_items) {
    json_array_node *next = array_items->next;
    if (buffer_write_value(b, &array_items->item) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, ", ", 2) < 0)
        return -1;
    }
    array_items = next;
  }
  if (buffer_putc(b, ']') < 0)
    return -1;
  return 0;
}

static int buffer_write_object_indent(buffer *b, const json_value *v, int indent) {
  json_object_node *object_items = v->u.object.items;
  if (object_items == NULL) {
    if (buffer_write(b, "{\n", 2) < 0)
      return -1;
    if (buffer_write_indent(b, indent) < 0)
      return -1;
    if (buffer_putc(b, '}') < 0)
      return -1;
    return 0;
  }
  if (buffer_write(b, "{\n", 2) < 0)
    return -1;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (buffer_write_indent(b, indent + 1) < 0)
      return -1;
    json_object *ent = &object_items->item;
    if (buffer_write_string(b, ent->key.ptr, ent->key.len) < 0)
      return -1;
    if (buffer_write(b, ": ", 2) < 0)
      return -1;
    if (buffer_write_value_indent(b, &ent->value, indent + 1) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, ",\n", 2) < 0)
        return -1;
    }
    object_items = next;
  }
  if (buffer_putc(b, '\n') < 0)
    return -1;
  if (buffer_write_indent(b, indent) < 0)
    return -1;
  if (buffer_putc(b, '}') < 0)
    return -1;
  return 0;
}

static int buffer_write_object(buffer *b, const json_value *v) {
  json_object_node *object_items = v->u.object.items;
  if (object_items == NULL)
    return buffer_write(b, "{}", 2);
  if (buffer_putc(b, '{') < 0)
    return -1;
  while (object_items) {
    json_object_node *next = object_items->next;
    json_object *ent = &object_items->item;
    if (buffer_write_string(b, ent->key.ptr, ent->key.len) < 0)
      return -1;
    if (buffer_write(b, ": ", 2) < 0)
      return -1;
    if (buffer_write_value(b, &ent->value) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, ", ", 2) < 0)
        return -1;
    }
    object_items = next;
  }
  if (buffer_putc(b, '}') < 0)
    return -1;
  return 0;
}

static int buffer_write_value_indent(buffer *b, const json_value *v, int indent) {
  if (!v)
    return buffer_write(b, "null", 4);
  switch (v->type) {
  case J_NULL:
    return buffer_write(b, "null", 4);
  case J_BOOLEAN:
    return buffer_write(b, v->u.boolean.ptr, v->u.boolean.len);
  case J_NUMBER:
    return buffer_write(b, v->u.number.ptr, (int)v->u.number.len);
  case J_STRING:
    return buffer_write_string(b, v->u.string.ptr, v->u.string.len);
  case J_ARRAY:
    return buffer_write_array(b, v);
  case J_OBJECT:
    return buffer_write_object_indent(b, v, indent);
  }
  return -1;
}

static int buffer_write_value(buffer *b, const json_value *v) {
  if (!v)
    return buffer_write(b, "null", 4);
  switch (v->type) {
  case J_NULL:
    return buffer_write(b, "null", 4);
  case J_BOOLEAN:
    return buffer_write(b, v->u.boolean.ptr, v->u.boolean.len);
  case J_NUMBER:
    return buffer_write(b, v->u.number.ptr, (int)v->u.number.len);
  case J_STRING:
    return buffer_write_string(b, v->u.string.ptr, v->u.string.len);
  case J_ARRAY:
    return buffer_write_array(b, v);
  case J_OBJECT:
    return buffer_write_object(b, v);
  }
  return -1;
}

/* --- buffer helpers --- */

static int buffer_write(buffer *b, const char *data, int len) {
  if (len <= 0)
    return 0;
  if (b->pos + 1 >= b->cap) {
    int new_cap = b->cap * 2;
    char *new_buf = (char *)realloc(b->buf, new_cap);
    if (!new_buf)
      return -1;
    b->buf = new_buf;
    b->cap = new_cap;
  }
  char *buf = &b->buf[b->pos];
  int i;
  for (i = 0; i < len; i++) {
    *buf++ = *data++;
    b->pos++;
  }
  return 0;
}

static int buffer_putc(buffer *b, char c) {
  if (b->pos + 1 >= b->cap) {
    int new_cap = b->cap * 2;
    char *new_buf = (char *)realloc(b->buf, new_cap);
    if (!new_buf)
      return -1;
    b->buf = new_buf;
    b->cap = new_cap;
  }
  b->buf[b->pos++] = c;
  return 0;
}

/* --- json helpers --- */

char *json_stringify(const json_value *v) {
  if (!v)
    return NULL;
  buffer b;
  b.cap = MAX_BUFFER_SIZE;
  b.pos = 0;
  b.buf = (char *)calloc(1, (size_t)b.cap);
  if (!b.buf)
    return NULL;
  if (buffer_write_value_indent(&b, v, 0) < 0) {
    free(b.buf);
    return NULL;
  }
  char *final_buf = (char *)realloc(b.buf, (size_t)b.pos + 1);
  if (!final_buf) {
    free(b.buf);
    return NULL;
  }
  final_buf[b.pos] = '\0';
  return final_buf;
}

static bool json_array_equal(const json_value *a, const json_value *b) {
  if (!a || !b)
    return a == b;
  if (a->type != J_ARRAY || b->type != J_ARRAY)
    return false;
  size_t a_len = 0;
  for (json_array_node *a_node = a->u.array.items; a_node; a_node = a_node->next) {
    a_len++;
  }
  size_t b_len = 0;
  for (json_array_node *b_node = b->u.array.items; b_node; b_node = b_node->next) {
    b_len++;
  }
  if (a_len != b_len) {
    return false;
  }
  json_array_node *a_array_items = a->u.array.items;
  json_array_node *b_array_items = b->u.array.items;
  while (a_array_items && b_array_items) {
    if (!json_equal(&a_array_items->item, &b_array_items->item))
      return false;
    a_array_items = a_array_items->next;
    b_array_items = b_array_items->next;
  }
  return true;
}

static bool json_object_equal(const json_value *a, const json_value *b) {
  if (!a || !b)
    return a == b;
  if (a->type != J_OBJECT || b->type != J_OBJECT)
    return false;
  size_t a_len = 0;
  for (json_object_node *a_node = a->u.object.items; a_node; a_node = a_node->next) {
    a_len++;
  }
  size_t b_len = 0;
  for (json_object_node *b_node = b->u.object.items; b_node; b_node = b_node->next) {
    b_len++;
  }
  if (a_len != b_len) {
    return false;
  }
  for (json_object_node *a_node = a->u.object.items; a_node; a_node = a_node->next) {
    json_object *e = &a_node->item;
    json_value *b_val = json_object_get(b, e->key.ptr, e->key.len);
    if (!b_val || !json_equal(&e->value, b_val)) {
      return false;
    }
  }
  return true;
}

/* --- public API --- */

json_error json_validate(const char **s) {
  if (**s == '\0')
    return E_NO_DATA;
  if (**s != '{' && **s != '[') {
    return E_INVALID_JSON;
  }
  json_value *stack[JSON_STACK_SIZE];
  json_value v;
  memset(&v, 0, sizeof(json_value));
  int top = -1;
  json_value *current = &v;
  while (true) {
    if (**s == '\0')
      break;
    skip_whitespace(s);
    if (**s == '\0')
      break;
    if (current) {
      do {
        if (**s == '{') {
          current->type = J_OBJECT;
          current->u.object.items = NULL;
          current->u.object.last = NULL;
          if (++top >= JSON_STACK_SIZE)
            return E_STACK_OVERFLOW_OBJECT;
          (*s)++;
          stack[top] = current;
          current = NULL;
          break;
        }
        if (**s == '[') {
          current->type = J_ARRAY;
          current->u.array.items = NULL;
          current->u.array.last = NULL;
          (*s)++;
          if (++top >= JSON_STACK_SIZE)
            return E_STACK_OVERFLOW_ARRAY;
          stack[top] = current;
          current = NULL;
          break;
        }
        if (**s == '"') {
          current->type = J_STRING;
          if (parse_string(s, current)) {
            current = NULL;
            break;
          }
          return E_EXPECTED_STRING;
        }
        if (**s == 't') {
          if (*(*s + 1) == 'r' && *(*s + 2) == 'u' && *(*s + 3) == 'e') {
            current->type = J_BOOLEAN;
            current->u.boolean.ptr = *s;
            current->u.boolean.len = 4;
            *s += 4;
            current = NULL;
            break;
          }
          return E_EXPECTED_BOOLEAN;
        }
        if (**s == 'f') {
          if (*(*s + 1) == 'a' && *(*s + 2) == 'l' && *(*s + 3) == 's' && *(*s + 4) == 'e') {
            current->type = J_BOOLEAN;
            current->u.boolean.ptr = *s;
            current->u.boolean.len = JSON_FALSE_LEN;
            *s += JSON_FALSE_LEN;
            current = NULL;
            break;
          }
          return E_EXPECTED_BOOLEAN;
        }
        if (**s == 'n') {
          if (*(*s + 1) == 'u' && *(*s + 2) == 'l' && *(*s + 3) == 'l') {
            current->type = J_NULL;
            *s += 4;
            current = NULL;
            break;
          }
          return E_EXPECTED_NULL;
        }
        if (!parse_number(s, current)) {
          return E_MAILFORMED_JSON;
        }
        current->type = J_NUMBER;
        current = NULL;
        break;
      } while (0);
      if (!current) {
        continue;
      }
    }
    if (top == -1) {
      break;
    }
    current = stack[top];
    if (current->type == J_OBJECT) {
      if (**s == '}') {
        top--;
        (*s)++;
        if (**s == '\0') {
          break;
        }
        current = NULL;
        continue;
      }
      if (current->u.object.items != NULL) {
        if (**s == ',') {
          (*s)++;
          skip_whitespace(s);
          if (**s == '\0' || **s == '}') {
            return E_EXPECTED_OBJECT_ELEMENT;
          }
        } else {
          return E_EXPECTED_OBJECT;
        }
      }
      if (**s != '"') {
        return E_OBJECT_KEY;
      }
      json_value key;
      if (!parse_string(s, &key))
        return E_EXPECTED_OBJECT_KEY;
      skip_whitespace(s);
      if (**s != ':') {
        return E_OBJECT_VALUE;
      }
      (*s)++;
      skip_whitespace(s);
      if (**s == '\0') {
        return E_EXPECTED_OBJECT_VALUE;
      }
      json_object_node *node = new_object_node();
      if (!node)
        return E_STACK_OVERFLOW_OBJECT;
      node->item.key = key.u.string;
      if (current->u.object.items == NULL) {
        current->u.object.items = node;
      } else {
        current->u.object.last->next = node;
      }
      current->u.object.last = node;
      current = &node->item.value;
      continue;
    }
    if (current->type == J_ARRAY) {
      if (**s == ']') {
        top--;
        (*s)++;
        if (**s == '\0') {
          break;
        }
        current = NULL;
        continue;
      }
      if (current->u.array.items != NULL) {
        if (**s == ',') {
          (*s)++;
          skip_whitespace(s);
          if (**s == '\0' || **s == ']') {
            return E_EXPECTED_ARRAY_ELEMENT;
          }
        } else {
          return E_EXPECTED_ARRAY;
        }
      }
      json_array_node *node = new_array_node();
      if (!node)
        return E_STACK_OVERFLOW_ARRAY;
      if (current->u.array.items == NULL) {
        current->u.array.items = node;
      } else {
        current->u.array.last->next = node;
      }
      current->u.array.last = node;
      current = &node->item;
      continue;
    }
    if (current) {
      return E_UNKNOWN_ERROR;
    }
  }
  if (top == -1) {
    if (**s == '\0')
      return E_NO_ERROR;
    else
      return E_INVALID_DATA;
  }
  return E_INVALID_JSON_DATA;
}

bool json_parse_iterative(const char *s, json_value *root) {
  if (*s == '\0')
    return false;
  if (*s != '{' && *s != '[') {
    return false;
  }
  json_value *stack[JSON_STACK_SIZE];
  int top = -1;
  json_value *current = root;
  while (true) {
    if (*s == '\0')
      break;
    skip_whitespace(&s);
    if (current) {
      switch (*s) {
      case '{':
        current->type = J_OBJECT;
        current->u.object.items = NULL;
        current->u.object.last = NULL;
        s++;
        if (++top >= JSON_STACK_SIZE)
          return false;
        stack[top] = current;
        current = NULL;
        break;
      case '[':
        current->type = J_ARRAY;
        current->u.array.items = NULL;
        current->u.array.last = NULL;
        s++;
        if (++top >= JSON_STACK_SIZE)
          return false;
        stack[top] = current;
        current = NULL;
        break;
      case '"':
        current->type = J_STRING;
        if (!parse_string(&s, current))
          return false;
        current = NULL;
        break;
      case 't':
        if (*(s + 1) == 'r' && *(s + 2) == 'u' && *(s + 3) == 'e') {
          current->type = J_BOOLEAN;
          current->u.boolean.ptr = s;
          current->u.boolean.len = 4;
          s += 4;
          current = NULL;
        } else {
          return false;
        }
        break;
      case 'f':
        if (*(s + 1) == 'a' && *(s + 2) == 'l' && *(s + 3) == 's' && *(s + 4) == 'e') {
          current->type = J_BOOLEAN;
          current->u.boolean.ptr = s;
          current->u.boolean.len = JSON_FALSE_LEN;
          s += JSON_FALSE_LEN;
          current = NULL;
        } else {
          return false;
        }
        break;
      case 'n':
        if (*(s + 1) == 'u' && *(s + 2) == 'l' && *(s + 3) == 'l') {
          current->type = J_NULL;
          s += 4;
          current = NULL;
        } else {
          return false;
        }
        break;
      default:
        if (parse_number(&s, current)) {
          current->type = J_NUMBER;
          current = NULL;
        } else {
          return false;
        }
      }
      continue;
    }
    if (top == -1) {
      break;
    }
    current = stack[top];
    if (current->type == J_OBJECT) {
      if (*s == '}') {
        s++;
        top--;
        current = NULL;
        continue;
      }
      if (current->u.object.items != NULL) {
        if (*s == ',') {
          s++;
          skip_whitespace(&s);
        } else {
          return false;
        }
      }
      if (*s != '"')
        return false;
      json_value key;
      if (!parse_string(&s, &key))
        return false;
      skip_whitespace(&s);
      if (*s != ':')
        return false;
      s++;
      json_object_node *node = new_object_node();
      if (!node)
        return false;
      node->item.key = key.u.string;
      if (current->u.object.items == NULL) {
        current->u.object.items = node;
      } else {
        current->u.object.last->next = node;
      }
      current->u.object.last = node;
      current = &node->item.value;
    } else if (current->type == J_ARRAY) {
      if (*s == ']') {
        s++;
        top--;
        current = NULL;
        continue;
      }
      if (current->u.array.items != NULL) {
        if (*s == ',') {
          s++;
        } else {
          return false;
        }
      }
      json_array_node *node = new_array_node();
      if (!node)
        return false;
      if (current->u.array.items == NULL) {
        current->u.array.items = node;
      } else {
        current->u.array.last->next = node;
      }
      current->u.array.last = node;
      current = &node->item;
    }
  }
  return *s == '\0' && top == -1;
}

bool json_parse(const char *s, json_value *root) {
  if (*s == '\0')
    return false;
  if (*s != '{' && *s != '[') {
    return false;
  }
  return parse_value_build(&s, root) && *s == '\0';
}

bool json_equal(const json_value *a, const json_value *b) {
  if (a == b)
    return true;
  if (!a || !b)
    return false;
  if (a->type != b->type)
    return false;
  switch (a->type) {
  case J_NULL:
    return true;
  case J_BOOLEAN:
    return a->u.boolean.len == b->u.boolean.len && strncmp(a->u.boolean.ptr, b->u.boolean.ptr, a->u.boolean.len) == 0;
  case J_NUMBER: {
    return a->u.number.len == b->u.number.len && strncmp(a->u.number.ptr, b->u.number.ptr, a->u.number.len) == 0;
  }
  case J_STRING:
    if (a->u.string.ptr == NULL && b->u.string.ptr == NULL)
      return true;
    return a->u.string.ptr && b->u.string.ptr && a->u.string.len == b->u.string.len && strncmp(a->u.string.ptr, b->u.string.ptr, a->u.string.len) == 0;
  case J_ARRAY:
    return json_array_equal(a, b);
  case J_OBJECT:
    return json_object_equal(a, b);
  default:
    return false;
  }
}

void json_free(json_value *v) {
  json_array_node *array_node = v->u.array.items;
  json_object_node *object_node = v->u.object.items;
  switch (v->type) {
  case J_STRING:
    break;
  case J_ARRAY:
    while (array_node) {
      json_array_node *next = array_node->next;
      json_free(&array_node->item);
      free_array_node(array_node);
      array_node = next;
    }
    v->u.array.items = NULL;
    v->u.array.last = NULL;
    break;
  case J_OBJECT:
    while (object_node) {
      json_object_node *next = object_node->next;
      json_free(&object_node->item.value);
      free_object_node(object_node);
      object_node = next;
    }
    v->u.object.items = NULL;
    v->u.object.last = NULL;
    break;
  default:
    break;
  }
  v->type = J_NULL;
}

void json_print(const json_value *v, FILE *out) {
  print_value(v, 0, out);
}
