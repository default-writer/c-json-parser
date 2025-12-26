/* Additional special forms for exjson.c - Insert after exjson_eval_lambda */

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
    
    // Check for 'else' clause
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
    
    // Match pattern
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

/* Additional built-in functions - Insert with other builtins */

json_value *exjson_builtin_div(json_value *args, env_frame *env) {
  (void)env; // unused
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

json_value *exjson_builtin_lt(json_value *args, env_frame *env) {
  (void)env; // unused
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
  (void)env; // unused
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
  (void)env; // unused
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
  (void)env; // unused
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
  (void)env; // unused
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
  (void)env; // unused
  if (!args || args->type != J_ARRAY)
    return create_error("string-append: requires arguments");
  
  size_t total_len = 0;
  json_array_node *node = args->u.array.items;
  
  // Calculate total length
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
