#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple JSON structures (a dictionary + value types) */

/* Forward declarations */
typedef struct json_value json_value;
typedef struct dict dict;

typedef enum {
  J_NULL,
  J_BOOLEAN,
  J_NUMBER,
  J_STRING,
  J_ARRAY,
  J_OBJECT
} json_type;

/* JSON value with references back to original text */
struct json_value {
  json_type type;
  union {
    struct {
      const char *ptr; /* start of this value in source JSON */
      size_t len;
    } string;
    struct {
      const char *ptr; /* start of this value in source JSON */
      size_t len;
    } boolean;
    struct {
      const char *ptr; /* start of this value in source JSON */
      size_t len;
    } number;
    struct {
      json_value **items;
      size_t count;
    } array;
    dict *object;
  } u;
};

/* Dictionary entry referencing original JSON */
typedef struct dict_entry {
  const char *key;   /* reference to key in original JSON */
  size_t len;        /* length of entire value in source */
  json_value *value; /* owned value with refs to original JSON */
} dict_entry;

/* Root dictionary owns the original JSON string */
struct dict {
  const char *json;    /* owned original JSON string */
  dict_entry *entries; /* dynamic array of entries in insertion order */
  size_t count;        /* number of used entries */
  size_t cap;          /* capacity of entries array */
};

static const size_t DICT_GROW = 16;

/* --- dict helpers --- */
static dict *dict_new_size(size_t initial) {
  (void)initial;
  dict *d = calloc(1, sizeof(*d));
  if (!d)
    return NULL;
  d->count = 0;
  d->cap = DICT_GROW;
  d->entries = calloc(d->cap, sizeof(dict_entry));
  if (!d->entries) {
    free(d);
    return NULL;
  }
  return d;
}

static dict *dict_new(void) { return dict_new_size(0); }

static void free_json_value(json_value *v); /* forward */

static void dict_free(dict *d) {
  if (!d)
    return;
  for (size_t i = 0; i < d->count; ++i) {
    /* free duplicated key buffer allocated in dict_set */
    if (d->entries[i].key)
      free((void *)d->entries[i].key);
    free_json_value(d->entries[i].value);
  }
  free(d->entries);
  free(d);
}

/* set (insert or replace) — duplicates key, takes ownership of 'value' */
static bool dict_set(dict *d, const char *key, int len, json_value *value) {
  if (!d || !key)
    return false;
  for (size_t i = 0; i < d->count; ++i) {
    if (strcmp(d->entries[i].key, key) == 0) {
      free_json_value(d->entries[i].value);
      d->entries[i].value = value;
      return true;
    }
  }
  /* ensure capacity */
  if (d->count == d->cap) {
    size_t ncap = d->cap ? d->cap * 2 : DICT_GROW;
    dict_entry *ne = realloc(d->entries, ncap * sizeof(dict_entry));
    if (!ne)
      return false;
    memset(ne + d->cap, 0, (ncap - d->cap) * sizeof(dict_entry));
    d->entries = ne;
    d->cap = ncap;
  }
  /* duplicate key (used for callers that pass literals) */
  d->entries[d->count].key = key;
  d->entries[d->count].len = len;
  d->entries[d->count].value = value;
  d->count++;
  return true;
}

/* set (insert) taking ownership of 'keyptr' (heap buffer) and of 'value' */
static bool dict_set_take_key(dict *d, char *keyptr, size_t keylen,
                              json_value *value) {
  if (!d || !keyptr)
    return false;
  /* check existing keys by comparing strings */
  for (size_t i = 0; i < d->count; ++i) {
    if (d->entries[i].key && strcmp(d->entries[i].key, keyptr) == 0) {
      /* replace existing value, free incoming key */
      free(keyptr);
      free_json_value(d->entries[i].value);
      d->entries[i].value = value;
      return true;
    }
  }
  /* ensure capacity */
  if (d->count == d->cap) {
    size_t ncap = d->cap ? d->cap * 2 : DICT_GROW;
    dict_entry *ne = realloc(d->entries, ncap * sizeof(dict_entry));
    if (!ne) {
      free(keyptr);
      return false;
    }
    memset(ne + d->cap, 0, (ncap - d->cap) * sizeof(dict_entry));
    d->entries = ne;
    d->cap = ncap;
  }
  d->entries[d->count].key = keyptr;
  d->entries[d->count].len = keylen;
  d->entries[d->count].value = value;
  d->count++;
  return true;
}

/* get by key (linear search) */
json_value *dict_get(dict *d, const char *key) {
  if (!d || !key)
    return NULL;
  for (size_t i = 0; i < d->count; ++i) {
    if (strcmp(d->entries[i].key, key) == 0)
      return d->entries[i].value;
  }
  return NULL;
}

/* --- json_value helpers --- */
static json_value *json_new_null(void) {
  json_value *v = calloc(1, sizeof(*v));
  if (!v)
    return NULL;
  v->type = J_NULL;
  return v;
}

static json_value *json_new_boolean(const char *ptr, size_t len) {
  json_value *v = calloc(1, sizeof(*v));
  if (!v)
    return NULL;
  v->type = J_BOOLEAN;
  v->u.boolean.ptr = ptr;
  v->u.boolean.len = len;
  return v;
}

static json_value *json_new_number(const char *ptr, size_t len) {
  json_value *v = calloc(1, sizeof(*v));
  if (!v)
    return NULL;
  v->type = J_NUMBER;
  v->u.number.ptr = ptr;
  v->u.number.len = len;
  return v;
}

/* json_new_string removed (unused) — use parse_string_value or
 * json_new_string-like logic where needed */

static json_value *json_new_array(void) {
  json_value *v = calloc(1, sizeof(*v));
  if (!v)
    return NULL;
  v->type = J_ARRAY;
  v->u.array.items = NULL;
  v->u.array.count = 0;
  return v;
}

static json_value *json_new_object(void) {
  json_value *v = calloc(1, sizeof(*v));
  if (!v)
    return NULL;
  v->type = J_OBJECT;
  v->u.object = dict_new();
  if (!v->u.object) {
    free(v);
    return NULL;
  }
  return v;
}

static void json_array_push(json_value *arr, json_value *item) {
  if (!arr || arr->type != J_ARRAY)
    return;
  size_t n = arr->u.array.count;
  json_value **newitems =
      realloc(arr->u.array.items, (n + 1) * sizeof(json_value *));
  if (!newitems)
    return;
  arr->u.array.items = newitems;
  arr->u.array.items[n] = item;
  arr->u.array.count = n + 1;
}

static void free_json_value(json_value *v) {
  if (!v)
    return;
  switch (v->type) {
  case J_STRING:
    free((void *)v->u.string.ptr);
    break;
  case J_ARRAY:
    for (size_t i = 0; i < v->u.array.count; ++i)
      free_json_value(v->u.array.items[i]);
    free(v->u.array.items);
    break;
  case J_OBJECT:
    dict_free(v->u.object);
    break;
  default:
    break;
  }
  free(v);
}

/* --- parsing that builds the structure --- */

static void skip_ws(const char **s) {
  while (isspace((unsigned char)**s))
    (*s)++;
}

/* parse string and return allocated json_value (string) */
static json_value *parse_string_value(const char **s) {
  if (**s != '"')
    return NULL;
  const char *p = *s + 1;
  (void)p; /* keep compiler quiet if p temporarily unused in some builds */
  /* we need to process escapes, we'll build a dynamic buffer */
  char *buf = calloc(1, 1);
  size_t blen = 0;
  if (!buf)
    return NULL;
  /* ensure empty buffer is a valid C string */
  buf[0] = '\0';
  while (*p && *p != '"') {
    if ((unsigned char)*p < 0x20) {
      free(buf);
      return NULL;
    }
    if (*p == '\\') {
      p++;
      if (!*p) {
        free(buf);
        return NULL;
      }
      char out = 0;
      if (*p == 'u') {
        /* simplistic: keep \uXXXX as-is (no unicode decode) */
        p++; /* move past 'u' */
        char *nb = realloc(buf, blen + 6 + 1);
        if (!nb) {
          free(buf);
          return NULL;
        }
        buf = nb;
        buf[blen++] = '\\';
        buf[blen++] = 'u';
        for (int i = 0; i < 4; ++i) {
          if (!isxdigit((unsigned char)*p)) {
            free(buf);
            return NULL;
          }
          buf[blen++] = *p++;
        }
        buf[blen] = '\0';
        continue;
      } else {
        switch (*p) {
        case '"':
          out = '"';
          break;
        case '\\':
          out = '\\';
          break;
        case '/':
          out = '/';
          break;
        case 'b':
          out = '\b';
          break;
        case 'f':
          out = '\f';
          break;
        case 'n':
          out = '\n';
          break;
        case 'r':
          out = '\r';
          break;
        case 't':
          out = '\t';
          break;
        default:
          free(buf);
          return NULL;
        }
        /* append out */
        char *nb = realloc(buf, blen + 1 + 1);
        if (!nb) {
          free(buf);
          return NULL;
        }
        buf = nb;
        buf[blen++] = out;
        buf[blen] = '\0';
        p++;
        continue;
      }
    } else {
      char *nb = realloc(buf, blen + 1 + 1);
      if (!nb) {
        free(buf);
        return NULL;
      }
      buf = nb;
      buf[blen++] = *p++;
      buf[blen] = '\0';
    }
  }
  if (*p != '"') {
    free(buf);
    return NULL;
  }
  p++; /* skip closing quote */
  *s = p;
  json_value *v = calloc(1, sizeof(*v));
  if (!v) {
    free(buf);
    return NULL;
  }
  v->type = J_STRING;
  /* shrink to fit */
  char *shr = realloc(buf, blen + 1);
  if (!shr) {
    free(buf);
    free(v);
    return NULL;
  }
  shr[blen] = '\0';
  v->u.string.ptr = shr;
  v->u.string.len = blen;
  return v;
}

/* parse number and return json_value */
static json_value *parse_number_value(const char **s) {
  const char *p = *s;
  char *end = NULL;
  strtod(p, &end);
  if (end == p)
    return NULL;
  *s = end;
  return json_new_number(p, (size_t)(end - p));
}

/* forward */
static json_value *parse_value_build(const char **s, int id);

/* forward declare print_value so it can be used before its definition */
void print_value(const json_value *v, int indent, FILE *out);

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
      free_json_value(arr);
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
    free_json_value(arr);
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
    /* parse key string - reuse parse_string_value but ensure we get raw string
     * (no escapes stored) */
    if (**s != '"') {
      dict_free(obj->u.object);
      free(obj);
      return NULL;
    }
    json_value *k = parse_string_value(s);
    if (!k || k->type != J_STRING) {
      free_json_value(k);
      dict_free(obj->u.object);
      free(obj);
      return NULL;
    }
    skip_ws(s);
    if (**s != ':') {
      /* k contains an allocated key buffer in k->ptr; free wrapper only */
      free(k);
      dict_free(obj->u.object);
      free(obj);
      return NULL;
    }
    (*s)++;
    skip_ws(s);
    json_value *val = parse_value_build(s, ++id);
    if (!val) {
      /* take care: k->ptr is an allocated buffer; free wrapper and value */
      free(k);
      dict_free(obj->u.object);
      free(obj);
      return NULL;
    }

    /* Take ownership of the parsed key buffer (no extra duplication). */
    if (!dict_set_take_key(obj->u.object, (char *)k->u.string.ptr,
                           k->u.string.len, val)) {
      /* insertion failed: free transferred resources */
      free_json_value(val);
      free(k);
      dict_free(obj->u.object);
      free(obj);
      return NULL;
    }
    /* free only the json_value wrapper for the key (do NOT free key.ptr) */
    free(k);

    skip_ws(s);
    if (**s == ',') {
      (*s)++;
      continue;
    }
    if (**s == '}') {
      (*s)++;
      return obj;
    }
    dict_free(obj->u.object);
    free(obj);
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
    if (match_literal_build(s, "true"))
      return json_new_boolean(*s - 4, 4);
    return NULL;
  }
  if (**s == 'f') {
    if (match_literal_build(s, "false"))
      return json_new_boolean(*s - 5, 5);
    return NULL;
  }
  if (**s == '-' || isdigit((unsigned char)**s))
    return parse_number_value(s);
  return NULL;
}

/* --- public API --- */

/* original validator function retained for compatibility */
bool func(const char *json) {
  if (!json)
    return false;
  const char *p = json;
  skip_ws(&p);
  /* use build parser only for validation here */
  const char *chk = p;
  json_value *v = parse_value_build(&chk, 0);
  if (!v)
    return false;
  skip_ws(&chk);
  if (*chk != '\0') {
    free_json_value(v);
    return false;
  }
  free_json_value(v);
  return true;
}

/* New function:
   parse JSON string and return a dict* (object) representing the top-level
   object. If the top-level JSON is not an object, it will be returned inside a
   new dictionary under the empty-string key (""). Caller is responsible for
   calling dict_free() on the returned pointer. Returns NULL on error.
*/
dict *func_parse_to_dict(const char *json) {
  if (!json)
    return NULL;
  const char *p = json;
  skip_ws(&p);

  /* parse entire JSON into an in-memory json_value tree */
  json_value *root = parse_value_build(&p, 0);
  if (!root)
    return NULL;

  // Debug print
  printf("\n--- parsed tree ---\n");
  print_value(root, 0, stdout);
  printf("\n------------------------\n");

  /* ensure there is no trailing garbage */
  skip_ws(&p);
  if (*p != '\0') {
    free_json_value(root);
    return NULL;
  }

  /* if top-level is an object use its dict directly (transfer ownership) */
  if (root->type == J_OBJECT) {
    dict *d = root->u.object;
    /* free the wrapper json_value but not the dict */
    free(root);
    return d;
  }

  /* otherwise wrap the single value into a new dict under the empty-string key
   */
  dict *d = dict_new();
  if (!d) {
    free_json_value(root);
    return NULL;
  }

  if (!dict_set(d, json, 0, root)) {
    /* dict_set failed and will not take ownership of root */
    free_json_value(root);
    dict_free(d);
    return NULL;
  }

  return d;
}

/* --- pretty-print helpers --- */

static void print_indent(FILE *out, int indent) {
  for (int i = 0; i < indent; ++i)
    fputs("    ", out); /* 4 spaces */
}

static void print_string_escaped(FILE *out, const char *s) {
  fputc('"', out);
  for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
    unsigned char c = *p;
    switch (c) {
    case '"':
      fputs("\\\"", out);
      break;
    case '\\':
      fputs("\\\\", out);
      break;
    case '\b':
      fputs("\\b", out);
      break;
    case '\f':
      fputs("\\f", out);
      break;
    case '\n':
      fputs("\\n", out);
      break;
    case '\r':
      fputs("\\r", out);
      break;
    case '\t':
      fputs("\\t", out);
      break;
    default:
      if (c < 0x20) {
        fprintf(out, "\\u%04x", c);
      } else {
        fputc(c, out);
      }
    }
  }
  fputc('"', out);
}

/* compact printer: prints a value without newlines or indentation (used for
   single-line arrays and when a compact representation is required inside
   arrays) */
static void print_value_compact(const json_value *v, FILE *out);

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
  for (size_t i = 0; i < v->u.object->count; ++i) {
    if (i)
      fputs(", ", out);
    dict_entry *e = &v->u.object->entries[i];
    print_string_escaped(out, e->key);
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
    print_string_escaped(out, v->u.string.ptr ? v->u.string.ptr : "");
    break;
  case J_ARRAY:
    print_array_compact(v, out);
    break;
  case J_OBJECT:
    print_object_compact(v, out);
    break;
  }
}

/* pretty printer: prints values with indentation and newlines; arrays are
   always printed as single line per request */
void print_value(const json_value *v, int indent, FILE *out) {
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
    print_string_escaped(out, v->u.string.ptr ? v->u.string.ptr : "");
    break;
  case J_ARRAY:
    /* arrays printed single-line */
    print_array_compact(v, out);
    break;
  case J_OBJECT: {
    fputs("{\n", out); /* keep object starting symbol on its own line with
                          properties below */
    for (size_t i = 0; i < v->u.object->count; ++i) {
      if (i)
        fputs(",\n", out);
      print_indent(out, indent + 1);
      dict_entry *e = &v->u.object->entries[i];
      print_string_escaped(out, e->key);
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

/* Replace func_print_dict(FILE*) with buffer-based implementation.
   New signature: int func_print_dict(const dict *d, char *buf, int bufsize);
   Returns number of bytes written (excluding trailing NUL) or -1 if buffer is
   too small.
*/

/* small buffer state (used by buffer-based printers) */
typedef struct {
  char *buf;
  int cap;
  int pos;
} bs;

/* --- buffer helpers --- */
static int bs_write(bs *b, const char *data, int len) {
  if (len <= 0)
    return 0;
  if (b->pos + len >= b->cap)
    return -1;
  memcpy(b->buf + b->pos, data, (size_t)len);
  b->pos += len;
  return 0;
}

static int bs_putc(bs *b, char c) {
  if (b->pos + 1 >= b->cap)
    return -1;
  b->buf[b->pos++] = c;
  return 0;
}

static int bs_printf(bs *b, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  va_list ap2;
  va_copy(ap2, ap);
  int needed = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (needed < 0) {
    va_end(ap2);
    return -1;
  }
  if (b->pos + needed >= b->cap) {
    va_end(ap2);
    return -1;
  }
  int wrote = vsnprintf(b->buf + b->pos, (size_t)(b->cap - b->pos), fmt, ap2);
  va_end(ap2);
  if (wrote < 0)
    return -1;
  b->pos += wrote;
  return 0;
}

/* buffer-based printers (mirror of FILE-based ones) */
static int print_indent_buf(bs *b, int indent) {
  for (int i = 0; i < indent; ++i) {
    if (bs_write(b, "    ", 4) < 0)
      return -1;
  }
  return 0;
}

static int print_string_escaped_buf(bs *b, const char *s) {
  if (bs_putc(b, '"') < 0)
    return -1;
  for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
    unsigned char c = *p;
    switch (c) {
    case '"':
      if (bs_write(b, "\\\"", 2) < 0)
        return -1;
      break;
    case '\\':
      if (bs_write(b, "\\\\", 2) < 0)
        return -1;
      break;
    case '\b':
      if (bs_write(b, "\\b", 2) < 0)
        return -1;
      break;
    case '\f':
      if (bs_write(b, "\\f", 2) < 0)
        return -1;
      break;
    case '\n':
      if (bs_write(b, "\\n", 2) < 0)
        return -1;
      break;
    case '\r':
      if (bs_write(b, "\\r", 2) < 0)
        return -1;
      break;
    case '\t':
      if (bs_write(b, "\\t", 2) < 0)
        return -1;
      break;
    default:
      if (c < 0x20) {
        if (bs_printf(b, "\\u%04x", c) < 0)
          return -1;
      } else {
        if (bs_putc(b, c) < 0)
          return -1;
      }
    }
  }
  if (bs_putc(b, '"') < 0)
    return -1;
  return 0;
}

/* forward declarations for functions/vars used before their definitions */
static int print_object_entries_buf(const dict *obj, bs *b, int indent,
                                    int compact);
static int print_value_compact_buf(const json_value *v, bs *b);
static int print_value_buf(const json_value *v, int indent, bs *b);
/* global toggle to request preserving insertion order during printing */

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
static int print_object_entries_buf(const dict *obj, bs *b, int indent,
                                    int compact);

static int print_object_entries_buf(const dict *obj, bs *b, int indent,
                                    int compact) {
  if (!obj) {
    return compact ? bs_write(b, "{}", 2) : bs_write(b, "{\n\n}", 4);
  }

  size_t n = obj->count;
  if (n == 0) {
    if (compact)
      return bs_write(b, "{}", 2);
    if (bs_write(b, "{\n", 2) < 0)
      return -1;
    if (bs_putc(b, '\n') < 0)
      return -1;
    if (print_indent_buf(b, indent) < 0)
      return -1;
    if (bs_putc(b, '}') < 0)
      return -1;
    return 0;
  }

  int rc = 0;
  if (compact) {
    if (bs_putc(b, '{') < 0)
      return -1;
    for (size_t i = 0; i < n; ++i) {
      if (i) {
        if (bs_write(b, ", ", 2) < 0)
          return -1;
      }
      dict_entry *ent = &obj->entries[i];
      if (print_string_escaped_buf(b, ent->key) < 0)
        return -1;
      if (bs_write(b, ": ", 2) < 0)
        return -1;
      if (print_value_compact_buf(ent->value, b) < 0)
        return -1;
    }
    if (bs_putc(b, '}') < 0)
      rc = -1;
  } else {
    if (bs_write(b, "{\n", 2) < 0)
      return -1;
    for (size_t i = 0; i < n; ++i) {
      if (i) {
        if (bs_write(b, ",\n", 2) < 0)
          return -1;
      }
      if (print_indent_buf(b, indent + 1) < 0)
        return -1;
      dict_entry *ent = &obj->entries[i];
      if (print_string_escaped_buf(b, ent->key) < 0)
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
  }
  return rc;
}

static int print_object_compact_buf(const json_value *v, bs *b) {
  if (!v || v->type != J_OBJECT)
    return bs_write(b, "{}", 2);
  const dict *obj = v->u.object;
  size_t n = obj->count;
  if (n == 0)
    return bs_write(b, "{}", 2);
  if (bs_putc(b, '{') < 0)
    return -1;
  for (size_t i = 0; i < n; ++i) {
    if (i) {
      if (bs_write(b, ", ", 2) < 0)
        return -1;
    }
    dict_entry *ent = &obj->entries[i];
    if (print_string_escaped_buf(b, ent->key) < 0)
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
    return bs_write(b, "null", 4);
  switch (v->type) {
  case J_NULL:
    return bs_write(b, "null", 4);
  case J_BOOLEAN:
    return bs_write(b, v->u.boolean.ptr, (int)v->u.boolean.len);
  case J_NUMBER: {
    /* write number into buffer using printf */
    return bs_write(b, v->u.number.ptr, (int)v->u.number.len);
  }
  case J_STRING:
    return print_string_escaped_buf(b, v->u.string.ptr ? v->u.string.ptr : "");
  case J_ARRAY:
    return print_array_compact_buf(v, b);
  case J_OBJECT:
    return print_object_compact_buf(v, b);
  }
  return -1;
}

static int print_value_buf(const json_value *v, int indent, bs *b) {
  if (!v)
    return bs_write(b, "null", 4);
  switch (v->type) {
  case J_NULL:
    return bs_write(b, "null", 4);
  case J_BOOLEAN:
    return bs_write(b, v->u.boolean.ptr, (int)v->u.boolean.len);
  case J_NUMBER:
    return bs_write(b, v->u.number.ptr, (int)v->u.number.len);
  case J_STRING:
    return print_string_escaped_buf(b, v->u.string.ptr ? v->u.string.ptr : "");
  case J_ARRAY:
    /* arrays printed single-line */
    return print_array_compact_buf(v, b);
  case J_OBJECT:
    return print_object_entries_buf(v->u.object, b, indent, 0);
  }
  return -1;
}

/* new func_print_dict: write into provided buffer, return bytes written or -1
  if insufficient preserve_order flag added (non-zero means preserve input
  insertion order) */
int func_print_dict(const dict *d, char *buf, int bufsize) {
  if (!buf || bufsize <= 0)
    return -1;
  bs bstate = {buf, bufsize, 0};

  if (!d) {
    if (bs_write(&bstate, "null\n", 5) < 0)
      return -1;
  } else {
    json_value tmp;
    memset(&tmp, 0, sizeof(tmp));
    tmp.type = J_OBJECT;
    tmp.u.object = (dict *)d; /* we don't modify it */
    if (print_value_buf(&tmp, 0, &bstate) < 0)
      return -1;
    if (bs_putc(&bstate, '\n') < 0)
      return -1;
  }

  /* ensure NUL termination if space remains */
  if (bstate.pos < bstate.cap)
    bstate.buf[bstate.pos] = '\0';
  else
    return -1;
  return bstate.pos;
}

/* Update func_parse_to_string to accept preserve_order param and call new
 * func_print_dict */
char *func_parse_to_string(const dict *d) {
  if (!d)
    return NULL;
  int cap = 65536;
  char *buf = calloc(1, (size_t)cap);
  if (!buf)
    return NULL;
  int rc = func_print_dict(d, buf, cap);
  if (rc >= 0) {
    /* rc is bytes written (excluding NUL) */
    /* shrink to fit */
    char *shr = realloc(buf, (size_t)rc + 1);
    if (shr)
      buf = shr;
    return buf;
    free(buf);
  }
  return NULL;
}

/* new: public wrapper to free a dict returned by func_parse_to_dict */
void func_free_dict(dict *d) { dict_free(d); }

/* --- structural equality helpers --- */

/* forward declare dict_get if necessary (it's defined above in this file) */
json_value *dict_get(dict *d, const char *key);

static bool json_value_equal(const json_value *a, const json_value *b);

static bool json_array_equal(const json_value *a, const json_value *b) {
  if (!a || !b)
    return a == b;
  if (a->type != J_ARRAY || b->type != J_ARRAY)
    return false;
  if (a->u.array.count != b->u.array.count)
    return false;
  for (size_t i = 0; i < a->u.array.count; ++i) {
    if (!json_value_equal(a->u.array.items[i], b->u.array.items[i]))
      return false;
  }
  return true;
}

static bool json_object_equal(const json_value *a, const json_value *b) {
  if (!a || !b)
    return a == b;
  if (a->type != J_OBJECT || b->type != J_OBJECT)
    return false;
  /* quick check: number of items should match */
  if (a->u.object->count != b->u.object->count)
    return false;
  /* iterate entries in insertion order and ensure b has same key with equal
   * value */
  for (size_t i = 0; i < a->u.object->count; ++i) {
    dict_entry *e = &a->u.object->entries[i];
    json_value *bv = dict_get(b->u.object, e->key);
    if (!bv)
      return false;
    if (!json_value_equal(e->value, bv))
      return false;
  }
  return true;
}

static bool json_value_equal(const json_value *a, const json_value *b) {
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
    return a->u.boolean.len == b->u.boolean.len &&
           strncmp(a->u.boolean.ptr, b->u.boolean.ptr, a->u.boolean.len) == 0;
  case J_NUMBER: {
    /* For numbers, compare floating point values, not text. */
    char *end_a;
    char *end_b;
    double val_a = strtod(a->u.number.ptr, &end_a);
    double val_b = strtod(b->u.number.ptr, &end_b);
    if ((size_t)(end_a - a->u.number.ptr) != a->u.number.len ||
        (size_t)(end_b - b->u.number.ptr) != b->u.number.len) {
      /* if strtod did not consume the entire string, it's not a valid float
       * or is a very large integer. Fall back to string comparison. */
      return a->u.number.len == b->u.number.len &&
             strncmp(a->u.number.ptr, b->u.number.ptr, a->u.number.len) == 0;
    }
    return val_a == val_b;
  }
  case J_STRING:
    if (a->u.string.ptr == NULL && b->u.string.ptr == NULL)
      return true;
    if (a->u.string.ptr == NULL || b->u.string.ptr == NULL)
      return false;
    return strcmp(a->u.string.ptr, b->u.string.ptr) == 0;
  case J_ARRAY:
    return json_array_equal(a, b);
  case J_OBJECT:
    return json_object_equal(a, b);
  default:
    return false;
  }
}
/* public API: compare two JSON texts for structural equality */
bool func_json_equal(const char *a, const char *b) {
  if (!a || !b)
    return false;
  const char *pa = a;
  const char *pb = b;
  skip_ws(&pa);
  skip_ws(&pb);
  json_value *va = parse_value_build(&pa, 0);
  if (!va)
    return false;
  json_value *vb = parse_value_build(&pb, 0);
  if (!vb) {
    free_json_value(va);
    return false;
  }
  skip_ws(&pa);
  skip_ws(&pb);
  if (*pa != '\0' || *pb != '\0') {
    free_json_value(va);
    free_json_value(vb);
    return false;
  }
  bool eq = json_value_equal(va, vb);

  if (!eq) {
    /* find first differing position in the original inputs (skip whitespace) */
    const char *xa = a;
    const char *xb = b;
    while (*xa || *xb) {
      while (isspace((unsigned char)*xa))
        xa++;
      while (isspace((unsigned char)*xb))
        xb++;
      if (*xa != *xb)
        break;
      if (*xa == '\0')
        break;
      xa++;
      xb++;
    }
    size_t off_a = (size_t)(xa - a);
    size_t off_b = (size_t)(xb - b);
    /* print brief context */
    size_t ctx_before = 2;
    size_t ctx_after = 40;
    size_t start_a = (off_a > ctx_before) ? off_a - ctx_before : 0;
    size_t start_b = (off_b > ctx_before) ? off_b - ctx_before : 0;
    fprintf(stderr, "JSON mismatch: first differing byte offsets a=%zu b=%zu\n",
            off_a, off_b);
    fprintf(stderr, "a context: \"");
    for (size_t i = start_a; i < off_a + ctx_after && a[i] != '\0'; ++i) {
      char c = a[i];
      if ((unsigned char)c < 0x20)
        fputc('.', stderr);
      else
        fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
    fprintf(stderr, "b context: \"");
    for (size_t i = start_b; i < off_b + ctx_after && b[i] != '\0'; ++i) {
      char c = b[i];
      if ((unsigned char)c < 0x20)
        fputc('.', stderr);
      else
        fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
  }

  free_json_value(va);
  free_json_value(vb);
  return eq;
}

/* End of file */