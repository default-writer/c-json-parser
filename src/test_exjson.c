/* SPDX-License-Identifier: BSD-3-Clause */
/*-*-coding:utf-8 -*-
 * microLISP with exJSON - Test Suite
 * Created: December 26, 2025
 */

#include "exjson.h"
#include <stdio.h>
#include <assert.h>

/* Helper to evaluate and print result */
static void test_eval(const char *name, const char *expr_str, env_frame *env) {
  printf("\n=== Test: %s ===\n", name);
  printf("Input: %s\n", expr_str);
  
  json_value expr;
  memset(&expr, 0, sizeof(json_value));
  
  if (!exjson_parse(expr_str, &expr, env)) {
    printf("Parse failed!\n");
    return;
  }
  
  json_value *result = exjson_eval(&expr, env);
  if (!result) {
    printf("Eval failed!\n");
    json_free(&expr);
    return;
  }
  
  printf("Result: ");
  json_print(result, stdout);
  printf("\n");
  
  json_free(&expr);
  json_free(result);
  free(result);
}

int main(void) {
  printf("========================================\n");
  printf("  microLISP with exJSON - Test Suite\n");
  printf("========================================\n\n");
  
  printf("Initializing environment...\n");
  fflush(stdout);
  
  /* Initialize environment */
  env_frame *global_env = exjson_create_global_env();
  if (!global_env) {
    printf("Failed to create global environment!\n");
    return 1;
  }
  
  printf("Environment created successfully!\n");
  fflush(stdout);
  
  /* Test 1: Basic arithmetic */
  test_eval("Basic addition", "(+ 1 2 3)", global_env);
  test_eval("Basic subtraction", "(- 10 3)", global_env);
  test_eval("Basic multiplication", "(* 2 3 4)", global_env);
  
  /* Test 2: Nested arithmetic */
  test_eval("Nested expression", "(+ (* 2 3) (- 10 5))", global_env);
  
  /* Test 3: Variable definition */
  test_eval("Define variable", "(define x 42)", global_env);
  test_eval("Use variable", "(+ x 8)", global_env);
  
  /* Test 4: Function definition */
  test_eval("Define function", "(define (square n) (* n n))", global_env);
  test_eval("Call function", "(square 5)", global_env);
  
  /* Test 5: Recursive function */
  test_eval("Define factorial", 
    "(define (fact n) (if (= n 0) 1 (* n (fact (- n 1)))))",
    global_env);
  test_eval("Call factorial", "(fact 5)", global_env);
  
  /* Test 6: Lambda expressions */
  test_eval("Lambda", "((lambda (x) (* x x)) 7)", global_env);
  
  /* Test 7: List operations */
  test_eval("cons", "(cons 1 (cons 2 (cons 3 null)))", global_env);
  
  /* Test 8: Conditional */
  test_eval("if true", "(if (= 5 5) 100 200)", global_env);
  test_eval("if false", "(if (= 5 6) 100 200)", global_env);
  
  /* Test 9: Quote */
  test_eval("Quote expression", "(quote (+ 1 2))", global_env);
  
  /* Test 10: Nested functions */
  test_eval("Define add", "(define (add a b) (+ a b))", global_env);
  test_eval("Define double", "(define (double x) (add x x))", global_env);
  test_eval("Call nested", "(double 21)", global_env);
  
  /* Test 11: Higher-order function */
  test_eval("Apply function", 
    "(define (apply-twice f x) (f (f x)))",
    global_env);
  test_eval("Use apply-twice", "(apply-twice square 2)", global_env);
  
  /* Test 12: List sum (recursive) */
  test_eval("Define sum-list",
    "(define (sum-list lst) (if (null? lst) 0 (+ (car lst) (sum-list (cdr lst)))))",
    global_env);
  
  /* Cleanup */
  exjson_env_free(global_env);
  
  printf("\n========================================\n");
  printf("  All tests completed!\n");
  printf("========================================\n");
  
  return 0;
}
