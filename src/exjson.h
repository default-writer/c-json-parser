/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * microLISP with exJSON Notation
 * Created: December 26, 2025
 */

#ifndef EXJSON_H
#define EXJSON_H

#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Extended JSON token types for LISP expressions */
/* Note: These are added to extend the json_token enum from json.h */
#define J_LISP_EXPR ((json_token)7)  /* LISP expression (function call) */
#define J_SYMBOL ((json_token)8)     /* Variable/symbol reference */
#define J_LAMBDA ((json_token)9)     /* Lambda function */
#define J_QUOTED ((json_token)10)    /* Quoted expression */

/* Environment frame for lexical scoping */
typedef struct env_frame {
  json_object_node *bindings;  /* Variable bindings (reuses json_object_node) */
  struct env_frame *parent;    /* Parent scope */
} env_frame;

/* Closure for lambda functions */
typedef struct closure {
  json_value *params;    /* Parameter list (array of symbols) */
  json_value *body;      /* Function body expression */
  env_frame *env;        /* Captured environment */
} closure;

/* Core parsing functions */

/**
 * @brief Parse exJSON (JSON + LISP) notation
 * Supports standard JSON plus LISP expressions: (function arg1 arg2 ...)
 */
bool exjson_parse(const char *json, json_value *root, env_frame *env);

/**
 * @brief Parse a LISP expression starting with '('
 */
bool exjson_parse_lisp_expr(const char **s, json_value *v);

/**
 * @brief Parse a symbol (variable name)
 */
bool exjson_parse_symbol(const char **s, json_value *v);

/* Environment management */

/**
 * @brief Create a new environment frame
 */
env_frame *exjson_env_create(env_frame *parent);

/**
 * @brief Free an environment frame and all its children
 */
void exjson_env_free(env_frame *env);

/**
 * @brief Look up a variable in the environment
 * Searches current frame and parent frames recursively
 */
json_value *exjson_env_lookup(env_frame *env, const char *symbol, size_t len);

/**
 * @brief Define a new variable in the current frame
 */
bool exjson_env_define(env_frame *env, const char *symbol, size_t len, json_value *value);

/**
 * @brief Set an existing variable (searches parent frames)
 */
bool exjson_env_set(env_frame *env, const char *symbol, size_t len, json_value *value);

/* Core evaluation */

/**
 * @brief Evaluate an exJSON expression in the given environment
 * Returns a newly allocated json_value (caller must free)
 */
json_value *exjson_eval(const json_value *expr, env_frame *env);

/* Special forms (not evaluated as regular functions) */

json_value *exjson_eval_quote(const json_value *expr, env_frame *env);
json_value *exjson_eval_define(const json_value *expr, env_frame *env);
json_value *exjson_eval_set(const json_value *expr, env_frame *env);
json_value *exjson_eval_if(const json_value *expr, env_frame *env);
json_value *exjson_eval_lambda(const json_value *expr, env_frame *env);
json_value *exjson_eval_case(const json_value *expr, env_frame *env);

/* Function application */

/**
 * @brief Apply a function to arguments
 */
json_value *exjson_apply(json_value *function, json_value *args, env_frame *env);

/* Built-in functions */

json_value *exjson_builtin_add(json_value *args, env_frame *env);
json_value *exjson_builtin_sub(json_value *args, env_frame *env);
json_value *exjson_builtin_mul(json_value *args, env_frame *env);
json_value *exjson_builtin_div(json_value *args, env_frame *env);
json_value *exjson_builtin_eq(json_value *args, env_frame *env);
json_value *exjson_builtin_lt(json_value *args, env_frame *env);
json_value *exjson_builtin_gt(json_value *args, env_frame *env);
json_value *exjson_builtin_cons(json_value *args, env_frame *env);
json_value *exjson_builtin_car(json_value *args, env_frame *env);
json_value *exjson_builtin_cdr(json_value *args, env_frame *env);
json_value *exjson_builtin_list(json_value *args, env_frame *env);
json_value *exjson_builtin_null_p(json_value *args, env_frame *env);
json_value *exjson_builtin_length(json_value *args, env_frame *env);
json_value *exjson_builtin_get_value(json_value *args, env_frame *env);
json_value *exjson_builtin_has_key(json_value *args, env_frame *env);
json_value *exjson_builtin_string_append(json_value *args, env_frame *env);

/* Utility functions */

/**
 * @brief Create a global environment with built-in functions
 */
env_frame *exjson_create_global_env(void);

/**
 * @brief Helper to check if expression is a symbol
 */
bool exjson_is_symbol(const json_value *v);

/**
 * @brief Helper to check if value is truthy
 */
bool exjson_is_truthy(const json_value *v);

/**
 * @brief Copy a json_value (deep copy)
 */
json_value *exjson_copy_value(const json_value *v);

/**
 * @brief Convert string reference to C string (allocates memory)
 */
char *exjson_ref_to_string(const reference *ref);

#ifdef __cplusplus
}
#endif

#endif /* EXJSON_H */
