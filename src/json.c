#include "json.h"

#define PREFIX_CHAR_OFFSET 10
#define POSTFIX_CHAR_OFFSET 10
#define STATE_INITIAL 1
#define STATE_ESCAPE_START 2
#define STATE_ESCAPE_UNICODE_BYTE1 3
#define STATE_ESCAPE_UNICODE_BYTE2 4
#define STATE_ESCAPE_UNICODE_BYTE3 5
#define STATE_ESCAPE_UNICODE_BYTE4 6
#define TEXT_SIZE(name) (sizeof((name)) - 1)

/* JSON value with references back to original text */

static const size_t DICT_GROW = 16;

/* small buffer state (used by buffer-based printers) */
typedef struct {
  char *buf;
  int cap;
  int pos;
} bs;

static void free_json_value(json_value *v); /* forward */

/* set (insert) taking ownership of 'keyptr' (heap buffer) and of 'value' */
static bool json_object_set_take_key(json_value *obj, const char *ptr, size_t len, json_value *value) {
  if (!obj || obj->type != J_OBJECT || !ptr)
    return false;
  /* check existing keys by comparing strings */
  for (size_t i = 0; i < obj->u.object.count; ++i) {
    if (obj->u.object.items[i].ptr && strncmp(obj->u.object.items[i].ptr, ptr, len) == 0) {
      free_json_value(obj->u.object.items[i].value);
      obj->u.object.items[i].value = value;
      return true;
    }
  }
  /* ensure capacity */
  if (obj->u.object.count == obj->u.object.capacity) {
    size_t ncap = obj->u.object.capacity ? obj->u.object.capacity * 2 : DICT_GROW;
    json_object *ne = realloc(obj->u.object.items, ncap * sizeof(json_object));
    if (!ne) {
      return false;
    }
    obj->u.object.items = ne;
    obj->u.object.capacity = ncap;
  }
  obj->u.object.items[obj->u.object.count].ptr = ptr;
  obj->u.object.items[obj->u.object.count].len = len;
  obj->u.object.items[obj->u.object.count].value = value;
  obj->u.object.count++;
  return true;
}

/* get by key (linear search) */
json_value *json_object_get(const json_value *obj, const char *key, size_t len) {
  if (!obj || obj->type != J_OBJECT || !key)
    return NULL;
  for (size_t i = 0; i < obj->u.object.count; ++i) {
    if (strncmp(obj->u.object.items[i].ptr, key, len) == 0)
      return obj->u.object.items[i].value;
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

/* json_new_string-like logic where needed */

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
  v->u.object.items = NULL;
  v->u.object.count = 0;
  v->u.object.capacity = 0;
  return v;
}

static void json_array_push(json_value *arr, json_value *item) {
  if (!arr || arr->type != J_ARRAY || !item)
    return;
  if (arr->u.array.count == arr->u.array.capacity) {
    size_t ncap = arr->u.array.capacity ? arr->u.array.capacity * 2 : DICT_GROW;
    json_value *newitems = realloc(arr->u.array.items, ncap * sizeof(json_value));
    if (!newitems)
      return;
    arr->u.array.items = newitems;
    arr->u.array.capacity = ncap;
  }
  arr->u.array.items[arr->u.array.count++] = *item;
}

static void free_json_value_contents(json_value *v) {
  if (!v)
    return;
  switch (v->type) {
  case J_STRING:
    break;
  case J_ARRAY:
    for (size_t i = 0; i < v->u.array.count; ++i)
      free_json_value_contents(&(v->u.array.items[i]));
    free(v->u.array.items);
    break;
  case J_OBJECT:
    for (size_t i = 0; i < v->u.object.count; ++i)
      free_json_value(v->u.object.items[i].value);
    free(v->u.object.items);
    break;
  default:
    break;
  }
}

static void free_json_value(json_value *v) {
  if (!v)
    return;
  free_json_value_contents(v);
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
  json_value *v = calloc(1, sizeof(*v));
  if (!v) {
    return NULL;
  }
  v->type = J_STRING;
  v->u.string.ptr = ptr;
  v->u.string.len = len;
  return v;
}
/* parse string and return allocated json_value (string) */
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
    free(elem);
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
    if (**s != '"') {
      free_json_value(obj);
      return NULL;
    }
    reference ref = {*s, 0};
    if (!parse_string_value_ptr(&ref, s)) {
      free_json_value(obj);
      return NULL;
    }
    skip_ws(s);
    if (**s != ':') {
      free_json_value(obj);
      return NULL;
    }
    (*s)++;
    skip_ws(s);
    json_value *val = parse_value_build(s, ++id);
    if (!val) {
      free_json_value(obj);
      return NULL;
    }

    /* Take ownership of the parsed key buffer (no extra duplication). */
    if (!json_object_set_take_key(obj, ref.ptr, ref.len, val)) {
      /* insertion failed: free transferred resources */
      free_json_value(val);
      free_json_value(obj);
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
    free_json_value(obj);
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
   new dictionary under the empty-string key ("" ). Caller is responsible for
   calling dict_free() on the returned pointer. Returns NULL on error.
*/
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
    free_json_value(root);
    return NULL;
  }

  return root;
}

/* --- pretty-print helpers --- */

static void print_indent(FILE *out, int indent) {
  for (int i = 0; i < indent; ++i)
    fputs("    ", out); /* 4 spaces */
}

static void print_string_escaped(FILE *out, const char *s, size_t len) {
  fputc('"', out);
  size_t i = 0;
  for (const unsigned char *p = (const unsigned char *)s; *p && i < len; ++i, ++p) {
    unsigned char c = *p;
    fputc(c, out);
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
    print_value_compact(&(v->u.array.items[i]), out);
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

/* buffer-based printers (mirror of FILE-based ones) */
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

/* forward declarations for functions/vars used before their definitions */
static int print_object_buf(const json_value *v, bs *b, int indent);
static int print_value_compact_buf(const json_value *v, bs *b);
static int print_value_buf(const json_value *v, int indent, bs *b);

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
    if (print_value_compact_buf(&(v->u.array.items[i]), b) < 0)
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
    return bs_write(b, "null", 4);
  switch (v->type) {
  case J_NULL:
    return bs_write(b, "null", 4);
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
    return bs_write(b, "null", 4);
  switch (v->type) {
  case J_NULL:
    return bs_write(b, "null", 4);
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

/* new func_print_dict: write into provided buffer, return bytes written or -1
  if insufficient preserve_order flag added (non-zero means preserve input
  insertion order) */
int json_stringify_to_buffer(const json_value *v, char *buf, int bufsize) {
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

char *json_stringify(const json_value *v) {
  if (!v)
    return NULL;
  char *buf = calloc(1, (size_t)MAX_BUFFER_SIZE);
  if (!buf)
    return NULL;
  int rc = json_stringify_to_buffer(v, buf, MAX_BUFFER_SIZE);
  if (rc >= 0) {
    /* rc is bytes written (excluding NUL) */
    /* shrink to fit */
    char *shr = realloc(buf, (size_t)rc + 1);
    if (shr)
      buf = shr;
    return buf;
  }
  free(buf);
  return NULL;
}

void json_free(json_value *v) { free_json_value(v); }

/* --- structural equality helpers --- */

/* forward declare dict_get if necessary (it's defined above in this file) */

static bool json_value_equal(const json_value *a, const json_value *b);

static bool json_array_equal(const json_value *a, const json_value *b) {
  if (!a || !b)
    return a == b;
  if (a->type != J_ARRAY || b->type != J_ARRAY)
    return false;
  if (a->u.array.count != b->u.array.count)
    return false;
  for (size_t i = 0; i < a->u.array.count; ++i) {
    if (!json_value_equal(&(a->u.array.items[i]), &(b->u.array.items[i])))
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
  if (a->u.object.count != b->u.object.count)
    return false;
  /* iterate entries in insertion order and ensure b has same key with equal
   * value */
  for (size_t i = 0; i < a->u.object.count; ++i) {
    json_object *e = &a->u.object.items[i];
    json_value *bv = json_object_get(b, e->ptr, e->len);
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
    return a->u.boolean.len == b->u.boolean.len && strncmp(a->u.boolean.ptr, b->u.boolean.ptr, a->u.boolean.len) == 0;
  case J_NUMBER: {
    /* For numbers, compare floating point values, not text. */
    char *end_a;
    char *end_b;
    double val_a = strtod(a->u.number.ptr, &end_a);
    double val_b = strtod(b->u.number.ptr, &end_b);
    if ((size_t)(end_a - a->u.number.ptr) != a->u.number.len || (size_t)(end_b - b->u.number.ptr) != b->u.number.len) {
      /* if strtod did not consume the entire string, it's not a valid float
       * or is a very large integer. Fall back to string comparison. */
      return a->u.number.len == b->u.number.len && strncmp(a->u.number.ptr, b->u.number.ptr, a->u.number.len) == 0;
    }
    return val_a == val_b;
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
      xa++;
      xb++;
    }
    size_t off_a = (size_t)(xa - a);
    size_t off_b = (size_t)(xb - b);
    /* print brief context */
    size_t ctx_before = PREFIX_CHAR_OFFSET;
    size_t ctx_after = POSTFIX_CHAR_OFFSET;
    size_t start_a = (off_a > ctx_before) ? off_a - ctx_before : 0;
    size_t start_b = (off_b > ctx_before) ? off_b - ctx_before : 0;
    fprintf(stderr, "JSON mismatch: first differing byte offsets a=%zu b=%zu\n", off_a, off_b);
    fprintf(stderr, "a context: \"");
    for (size_t i = start_a; i < off_a + ctx_after && a[i] != '\0'; ++i) {
      char c = a[i];
      printf("a:%x\n", c);
      fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
    fprintf(stderr, "b context: \"");
    for (size_t i = start_b; i < off_b + ctx_after && b[i] != '\0'; ++i) {
      char c = b[i];
      printf("b:%x\n", c);
      fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
  }

  free_json_value(va);
  free_json_value(vb);
  return eq;
}

/* End of file */
