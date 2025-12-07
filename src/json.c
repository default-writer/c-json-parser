/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * Auto updated?
 *   Yes
 * Created:
 *   April 12, 1961 at 09:07:34 PM GMT+3
 * Modified:
 *   December 8, 2025 at 12:53:15 AM GMT+3
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

/* small buffer state (used by buffer-based printers) */
typedef struct {
  char *buf;
  int cap;
  int pos;
} buffer;

#ifndef USE_ALLOC
/* json_array_node pool */
static json_array_node json_array_node_pool[JSON_VALUE_POOL_SIZE];
static json_array_node *json_array_node_free_pool[JSON_VALUE_POOL_SIZE];
static size_t json_array_node_free_count = JSON_VALUE_POOL_SIZE;

/* json_object_node pool */
static json_object_node json_object_node_pool[JSON_VALUE_POOL_SIZE];
static json_object_node *json_object_node_free_pool[JSON_VALUE_POOL_SIZE];
static size_t json_object_node_free_count = JSON_VALUE_POOL_SIZE;
#endif

/* forward declarations */
static json_value *json_object_get(const json_value *obj, const char *key, size_t len);
static void free_json_value_contents(json_value *v);

/* --- parser helpers --- */
static bool parse_string_value(const char **s, json_value *v);
static bool parse_string_value_ptr(reference *ref, const char **s);
static bool parse_array_value(const char **s, json_value *v);
static bool parse_object_value(const char **s, json_value *v);
static bool parse_value_build(const char **s, json_value *v);

/* --- pretty-print helpers --- */
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

/* --- buffer helpers --- */
static int buffer_write(buffer *b, const char *data, int len);
static int buffer_putc(buffer *b, char c);

/* --- json helpers --- */

static bool json_array_equal(const json_value *a, const json_value *b);
static bool json_object_equal(const json_value *a, const json_value *b);

/* implementation */

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

static void free_json_value_contents(json_value *v) {
  if (!v)
    return;
  json_array_node *array_node = v->u.array.items;
  json_object_node *object_node = v->u.object.items;
  switch (v->type) {
  case J_STRING:
    break;
  case J_ARRAY:
    while (array_node) {
      json_array_node *next = array_node->next;
      free_json_value_contents(&array_node->item);
#ifdef USE_ALLOC
      free(array_node);
#else
      if (json_array_node_free_count < JSON_VALUE_POOL_SIZE) {
        json_array_node_free_pool[JSON_VALUE_POOL_SIZE - json_array_node_free_count++] = array_node;
      }
      array_node->item.type = J_NULL;
      array_node->item.u.array.items = NULL;
      array_node->item.u.array.last = NULL;
      array_node->next = NULL;
#endif
      array_node = next;
    }
    v->u.array.items = NULL;
    break;
  case J_OBJECT:
    while (object_node) {
      json_object_node *next = object_node->next;
      free_json_value_contents(&object_node->item.value);
#ifdef USE_ALLOC
      free(object_node);
#else
      if (json_object_node_free_count < JSON_VALUE_POOL_SIZE) {
        json_object_node_free_pool[JSON_VALUE_POOL_SIZE - json_object_node_free_count++] = object_node;
      }
      object_node->item.value.type = J_NULL;
      object_node->item.value.u.object.items = NULL;
      object_node->item.value.u.object.last = NULL;
      object_node->next = NULL;
#endif
      object_node = next;
    }
    v->u.object.items = NULL;
    break;
  default:
    break;
  }
  v->type = J_NULL;
}

/* --- parser helpers --- */

static bool parse_string_value(const char **s, json_value *v) {
  const char *p = *s + 1;
  const char *ptr = *s + 1;
  size_t len = 0;
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
  v->u.string.ptr = ptr;
  v->u.string.len = len;
  return true;
}

static bool parse_string_value_ptr(reference *ref, const char **s) {
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

static bool parse_array_value(const char **s, json_value *v) {
  (*s)++;
  NEXT_TOKEN(s);
  if (**s == ']') {
    (*s)++;
    return true;
  }
  while (1) {
    NEXT_TOKEN(s);
#ifdef USE_ALLOC
    json_array_node *array_node = calloc(1, sizeof(json_array_node));
    if (array_node == NULL) {
      return false;
    }
#else
    json_array_node *array_node = json_array_node_free_pool[JSON_VALUE_POOL_SIZE - --json_array_node_free_count];
    if (json_array_node_free_count == 0) {
      return false;
    }
#endif
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
      free_json_value_contents(v);
#ifdef USE_ALLOC
      free(array_node);
#else
      if (json_array_node_free_count < JSON_VALUE_POOL_SIZE) {
        json_array_node_free_pool[JSON_VALUE_POOL_SIZE - json_array_node_free_count++] = array_node;
      }
#endif
      v->u.array.items = NULL;
      return false;
    }

    NEXT_TOKEN(s);
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
  (*s)++;
  NEXT_TOKEN(s);
  if (**s == '}') {
    (*s)++;
    return true;
  }
  while (1) {
    NEXT_TOKEN(s);
    if (**s != '"') {
      free_json_value_contents(v);
      return false;
    }
    reference ref = {*s, 0};
    if (!parse_string_value_ptr(&ref, s)) {
      free_json_value_contents(v);
      return false;
    }
    NEXT_TOKEN(s);
    if (**s != ':') {
      free_json_value_contents(v);
      return false;
    }
    (*s)++;
    NEXT_TOKEN(s);
    json_object_node *object_node = NULL;
    json_object_node *object_items = v->u.object.items;
    while (object_items) {
      json_object_node *next = object_items->next;
      if (object_items->item.key.ptr && ref.ptr && object_items->item.key.len == ref.len && strncmp(object_items->item.key.ptr, ref.ptr, ref.len) == 0) {
        break;
      }
      object_items = next;
    }
    if (object_items == NULL) {
#ifdef USE_ALLOC
      object_node = calloc(1, sizeof(json_object_node));
      if (object_node == NULL) {
        return false;
      }
#else
      object_node = json_object_node_free_pool[JSON_VALUE_POOL_SIZE - --json_object_node_free_count];
      if (json_object_node_free_count == 0) {
        return false;
      }
#endif
      object_node->item.key.ptr = ref.ptr;
      object_node->item.key.len = ref.len;
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
      free_json_value_contents(v);
#ifdef USE_ALLOC
      free(object_node);
#else
      if (json_object_node_free_count < JSON_VALUE_POOL_SIZE) {
        json_object_node_free_pool[JSON_VALUE_POOL_SIZE - json_object_node_free_count++] = object_node;
      }
#endif
      v->u.object.items = NULL;
      return false;
    }

    NEXT_TOKEN(s);
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
  v->type = J_NULL;
}

static bool parse_value_build(const char **s, json_value *v) {
  NEXT_TOKEN(s);
  if (**s == 'n' && strncmp(*s, "null", TEXT_SIZE("null")) == 0) {
    v->type = J_NULL;
    *s += TEXT_SIZE("null");
    v->u.string.ptr = "null";
    v->u.string.len = TEXT_SIZE("null");
    return true;
  }
  if (**s == 't' && strncmp(*s, "true", TEXT_SIZE("true")) == 0) {
    v->type = J_BOOLEAN;
    *s += TEXT_SIZE("true");
    v->u.string.ptr = "true";
    v->u.string.len = TEXT_SIZE("true");
    return true;
  }
  if (**s == 'f' && strncmp(*s, "false", TEXT_SIZE("false")) == 0) {
    v->type = J_BOOLEAN;
    *s += TEXT_SIZE("false");
    v->u.string.ptr = "false";
    v->u.string.len = TEXT_SIZE("false");
    return true;
  }
  if (**s == '-' || isdigit((unsigned char)**s)) {
    v->type = J_NUMBER;
    const char *p = *s;
    char *end = NULL;
    strtod(p, &end);
    if (end == p)
      return false;
    *s = end;
    v->u.number.ptr = p;
    v->u.number.len = (size_t)(end - p);
    return true;
  }
  if (**s == '"') {
    v->type = J_STRING;
    return parse_string_value(s, v);
  }
  if (**s == '[') {
    v->type = J_ARRAY;
    return parse_array_value(s, v);
  }
  if (**s == '{') {
    v->type = J_OBJECT;
    return parse_object_value(s, v);
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
    fputs("    ", out); /* 4 spaces */
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
    if (buffer_write(b, TOKEN("    ")) < 0)
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
      if (buffer_write(b, TOKEN(", ")) < 0)
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
    if (buffer_write(b, TOKEN("{\n")) < 0)
      return -1;
    if (buffer_write_indent(b, indent) < 0)
      return -1;
    if (buffer_putc(b, '}') < 0)
      return -1;
    return 0;
  }

  if (buffer_write(b, TOKEN("{\n")) < 0)
    return -1;
  while (object_items) {
    json_object_node *next = object_items->next;
    if (buffer_write_indent(b, indent + 1) < 0)
      return -1;
    json_object *ent = &object_items->item;
    if (buffer_write_string(b, ent->key.ptr, ent->key.len) < 0)
      return -1;
    if (buffer_write(b, TOKEN(": ")) < 0)
      return -1;
    if (buffer_write_value_indent(b, &ent->value, indent + 1) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, TOKEN(",\n")) < 0)
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
    return buffer_write(b, TOKEN("{}"));
  if (buffer_putc(b, '{') < 0)
    return -1;
  while (object_items) {
    json_object_node *next = object_items->next;
    json_object *ent = &object_items->item;
    if (buffer_write_string(b, ent->key.ptr, ent->key.len) < 0)
      return -1;
    if (buffer_write(b, TOKEN(": ")) < 0)
      return -1;
    if (buffer_write_value(b, &ent->value) < 0)
      return -1;
    if (next) {
      if (buffer_write(b, TOKEN(", ")) < 0)
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
    return buffer_write(b, TOKEN("null"));
  switch (v->type) {
  case J_NULL:
    return buffer_write(b, TOKEN("null"));
  case J_BOOLEAN:
    return buffer_write(b, v->u.boolean.ptr, (int)v->u.boolean.len);
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
    return buffer_write(b, TOKEN("null"));
  switch (v->type) {
  case J_NULL:
    return buffer_write(b, TOKEN("null"));
  case J_BOOLEAN:
    return buffer_write(b, v->u.boolean.ptr, (int)v->u.boolean.len);
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

  /* Shrink to fit and null terminate. */
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
  json_array_node *a_array_items = a->u.array.items;
  json_array_node *b_array_items = b->u.array.items;
  while (a_array_items && b_array_items) {
    if (!json_equal(&a_array_items->item, &b_array_items->item))
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
    if (!json_equal(&e->value, bv))
      return false;
    a_object_items = a_object_items->next;
    b_object_items = b_object_items->next;
  }
  return true;
}

/* --- public API --- */

bool json_parse(const char *json, json_value *root) {
  if (!json)
    return false;
  const char *p = json;

  NEXT_TOKEN(&p);

  /* parse entire JSON into an in-memory json_value tree */
  if (!parse_value_build(&p, root))
    return false;

  /* ensure there is no trailing garbage */
  NEXT_TOKEN(&p);

  if (*p != '\0') {
    free_json_value_contents(root);
    return false;
  }

  return true;
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
    return a->u.boolean.ptr && b->u.boolean.ptr && a->u.boolean.len == b->u.boolean.len && strncmp(a->u.boolean.ptr, b->u.boolean.ptr, a->u.boolean.len) == 0;
  case J_NUMBER: {
    return a->u.number.ptr && b->u.number.ptr && a->u.number.len == b->u.number.len && strncmp(a->u.number.ptr, b->u.number.ptr, a->u.number.len) == 0;
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
  free_json_value_contents(v);
}

void json_initialize(void) {
#ifdef USE_ALLOC
#else
  int i = 0;
  for (i = 0; i < JSON_VALUE_POOL_SIZE; i++) {
    json_array_node_free_pool[i] = &json_array_node_pool[i];
    json_object_node_free_pool[i] = &json_object_node_pool[i];
  }
  json_array_node_free_count = JSON_VALUE_POOL_SIZE;
  json_object_node_free_count = JSON_VALUE_POOL_SIZE;
#endif
}

void json_print(const json_value *v, FILE *out) {
  print_value(v, 0, out);
}

/* End of file */
