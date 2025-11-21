/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   November 21, 2025 at 9:50:39 PM GMT+3
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

#define DICTIONARY_SIZE 16
#define JSON_VALUE_POOL_SIZE 0x200

#define STATE_INITIAL 1
#define STATE_ESCAPE_START 2
#define STATE_ESCAPE_UNICODE_BYTE1 3
#define STATE_ESCAPE_UNICODE_BYTE2 4
#define STATE_ESCAPE_UNICODE_BYTE3 5
#define STATE_ESCAPE_UNICODE_BYTE4 6
#define TEXT_SIZE(name) (sizeof((name)) - 1)

/* small buffer state (used by buffer-based printers) */
typedef struct {
  char *buf;
  int cap;
  int pos;
} bs;

/* json_value pool */
static json_value json_value_pool[JSON_VALUE_POOL_SIZE];
static json_value *json_value_free_pool[JSON_VALUE_POOL_SIZE];
static size_t json_value_free_count;

/* forward declarations */
static void *new_json_value(void);
static bool json_object_set_take_key(json_value *obj, const char *ptr, size_t len, json_value *value);
static json_value *json_object_get(const json_value *obj, const char *key, size_t len);
static json_value *json_new_null(const char *ptr, size_t len);
static json_value *json_new_boolean(const char *ptr, size_t len);
static json_value *json_new_number(const char *ptr, size_t len);
static json_value *json_new_array(void);
static json_value *json_new_object(void);
static void json_array_push(json_value *arr, json_value *item);

static void free_json_value_contents(json_value *v);
static void skip_ws(const char **s);

/* --- parser helpers --- */
static json_value *parse_string_value(const char **s);
static bool parse_string_value_ptr(reference *ref, const char **s);
static json_value *parse_number_value(const char **s);
static json_value *parse_array_value(const char **s, int id);
static json_value *parse_object_value(const char **s, int id);
static bool match_literal_build(const char **s, const char *lit);
static json_value *parse_value_build(const char **s, int id);

/* --- pretty-print helpers --- */
static void print_string_escaped(FILE *out, const char *s, size_t len);
static void print_indent(FILE *out, int indent);
static void print_array_compact(const json_value *v, FILE *out);
static void print_object_compact(const json_value *v, FILE *out);
static void print_value_compact(const json_value *v, FILE *out);
static void print_value(const json_value *v, int indent, FILE *out);

static int print_indent_buf(bs *b, int indent);
static int print_string_escaped_buf(bs *b, const char *s, size_t len);
static int print_array_compact_buf(const json_value *v, bs *b);
static int print_object_buf(const json_value *v, bs *b, int indent);
static int print_object_compact_buf(const json_value *v, bs *b);
static int print_value_compact_buf(const json_value *v, bs *b);
static int print_value_buf(const json_value *v, int indent, bs *b);

/* --- buffer helpers --- */
static int bs_write(bs *b, const char *data, int len);
static int bs_putc(bs *b, char c);

/* --- json helpers --- */
static int json_stringify_to_buffer(const json_value *v, char *buf, int bufsize);
static bool json_array_equal(const json_value *a, const json_value *b);
static bool json_object_equal(const json_value *a, const json_value *b);

/* implementation */
static void *new_json_value(void) {
  if (json_value_free_count == 0) {
    return NULL;
  }
  void *ptr = json_value_free_pool[JSON_VALUE_POOL_SIZE - json_value_free_count--];
  memset(ptr, 0, sizeof(json_value));
  return ptr;
}

static bool json_object_set_take_key(json_value *obj, const char *ptr, size_t len, json_value *value) {
  if (!obj || obj->type != J_OBJECT || !ptr)
    return false;

  /* check existing keys by comparing strings */
  json_object_node *object_items = obj->u.object.items;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (object_items->item.key.ptr && strncmp(object_items->item.key.ptr, ptr, len) == 0) {
      free_json_value_contents(object_items->item.value);
      object_items->item.value = value;
      return true;
    }
    object_items = next;
  }
  json_object_node *new_item = calloc(1, sizeof(json_object_node));
  if (!new_item)
    return false;
  // new_item->item = calloc(1, sizeof(json_object));
  new_item->item.key.ptr = ptr;
  new_item->item.key.len = len;
  new_item->item.value = value;  
  if (obj->u.object.items == NULL) {
    obj->u.object.items = new_item;
    return true;
  }
  if (obj->u.object.last == NULL) {
    obj->u.object.last = new_item;
    obj->u.object.items->next = new_item;
    return true;
  }
  json_object_node* last = obj->u.object.last;
  obj->u.object.last = new_item;
  last->next = new_item;
  return true;
}

static json_value *json_object_get(const json_value *obj, const char *key, size_t len) {
  if (!obj || obj->type != J_OBJECT || !key)
    return NULL;
  json_object_node *object_items = obj->u.object.items;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (strncmp(object_items->item.key.ptr, key, len) == 0)
      return object_items->item.value;
    object_items = next;
  }
  return NULL;
}

static json_value *json_new_null(const char *ptr, size_t len) {
  json_value *v = new_json_value();
  if (!v)
    return NULL;
  v->type = J_NULL;
  v->u.string.ptr = ptr;
  v->u.string.len = len;
  return v;
}

static json_value *json_new_boolean(const char *ptr, size_t len) {
  json_value *v = new_json_value();
  if (!v)
    return NULL;
  v->type = J_BOOLEAN;
  v->u.boolean.ptr = ptr;
  v->u.boolean.len = len;
  return v;
}

static json_value *json_new_number(const char *ptr, size_t len) {
  json_value *v = new_json_value();
  if (!v)
    return NULL;
  v->type = J_NUMBER;
  v->u.number.ptr = ptr;
  v->u.number.len = len;
  return v;
}

static json_value *json_new_array(void) {
  json_value *v = new_json_value();
  if (!v)
    return NULL;
  v->type = J_ARRAY;
  v->u.array.items = NULL;
  return v;
}

static json_value *json_new_object(void) {
  json_value *v = new_json_value();
  if (!v)
    return NULL;
  v->type = J_OBJECT;
  v->u.object.items = NULL;
  return v;
}

static void json_array_push(json_value *arr, json_value *item) {
  if (!arr || arr->type != J_ARRAY || !item)
    return;
  json_value_node *new_item = calloc(1, sizeof(json_value_node));
  if (!new_item)
    return;
  new_item->item = item;
  if (arr->u.array.items == NULL) {
    arr->u.array.items = new_item;
    return;
  }
  if (arr->u.array.last == NULL) {
    arr->u.array.last = new_item;
    arr->u.array.items->next = new_item;
    return;
  }
  json_value_node* last = arr->u.array.last;
  arr->u.array.last = new_item;
  last->next = new_item;
}

static void free_json_value_contents(json_value *v) {
  if (!v)
    return;
  json_value_node *array_items = v->u.array.items;
  json_object_node *object_items = v->u.object.items;
  switch (v->type) {
  case J_STRING:
    break;
  case J_ARRAY:
    while (array_items) {
      json_value_node *next = array_items->next;
      free_json_value_contents(array_items->item);
      free(array_items);
      array_items = next;
    }
    break;
  case J_OBJECT:
    while (object_items) {
      json_object_node *next = object_items->next;
      free_json_value_contents(object_items->item.value);
      // free(object_items->item);
      free(object_items);
      object_items = next;
    }
    break;
  default:
    break;
  }
  json_value_free_pool[json_value_free_count++] = v;
  v->type = 0;
}

static void skip_ws(const char **s) {
  while (isspace((unsigned char)**s))
    (*s)++;
}

/* --- parser helpers --- */

  static json_value *parse_string_value(const char **s) {
    if (**s != '"')
      return NULL;
    const char *p = *s + 1;
    const char *ptr = *s + 1;
    /* we need to process escapes, we'll build a dynamic buffer */
    size_t len = 0;
    /* ensure empty buffer is a valid C string */
    int state = STATE_INITIAL;
    while (*p && state) {
      switch (state) {
      case STATE_INITIAL:
        if (*p == '"') {
          state = 0;
          continue;
        } else if (*p == '\\')
          state = STATE_ESCAPE_START;
        break;
      case STATE_ESCAPE_START:
        if (*p == '\\')
          state = STATE_INITIAL;
        else if (*p == '"')
          state = STATE_INITIAL;
        else if (*p == 'b')
          state = STATE_INITIAL;
        else if (*p == 'f')
          state = STATE_INITIAL;
        else if (*p == 'n')
          state = STATE_INITIAL;
        else if (*p == 'r')
          state = STATE_INITIAL;
        else if (*p == 't')
          state = STATE_INITIAL;
        else if (*p == 'u')
          state = STATE_ESCAPE_UNICODE_BYTE1;
        else
          return NULL;
        break;
      case STATE_ESCAPE_UNICODE_BYTE1:
        if (!isxdigit((unsigned char)*p)) {
          return NULL;
        }
        state = STATE_ESCAPE_UNICODE_BYTE2;
        break;
      case STATE_ESCAPE_UNICODE_BYTE2:
        if (!isxdigit((unsigned char)*p)) {
          return NULL;
        }
        state = STATE_ESCAPE_UNICODE_BYTE3;
        break;
      case STATE_ESCAPE_UNICODE_BYTE3:
        if (!isxdigit((unsigned char)*p)) {
          return NULL;
        }
        state = STATE_ESCAPE_UNICODE_BYTE4;
        break;
      case STATE_ESCAPE_UNICODE_BYTE4:
        if (!isxdigit((unsigned char)*p)) {
          return NULL;
        }
        state = STATE_INITIAL;
        break;
      }
      len++;
      p++;
    }
    if (*p != '"') {
      return NULL;
    }
    p++;
    *s = p;
    json_value *v = new_json_value();
    if (!v) {
      return NULL;
    }
    v->type = J_STRING;
    v->u.string.ptr = ptr;
    v->u.string.len = len;
    return v;
  }

  static bool parse_string_value_ptr(reference * ref, const char **s) {
    if (**s != '"')
      return false;
    const char *p = *s + 1;
    const char *ptr = *s + 1;
    /* we need to process escapes, we'll build a dynamic buffer */
    size_t len = 0;
    /* ensure empty buffer is a valid C string */
    int state = STATE_INITIAL;
    while (*p && state) {
      switch (state) {
      case STATE_INITIAL:
        if (*p == '"') {
          state = 0;
          continue;
        } else if (*p == '\\')
          state = STATE_ESCAPE_START;
        break;
      case STATE_ESCAPE_START:
        if (*p == '\\')
          state = STATE_INITIAL;
        else if (*p == '"')
          state = STATE_INITIAL;
        else if (*p == 'b')
          state = STATE_INITIAL;
        else if (*p == 'f')
          state = STATE_INITIAL;
        else if (*p == 'n')
          state = STATE_INITIAL;
        else if (*p == 'r')
          state = STATE_INITIAL;
        else if (*p == 't')
          state = STATE_INITIAL;
        else if (*p == 'u')
          state = STATE_ESCAPE_UNICODE_BYTE1;
        else
          return false;
        break;
      case STATE_ESCAPE_UNICODE_BYTE1:
        if (!isxdigit((unsigned char)*p)) {
          return false;
        }
        state = STATE_ESCAPE_UNICODE_BYTE2;
        break;
      case STATE_ESCAPE_UNICODE_BYTE2:
        if (!isxdigit((unsigned char)*p)) {
          return false;
        }
        state = STATE_ESCAPE_UNICODE_BYTE3;
        break;
      case STATE_ESCAPE_UNICODE_BYTE3:
        if (!isxdigit((unsigned char)*p)) {
          return false;
        }
        state = STATE_ESCAPE_UNICODE_BYTE4;
        break;
      case STATE_ESCAPE_UNICODE_BYTE4:
        if (!isxdigit((unsigned char)*p)) {
          return false;
        }
        state = STATE_INITIAL;
        break;
      }
      len++;
      p++;
    }
    if (*p != '"') {
      return false;
    }
    p++;
    *s = p;
    ref->ptr = ptr;
    ref->len = len;
    return true;
  }

  static json_value *parse_number_value(const char **s) {
    const char *p = *s;
    char *end = NULL;
    strtod(p, &end);
    if (end == p)
      return NULL;
    *s = end;
    return json_new_number(p, (size_t)(end - p));
  }

  static json_value *parse_array_value(const char **s, int id) {
    if (**s != '[')
      return NULL;
    (*s)++;
    skip_ws(s);
    json_value *arr = json_new_array();
    if (!arr)
      return NULL;
    if (**s == ']') {
      (*s)++;
      return arr;
    }
    while (1) {
      skip_ws(s);
      json_value *elem = parse_value_build(s, ++id);
      if (!elem) {
        free_json_value_contents(arr);
        return NULL;
      }
      json_array_push(arr, elem);
      skip_ws(s);
      if (**s == ',') {
        (*s)++;
        continue;
      }
      if (**s == ']') {
        (*s)++;
        return arr;
      }
      free_json_value_contents(arr);
      return NULL;
    }
  }

  static json_value *parse_object_value(const char **s, int id) {
    if (**s != '{')
      return NULL;
    (*s)++;
    skip_ws(s);
    json_value *obj = json_new_object();
    if (!obj)
      return NULL;
    if (**s == '}') {
      (*s)++;
      return obj;
    }
    while (1) {
      skip_ws(s);
      if (**s != '"') {
        free_json_value_contents(obj);
        return NULL;
      }
      reference ref = {*s, 0};
      if (!parse_string_value_ptr(&ref, s)) {
        free_json_value_contents(obj);
        return NULL;
      }
      skip_ws(s);
      if (**s != ':') {
        free_json_value_contents(obj);
        return NULL;
      }
      (*s)++;
      skip_ws(s);
      json_value *val = parse_value_build(s, ++id);
      if (!val) {
        free_json_value_contents(obj);
        return NULL;
      }

      /* Take ownership of the parsed key buffer (no extra duplication). */
      if (!json_object_set_take_key(obj, ref.ptr, ref.len, val)) {
        /* insertion failed: free transferred resources */
        free_json_value_contents(val);
        free_json_value_contents(obj);
        return NULL;
      }
      /* free only the json_value wrapper for the key (do NOT free key.ptr) */
      skip_ws(s);
      if (**s == ',') {
        (*s)++;
        continue;
      }
      if (**s == '}') {
        (*s)++;
        return obj;
      }
      free_json_value_contents(obj);
      return NULL;
    }
  }

  static bool match_literal_build(const char **s, const char *lit) {
    size_t n = strlen(lit);
    if (strncmp(*s, lit, n) == 0) {
      *s += n;
      return true;
    }
    return false;
  }

  static json_value *parse_value_build(const char **s, int id) {
    skip_ws(s);
    if (**s == '"')
      return parse_string_value(s);
    if (**s == '{')
      return parse_object_value(s, ++id);
    if (**s == '[')
      return parse_array_value(s, ++id);
    if (**s == 'n') {
      const char *ptr = *s;
      if (match_literal_build(s, "null"))
        return json_new_null(ptr, TEXT_SIZE("null"));
      return NULL;
    }
    if (**s == 't') {
      const char *ptr = *s;
      if (match_literal_build(s, "true"))
        return json_new_boolean(ptr, TEXT_SIZE("true"));
      return NULL;
    }
    if (**s == 'f') {
      const char *ptr = *s;
      if (match_literal_build(s, "false"))
        return json_new_boolean(ptr, TEXT_SIZE("false"));
      return NULL;
    }
    if (**s == '-' || isdigit((unsigned char)**s))
      return parse_number_value(s);
    return NULL;
  }

  /* --- pretty-print helpers --- */

  static void print_string_escaped(FILE * out, const char *s, size_t len) {
    fputc('"', out);
    size_t i = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p && i < len; ++i, ++p) {
      unsigned char c = *p;
      fputc(c, out);
    }
    fputc('"', out);
  }

  static void print_indent(FILE * out, int indent) {
    for (int i = 0; i < indent; ++i)
      fputs("    ", out); /* 4 spaces */
  }

  static void print_array_compact(const json_value *v, FILE *out) {
    if (!v || v->type != J_ARRAY) {
      fputs("[]", out);
      return;
    }
    fputc('[', out);
    json_value_node *array_items = v->u.array.items;
    while (array_items) {
      json_value_node *next = array_items->next;
      print_value_compact(array_items->item, out);
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
      print_value_compact(object_items->item.value, out);
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
      /* arrays printed single-line */
      print_array_compact(v, out);
      break;
    case J_OBJECT: {
      fputs("{\n", out); /* keep object starting symbol on its own line with
                            properties below */
      json_object_node *object_items = v->u.object.items;
      while (object_items) {
        json_object_node *next = object_items->next;
        print_indent(out, indent + 1);
        print_string_escaped(out, object_items->item.key.ptr, object_items->item.key.len);
        fputs(": ", out);
        print_value(object_items->item.value, indent + 1, out);
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
  }

  static int print_indent_buf(bs * b, int indent) {
    for (int i = 0; i < indent; ++i) {
      if (bs_write(b, "    ", 4) < 0)
        return -1;
    }
    return 0;
  }

  static int print_string_escaped_buf(bs * b, const char *s, size_t len) {
    if (bs_putc(b, '"') < 0)
      return -1;
    size_t i = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p && i < len; ++i, ++p) {
      unsigned char c = *p;
      if (bs_putc(b, c) < 0)
        return -1;
    }
    if (bs_putc(b, '"') < 0)
      return -1;
    return 0;
  }

  static int print_array_compact_buf(const json_value *v, bs *b) {
    if (!v || v->type != J_ARRAY) {
      return bs_write(b, "[]", 2);
    }
    if (bs_putc(b, '[') < 0)
      return -1;
    json_value_node *array_items = v->u.array.items;
    while (array_items) {
      json_value_node *next = array_items->next;
      if (print_value_compact_buf(array_items->item, b) < 0)
        return -1;
      if (next) {
        if (bs_write(b, ", ", 2) < 0)
          return -1;
      }
      array_items = next;
    }
    if (bs_putc(b, ']') < 0)
      return -1;
    return 0;
  }

  static int print_object_buf(const json_value *v, bs *b, int indent) {
    if (!v || v->type != J_OBJECT) {
      return bs_write(b, "{\n}", 3);
    }
    json_object_node *object_items = v->u.object.items;
    if (object_items == NULL) {
      if (bs_write(b, "{\n", 2) < 0)
        return -1;
      if (print_indent_buf(b, indent) < 0)
        return -1;
      if (bs_putc(b, '}') < 0)
        return -1;
      return 0;
    }

    if (bs_write(b, "{\n", 2) < 0)
      return -1;
    while (object_items) {
      json_object_node *next = object_items->next;
      if (print_indent_buf(b, indent + 1) < 0)
        return -1;
      json_object *ent = &object_items->item;
      if (print_string_escaped_buf(b, ent->key.ptr, ent->key.len) < 0)
        return -1;
      if (bs_write(b, ": ", 2) < 0)
        return -1;
      if (print_value_buf(ent->value, indent + 1, b) < 0)
        return -1;
      if (next) {
        if (bs_write(b, ",\n", 2) < 0)
          return -1;
      }
      object_items = next;
    }
    if (bs_putc(b, '\n') < 0)
      return -1;
    if (print_indent_buf(b, indent) < 0)
      return -1;
    if (bs_putc(b, '}') < 0)
      return -1;
    return 0;
  }

  static int print_object_compact_buf(const json_value *v, bs *b) {
    if (!v || v->type != J_OBJECT)
      return bs_write(b, "{}", 2);
    json_object_node *object_items = v->u.object.items;
    if (object_items == NULL)
      return bs_write(b, "{}", 2);
    if (bs_putc(b, '{') < 0)
      return -1;
    while (object_items) {
      json_object_node *next = object_items->next;
      json_object *ent = &object_items->item;
      if (print_string_escaped_buf(b, ent->key.ptr, ent->key.len) < 0)
        return -1;
      if (bs_write(b, ": ", 2) < 0)
        return -1;
      if (print_value_compact_buf(ent->value, b) < 0)
        return -1;
      if (next) {
        if (bs_write(b, ", ", 2) < 0)
          return -1;
      }
      object_items = next;
    }
    if (bs_putc(b, '}') < 0)
      return -1;
    return 0;
  }

  static int print_value_compact_buf(const json_value *v, bs *b) {
    if (!v)
      return bs_write(b, "null", TEXT_SIZE("null"));
    switch (v->type) {
    case J_NULL:
      return bs_write(b, "null", TEXT_SIZE("null"));
    case J_BOOLEAN:
      return bs_write(b, v->u.boolean.ptr, (int)v->u.boolean.len);
    case J_NUMBER:
      /* write number into buffer */
      return bs_write(b, v->u.number.ptr, (int)v->u.number.len);
    case J_STRING:
      return print_string_escaped_buf(b, v->u.string.ptr, v->u.string.len);
    case J_ARRAY:
      return print_array_compact_buf(v, b);
    case J_OBJECT:
      return print_object_compact_buf(v, b);
    }
    return -1;
  }

  static int print_value_buf(const json_value *v, int indent, bs *b) {
    if (!v)
      return bs_write(b, "null", TEXT_SIZE("null"));
    switch (v->type) {
    case J_NULL:
      return bs_write(b, "null", TEXT_SIZE("null"));
    case J_BOOLEAN:
      return bs_write(b, v->u.boolean.ptr, (int)v->u.boolean.len);
    case J_NUMBER:
      return bs_write(b, v->u.number.ptr, (int)v->u.number.len);
    case J_STRING:
      return print_string_escaped_buf(b, v->u.string.ptr, v->u.string.len);
    case J_ARRAY:
      /* arrays printed single-line */
      return print_array_compact_buf(v, b);
    case J_OBJECT:
      return print_object_buf(v, b, indent);
    }
    return -1;
  }

  /* --- buffer helpers --- */

  static int bs_write(bs * b, const char *data, int len) {
    if (len <= 0)
      return 0;
    if (b->pos + len >= b->cap)
      return -1;
    char *buf = &b->buf[b->pos];
    for (int i = 0; i < len; i++) {
      *buf++ = *data++;
      b->pos++;
    }
    return 0;
  }

  static int bs_putc(bs * b, char c) {
    if (b->pos + 1 >= b->cap)
      return -1;
    b->buf[b->pos++] = c;
    return 0;
  }

  /* --- json helpers --- */

  static int json_stringify_to_buffer(const json_value *v, char *buf, int bufsize) {
    if (!buf || bufsize <= 0)
      return -1;
    bs bstate = {buf, bufsize, 0};

    if (print_value_buf(v, 0, &bstate) < 0)
      return -1;

    if (bs_putc(&bstate, '\n') < 0)
      return -1;

    /* ensure NUL termination if space remains */
    if (bstate.pos < bstate.cap)
      bstate.buf[bstate.pos] = '\0';
    else
      return -1;
    return bstate.pos;
  }

  static bool json_array_equal(const json_value *a, const json_value *b) {
    if (!a || !b)
      return a == b;
    if (a->type != J_ARRAY || b->type != J_ARRAY)
      return false;
    json_value_node *a_array_items = a->u.array.items;
    json_value_node *b_array_items = b->u.array.items;
    while (a_array_items && b_array_items) {
      if (!json_equal(a_array_items->item, b_array_items->item))
        return false;
      a_array_items = a_array_items->next;
      b_array_items = b_array_items->next;
    }
    if (a_array_items || b_array_items)
      return false;
    return true;
  }

  static bool json_object_equal(const json_value *a, const json_value *b) {
    if (!a || !b)
      return a == b;
    if (a->type != J_OBJECT || b->type != J_OBJECT)
      return false;
    json_object_node *a_object_items = a->u.object.items;
    json_object_node *b_object_items = b->u.object.items;
    while (a_object_items && b_object_items) {
      json_object *e = &a_object_items->item;
      json_value *bv = json_object_get(b, e->key.ptr, e->key.len);
      if (!bv)
        return false;
      if (!json_equal(e->value, bv))
        return false;
      a_object_items = a_object_items->next;
    }
    return true;
  }

  /* --- public API --- */

  const char *json_source(const json_value *v) {
    if (!v)
      return NULL;
    switch (v->type) {
    case J_NULL:
      return v->u.string.ptr;
    case J_BOOLEAN:
      return v->u.boolean.ptr;
    case J_NUMBER:
      return v->u.number.ptr;
    case J_STRING:
      return v->u.string.ptr;
    case J_ARRAY:
      return "array";
    case J_OBJECT:
      return "object";
    default:
      return NULL;
    }
  }

  json_value *json_parse(const char *json) {
    if (!json)
      return NULL;
    const char *p = json;
    skip_ws(&p);

    /* parse entire JSON into an in-memory json_value tree */
    json_value *root = parse_value_build(&p, 0);
    if (!root)
      return NULL;

    /* ensure there is no trailing garbage */
    skip_ws(&p);
    if (*p != '\0') {
      free_json_value_contents(root);
      return NULL;
    }

    return root;
  }

  char *json_stringify(const json_value *v) {
    if (!v)
      return NULL;
    char *buf = calloc(1, (size_t)MAX_BUFFER_SIZE);
    if (!buf)
      return NULL;
    int rc = json_stringify_to_buffer(v, buf, MAX_BUFFER_SIZE);
    if (rc >= 0) {
      /* rc is bytes written (excluding '\0') */
      /* shrink to fit */
      char *shr = realloc(buf, (size_t)rc + 1);
      if (shr)
        buf = shr;
      return buf;
    }
    free(buf);
    return NULL;
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
      if (a->u.string.ptr == NULL || b->u.string.ptr == NULL)
        return false;
      if (a->u.string.len != b->u.string.len)
        return false;
      return strncmp(a->u.string.ptr, b->u.string.ptr, a->u.string.len) == 0;
    case J_ARRAY:
      return json_array_equal(a, b);
    case J_OBJECT:
      return json_object_equal(a, b);
    default:
      return false;
    }
  }

  void json_free(json_value * v) {
    free_json_value_contents(v);
  }

  void json_pool_reset(void) {
    for (size_t i = 0; i < JSON_VALUE_POOL_SIZE; i++) {
      json_value_free_pool[i] = &json_value_pool[i];
    }
    json_value_free_count = JSON_VALUE_POOL_SIZE;
  }

  void json_print(const json_value *v, FILE *out) {
    print_value(v, 0, out);
  }

  /* End of file */
