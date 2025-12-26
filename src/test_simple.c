/* Simple test for exjson */
#include "exjson.h"
#include <stdio.h>

int main(void) {
  printf("Testing exJSON microLISP...\n\n");
  
  env_frame *env = exjson_create_global_env();
  if (!env) {
    printf("Failed to create environment\n");
    return 1;
  }
  
  // Test 1: Simple addition
  printf("Test 1: (+ 1 2 3)\n");
  json_value expr1;
  memset(&expr1, 0, sizeof(json_value));
  if (!exjson_parse("(+ 1 2 3)", &expr1, env)) {
    printf("Parse failed!\n");
  } else {
    json_value *result = exjson_eval(&expr1, env);
    if (result) {
      printf("Result: ");
      json_print(result, stdout);
      printf("\n");
      json_free(result);
      free(result);
    } else {
      printf("Eval failed!\n");
    }
    json_free(&expr1);
  }
  
  // Test 2: Define variable
  printf("\nTest 2: (define x 42)\n");
  json_value expr2;
  memset(&expr2, 0, sizeof(json_value));
  if (!exjson_parse("(define x 42)", &expr2, env)) {
    printf("Parse failed!\n");
  } else {
    json_value *result = exjson_eval(&expr2, env);
    if (result) {
      printf("Result: ");
      json_print(result, stdout);
      printf("\n");
      json_free(result);
      free(result);
    } else {
      printf("Eval failed!\n");
    }
    json_free(&expr2);
  }
  
  // Test 3: Use variable
  printf("\nTest 3: (+ x 8)\n");
  json_value expr3;
  memset(&expr3, 0, sizeof(json_value));
  if (!exjson_parse("(+ x 8)", &expr3, env)) {
    printf("Parse failed!\n");
  } else {
    json_value *result = exjson_eval(&expr3, env);
    if (result) {
      printf("Result: ");
      json_print(result, stdout);
      printf("\n");
      json_free(result);
      free(result);
    } else {
      printf("Eval failed!\n");
    }
    json_free(&expr3);
  }
  
  // Test 4: Function definition
  printf("\nTest 4: (define (square n) (* n n))\n");
  json_value expr4;
  memset(&expr4, 0, sizeof(json_value));
  if (!exjson_parse("(define (square n) (* n n))", &expr4, env)) {
    printf("Parse failed!\n");
  } else {
    json_value *result = exjson_eval(&expr4, env);
    if (result) {
      printf("Result: ");
      json_print(result, stdout);
      printf("\n");
      json_free(result);
      free(result);
    } else {
      printf("Eval failed!\n");
    }
    json_free(&expr4);
  }
  
  // Test 5: Call function
  printf("\nTest 5: (square 5)\n");
  json_value expr5;
  memset(&expr5, 0, sizeof(json_value));
  if (!exjson_parse("(square 5)", &expr5, env)) {
    printf("Parse failed!\n");
  } else {
    json_value *result = exjson_eval(&expr5, env);
    if (result) {
      printf("Result: ");
      json_print(result, stdout);
      printf("\n");
      json_free(result);
      free(result);
    } else {
      printf("Eval failed!\n");
    }
    json_free(&expr5);
  }
  
  exjson_env_free(env);
  printf("\nAll tests completed!\n");
  return 0;
}
