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
static json_value *json_new_null(void);
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
  for (size_t i = 0; i < obj->u.object.count; ++i) {
    if (obj->u.object.items[i].ptr && strncmp(obj->u.object.items[i].ptr, ptr, len) == 0) {
      free_json_value_contents(obj->u.object.items[i].value);
      obj->u.object.items[i].value = value;
      return true;
    }
  }
  /* ensure capacity */
  if (obj->u.object.count == obj->u.object.capacity) {
    size_t capacity = obj->u.object.capacity ? obj->u.object.capacity * 2 : DICTIONARY_SIZE;
    json_object *items = realloc(obj->u.object.items, capacity * sizeof(json_object));
    if (!items) {
      return false;
    }
    obj->u.object.items = items;
    obj->u.object.capacity = capacity;
  }
  obj->u.object.items[obj->u.object.count].ptr = ptr;
  obj->u.object.items[obj->u.object.count].len = len;
  obj->u.object.items[obj->u.object.count].value = value;
  obj->u.object.count++;
  return true;
}

static json_value *json_object_get(const json_value *obj, const char *key, size_t len) {
  if (!obj || obj->type != J_OBJECT || !key)
    return NULL;
  for (size_t i = 0; i < obj->u.object.count; ++i) {
    if (strncmp(obj->u.object.items[i].ptr, key, len) == 0)
      return obj->u.object.items[i].value;
  }
  return NULL;
}

static json_value *json_new_null(void) {
  json_value *v = (json_value *)new_json_value();
  if (!v)
    return NULL;
  v->type = J_NULL;
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
  v->u.array.count = 0;
  v->u.array.capacity = 0;
  return v;
}

static json_value *json_new_object(void) {
  json_value *v = new_json_value();
  if (!v)
    return NULL;
  v->type = J_OBJECT;
  v->u.object.items = NULL;
  v->u.object.count = 0;
  v->u.object.capacity = 0;
  return v;
}

static void json_array_push(json_value *arr, json_value *item) {
  if (!arr || arr->type != J_ARRAY || !item)
    return;
  if (arr->u.array.count == arr->u.array.capacity) {
    size_t ncap = arr->u.array.capacity ? arr->u.array.capacity * 2 : DICTIONARY_SIZE;
    json_value **newitems = realloc(arr->u.array.items, ncap * sizeof(json_value *));
    if (!newitems)
      return;
    arr->u.array.items = newitems;
    arr->u.array.capacity = ncap;
  }
  arr->u.array.items[arr->u.array.count++] = item;
}

static void free_json_value_contents(json_value *v) {
  if (!v)
    return;
  switch (v->type) {
  case J_STRING:
    break;
  case J_ARRAY:
    for (size_t i = 0; i < v->u.array.count; ++i)
      free_json_value_contents(v->u.array.items[i]);
    free(v->u.array.items);
    v->u.array.count = 0;
    v->u.array.capacity = 0;
    break;
  case J_OBJECT:
    for (size_t i = 0; i < v->u.object.count; ++i)
      free_json_value_contents(v->u.object.items[i].value);
    free(v->u.object.items);
    v->u.object.count = 0;
    v->u.object.capacity = 0;
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
    if (match_literal_build(s, "null"))
      return json_new_null();
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

static void print_string_escaped(FILE *out, const char *s, size_t len) {
  fputc('"', out);
  size_t i = 0;
  for (const unsigned char *p = (const unsigned char *)s; *p && i < len; ++i, ++p) {
    unsigned char c = *p;
    fputc(c, out);
  }
  fputc('"', out);
}

static void print_indent(FILE *out, int indent) {
  for (int i = 0; i < indent; ++i)
    fputs("    ", out); /* 4 spaces */
}

static void print_array_compact(const json_value *v, FILE *out) {
  if (!v || v->type != J_ARRAY) {
    fputs("[]", out);
    return;
  }
  fputc('[', out);
  for (size_t i = 0; i < v->u.array.count; ++i) {
    if (i)
      fputs(", ", out);
    print_value_compact(v->u.array.items[i], out);
  }
  fputc(']', out);
}

static void print_object_compact(const json_value *v, FILE *out) {
  if (!v || v->type != J_OBJECT) {
    fputs("{}", out);
    return;
  }
  fputc('{', out);
  for (size_t i = 0; i < v->u.object.count; ++i) {
    if (i)
      fputs(", ", out);
    json_object *e = &v->u.object.items[i];
    print_string_escaped(out, e->ptr, e->len);
    fputs(": ", out);
    print_value_compact(e->value, out);
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
    for (size_t i = 0; i < v->u.object.count; ++i) {
      if (i)
        fputs(",\n", out);
      print_indent(out, indent + 1);
      json_object *e = &v->u.object.items[i];
      print_string_escaped(out, e->ptr, e->len);
      fputs(": ", out);
      /* If value is an object, recurse to pretty format; arrays remain
       * single-line */
      print_value(e->value, indent + 1, out);
    }
    fputc('\n', out);
    print_indent(out, indent);
    fputc('}', out);
    break;
  }
  }
}

static int print_indent_buf(bs *b, int indent) {
  for (int i = 0; i < indent; ++i) {
    if (bs_write(b, "    ", 4) < 0)
      return -1;
  }
  return 0;
}

static int print_string_escaped_buf(bs *b, const char *s, size_t len) {
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
  for (size_t i = 0; i < v->u.array.count; ++i) {
    if (i) {
      if (bs_write(b, ", ", 2) < 0)
        return -1;
    }
    if (print_value_compact_buf(v->u.array.items[i], b) < 0)
      return -1;
  }
  if (bs_putc(b, ']') < 0)
    return -1;
  return 0;
}

static int print_object_buf(const json_value *v, bs *b, int indent) {
  if (!v || v->type != J_OBJECT) {
    return bs_write(b, "{\n}", 3);
  }

  size_t n = v->u.object.count;
  if (n == 0) {
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
  for (size_t i = 0; i < n; ++i) {
    if (i) {
      if (bs_write(b, ",\n", 2) < 0)
        return -1;
    }
    if (print_indent_buf(b, indent + 1) < 0)
      return -1;
    json_object *ent = &v->u.object.items[i];
    if (print_string_escaped_buf(b, ent->ptr, ent->len) < 0)
      return -1;
    if (bs_write(b, ": ", 2) < 0)
      return -1;
    if (print_value_buf(ent->value, indent + 1, b) < 0)
      return -1;
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
  size_t n = v->u.object.count;
  if (n == 0)
    return bs_write(b, "{}", 2);
  if (bs_putc(b, '{') < 0)
    return -1;
  for (size_t i = 0; i < n; ++i) {
    if (i) {
      if (bs_write(b, ", ", 2) < 0)
        return -1;
    }
    json_object *ent = &v->u.object.items[i];
    if (print_string_escaped_buf(b, ent->ptr, ent->len) < 0)
      return -1;
    if (bs_write(b, ": ", 2) < 0)
      return -1;
    if (print_value_compact_buf(ent->value, b) < 0)
      return -1;
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
    /* write number into buffer using printf */
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

static int bs_write(bs *b, const char *data, int len) {
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

static int bs_putc(bs *b, char c) {
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
  if (a->u.array.count != b->u.array.count)
    return false;
  for (size_t i = 0; i < a->u.array.count; ++i) {
    if (!json_equal(a->u.array.items[i], b->u.array.items[i]))
      return false;
  }
  return true;
}

static bool json_object_equal(const json_value *a, const json_value *b) {
  if (!a || !b)
    return a == b;
  if (a->type != J_OBJECT || b->type != J_OBJECT)
    return false;
  if (a->u.object.count != b->u.object.count)
    return false;
  for (size_t i = 0; i < a->u.object.count; ++i) {
    json_object *e = &a->u.object.items[i];
    json_value *bv = json_object_get(b, e->ptr, e->len);
    if (!bv)
      return false;
    if (!json_equal(e->value, bv))
      return false;
  }
  return true;
}

/* --- public API --- */

const char *json_source(const json_value *v) {
  if (!v)
    return NULL;
  switch (v->type) {
  case J_NULL:
    return v->u.number.ptr;
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

void json_free(json_value *v) {
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
