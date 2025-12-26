/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * microLISP with exJSON Notation - Implementation
 * Created: December 26, 2025
 */

#include "exjson.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_SYMBOL_LEN 256
#define ENV_POOL_SIZE 1024

/* Pool for environment frames */
#ifndef USE_ALLOC
static env_frame env_frame_pool[ENV_POOL_SIZE];
static env_frame *env_frame_free_pool[ENV_POOL_SIZE];
static size_t env_frame_free_count = ENV_POOL_SIZE;
static bool env_pool_initialized = false;
#endif

/* Forward declarations */
static json_value *eval_list(const json_value *list, env_frame *env);
static json_value *create_error(const char *message);

/* ========== Pool Initialization ========== */

static void exjson_init_pools(void) {
#ifndef USE_ALLOC
  if (!env_pool_initialized) {
    for (size_t i = 0; i < ENV_POOL_SIZE; i++) {
      env_frame_free_pool[i] = &env_frame_pool[i];
    }
    env_pool_initialized = true;
  }
#endif
}

/* ========== Utility Functions ========== */

char *exjson_ref_to_string(const reference *ref) {
  if (!ref || !ref->ptr)
    return NULL;
  char *str = (char *)malloc(ref->len + 1);
  if (!str)
    return NULL;
  memcpy(str, ref->ptr, ref->len);
  str[ref->len] = '\0';
  return str;
}

bool exjson_is_symbol(const json_value *v) {
  return v && v->type == J_SYMBOL;
}

bool exjson_is_truthy(const json_value *v) {
  if (!v || v->type == J_NULL)
    return false;
  if (v->type == J_BOOLEAN) {
    return v->u.boolean.len == 4 && strncmp(v->u.boolean.ptr, "true", 4) == 0;
  }
  return true;
}

json_value *exjson_copy_value(const json_value *v) {
  if (!v)
    return NULL;
  json_value *copy = (json_value *)calloc(1, sizeof(json_value));
  if (!copy)
    return NULL;
  copy->type = v->type;
  
  switch (v->type) {
  case J_NULL:
  case J_BOOLEAN:
  case J_NUMBER:
  case J_STRING:
    return exjson_copy_value(v);
  case J_SYMBOL:
    copy->u = v->u;
    break;
  case J_ARRAY: {
    json_array_node *src = v->u.array.items;
    json_array_node *prev = NULL;
    while (src) {
      json_array_node *node = (json_array_node *)calloc(1, sizeof(json_array_node));
      if (!node) {
        json_free(copy);
        return NULL;
      }
      json_value *item_copy = exjson_copy_value(&src->item);
      if (!item_copy) {
        free(node);
        json_free(copy);
        return NULL;
      }
      node->item = *item_copy;
      free(item_copy);
      if (!prev) {
        copy->u.array.items = node;
      } else {
        prev->next = node;
      }
      prev = node;
      src = src->next;
    }
    copy->u.array.last = prev;
    break;
  }
  case J_OBJECT: {
    json_object_node *src = v->u.object.items;
    json_object_node *prev = NULL;
    while (src) {
      json_object_node *node = (json_object_node *)calloc(1, sizeof(json_object_node));
      if (!node) {
        json_free(copy);
        return NULL;
      }
      node->item.key = src->item.key;
      json_value *val_copy = exjson_copy_value(&src->item.value);
      if (!val_copy) {
        free(node);
        json_free(copy);
        return NULL;
      }
      node->item.value = *val_copy;
      free(val_copy);
      if (!prev) {
        copy->u.object.items = node;
      } else {
        prev->next = node;
      }
      prev = node;
      src = src->next;
    }
    copy->u.object.last = prev;
    break;
  }
  default:
    free(copy);
    return NULL;
  }
  return copy;
}

static json_value *create_error(const char *message) {
  json_value *err = (json_value *)calloc(1, sizeof(json_value));
  if (!err)
    return NULL;
  err->type = J_OBJECT;
  json_object_node *node = (json_object_node *)calloc(1, sizeof(json_object_node));
  if (!node) {
    free(err);
    return NULL;
  }
  node->item.key.ptr = "error";
  node->item.key.len = 5;
  node->item.value.type = J_STRING;
  node->item.value.u.string.ptr = message;
  node->item.value.u.string.len = strlen(message);
  err->u.object.items = node;
  err->u.object.last = node;
  return err;
}

/* ========== Environment Management ========== */

env_frame *exjson_env_create(env_frame *parent) {
#ifdef USE_ALLOC
  env_frame *frame = (env_frame *)calloc(1, sizeof(env_frame));
#else
  if (env_frame_free_count == 0)
    return NULL;
  env_frame *frame = env_frame_free_pool[--env_frame_free_count];
  memset(frame, 0, sizeof(env_frame));
#endif
  if (!frame)
    return NULL;
  frame->bindings = NULL;
  frame->parent = parent;
  return frame;
}

void exjson_env_free(env_frame *env) {
  if (!env)
    return;
  json_object_node *node = env->bindings;
  while (node) {
    json_object_node *next = node->next;
    json_free(&node->item.value);
#ifdef USE_ALLOC
    free(node);
#endif
    node = next;
  }
#ifdef USE_ALLOC
  free(env);
#else
  if (env_frame_free_count < ENV_POOL_SIZE) {
    env_frame_free_pool[env_frame_free_count++] = env;
  }
#endif
}

json_value *exjson_env_lookup(env_frame *env, const char *symbol, size_t len) {
  if (!env || !symbol)
    return NULL;
  json_object_node *node = env->bindings;
  while (node) {
    if (node->item.key.len == len && strncmp(node->item.key.ptr, symbol, len) == 0) {
      return &node->item.value;
    }
    node = node->next;
  }
  if (env->parent)
    return exjson_env_lookup(env->parent, symbol, len);
  return NULL;
}

bool exjson_env_define(env_frame *env, const char *symbol, size_t len, json_value *value) {
  if (!env || !symbol || !value)
    return false;
  json_object_node *node = (json_object_node *)calloc(1, sizeof(json_object_node));
  if (!node)
    return false;
  node->item.key.ptr = symbol;
  node->item.key.len = len;
  node->item.value = *value;
  node->next = env->bindings;
  env->bindings = node;
  return true;
}

bool exjson_env_set(env_frame *env, const char *symbol, size_t len, json_value *value) {
  if (!env || !symbol || !value)
    return false;
  json_object_node *node = env->bindings;
  while (node) {
    if (node->item.key.len == len && strncmp(node->item.key.ptr, symbol, len) == 0) {
      json_free(&node->item.value);
      node->item.value = *value;
      return true;
    }
    node = node->next;
  }
  if (env->parent)
    return exjson_env_set(env->parent, symbol, len, value);
  return false;
}

/* ========== Parsing Functions ========== */

static bool is_symbol_char(char c) {
  return isalnum((unsigned char)c) || c == '_' || c == '-' || c == '?' || 
         c == '!' || c == '+' || c == '*' || c == '/' || c == '<' || 
         c == '>' || c == '=';
}

bool exjson_parse_symbol(const char **s, json_value *v) {
  const char *start = *s;
  size_t len = 0;
  while (is_symbol_char(**s)) {
    (*s)++;
    len++;
  }
  if (len == 0)
    return false;
  v->type = J_SYMBOL;
  v->u.string.ptr = start;
  v->u.string.len = len;
  return true;
}

/* Helper to skip whitespace */
static void skip_ws(const char **s) {
  while (**s && isspace((unsigned char)**s)) {
    (*s)++;
  }
}

bool exjson_parse_lisp_expr(const char **s, json_value *v) {
  if (**s != '(')
    return false;
  (*s)++;
  
  skip_ws(s);
  
  if (**s == ')') {
    (*s)++;
    v->type = J_ARRAY;
    v->u.array.items = NULL;
    v->u.array.last = NULL;
    return true;
  }
  
  v->type = J_ARRAY;
  v->u.array.items = NULL;
  v->u.array.last = NULL;
  
  json_array_node *prev = NULL;
  while (**s != ')') {
    json_array_node *node = (json_array_node *)calloc(1, sizeof(json_array_node));
    if (!node)
      return false;
    
    if (**s == '(') {
      if (!exjson_parse_lisp_expr(s, &node->item)) {
        free(node);
        return false;
      }
    } else if (is_symbol_char(**s)) {
      if (!exjson_parse_symbol(s, &node->item)) {
        free(node);
        return false;
      }
    } else if (**s == '"') {
      node->item.type = J_STRING;
      if (!json_parse((const char *)s, &node->item)) {
        (*s)++;
        const char *p = *s;
        while (*p && *p != '"') {
          if (*p == '\\')
            p++;
          p++;
        }
        if (*p == '"')
          p++;
        node->item.u.string.ptr = *s;
        node->item.u.string.len = p - *s - 1;
        *s = p;
      }
    } else if (**s == '-' || isdigit((unsigned char)**s)) {
      node->item.type = J_NUMBER;
      const char *num_start = *s;
      char *end;
      strtod(*s, &end);
      node->item.u.number.ptr = num_start;
      node->item.u.number.len = end - num_start;
      *s = end;
    } else if (strncmp(*s, "true", 4) == 0) {
      node->item.type = J_BOOLEAN;
      node->item.u.boolean.ptr = *s;
      node->item.u.boolean.len = 4;
      *s += 4;
    } else if (strncmp(*s, "false", 5) == 0) {
      node->item.type = J_BOOLEAN;
      node->item.u.boolean.ptr = *s;
      node->item.u.boolean.len = 5;
      *s += 5;
    } else if (strncmp(*s, "null", 4) == 0) {
      node->item.type = J_NULL;
      *s += 4;
    } else {
      free(node);
      return false;
    }
    
    if (!prev) {
      v->u.array.items = node;
    } else {
      prev->next = node;
    }
    prev = node;
    v->u.array.last = node;
    
    skip_ws(s);
  }
  
  (*s)++;
  return true;
}

bool exjson_parse(const char *json, json_value *root, env_frame *env) {
  (void)env; // unused - environment not needed during parsing
  if (!json || !root)
    return false;
  const char *p = json;
  
  skip_ws(&p);
  
  if (*p == '(') {
    return exjson_parse_lisp_expr(&p, root);
  } else {
    return json_parse(json, root);
  }
}

/* ========== Special Forms ========== */

json_value *exjson_eval_quote(const json_value *expr, env_frame *env) {
  (void)env; // unused
  if (!expr || expr->type != J_ARRAY)
    return create_error("quote: invalid arguments");
  json_array_node *args = expr->u.array.items;
  if (!args || !args->next)
    return create_error("quote: requires one argument");
  return exjson_copy_value(&args->next->item);
}

json_value *exjson_eval_define(const json_value *expr, env_frame *env) {
  if (!expr || expr->type != J_ARRAY || !env)
    return create_error("define: invalid arguments");
  json_array_node *args = expr->u.array.items;
  if (!args || !args->next)
    return create_error("define: requires at least 2 arguments");
  
  json_value *first = &args->next->item;
  
  if (first->type == J_SYMBOL) {
    if (!args->next->next)
      return create_error("define: requires value");
    json_value *value = exjson_eval(&args->next->next->item, env);
    if (!value)
      return create_error("define: failed to evaluate value");
    exjson_env_define(env, first->u.string.ptr, first->u.string.len, value);
    return value;
  } else if (first->type == J_ARRAY) {
    json_array_node *func_sig = first->u.array.items;
    if (!func_sig || func_sig->item.type != J_SYMBOL)
      return create_error("define: function name must be symbol");
    
    json_value *params = (json_value *)calloc(1, sizeof(json_value));
    params->type = J_ARRAY;
    params->u.array.items = func_sig->next;
    
    json_value *body = &args->next->next->item;
    
    json_value *lambda = (json_value *)calloc(1, sizeof(json_value));
    lambda->type = J_OBJECT;
    
    json_object_node *type_node = (json_object_node *)calloc(1, sizeof(json_object_node));
    type_node->item.key.ptr = "type";
    type_node->item.key.len = 4;
    type_node->item.value.type = J_STRING;
    type_node->item.value.u.string.ptr = "lambda";
    type_node->item.value.u.string.len = 6;
    
    json_object_node *params_node = (json_object_node *)calloc(1, sizeof(json_object_node));
    params_node->item.key.ptr = "params";
    params_node->item.key.len = 6;
    params_node->item.value = *params;
    
    json_object_node *body_node = (json_object_node *)calloc(1, sizeof(json_object_node));
    body_node->item.key.ptr = "body";
    body_node->item.key.len = 4;
    body_node->item.value = *body;
    
    type_node->next = params_node;
    params_node->next = body_node;
    lambda->u.object.items = type_node;
    lambda->u.object.last = body_node;
    
    exjson_env_define(env, func_sig->item.u.string.ptr, 
                     func_sig->item.u.string.len, lambda);
    return lambda;
  }
  
  return create_error("define: invalid syntax");
}

json_value *exjson_eval_if(const json_value *expr, env_frame *env) {
  if (!expr || expr->type != J_ARRAY)
    return create_error("if: invalid arguments");
  json_array_node *args = expr->u.array.items;
  if (!args || !args->next || !args->next->next || !args->next->next->next)
    return create_error("if: requires 3 arguments (condition then else)");
  
  json_value *cond = exjson_eval(&args->next->item, env);
  if (!cond)
    return create_error("if: failed to evaluate condition");
  
  bool is_true = exjson_is_truthy(cond);
  json_free(cond);
  free(cond);
  
  if (is_true) {
    return exjson_eval(&args->next->next->item, env);
  } else {
    return exjson_eval(&args->next->next->next->item, env);
  }
}

json_value *exjson_eval_lambda(const json_value *expr, env_frame *env) {
  (void)env; // unused - environment captured at application time
  if (!expr || expr->type != J_ARRAY)
    return create_error("lambda: invalid arguments");
  json_array_node *args = expr->u.array.items;
  if (!args || !args->next || !args->next->next)
    return create_error("lambda: requires (params) and body");
  
  json_value *params = &args->next->item;
  json_value *body = &args->next->next->item;
  
  if (params->type != J_ARRAY)
    return create_error("lambda: params must be a list");
  
  json_value *lambda = (json_value *)calloc(1, sizeof(json_value));
  lambda->type = J_OBJECT;
  
  json_object_node *type_node = (json_object_node *)calloc(1, sizeof(json_object_node));
  type_node->item.key.ptr = "type";
  type_node->item.key.len = 4;
  type_node->item.value.type = J_STRING;
  type_node->item.value.u.string.ptr = "lambda";
  type_node->item.value.u.string.len = 6;
  
  json_object_node *params_node = (json_object_node *)calloc(1, sizeof(json_object_node));
  params_node->item.key.ptr = "params";
  params_node->item.key.len = 6;
  params_node->item.value = *exjson_copy_value(params);
  
  json_object_node *body_node = (json_object_node *)calloc(1, sizeof(json_object_node));
  body_node->item.key.ptr = "body";
  body_node->item.key.len = 4;
  body_node->item.value = *exjson_copy_value(body);
  
  type_node->next = params_node;
  params_node->next = body_node;
  lambda->u.object.items = type_node;
  lambda->u.object.last = body_node;
  
  return lambda;
}

json_value *exjson_eval_set(const json_value *expr, env_frame *env) {
  if (!expr || expr->type != J_ARRAY || !env)
    return create_error("set!: invalid arguments");
  json_array_node *args = expr->u.array.items;
  if (!args || !args->next || !args->next->next)
    return create_error("set!: requires 2 arguments");
  
  json_value *var = &args->next->item;
  if (var->type != J_SYMBOL)
    return create_error("set!: first argument must be a symbol");
  
  json_value *value = exjson_eval(&args->next->next->item, env);
  if (!value)
    return create_error("set!: failed to evaluate value");
  
  if (!exjson_env_set(env, var->u.string.ptr, var->u.string.len, value)) {
    json_free(value);
    free(value);
    return create_error("set!: variable not defined");
  }
  
  return value;
}

json_value *exjson_eval_case(const json_value *expr, env_frame *env) {
  if (!expr || expr->type != J_ARRAY || !env)
    return create_error("case: invalid arguments");
  json_array_node *args = expr->u.array.items;
  if (!args || !args->next)
    return create_error("case: requires key expression");
  
  json_value *key = exjson_eval(&args->next->item, env);
  if (!key)
    return create_error("case: failed to evaluate key");
  
  json_array_node *clause = args->next->next;
  while (clause) {
    if (clause->item.type != J_ARRAY || !clause->item.u.array.items)
      continue;
    
    json_value *pattern = &clause->item.u.array.items->item;
    
    if (pattern->type == J_SYMBOL && pattern->u.string.len == 4 && 
        strncmp(pattern->u.string.ptr, "else", 4) == 0) {
      json_value *result = NULL;
      if (clause->item.u.array.items->next) {
        result = exjson_eval(&clause->item.u.array.items->next->item, env);
      }
      json_free(key);
      free(key);
      return result ? result : create_error("case: else clause has no expression");
    }
    
    if (json_equal(key, pattern)) {
      json_value *result = NULL;
      if (clause->item.u.array.items->next) {
        result = exjson_eval(&clause->item.u.array.items->next->item, env);
      }
      json_free(key);
      free(key);
      return result ? result : create_error("case: matched clause has no expression");
    }
    
    clause = clause->next;
  }
  
  json_free(key);
  free(key);
  return create_error("case: no matching clause");
}

/* ========== Built-in Functions ========== */

json_value *exjson_builtin_add(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY)
    return create_error("+: requires arguments");
  double sum = 0;
  json_array_node *node = args->u.array.items;
  while (node) {
    if (node->item.type != J_NUMBER)
      return create_error("+: all arguments must be numbers");
    char *numstr = exjson_ref_to_string(&node->item.u.number);
    sum += strtod(numstr, NULL);
    free(numstr);
    node = node->next;
  }
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_NUMBER;
  char *buf = (char *)malloc(32);
  snprintf(buf, 32, "%g", sum);
  result->u.number.ptr = buf;
  result->u.number.len = strlen(buf);
  return result;
}

json_value *exjson_builtin_sub(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY || !args->u.array.items)
    return create_error("-: requires at least one argument");
  json_array_node *node = args->u.array.items;
  if (node->item.type != J_NUMBER)
    return create_error("-: all arguments must be numbers");
  char *numstr = exjson_ref_to_string(&node->item.u.number);
  double result = strtod(numstr, NULL);
  free(numstr);
  node = node->next;
  if (!node) {
    result = -result;
  } else {
    while (node) {
      if (node->item.type != J_NUMBER)
        return create_error("-: all arguments must be numbers");
      numstr = exjson_ref_to_string(&node->item.u.number);
      result -= strtod(numstr, NULL);
      free(numstr);
      node = node->next;
    }
  }
  json_value *ret = (json_value *)calloc(1, sizeof(json_value));
  ret->type = J_NUMBER;
  char *buf = (char *)malloc(32);
  snprintf(buf, 32, "%g", result);
  ret->u.number.ptr = buf;
  ret->u.number.len = strlen(buf);
  return ret;
}

json_value *exjson_builtin_mul(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY)
    return create_error("*: requires arguments");
  double product = 1;
  json_array_node *node = args->u.array.items;
  while (node) {
    if (node->item.type != J_NUMBER)
      return create_error("*: all arguments must be numbers");
    char *numstr = exjson_ref_to_string(&node->item.u.number);
    product *= strtod(numstr, NULL);
    free(numstr);
    node = node->next;
  }
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_NUMBER;
  char *buf = (char *)malloc(32);
  snprintf(buf, 32, "%g", product);
  result->u.number.ptr = buf;
  result->u.number.len = strlen(buf);
  return result;
}

json_value *exjson_builtin_cons(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY || !args->u.array.items || !args->u.array.items->next)
    return create_error("cons: requires 2 arguments");
  
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_ARRAY;
  
  json_array_node *new_node = (json_array_node *)calloc(1, sizeof(json_array_node));
  new_node->item = *exjson_copy_value(&args->u.array.items->item);
  
  json_value *rest = &args->u.array.items->next->item;
  if (rest->type == J_ARRAY) {
    new_node->next = rest->u.array.items;
    result->u.array.items = new_node;
  } else {
    new_node->next = NULL;
    result->u.array.items = new_node;
  }
  
  return result;
}

json_value *exjson_builtin_car(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY || !args->u.array.items)
    return create_error("car: requires one argument");
  json_value *list = &args->u.array.items->item;
  if (list->type != J_ARRAY || !list->u.array.items)
    return create_error("car: argument must be non-empty list");
  return exjson_copy_value(&list->u.array.items->item);
}

json_value *exjson_builtin_cdr(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY || !args->u.array.items)
    return create_error("cdr: requires one argument");
  json_value *list = &args->u.array.items->item;
  if (list->type != J_ARRAY || !list->u.array.items)
    return create_error("cdr: argument must be non-empty list");
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_ARRAY;
  result->u.array.items = list->u.array.items->next;
  return result;
}

json_value *exjson_builtin_list(json_value *args, env_frame *env) {
  (void)env; // unused
  return exjson_copy_value(args);
}

json_value *exjson_builtin_null_p(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY || !args->u.array.items)
    return create_error("null?: requires one argument");
  json_value *val = &args->u.array.items->item;
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_BOOLEAN;
  if (val->type == J_NULL || (val->type == J_ARRAY && !val->u.array.items)) {
    result->u.boolean.ptr = "true";
    result->u.boolean.len = 4;
  } else {
    result->u.boolean.ptr = "false";
    result->u.boolean.len = 5;
  }
  return result;
}

json_value *exjson_builtin_eq(json_value *args, env_frame *env) {
  (void)env; // unused
  if (!args || args->type != J_ARRAY || !args->u.array.items || !args->u.array.items->next)
    return create_error("=: requires 2 arguments");
  bool eq = json_equal(&args->u.array.items->item, &args->u.array.items->next->item);
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_BOOLEAN;
  result->u.boolean.ptr = eq ? "true" : "false";
  result->u.boolean.len = eq ? 4 : 5;
  return result;
}

json_value *exjson_builtin_div(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY || !args->u.array.items)
    return create_error("/: requires at least one argument");
  json_array_node *node = args->u.array.items;
  if (node->item.type != J_NUMBER)
    return create_error("/: all arguments must be numbers");
  char *numstr = exjson_ref_to_string(&node->item.u.number);
  double result = strtod(numstr, NULL);
  free(numstr);
  node = node->next;
  if (!node) {
    result = 1.0 / result;
  } else {
    while (node) {
      if (node->item.type != J_NUMBER)
        return create_error("/: all arguments must be numbers");
      numstr = exjson_ref_to_string(&node->item.u.number);
      double divisor = strtod(numstr, NULL);
      free(numstr);
      if (divisor == 0.0)
        return create_error("/: division by zero");
      result /= divisor;
      node = node->next;
    }
  }
  json_value *ret = (json_value *)calloc(1, sizeof(json_value));
  ret->type = J_NUMBER;
  char *buf = (char *)malloc(32);
  snprintf(buf, 32, "%g", result);
  ret->u.number.ptr = buf;
  ret->u.number.len = strlen(buf);
  return ret;
}

/* ========== Evaluation Engine ========== */

json_value *exjson_apply(json_value *function, json_value *args, env_frame *env) {
  if (!function || !env)
    return create_error("apply: invalid function");
  
  if (function->type == J_OBJECT) {
    json_object_node *node = function->u.object.items;
    json_value *type_val = NULL;
    json_value *params_val = NULL;
    json_value *body_val = NULL;
    
    while (node) {
      if (node->item.key.len == 4 && strncmp(node->item.key.ptr, "type", 4) == 0)
        type_val = &node->item.value;
      else if (node->item.key.len == 6 && strncmp(node->item.key.ptr, "params", 6) == 0)
        params_val = &node->item.value;
      else if (node->item.key.len == 4 && strncmp(node->item.key.ptr, "body", 4) == 0)
        body_val = &node->item.value;
      node = node->next;
    }
    
    if (type_val && type_val->type == J_STRING && 
        type_val->u.string.len == 6 && strncmp(type_val->u.string.ptr, "lambda", 6) == 0) {
      
      env_frame *new_env = exjson_env_create(env);
      if (!new_env)
        return create_error("apply: failed to create environment");
      
      json_array_node *param = params_val ? params_val->u.array.items : NULL;
      json_array_node *arg = args ? args->u.array.items : NULL;
      
      while (param && arg) {
        if (param->item.type != J_SYMBOL)
          return create_error("apply: parameter must be symbol");
        exjson_env_define(new_env, param->item.u.string.ptr, 
                         param->item.u.string.len, &arg->item);
        param = param->next;
        arg = arg->next;
      }
      
      json_value *result = exjson_eval(body_val, new_env);
      exjson_env_free(new_env);
      return result;
    }
  }
  
  return create_error("apply: not a function");
}

static json_value *eval_list(const json_value *list, env_frame *env) {
  if (!list || list->type != J_ARRAY)
    return NULL;
  
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_ARRAY;
  result->u.array.items = NULL;
  result->u.array.last = NULL;
  
  json_array_node *src = list->u.array.items;
  json_array_node *prev = NULL;
  
  while (src) {
    json_array_node *node = (json_array_node *)calloc(1, sizeof(json_array_node));
    json_value *evaled = exjson_eval(&src->item, env);
    if (!evaled) {
      json_free(result);
      return NULL;
    }
    node->item = *evaled;
    free(evaled);
    
    if (!prev) {
      result->u.array.items = node;
    } else {
      prev->next = node;
    }
    prev = node;
    result->u.array.last = node;
    
    src = src->next;
  }
  
  return result;
}

json_value *exjson_eval(const json_value *expr, env_frame *env) {
  if (!expr || !env)
    return NULL;
  
  switch (expr->type) {
  case J_NULL:
  case J_BOOLEAN:
  case J_NUMBER:
  case J_STRING:
    return exjson_copy_value(expr);
    
  case J_SYMBOL: {
    json_value *val = exjson_env_lookup(env, expr->u.string.ptr, expr->u.string.len);
    if (!val)
      return create_error("undefined variable");
    return exjson_copy_value(val);
  }
  
  case J_ARRAY: {
    if (!expr->u.array.items) {
      return exjson_copy_value(expr);
    }
    
    json_value *first = &expr->u.array.items->item;
    
    if (first->type == J_SYMBOL) {
      const char *name = first->u.string.ptr;
      size_t len = first->u.string.len;
      
      if (len == 5 && strncmp(name, "quote", 5) == 0)
        return exjson_eval_quote(expr, env);
      if (len == 6 && strncmp(name, "define", 6) == 0)
        return exjson_eval_define(expr, env);
      if (len == 4 && strncmp(name, "set!", 4) == 0)
        return exjson_eval_set(expr, env);
      if (len == 2 && strncmp(name, "if", 2) == 0)
        return exjson_eval_if(expr, env);
      if (len == 6 && strncmp(name, "lambda", 6) == 0)
        return exjson_eval_lambda(expr, env);
      if (len == 4 && strncmp(name, "case", 4) == 0)
        return exjson_eval_case(expr, env);
      
      if (len == 1 && name[0] == '+') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("+: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_add(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 1 && name[0] == '-') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("-: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_sub(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 1 && name[0] == '*') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("*: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_mul(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 1 && name[0] == '=') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("=: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_eq(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 1 && name[0] == '/') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("/: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_div(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 1 && name[0] == '<') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("<: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_lt(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 1 && name[0] == '>') {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error(">: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_gt(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 4 && strncmp(name, "cons", 4) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("cons: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_cons(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 3 && strncmp(name, "car", 3) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("car: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_car(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 3 && strncmp(name, "cdr", 3) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("cdr: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_cdr(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 5 && strncmp(name, "null?", 5) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("null?: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_null_p(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 6 && strncmp(name, "length", 6) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("length: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_length(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 9 && strncmp(name, "get-value", 9) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("get-value: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_get_value(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 8 && strncmp(name, "has-key?", 8) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("has-key?: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_has_key(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      if (len == 13 && strncmp(name, "string-append", 13) == 0) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("string-append: evaluation failed");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_builtin_string_append(&rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
      
      json_value *func = exjson_env_lookup(env, name, len);
      if (func) {
        json_value *args = eval_list(expr, env);
        if (!args || !args->u.array.items)
          return create_error("function application: failed to evaluate arguments");
        json_array_node *rest_node = args->u.array.items->next;
        json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
        json_value *result = exjson_apply(func, &rest_args, env);
        json_free(args);
        free(args);
        return result;
      }
    }
    
    json_value *func = exjson_eval(first, env);
    if (!func)
      return create_error("cannot evaluate function");
    
    json_value *args = eval_list(expr, env);
    if (!args || !args->u.array.items) {
      json_free(func);
      free(func);
      return create_error("function application: failed to evaluate arguments");
    }
    
    json_array_node *rest_node = args->u.array.items->next;
    json_value rest_args = {.type = J_ARRAY, .u.array.items = rest_node};
    json_value *result = exjson_apply(func, &rest_args, env);
    
    json_free(func);
    free(func);
    json_free(args);
    free(args);
    
    return result;
  }
  
  case J_OBJECT:
    return exjson_copy_value(expr);
    
  default:
    return create_error("eval: unsupported type");
  }
}

/* ========== Global Environment ========== */

env_frame *exjson_create_global_env(void) {
  exjson_init_pools();
  env_frame *env = exjson_env_create(NULL);
  if (!env)
    return NULL;
  
  return env;
}

/* End of file */

json_value *exjson_builtin_lt(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY || !args->u.array.items || !args->u.array.items->next)
    return create_error("<: requires 2 arguments");
  if (args->u.array.items->item.type != J_NUMBER || 
      args->u.array.items->next->item.type != J_NUMBER)
    return create_error("<: arguments must be numbers");
  char *str1 = exjson_ref_to_string(&args->u.array.items->item.u.number);
  char *str2 = exjson_ref_to_string(&args->u.array.items->next->item.u.number);
  double val1 = strtod(str1, NULL);
  double val2 = strtod(str2, NULL);
  free(str1);
  free(str2);
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_BOOLEAN;
  result->u.boolean.ptr = (val1 < val2) ? "true" : "false";
  result->u.boolean.len = (val1 < val2) ? 4 : 5;
  return result;
}

json_value *exjson_builtin_gt(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY || !args->u.array.items || !args->u.array.items->next)
    return create_error(">: requires 2 arguments");
  if (args->u.array.items->item.type != J_NUMBER || 
      args->u.array.items->next->item.type != J_NUMBER)
    return create_error(">: arguments must be numbers");
  char *str1 = exjson_ref_to_string(&args->u.array.items->item.u.number);
  char *str2 = exjson_ref_to_string(&args->u.array.items->next->item.u.number);
  double val1 = strtod(str1, NULL);
  double val2 = strtod(str2, NULL);
  free(str1);
  free(str2);
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_BOOLEAN;
  result->u.boolean.ptr = (val1 > val2) ? "true" : "false";
  result->u.boolean.len = (val1 > val2) ? 4 : 5;
  return result;
}

json_value *exjson_builtin_length(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY || !args->u.array.items)
    return create_error("length: requires one argument");
  json_value *val = &args->u.array.items->item;
  size_t len = 0;
  if (val->type == J_ARRAY) {
    json_array_node *node = val->u.array.items;
    while (node) {
      len++;
      node = node->next;
    }
  } else if (val->type == J_STRING) {
    len = val->u.string.len;
  } else {
    return create_error("length: argument must be array or string");
  }
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_NUMBER;
  char *buf = (char *)malloc(32);
  snprintf(buf, 32, "%zu", len);
  result->u.number.ptr = buf;
  result->u.number.len = strlen(buf);
  return result;
}

json_value *exjson_builtin_get_value(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY || !args->u.array.items || !args->u.array.items->next)
    return create_error("get-value: requires 2 arguments");
  json_value *obj = &args->u.array.items->item;
  json_value *key = &args->u.array.items->next->item;
  if (obj->type != J_OBJECT)
    return create_error("get-value: first argument must be object");
  if (key->type != J_STRING)
    return create_error("get-value: second argument must be string");
  json_object_node *node = obj->u.object.items;
  while (node) {
    if (node->item.key.len == key->u.string.len &&
        strncmp(node->item.key.ptr, key->u.string.ptr, key->u.string.len) == 0) {
      return exjson_copy_value(&node->item.value);
    }
    node = node->next;
  }
  json_value *null_val = (json_value *)calloc(1, sizeof(json_value));
  null_val->type = J_NULL;
  return null_val;
}

json_value *exjson_builtin_has_key(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY || !args->u.array.items || !args->u.array.items->next)
    return create_error("has-key?: requires 2 arguments");
  json_value *obj = &args->u.array.items->item;
  json_value *key = &args->u.array.items->next->item;
  if (obj->type != J_OBJECT)
    return create_error("has-key?: first argument must be object");
  if (key->type != J_STRING)
    return create_error("has-key?: second argument must be string");
  json_object_node *node = obj->u.object.items;
  while (node) {
    if (node->item.key.len == key->u.string.len &&
        strncmp(node->item.key.ptr, key->u.string.ptr, key->u.string.len) == 0) {
      json_value *result = (json_value *)calloc(1, sizeof(json_value));
      result->type = J_BOOLEAN;
      result->u.boolean.ptr = "true";
      result->u.boolean.len = 4;
      return result;
    }
    node = node->next;
  }
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_BOOLEAN;
  result->u.boolean.ptr = "false";
  result->u.boolean.len = 5;
  return result;
}

json_value *exjson_builtin_string_append(json_value *args, env_frame *env) {
  (void)env;
  if (!args || args->type != J_ARRAY)
    return create_error("string-append: requires arguments");
  size_t total_len = 0;
  json_array_node *node = args->u.array.items;
  while (node) {
    if (node->item.type != J_STRING)
      return create_error("string-append: all arguments must be strings");
    total_len += node->item.u.string.len;
    node = node->next;
  }
  char *buf = (char *)malloc(total_len + 1);
  if (!buf)
    return create_error("string-append: memory allocation failed");
  char *p = buf;
  node = args->u.array.items;
  while (node) {
    memcpy(p, node->item.u.string.ptr, node->item.u.string.len);
    p += node->item.u.string.len;
    node = node->next;
  }
  *p = '\0';
  json_value *result = (json_value *)calloc(1, sizeof(json_value));
  result->type = J_STRING;
  result->u.string.ptr = buf;
  result->u.string.len = total_len;
  return result;
}

/* End of exjson.c additions */
