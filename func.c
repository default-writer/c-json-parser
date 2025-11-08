#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple JSON structures (a dictionary + value types) */

/* Forward decl */
typedef struct json_value json_value;
typedef struct dict dict;

typedef enum {
    J_NULL,
    J_BOOL,
    J_NUMBER,
    J_STRING,
    J_ARRAY,
    J_OBJECT
} json_type;

struct json_value {
    json_type type;
    union {
        bool b;
        double num;
        char *str;            /* heap allocated */
        struct {
            json_value **items;
            size_t count;
        } array;
        dict *object;         /* dictionary for object */
    } u;
};

/* Simple hash table (separate chaining) for string keys */
typedef struct dict_entry {
    char *key;
    json_value *value;
    struct dict_entry *next;
} dict_entry;

struct dict {
    dict_entry **buckets;
    size_t nbuckets;
    size_t nitems;
};

/* --- hash function as requested --- */
/* Collapsing 4-byte blocks: xor with a magic then add sum of bytes, iterate */
static uint32_t string_hash(const char *s) {
    const uint8_t *p = (const uint8_t *)s;
    uint32_t h = 0x9e3779b9u; /* magic */
    while (*p) {
        uint32_t block = 0;
        uint32_t sum = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t c = *p ? *p++ : 0;
            block |= ((uint32_t)c) << (i * 8);
            sum += c;
            if (c == 0) break;
        }
        h ^= block;
        h += sum;
        /* small avalanche */
        h = (h << 7) | (h >> (32 - 7));
    }
    return h;
}

/* --- dict helpers --- */
static dict *dict_new_size(size_t nbuckets) {
    dict *d = malloc(sizeof(*d));
    if (!d) return NULL;
    d->nbuckets = nbuckets;
    d->nitems = 0;
    d->buckets = calloc(nbuckets, sizeof(dict_entry *));
    if (!d->buckets) { free(d); return NULL; }
    return d;
}

static dict *dict_new(void) {
    return dict_new_size(101); /* small prime */
}

static void free_json_value(json_value *v); /* forward */

static void dict_free(dict *d) {
    if (!d) return;
    for (size_t i = 0; i < d->nbuckets; ++i) {
        dict_entry *e = d->buckets[i];
        while (e) {
            dict_entry *nx = e->next;
            free(e->key);
            free_json_value(e->value);
            free(e);
            e = nx;
        }
    }
    free(d->buckets);
    free(d);
}

static bool dict_set(dict *d, const char *key, json_value *value) {
    if (!d || !key) return false;
    uint32_t h = string_hash(key);
    size_t idx = h % d->nbuckets;
    dict_entry *e = d->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            /* replace value */
            free_json_value(e->value);
            e->value = value;
            return true;
        }
        e = e->next;
    }
    /* insert new */
    dict_entry *ne = malloc(sizeof(*ne));
    if (!ne) return false;
    ne->key = malloc(strlen(key) + 1);
    if (!ne->key) { free(ne); return false; }
    strcpy(ne->key, key);
    ne->value = value;
    ne->next = d->buckets[idx];
    d->buckets[idx] = ne;
    d->nitems++;
    return true;
}

json_value *dict_get(dict *d, const char *key) {
    if (!d || !key) return NULL;
    uint32_t h = string_hash(key);
    size_t idx = h % d->nbuckets;
    dict_entry *e = d->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) return e->value;
        e = e->next;
    }
    return NULL;
}
 
/* --- json_value helpers --- */
static json_value *json_new_null(void) {
    json_value *v = calloc(1, sizeof(*v));
    if (!v) return NULL;
    v->type = J_NULL;
    return v;
}

static json_value *json_new_bool(bool b) {
    json_value *v = calloc(1, sizeof(*v));
    if (!v) return NULL;
    v->type = J_BOOL;
    v->u.b = b;
    return v;
}

static json_value *json_new_number(double num) {
    json_value *v = calloc(1, sizeof(*v));
    if (!v) return NULL;
    v->type = J_NUMBER;
    v->u.num = num;
    return v;
}

/* json_new_string removed (unused) — use parse_string_value or json_new_string-like logic where needed */

static json_value *json_new_array(void) {
    json_value *v = calloc(1, sizeof(*v));
    if (!v) return NULL;
    v->type = J_ARRAY;
    v->u.array.items = NULL;
    v->u.array.count = 0;
    return v;
}

static json_value *json_new_object(void) {
    json_value *v = calloc(1, sizeof(*v));
    if (!v) return NULL;
    v->type = J_OBJECT;
    v->u.object = dict_new();
    if (!v->u.object) { free(v); return NULL; }
    return v;
}

static void json_array_push(json_value *arr, json_value *item) {
    if (!arr || arr->type != J_ARRAY) return;
    size_t n = arr->u.array.count;
    json_value **newitems = realloc(arr->u.array.items, (n + 1) * sizeof(json_value *));
    if (!newitems) return;
    arr->u.array.items = newitems;
    arr->u.array.items[n] = item;
    arr->u.array.count = n + 1;
}

static void free_json_value(json_value *v) {
    if (!v) return;
    switch (v->type) {
    case J_STRING:
        free(v->u.str);
        break;
    case J_ARRAY:
        for (size_t i = 0; i < v->u.array.count; ++i) free_json_value(v->u.array.items[i]);
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
    while (isspace((unsigned char)**s)) (*s)++;
}

/* parse string and return allocated json_value (string) */
static json_value *parse_string_value(const char **s) {
    if (**s != '"') return NULL;
    const char *p = *s + 1;
    (void)p; /* keep compiler quiet if p temporarily unused in some builds */
    /* we need to process escapes, we'll build a dynamic buffer */
    char *buf = malloc(1);
    size_t blen = 0;
    if (!buf) return NULL;
    /* ensure empty buffer is a valid C string */
    buf[0] = '\0';
    while (*p && *p != '"') {
        if ((unsigned char)*p < 0x20) { free(buf); return NULL; }
        if (*p == '\\') {
            p++;
            if (!*p) { free(buf); return NULL; }
            char out = 0;
            if (*p == 'u') {
                /* simplistic: skip \uXXXX and copy as-is (no unicode decode) */
                p++;
                for (int i = 0; i < 4; ++i) {
                    if (!isxdigit((unsigned char)*p)) { free(buf); return NULL; }
                    p++;
                }
                /* keep the literal sequence (not decoded) */
                /* Append nothing for simplicity */
                continue;
            } else {
                switch (*p) {
                    case '"': out = '"'; break;
                    case '\\': out = '\\'; break;
                    case '/': out = '/'; break;
                    case 'b': out = '\b'; break;
                    case 'f': out = '\f'; break;
                    case 'n': out = '\n'; break;
                    case 'r': out = '\r'; break;
                    case 't': out = '\t'; break;
                    default:
                        free(buf); return NULL;
                }
                /* append out */
                char *nb = realloc(buf, blen + 1 + 1);
                if (!nb) { free(buf); return NULL; }
                buf = nb;
                buf[blen++] = out;
                buf[blen] = '\0';
                p++;
                continue;
            }
        } else {
            char *nb = realloc(buf, blen + 1 + 1);
            if (!nb) { free(buf); return NULL; }
            buf = nb;
            buf[blen++] = *p++;
            buf[blen] = '\0';
        }
    }
    if (*p != '"') { free(buf); return NULL; }
    p++; /* skip closing quote */
    *s = p;
    json_value *v = calloc(1, sizeof(*v));
    if (!v) { free(buf); return NULL; }
    v->type = J_STRING;
    /* shrink to fit */
    char *shr = realloc(buf, blen + 1);
    if (!shr) { free(buf); free(v); return NULL; }
    shr[blen] = '\0';
    v->u.str = shr;
    return v;
}

/* parse number and return json_value */
static json_value *parse_number_value(const char **s) {
    const char *p = *s;
    char *end = NULL;
    double val = strtod(p, &end);
    if (end == p) return NULL;
    *s = end;
    return json_new_number(val);
}

/* forward */
static json_value *parse_value_build(const char **s);

static json_value *parse_array_value(const char **s) {
    if (**s != '[') return NULL;
    (*s)++;
    skip_ws(s);
    json_value *arr = json_new_array();
    if (!arr) return NULL;
    if (**s == ']') { (*s)++; return arr; }
    while (1) {
        skip_ws(s);
        json_value *elem = parse_value_build(s);
        if (!elem) { free_json_value(arr); return NULL; }
        json_array_push(arr, elem);
        skip_ws(s);
        if (**s == ',') { (*s)++; continue; }
        if (**s == ']') { (*s)++; return arr; }
        free_json_value(arr);
        return NULL;
    }
}

static json_value *parse_object_value(const char **s) {
    if (**s != '{') return NULL;
    (*s)++;
    skip_ws(s);
    json_value *obj = json_new_object();
    if (!obj) return NULL;
    if (**s == '}') { (*s)++; return obj; }
    while (1) {
        skip_ws(s);
        /* parse key string - reuse parse_string_value but ensure we get raw string (no escapes stored) */
        if (**s != '"') { dict_free(obj->u.object); free(obj); return NULL; }
        json_value *k = parse_string_value(s);
        if (!k || k->type != J_STRING) { free_json_value(k); dict_free(obj->u.object); free(obj); return NULL; }
        skip_ws(s);
        if (**s != ':') { free_json_value(k); dict_free(obj->u.object); free(obj); return NULL; }
        (*s)++;
        skip_ws(s);
        json_value *val = parse_value_build(s);
        if (!val) { free_json_value(k); dict_free(obj->u.object); free(obj); return NULL; }
        dict_set(obj->u.object, k->u.str, val);
        free_json_value(k); /* key string copy already used by dict_set (it duplicates key) */
        skip_ws(s);
        if (**s == ',') { (*s)++; continue; }
        if (**s == '}') { (*s)++; return obj; }
        dict_free(obj->u.object); free(obj);
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

static json_value *parse_value_build(const char **s) {
    skip_ws(s);
    if (**s == '"') return parse_string_value(s);
    if (**s == '{') return parse_object_value(s);
    if (**s == '[') return parse_array_value(s);
    if (**s == 'n') {
        if (match_literal_build(s, "null")) return json_new_null();
        return NULL;
    }
    if (**s == 't') {
        if (match_literal_build(s, "true")) return json_new_bool(true);
        return NULL;
    }
    if (**s == 'f') {
        if (match_literal_build(s, "false")) return json_new_bool(false);
        return NULL;
    }
    if (**s == '-' || isdigit((unsigned char)**s)) return parse_number_value(s);
    return NULL;
}

/* --- public API --- */

/* original validator function retained for compatibility */
bool func(const char *json) {
    if (!json) return false;
    const char *p = json;
    skip_ws(&p);
    /* use build parser only for validation here */
    const char *chk = p;
    json_value *v = parse_value_build(&chk);
    if (!v) return false;
    skip_ws(&chk);
    if (*chk != '\0') { free_json_value(v); return false; }
    free_json_value(v);
    return true;
}

/* New function:
   parse JSON string and return a dict* (object) representing the top-level object.
   If the top-level JSON is not an object, it will be returned inside a new dictionary
   under the empty-string key ("").
   Caller is responsible for calling dict_free() on the returned pointer.
   Returns NULL on error.
*/
dict *func_parse_to_dict(const char *json) {
    if (!json) return NULL;
    const char *p = json;
    skip_ws(&p);
    json_value *root = parse_value_build(&p);
    if (!root) return NULL;
    skip_ws(&p);
    if (*p != '\0') { free_json_value(root); return NULL; }
    if (root->type == J_OBJECT) {
        dict *d = root->u.object;
        free(root); /* free wrapper but not dict */
        return d;
    } else {
        dict *d = dict_new();
        if (!d) { free_json_value(root); return NULL; }
        /* store root under empty key */
        dict_set(d, "", root);
        return d;
    }
}

/* Example small helper to free dict that may have been returned by func_parse_to_dict */
/* (dict_free is already provided above) */

/* End of file */