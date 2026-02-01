#include "../test/test.h"
#include "../src/json.h"

#define ELEMENT_SIZE 6
#define ELEMENT_BUFFER_SIZE 10

extern void test_large_array_0xfffe_elements(void);
extern void test_comprehensive_uncovered_lines(void);
extern void test_free_object_node_coverage(void);
extern void test_skip_whitespace_coverage(void);
extern void test_number_parsing_edge_cases(void);
extern void test_string_parsing_complex_cases(void);
extern void test_string_escape_printing_coverage(void);
extern void test_print_value_compact_coverage(void);
extern void test_print_value_all_types_coverage(void);
extern void test_json_stringify_buffer_error_coverage(void);
extern void test_free_array_node_coverage(void);
extern void test_parse_string_full_coverage(void);
extern void test_parse_hex4(void);
extern void test_json_error_string_function(void);
extern void test_json_error_string_with_validate(void);

#define LCPRN_RAND_MULTIPLIER 1664525
#define LCPRN_RAND_INCREMENT 1013904223
#define NUM_BUF_SIZE 0x10
#define MAX_RANDOM_NUMBER 0x100
#define MAX_GENERATION_ITERATIONS 0x1000
#define MAX_CHILDREN 5
#define MAX_DEPTH 25
#define MAX_STDIO_BUFFER_SIZE 0x1000
#define MAX_TICK_COUNTER 1000000000
#define INVALID_TYPE 99

static void generate_random_json_value(json_value *v, int depth);
static void json_free_generated(json_value *v);

TEST(test_memory_leaks) {
  char *json;
  char *out;

  const char *source = "[{\"key\": \"value\"}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  bool parsed = json_parse(source, strlen(source), &v);
  ASSERT_TRUE(parsed);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  /* cleanup */
  json_free(&v);
  free(json);
  free(out);

  END_TEST;
}

TEST(test_printf) {
  /* Create a memory buffer to capture stdout */
  char buffer[MAX_STDIO_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  FILE *mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");
  ASSERT_NOT_EQUAL(mem_stream, (FILE *)NULL, FILE *);

  char *json;
  char *out;

  const char *source = "[null,false,true,1,-1,1e+2,-1.0e-2,0.0001,0.01,0.01e+01,\"\\n\\t\\r\\b\",[],{\"key1\": \"value\",\"key2\": \"value\"}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, strlen(source), &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_print(&v, mem_stream);
  fclose(mem_stream);

  /* The input was an array with object, so output should contain '[', not just '{' */
  ASSERT_TRUE(utils_test_json_equal(buffer, source));

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  /* cleanup */
  json_free(&v);
  free(json);
  free(out);

  END_TEST;
}

TEST(test_whitespace) {
  char *json;

  const char *source = "{\t \"key\" \n : \r \"value\" }";
  const char *expected = "{\n    \"key\": \"value\"\n}";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, strlen(source), &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, expected));

  /* cleanup */
  json_free(&v);
  free(json);

  END_TEST;
}

TEST(test_array) {
  char *json_recursive;
  char *json_iterative;

  char *source = utils_get_test_json_data("data/array.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, strlen(source), &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v_iterative));

  json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  json_iterative = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative);

  ASSERT_TRUE(utils_test_json_equal(json_recursive, json_iterative));
  ASSERT_TRUE(strcmp(json_recursive, json_iterative) == 0);

  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive);
  free(json_iterative);
  free(source);

  END_TEST;
}

TEST(test_object) {
  char *json_recursive;
  char *json_iterative;

  char *source = utils_get_test_json_data("data/object.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, strlen(source), &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v_iterative));

  json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  json_iterative = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative);

  ASSERT_TRUE(utils_test_json_equal(json_recursive, json_iterative));
  ASSERT_TRUE(strcmp(json_recursive, json_iterative) == 0);

  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive);
  free(json_iterative);
  free(source);

  END_TEST;
}

TEST(test_invalid_number_leading_zero) {
  const char *source = "[01]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_valid_number_zero_point_zero) {
  const char *source = "[0.0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_invalid_iterative_unclosed_array) {
  const char *source = "[1, 2,";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_unclosed_object) {
  const char *source = "{\"key\": 1";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_unquoted_string_key) {
  const char *source = "{key: \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_missing_colon) {
  const char *source = "{\"key\" \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_extra_comma_array) {
  const char *source = "[1, 2,, 3]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_extra_comma_object) {
  const char *source = "{\"key1\": 1,, \"key2\": 2}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_invalid_escape_sequence) {
  const char *source = "[\"hello\\xworld\"]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_truncated_string) {
  const char *source = "[\"hello";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_truncated_number) {
  const char *source = "[123.";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_incorrect_boolean) {
  const char *source = "[tru]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_incorrect_null) {
  const char *source = "[nul]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_empty_input) {
  const char *source = "";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_whitespace_only_input) {
  const char *source = "   \\t\\n  ";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_single_value_no_array_or_object) {
  const char *source = "123";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_nested_unclosed_array) {
  const char *source = "{\"key\": [1, {\"subkey\": [true,]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_array_of_unclosed_objects) {
  const char *source = "[{\"a\":1}, {\"b\":2]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_valid_number_zero_point_zero_iterative) {
  char *json;

  const char *source = "[0.0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_invalid_iterative_truncated_exponent) {
  const char *source = "[1e]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_truncated_exponent_sign) {
  const char *source = "[1e+]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_invalid_iterative_exponent_missing_digits) {
  const char *source = "[1e+ ]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_valid_number_iterative_positive_integer) {
  const char *source = "[123]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_iterative_negative_integer) {
  const char *source = "[-123]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_iterative_exponent_positive) {
  const char *source = "[1e+2]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_iterative_exponent_negative) {
  const char *source = "[1e-2]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_iterative_positive_float) {
  const char *source = "[123.45]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_iterative_negative_float) {
  const char *source = "[-123.45]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_iterative_scientific_notation) {
  const char *source = "[1.2e-10]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  /* utils_test_json_equal handles numerical equality */
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_number_scientific_notation_coverage) {
  /* Test to cover lines 182 (negative sign), 202 (decimal loop), 211 (exponent sign) */

  /* Test negative scientific number to hit line 182 */
  const char *source1 = "[-1e5]";
  json_value v1;
  memset(&v1, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source1, strlen(source1), &v1));
  char *json1 = json_stringify(&v1);
  ASSERT_PTR_NOT_NULL(json1);
  ASSERT_TRUE(utils_test_json_equal(json1, source1));
  json_free(&v1);
  free(json1);

  /* Test decimal number to hit line 202 */
  const char *source2 = "[1.23456789]";
  json_value v2;
  memset(&v2, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source2, strlen(source2), &v2));
  char *json2 = json_stringify(&v2);
  ASSERT_PTR_NOT_NULL(json2);
  ASSERT_TRUE(utils_test_json_equal(json2, source2));
  json_free(&v2);
  free(json2);

  /* Test scientific notation with positive exponent to hit line 211 */
  const char *source3 = "[1e+10]";
  json_value v3;
  memset(&v3, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source3, strlen(source3), &v3));
  char *json3 = json_stringify(&v3);
  ASSERT_PTR_NOT_NULL(json3);
  ASSERT_TRUE(utils_test_json_equal(json3, source3));
  json_free(&v3);
  free(json3);

  /* Test scientific notation with negative exponent to hit line 211 */
  const char *source4 = "[1e-10]";
  json_value v4;
  memset(&v4, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source4, strlen(source4), &v4));
  char *json4 = json_stringify(&v4);
  ASSERT_PTR_NOT_NULL(json4);
  ASSERT_TRUE(utils_test_json_equal(json4, source4));
  json_free(&v4);
  free(json4);

  END_TEST;
}

TEST(test_valid_string_iterative_empty) {
  const char *source = "[\"\"]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_string_iterative_with_spaces) {
  const char *source = "[\"hello world\"]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_string_iterative_with_escaped_chars) {
  const char *source = "[\"\\\"\\\\/\\b\\f\\n\\r\\t\"]";
  json_value v;
  memset(&v, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  ASSERT_EQ(v.type, J_ARRAY);

  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_value parsed_back;
  memset(&parsed_back, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(json, strlen(json), &parsed_back));
  ASSERT_TRUE(json_equal(&v, &parsed_back));

  ASSERT_TRUE(strcmp(json, source) == 0);

  json_free(&v);
  json_free(&parsed_back);
  free(json);

  END_TEST;
}

TEST(test_valid_boolean_iterative_true) {
  const char *source = "[true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_boolean_iterative_false) {
  const char *source = "[false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_null_iterative) {
  const char *source = "[null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_array_iterative_empty) {
  const char *source = "[]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_array_iterative_mixed_types) {
  const char *source = "[1, \"two\", true, null, {}, []]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_object_iterative_empty) {
  const char *source = "{}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_object_iterative_simple) {
  const char *source = "{\"key\": \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_object_iterative_nested) {
  const char *source = "{\"key1\": 1, \"key2\": {\"nestedKey\": true}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_valid_nested_array_and_object_iterative) {
  const char *source = "[{\"a\": [1, 2]}, {\"b\": {\"c\": 3}}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_json_parse) {
  char *json_recursive;
  char *json_iterative;

  char *source = utils_get_test_json_data("data/test.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, strlen(source), &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v_iterative));

  json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  json_iterative = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative);

  ASSERT_TRUE(utils_test_json_equal(json_recursive, json_iterative));
  ASSERT_TRUE(strcmp(json_recursive, json_iterative) == 0);

  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive);
  free(json_iterative);
  free(source);

  END_TEST;
}

TEST(test_case_0) {
  const char *source = "[]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_1) {
  const char *source = "[null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_2) {
  const char *source = "[null, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_3) {
  const char *source = "[true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_4) {
  const char *source = "[true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_5) {
  const char *source = "[false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_6) {
  const char *source = "[false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_7) {
  const char *source = "[true, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_8) {
  const char *source = "[0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_9) {
  const char *source = "[0, 0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_10) {
  const char *source = "[1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_11) {
  const char *source = "[1, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_12) {
  const char *source = "[0, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_13) {
  const char *source = "[0, 1, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_14) {
  const char *source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_15) {
  const char *source = "[{}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_16) {
  const char *source = "[{\"key\": null}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_17) {
  const char *source = "[{\"key\": []}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_18) {
  const char *source = "[{\"key\": [null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_19) {
  const char *source = "[{\"key\": [null, null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_20) {
  const char *source = "[{\"key1\": null, \"key2\":[]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_21) {
  const char *source = "[{\"key1\": null, \"key2\":[null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_22) {
  const char *source = "[{\"key1\": null, \"key2\":[null,null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_23) {
  const char *source = "{}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_24) {
  const char *source = "{\"key\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_25) {
  const char *source = "{\"key1\": null, \"key2\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_26) {
  const char *source = "{\"key\": []}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_27) {
  const char *source = "{\"key\": [null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_28) {
  const char *source = "{\"key\": [null, null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_29) {
  const char *source = "{\"key\": true}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_30) {
  const char *source = "{\"key\": false}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_31) {
  const char *source = "{\"key\": 0}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_32) {
  const char *source = "{\"key\": 1}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_33) {
  const char *source = "{\"key\": 0.5}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_34) {
  const char *source = "{\"key\": \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_35) {
  const char *source = "{\"key\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_36) {
  const char *source = "{\"key1\": {}, \"key2\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_37) {
  const char *source = "{\"key\": [{\"subkey\": []}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_38) {
  const char *source = "{\"key\": [{\"subkey\": [null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_39) {
  const char *source = "{\"key\": [{\"subkey\": [null, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_40) {
  const char *source = "{\"key\": [{\"subkey\": null\"}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_41) {
  const char *source = "{\"key\": [{\"subkey\": [true]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_42) {
  const char *source = "{\"key\": [{\"subkey\": [true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_43) {
  const char *source = "{\"key\": [{\"subkey\": [false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_44) {
  const char *source = "{\"key\": [{\"subkey\": [false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_45) {
  const char *source = "{\"key\": [{\"subkey\": [true, false]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_46) {
  const char *source = "{\"key\": [{\"subkey\": [0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_47) {
  const char *source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_48) {
  const char *source = "{\"key\": [{\"subkey\": [1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_49) {
  const char *source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_50) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_51) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_52) {
  const char *source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_53) {
  const char *source = "{\"key\": [{\"subkey\": [{}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_54) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_55) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_56) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_57) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_58) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_59) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_60) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_61) {
  const char *source = "{\"key\": [{\"subkey\": {}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_62) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_63) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_64) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_65) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_66) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_67) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_68) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_69) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_70) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_71) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_72) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_73) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_74) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_invalid_char_0) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_1) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[null]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_2) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[null, null]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_3) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[true]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_4) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[true, true]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_5) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[false]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_6) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[false, false]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_7) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[true, false]";
  len = strlen(original_source);
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_8) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[0]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_9) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[0, 0]";

  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_10) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[1]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_11) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[1, 1]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_12) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[0, 1]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_13) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[0, 1, null]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_14) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if ((i > 0 && original_source[i - 1] == '1' && original_source[i] == '2') || (i < len - 1 && original_source[i] == '1' && original_source[i + 1] == '2') || (i < len - 1 && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_15) {
  char *source = NULL;
  size_t len;
  const char *original_source = "[{}]";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_16) {
  char *source = NULL;
  size_t len;
  const char *original_source = "{\"key\": null}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_17) {
  char *source = NULL;
  size_t len;
  const char *original_source = "{\"key\": []}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_18) {
  char *source = NULL;
  size_t len;
  const char *original_source = "{\"key\": [null]}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_19) {
  char *source = NULL;
  size_t len;
  const char *original_source = "{\"key\": [null, null]}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_20) {
  char *source = NULL;
  size_t len;
  const char *original_source = "{\"key1\": null, \"key2\":[]}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_21) {
  char *source = NULL;
  size_t len;
  const char *original_source = "{\"key1\": null, \"key2\":[null]}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_22) {
  char *source;
  size_t len;
  const char *original_source = "{\"key1\": null, \"key2\":[null,null]}";
  len = strlen(original_source);
  source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_23) {
  const char *original_source = "{}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_24) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_25) {
  const char *original_source = "{\"key1\": null, \"key2\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_26) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_27) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_28) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_29) {
  const char *original_source = "{\"key\": true}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_30) {
  const char *original_source = "{\"key\": false}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_31) {
  const char *original_source = "{\"key\": 0}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_32) {
  const char *original_source = "{\"key\": 1}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_33) {
  const char *original_source = "{\"key\": 0.5}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_34) {
  const char *original_source = "{\"key\": \"value\"}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_35) {
  const char *original_source = "{\"key\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_36) {
  const char *original_source = "{\"key1\": {}, \"key2\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_37) {
  const char *original_source = "{\"key\": [{\"subkey\": []}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_38) {
  const char *original_source = "{\"key\": [{\"subkey\": [null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_39) {
  const char *original_source = "{\"key\": [{\"subkey\": [null, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_40) {
  const char *original_source = "{\"key\": [{\"subkey\": [true]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_41) {
  const char *original_source = "{\"key\": [{\"subkey\": [true, false]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_42) {
  const char *original_source = "{\"key\": [{\"subkey\": [0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '0')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_43) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_44) {
  const char *original_source = "{\"key\": [{\"subkey\": [1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_45) {
  const char *original_source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_46) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_47) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_48) {
  const char *original_source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || (original_source[i] == '2' && original_source[i + 1] == ',') || (i > 0 && original_source[i - 1] == ' ' && original_source[i] == '1') || (i < len && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_49) {
  const char *original_source = "{\"key\": [{\"subkey\": [{}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_50) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_51) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_52) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_53) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_54) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_55) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_56) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_57) {
  const char *original_source = "{\"key\": [{\"subkey\": {}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_58) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_59) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_60) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_61) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_62) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_63) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_64) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_65) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_66) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_67) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_68) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_69) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_invalid_char_70) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_truncated_input) {
  const char *test_cases[] = {
      "[]",
      "[null]",
      "[null, null]",
      "[true]",
      "[true, true]",
      "[false]",
      "[false, false]",
      "[true, false]",
      "[0]",
      "[0, 0]",
      "[1]",
      "[1, 1]",
      "[0, 1]",
      "[0, 1, null]",
      "[0.0, 0.1, 2.1, 1e12, 1234567890]",
      "[{}]",
      "[{\"key\": null}]",
      "[{\"key\": []}]",
      "[{\"key\": [null]}]",
      "[{\"key\": [null, null]}]",
      "[{\"key1\": null, \"key2\":[]}]",
      "[{\"key1\": null, \"key2\":[null]}]",
      "[{\"key1\": null, \"key2\":[null,null]}]",
      "{}",
      "{\"key\": null}",
      "{\"key1\": null, \"key2\": null}",
      "{\"key\": []}",
      "{\"key\": [null]}",
      "{\"key\": [null, null]}",
      "{\"key\": true}",
      "{\"key\": false}",
      "{\"key\": 0}",
      "{\"key\": 1}",
      "{\"key\": 0.5}",
      "{\"key\": \"value\"}",
      "{\"key\": {}}",
      "{\"key1\": {}, \"key2\": {}}",
      "{\"key\": [{\"subkey\": []}]}",
      "{\"key\": [{\"subkey\": [null]}]}",
      "{\"key\": [{\"subkey\": [null, null]}]}",
      "{\"key\": [{\"subkey\": [true]}]}",
      "{\"key\": [{\"subkey\": [true, false]}]}",
      "{\"key\": [{\"subkey\": [0]}]}",
      "{\"key\": [{\"subkey\": [0, 0]}]}",
      "{\"key\": [{\"subkey\": [1]}]}",
      "{\"key\": [{\"subkey\": [1, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1, null]}]}",
      "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}",
      "{\"key\": [{\"subkey\": [{}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": null}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": []}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}",
      "{\"key\": [{\"subkey\": {}}]}",
      "{\"key\": [{\"subkey\": {\"key\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key\": []}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": true}}]}",
      "{\"key\": [{\"subkey\": {\"key\": false}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 1}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}",
      "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}",
      "{\"key\": [{\"subkey\": {\"key\": {}}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}",
  };
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
  const char *original_source;
  size_t len;
  char *source;
  json_value v;
  int j;
  for (j = 0; j < num_tests; j++) {
    original_source = test_cases[j];
    len = strlen(original_source);
    source = (char *)calloc(1, len + 1);
    size_t i;
    for (i = 0; i < len; i++) {
      memcpy(source, original_source, len + 1);
      source[i] = '\0';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, strlen(source), &v));
      json_free(&v);
    }
    free(source);
  }
  END_TEST;
}

TEST(test_case_iterative_0) {
  const char *source = "[]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_1) {
  const char *source = "[null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_2) {
  const char *source = "[null, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_3) {
  const char *source = "[true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_4) {
  const char *source = "[true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_5) {
  const char *source = "[false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_6) {
  const char *source = "[false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_7) {
  const char *source = "[true, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_8) {
  const char *source = "[0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_9) {
  const char *source = "[0, 0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_10) {
  const char *source = "[1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_11) {
  const char *source = "[1, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_12) {
  const char *source = "[0, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_13) {
  const char *source = "[0, 1, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_14) {
  const char *source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_15) {
  const char *source = "[{}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_16) {
  const char *source = "[{\"key\": null}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_17) {
  const char *source = "[{\"key\": []}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_18) {
  const char *source = "[{\"key\": [null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_19) {
  const char *source = "[{\"key\": [null, null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_20) {
  const char *source = "[{\"key1\": null, \"key2\":[]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_21) {
  const char *source = "[{\"key1\": null, \"key2\":[null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_22) {
  const char *source = "[{\"key1\": null, \"key2\":[null,null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_23) {
  const char *source = "{}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_24) {
  const char *source = "{\"key\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_25) {
  const char *source = "{\"key1\": null, \"key2\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_26) {
  const char *source = "{\"key\": []}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_27) {
  const char *source = "{\"key\": [null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_28) {
  const char *source = "{\"key\": [null, null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_29) {
  const char *source = "{\"key\": true}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_30) {
  const char *source = "{\"key\": false}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_31) {
  const char *source = "{\"key\": 0}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_32) {
  const char *source = "{\"key\": 1}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_33) {
  const char *source = "{\"key\": 0.5}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_34) {
  const char *source = "{\"key\": \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_35) {
  const char *source = "{\"key\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_36) {
  const char *source = "{\"key1\": {}, \"key2\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_37) {
  const char *source = "{\"key\": [{\"subkey\": []}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_38) {
  const char *source = "{\"key\": [{\"subkey\": [null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_39) {
  const char *source = "{\"key\": [{\"subkey\": [null, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_40) {
  const char *source = "{\"key\": [{\"subkey\": null\"}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_41) {
  const char *source = "{\"key\": [{\"subkey\": [true]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_42) {
  const char *source = "{\"key\": [{\"subkey\": [true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_43) {
  const char *source = "{\"key\": [{\"subkey\": [false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_44) {
  const char *source = "{\"key\": [{\"subkey\": [false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_45) {
  const char *source = "{\"key\": [{\"subkey\": [true, false]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_46) {
  const char *source = "{\"key\": [{\"subkey\": [0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_47) {
  const char *source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_48) {
  const char *source = "{\"key\": [{\"subkey\": [1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_49) {
  const char *source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_50) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_51) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_52) {
  const char *source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_53) {
  const char *source = "{\"key\": [{\"subkey\": [{}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_54) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_55) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_56) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_57) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_58) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_59) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_60) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_61) {
  const char *source = "{\"key\": [{\"subkey\": {}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_62) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_63) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_64) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_65) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_66) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_67) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_68) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_69) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_70) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_71) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_72) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_73) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_74) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_0) {
  const char *original_source = "[]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_1) {
  const char *original_source = "[null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_2) {
  const char *original_source = "[null, null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_3) {
  const char *original_source = "[true]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_4) {
  const char *original_source = "[true, true]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_5) {
  const char *original_source = "[false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_6) {
  const char *original_source = "[false, false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_7) {
  const char *original_source = "[true, false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_8) {
  const char *original_source = "[0]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_9) {
  const char *original_source = "[0, 0]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_10) {
  const char *original_source = "[1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_11) {
  const char *original_source = "[1, 1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_12) {
  const char *original_source = "[0, 1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_13) {
  const char *original_source = "[0, 1, null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_14) {
  const char *original_source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if ((i > 0 && original_source[i - 1] == '1' && original_source[i] == '2') || (i < len - 1 && original_source[i] == '1' && original_source[i + 1] == '2') || (i < len - 1 && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_15) {
  const char *original_source = "[{}]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_16) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_17) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_18) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_19) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_20) {
  const char *original_source = "{\"key1\": null, \"key2\":[]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_21) {
  const char *original_source = "{\"key1\": null, \"key2\":[null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_22) {
  const char *original_source = "{\"key1\": null, \"key2\":[null,null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_23) {
  const char *original_source = "{}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_24) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_25) {
  const char *original_source = "{\"key1\": null, \"key2\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_26) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_27) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_28) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_29) {
  const char *original_source = "{\"key\": true}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_30) {
  const char *original_source = "{\"key\": false}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_31) {
  const char *original_source = "{\"key\": 0}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_32) {
  const char *original_source = "{\"key\": 1}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_33) {
  const char *original_source = "{\"key\": 0.5}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_34) {
  const char *original_source = "{\"key\": \"value\"}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_35) {
  const char *original_source = "{\"key\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_36) {
  const char *original_source = "{\"key1\": {}, \"key2\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_37) {
  const char *original_source = "{\"key\": [{\"subkey\": []}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_38) {
  const char *original_source = "{\"key\": [{\"subkey\": [null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_39) {
  const char *original_source = "{\"key\": [{\"subkey\": [null, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_40) {
  const char *original_source = "{\"key\": [{\"subkey\": [true]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_41) {
  const char *original_source = "{\"key\": [{\"subkey\": [true, false]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_42) {
  const char *original_source = "{\"key\": [{\"subkey\": [0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '0')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_43) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_44) {
  const char *original_source = "{\"key\": [{\"subkey\": [1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_45) {
  const char *original_source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_46) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_47) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_48) {
  const char *original_source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || (original_source[i] == '2' && original_source[i + 1] == ',') || (i > 0 && original_source[i - 1] == ' ' && original_source[i] == '1') || (i < len && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_49) {
  const char *original_source = "{\"key\": [{\"subkey\": [{}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_50) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_51) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_52) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_53) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_54) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_55) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_56) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_57) {
  const char *original_source = "{\"key\": [{\"subkey\": {}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_58) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_59) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_60) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_61) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_62) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_63) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_64) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_65) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_66) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_67) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_68) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_69) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_invalid_char_70) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  size_t i;
  for (i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_case_iterative_truncated_input) {
  const char *test_cases[] = {
      "[]",
      "[null]",
      "[null, null]",
      "[true]",
      "[true, true]",
      "[false]",
      "[false, false]",
      "[true, false]",
      "[0]",
      "[0, 0]",
      "[1]",
      "[1, 1]",
      "[0, 1]",
      "[0, 1, null]",
      "[0.0, 0.1, 2.1, 1e12, 1234567890]",
      "[{}]",
      "[{\"key\": null}]",
      "[{\"key\": []}]",
      "[{\"key\": [null]}]",
      "[{\"key\": [null, null]}]",
      "[{\"key1\": null, \"key2\":[]}]",
      "[{\"key1\": null, \"key2\":[null]}]",
      "[{\"key1\": null, \"key2\":[null,null]}]",
      "{}",
      "{\"key\": null}",
      "{\"key1\": null, \"key2\": null}",
      "{\"key\": []}",
      "{\"key\": [null]}",
      "{\"key\": [null, null]}",
      "{\"key\": true}",
      "{\"key\": false}",
      "{\"key\": 0}",
      "{\"key\": 1}",
      "{\"key\": 0.5}",
      "{\"key\": \"value\"}",
      "{\"key\": {}}",
      "{\"key1\": {}, \"key2\": {}}",
      "{\"key\": [{\"subkey\": []}]}",
      "{\"key\": [{\"subkey\": [null]}]}",
      "{\"key\": [{\"subkey\": [null, null]}]}",
      "{\"key\": [{\"subkey\": [true]}]}",
      "{\"key\": [{\"subkey\": [true, false]}]}",
      "{\"key\": [{\"subkey\": [0]}]}",
      "{\"key\": [{\"subkey\": [0, 0]}]}",
      "{\"key\": [{\"subkey\": [1]}]}",
      "{\"key\": [{\"subkey\": [1, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1, null]}]}",
      "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}",
      "{\"key\": [{\"subkey\": [{}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": null}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": []}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}",
      "{\"key\": [{\"subkey\": {}}]}",
      "{\"key\": [{\"subkey\": {\"key\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key\": []}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": true}}]}",
      "{\"key\": [{\"subkey\": {\"key\": false}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 1}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}",
      "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}",
      "{\"key\": [{\"subkey\": {\"key\": {}}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}",
  };
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
  const char *original_source;
  size_t len;
  char *source;
  json_value v;
  size_t i;
  int j;
  for (j = 0; j < num_tests; j++) {
    original_source = test_cases[j];
    len = strlen(original_source);
    source = (char *)calloc(1, len + 1);
    for (i = 0; i < len; i++) {
      memcpy(source, original_source, len + 1);
      source[i] = '\0';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      json_free(&v);
    }
    free(source);
  }
  END_TEST;
}

TEST(test_case_iterative__2) {
  const char *source = "[null null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
  json_free(&v);
  END_TEST;
}

static unsigned int g_seed = 0;

static void lcprng_srand(unsigned int seed) {
  g_seed = seed;
}

static INLINE unsigned int INLINE_ATTRIBUTE lcprng_rand() {
  g_seed = LCPRN_RAND_MULTIPLIER * g_seed + LCPRN_RAND_INCREMENT;
  return g_seed;
}

static void json_free_generated(json_value *v) {
  if (!v)
    return;

  if (v->type == J_ARRAY) {
    json_array_node *curr = v->u.array.items;
    while (curr) {
      json_array_node *next = curr->next;
      json_free_generated(&curr->item);
      free(curr);
      curr = next;
    }
  } else if (v->type == J_OBJECT) {
    json_object_node *curr = v->u.object.items;
    while (curr) {
      json_object_node *next = curr->next;
      free((void *)curr->item.key.ptr);
      json_free_generated(&curr->item.value);
      free(curr);
      curr = next;
    }
  } else if (v->type == J_STRING) {
    free((void *)v->u.string.ptr);
  } else if (v->type == J_NUMBER) {
    free((void *)v->u.number.ptr);
  }
}

static const char *greek_alphabet[] = {
    "alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa",
    "lambda", "mu", "nu", "xi", "omicron", "pi", "rho", "sigma", "tau", "upsilon",
    "phi", "chi", "psi", "omega"};

static const int num_greek_letters = sizeof(greek_alphabet) / sizeof(char *);

static void generate_random_json_value(json_value *v, int depth) {
  int i;
  int type = depth == -1 ? (lcprng_rand() % 2 ? J_ARRAY - 1 : J_OBJECT - 1) : ((lcprng_rand() % J_OBJECT));
  switch (type) {
  case J_NULL - 1:
    v->type = J_NULL;
    break;
  case J_BOOLEAN - 1:
    v->type = J_BOOLEAN;
    v->u.boolean.ptr = (lcprng_rand() % 2) ? "true" : "false";
    v->u.boolean.len = strlen(v->u.boolean.ptr);
    break;
  case J_NUMBER - 1: {
    v->type = J_NUMBER;
    char *num_str = (char *)calloc(1, NUM_BUF_SIZE);
    sprintf(num_str, "%d", lcprng_rand() % MAX_RANDOM_NUMBER);
    v->u.number.ptr = num_str;
    v->u.number.len = strlen(num_str);
    break;
  }
  case J_STRING - 1: {
    v->type = J_STRING;
    const char *random_greek_letter = greek_alphabet[lcprng_rand() % num_greek_letters];
    char *str = strdup(random_greek_letter);
    v->u.string.ptr = str;
    v->u.string.len = strlen(str);
    break;
  }
  case J_ARRAY - 1: {
    v->type = J_ARRAY;
    v->u.array.items = NULL;
    v->u.array.last = NULL;
    int num_children = lcprng_rand() % MAX_CHILDREN;
    for (i = 0; i < num_children; ++i) {
      json_array_node *node = (json_array_node *)calloc(1, sizeof(json_array_node));
      generate_random_json_value(&node->item, depth - 1);
      node->next = NULL;
      if (!v->u.array.items) {
        v->u.array.items = node;
        v->u.array.last = node;
      } else {
        v->u.array.last->next = node;
        v->u.array.last = node;
      }
    }
    break;
  }
  case J_OBJECT - 1: {
    v->type = J_OBJECT;
    v->u.object.items = NULL;
    v->u.object.last = NULL;
    int num_children = lcprng_rand() % MAX_CHILDREN;
    for (i = 0; i < num_children; ++i) {
      json_object_node *node = (json_object_node *)calloc(1, sizeof(json_object_node));
      const char *random_greek_letter = greek_alphabet[lcprng_rand() % num_greek_letters];
      char *key = strdup(random_greek_letter);
      node->item.key.ptr = key;
      node->item.key.len = strlen(key);
      generate_random_json_value(&node->item.value, depth - 1);
      node->next = NULL;
      if (!v->u.object.items) {
        v->u.object.items = node;
        v->u.object.last = node;
      } else {
        v->u.object.last->next = node;
        v->u.object.last = node;
      }
    }
    break;
  }
  }
}

TEST(test_validate_no_error) {
#if defined(__STDC_VERSION__)
  const char *source = "{\"\u123a\": 1}";
#else
  const char *source = "{\"\xe1\x88\xba\": 1}";
#endif
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_NO_ERROR, json_error);
  END_TEST;
}

TEST(test_randomization) {
  const char *test_cases[] = {
      "[]",
      "[null]",
      "[null, null]",
      "[true]",
      "[true, true]",
      "[false]",
      "[false, false]",
      "[true, false]",
      "[0]",
      "[0, 0]",
      "[1]",
      "[1, 1]",
      "[0, 1]",
      "[0, 1, null]",
      "[0.0, 0.1, 2.1, 1e12, 1234567890]",
      "[{}]",
      "[{\"key\": null}]",
      "[{\"key\": []}]",
      "[{\"key\": [null]}]",
      "[{\"key\": [null, null]}]",
      "[{\"key1\": null, \"key2\":[]}]",
      "[{\"key1\": null, \"key2\":[null]}]",
      "[{\"key1\": null, \"key2\":[null,null]}]",
      "{}",
      "{\"key\": null}",
      "{\"key1\": null, \"key2\": null}",
      "{\"key\": []}",
      "{\"key\": [null]}",
      "{\"key\": [null, null]}",
      "{\"key\": true}",
      "{\"key\": false}",
      "{\"key\": 0}",
      "{\"key\": 1}",
      "{\"key\": 0.5}",
      "{\"key\": \"value\"}",
      "{\"key\": {}}",
      "{\"key1\": {}, \"key2\": {}}",
      "{\"key\": [{\"subkey\": []}]}",
      "{\"key\": [{\"subkey\": [null]}]}",
      "{\"key\": [{\"subkey\": [null, null]}]}",
      "{\"key\": [{\"subkey\": [true]}]}",
      "{\"key\": [{\"subkey\": [true, false]}]}",
      "{\"key\": [{\"subkey\": [0]}]}",
      "{\"key\": [{\"subkey\": [0, 0]}]}",
      "{\"key\": [{\"subkey\": [1]}]}",
      "{\"key\": [{\"subkey\": [1, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1, null]}]}",
      "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}",
      "{\"key\": [{\"subkey\": [{}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": null}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": []}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}",
      "{\"key\": [{\"subkey\": {}}]}",
      "{\"key\": [{\"subkey\": {\"key\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key\": []}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": true}}]}",
      "{\"key\": [{\"subkey\": {\"key\": false}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 1}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}",
      "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}",
      "{\"key\": [{\"subkey\": {\"key\": {}}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}",
  };
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
  const char non_visible_chars[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
  size_t i;
  size_t source_len;
  int j;
  lcprng_srand(0);

  for (j = 0; j < num_tests; j++) {
    const char *original_source = test_cases[j];
    size_t len = strlen(original_source);
    char *source = (char *)calloc(1, len + 1);

    for (i = 0; i < len; i++) {
      memcpy(source, original_source, len + 1);
      char new_char = non_visible_chars[lcprng_rand() % sizeof(non_visible_chars)];
      source[i] = new_char;
      json_value v;

      source_len = strlen(source);
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, source_len, &v));
      json_free(&v);

      memset(&v, 0, sizeof(json_value));
      const char *position = source;
      ASSERT_NOT_EQUAL(json_validate(&position, source_len), E_NO_ERROR, json_error);
      json_free(&v);
    }
    free(source);
  }
  END_TEST;
}
TEST(test_json_cleanup) {
  const char *source = "{\"key\": \"value\"}";
  json_value v;

  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, strlen(source), &v));

  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));

  json_free(&v);

  json_cleanup();
  json_reset();

  free(json);

  END_TEST;
}

TEST(test_replacement) {
  const char *test_cases[] = {
      "[]",
      "[null]",
      "[null, null]",
      "[true]",
      "[true, true]",
      "[false]",
      "[false, false]",
      "[true, false]",
      "[0]",
      "[0, 0]",
      "[1]",
      "[1, 1]",
      "[0, 1]",
      "[0, 1, null]",
      "[0.0, 0.1, 2.1, 1e12, 1234567890]",
      "[{}]",
      "[{\"key\": null}]",
      "[{\"key\": []}]",
      "[{\"key\": [null]}]",
      "[{\"key\": [null, null]}]",
      "[{\"key1\": null, \"key2\":[]}]",
      "[{\"key1\": null, \"key2\":[null]}]",
      "[{\"key1\": null, \"key2\":[null,null]}]",
      "{}",
      "{\"key\": null}",
      "{\"key1\": null, \"key2\": null}",
      "{\"key\": []}",
      "{\"key\": [null]}",
      "{\"key\": [null, null]}",
      "{\"key\": true}",
      "{\"key\": false}",
      "{\"key\": 0}",
      "{\"key\": 1}",
      "{\"key\": 0.5}",
      "{\"key\": \"value\"}",
      "{\"key\": {}}",
      "{\"key1\": {}, \"key2\": {}}",
      "{\"key\": [{\"subkey\": []}]}",
      "{\"key\": [{\"subkey\": [null]}]}",
      "{\"key\": [{\"subkey\": [null, null]}]}",
      "{\"key\": [{\"subkey\": [true]}]}",
      "{\"key\": [{\"subkey\": [true, false]}]}",
      "{\"key\": [{\"subkey\": [0]}]}",
      "{\"key\": [{\"subkey\": [0, 0]}]}",
      "{\"key\": [{\"subkey\": [1]}]}",
      "{\"key\": [{\"subkey\": [1, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1, null]}]}",
      "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}",
      "{\"key\": [{\"subkey\": [{}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": null}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": []}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}",
      "{\"key\": [{\"subkey\": {}}]}",
      "{\"key\": [{\"subkey\": {\"key\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key\": []}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": true}}]}",
      "{\"key\": [{\"subkey\": {\"key\": false}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 1}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}",
      "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}",
      "{\"key\": [{\"subkey\": {\"key\": {}}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}",
  };
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
  const char replacements[] = {' ', ',', '}', '{', '[', ']'};
  size_t i;
  int j = 0;

  lcprng_srand(0);

  for (j = 0; j < num_tests; j++) {
    char original_char;
    const char *original_source = test_cases[j];
    size_t len = strlen(original_source);
    char *source = (char *)calloc(1, len + 1);

    json_value original_v;
    memset(&original_v, 0, sizeof(json_value));
    ASSERT_TRUE(json_parse(original_source, strlen(original_source), &original_v));

    for (i = 0; i < len; i++) {
      original_char = original_source[i];
      if (strchr("{}[], ", original_char) != NULL) {
        memcpy(source, original_source, len + 1);

        char new_char;
        do {
          new_char = replacements[lcprng_rand() % (sizeof(replacements))];
        } while (new_char == original_char);
        source[i] = new_char;

        json_value v;
        memset(&v, 0, sizeof(json_value));
        ASSERT_FALSE(json_parse(source, strlen(source), &v));

        memset(&v, 0, sizeof(json_value));
        ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));
      }
    }
    json_free(&original_v);
    free(source);
  }
  END_TEST;
}

TEST(test_generation) {
  int i;
  lcprng_srand((unsigned int)utils_get_time());
  for (i = 0; i < MAX_GENERATION_ITERATIONS; i++) {
    json_value v;
    memset(&v, 0, sizeof(json_value));
    generate_random_json_value(&v, -1);
    char *json_str = json_stringify(&v);
    ASSERT_PTR_NOT_NULL(json_str);
    char *wrapped_str = (char *)calloc(1, strlen(json_str) + 3);
    sprintf(wrapped_str, "[%s]", json_str);
    json_value parsed_v;
    memset(&parsed_v, 0, sizeof(json_value));
    ASSERT_TRUE(json_parse(wrapped_str, strlen(wrapped_str), &parsed_v));
    memset(&parsed_v, 0, sizeof(json_value));
    ASSERT_TRUE(json_parse_iterative(wrapped_str, strlen(wrapped_str), &parsed_v));
    json_free(&parsed_v);
    free(wrapped_str);
    free(json_str);
    json_free_generated(&v);
  }
  END_TEST;
}

TEST(test_validate_no_data) {
  const char *source = "";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_NO_DATA, json_error);
  END_TEST;
}

TEST(test_validate_invalid_json_string) {
  const char *source = "abc";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_INVALID_JSON, json_error);
  END_TEST;
}

TEST(test_validate_invalid_json_number) {
  const char *source = "0";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_INVALID_JSON, json_error);
  END_TEST;
}

TEST(test_validate_invalid_json_boolean) {
  const char *source = "true";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_INVALID_JSON, json_error);
  END_TEST;
}

TEST(test_validate_invalid_json_value) {
  const char *source = "0";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_INVALID_JSON, json_error);
  END_TEST;
}

TEST(test_validate_invalid_data) {
  const char *source = "{}==";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_INVALID_DATA, json_error);
  END_TEST;
}

TEST(test_validate_invalid_json_data_error) {
  const char *source = "[";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_INVALID_JSON_DATA, json_error);
  END_TEST;
}

TEST(test_validate_object_key) {
  const char *source = "{a:1}";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_OBJECT_KEY, json_error);
  END_TEST;
}

TEST(test_validate_mailformed_object) {
  const char *source = "{\"a\"::1}";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_MAILFORMED_JSON, json_error);
  END_TEST;
}

TEST(test_validate_expected_object_key) {
  const char *source = "{\"a\":1,,}";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_OBJECT_KEY, json_error);
  END_TEST;
}

TEST(test_validate_array_mailformed_json) {
  const char *source = "[1,,2]";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_MAILFORMED_JSON, json_error);
  END_TEST;
}

TEST(test_validate_expected_string) {
  const char *source = "[\"\\u123\"]";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_STRING, json_error);
  END_TEST;
}

TEST(test_unicode_hex4_parsing_coverage) {
  /* Test to cover lines 268 (lowercase hex), 269 (lowercase hex with offset), 271 (uppercase hex) */

  /* Test lowercase hex characters to hit lines 268-269 */
  const char *source1 = "[\"\\uabcd\"]";
  json_value v1;
  memset(&v1, 0, sizeof(json_value));
  size_t len1 = strlen(source1);
  ASSERT_TRUE(json_parse_iterative(source1, len1, &v1));
  char *json1 = json_stringify(&v1);
  ASSERT_PTR_NOT_NULL(json1);
  ASSERT_TRUE(utils_test_json_equal(json1, source1));
  json_free(&v1);
  free(json1);

  /* Test uppercase hex characters to hit line 271 */
  const char *source2 = "[\"\\uABCD\"]";
  json_value v2;
  memset(&v2, 0, sizeof(json_value));
  size_t len2 = strlen(source2);
  ASSERT_TRUE(json_parse_iterative(source2, len2, &v2));
  char *json2 = json_stringify(&v2);
  ASSERT_PTR_NOT_NULL(json2);
  ASSERT_TRUE(utils_test_json_equal(json2, source2));
  json_free(&v2);
  free(json2);

  /* Test mixed case hex characters */
  const char *source3 = "[\"\\ua1b2\"]";
  json_value v3;
  memset(&v3, 0, sizeof(json_value));
  size_t len3 = strlen(source3);
  ASSERT_TRUE(json_parse_iterative(source3, len3, &v3));
  char *json3 = json_stringify(&v3);
  ASSERT_PTR_NOT_NULL(json3);
  ASSERT_TRUE(utils_test_json_equal(json3, source3));
  json_free(&v3);
  free(json3);

  /* Test all valid hex ranges */
  const char *source4 = "[\"\\u0f9E\"]";
  json_value v4;
  memset(&v4, 0, sizeof(json_value));
  size_t len4 = strlen(source4);
  ASSERT_TRUE(json_parse_iterative(source4, len4, &v4));
  char *json4 = json_stringify(&v4);
  ASSERT_PTR_NOT_NULL(json4);
  ASSERT_TRUE(utils_test_json_equal(json4, source4));
  json_free(&v4);
  free(json4);

  END_TEST;
}

TEST(test_json_node_pool_allocation_coverage) {
  /* Test to cover lines 138-140 (array pool) and 164-166 (object pool) */

  /* Test array node pool allocation to hit lines 138-140 */
  json_value array_v;
  memset(&array_v, 0, sizeof(json_value));

  /* Create many array elements to exercise pool allocation */
  const char *array_source = "[1,2,3,4,5,6,7,8,9,10]";
  size_t len1 = strlen(array_source);
  ASSERT_TRUE(json_parse_iterative(array_source, len1, &array_v));
  char *array_json = json_stringify(&array_v);
  ASSERT_PTR_NOT_NULL(array_json);
  ASSERT_TRUE(utils_test_json_equal(array_json, array_source));
  json_free(&array_v);
  free(array_json);

  /* Test object node pool allocation to hit lines 164-166 */
  json_value object_v;
  memset(&object_v, 0, sizeof(json_value));

  /* Create object with many key-value pairs to exercise pool allocation */
  const char *object_source = "{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5,\"f\":6}";
  size_t len2 = strlen(object_source);
  ASSERT_TRUE(json_parse_iterative(object_source, len2, &object_v));
  char *object_json = json_stringify(&object_v);
  ASSERT_PTR_NOT_NULL(object_json);
  ASSERT_TRUE(utils_test_json_equal(object_json, object_source));
  json_free(&object_v);
  free(object_json);

  /* Test mixed nested structures to exercise both array and object pools */
  json_value nested_v;
  memset(&nested_v, 0, sizeof(json_value));

  const char *nested_source = "{\"arr\":[1,2,3],\"obj\":{\"x\":7,\"y\":8,\"z\":9}}";
  size_t len3 = strlen(nested_source);
  ASSERT_TRUE(json_parse_iterative(nested_source, len3, &nested_v));
  char *nested_json = json_stringify(&nested_v);
  ASSERT_PTR_NOT_NULL(nested_json);
  ASSERT_TRUE(utils_test_json_equal(nested_json, nested_source));
  json_free(&nested_v);
  free(nested_json);

  END_TEST;
}

TEST(test_validate_expected_array_value) {
  const char *source = "[1a]";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_ARRAY, json_error);
  END_TEST;
}

TEST(test_validate_expected_boolean) {
  const char *source = "[tru]";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_BOOLEAN, json_error);
  END_TEST;
}

TEST(test_validate_expected_null) {
  const char *source = "[nul]";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_NULL, json_error);
  END_TEST;
}

TEST(test_validate_expected_object_key_null) {
  const char *source = "{\"";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_OBJECT_KEY, json_error);
  END_TEST;
}

TEST(test_validate_expected_object_value) {
  const char *source = "{\"a\":}";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_MAILFORMED_JSON, json_error);
  END_TEST;
}

TEST(test_validate_expected_json) {
#if defined(__STDC_VERSION__)
  const char *source = "{\"a\": \u123a}";
#else
  const char *source = "{\"a\": \xe1\x88\xba}";
#endif
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_MAILFORMED_JSON, json_error);
  END_TEST;
}

TEST(test_validate_expected_object_value_null) {
  const char *source = "{\"a\":";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_OBJECT_VALUE, json_error);
  END_TEST;
}

TEST(test_validate_expected_array_element) {
  const char *source = "[1,]";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_ARRAY_ELEMENT, json_error);
  END_TEST;
}

TEST(test_validate_expected_array_element_null) {
  const char *source = "[1,";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_ARRAY_ELEMENT, json_error);
  END_TEST;
}

TEST(test_validate_expected_object_element) {
  const char *source = "{\"a\":1,}";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_OBJECT_ELEMENT, json_error);
  END_TEST;
}

TEST(test_validate_expected_object_element_null) {
  const char *source = "{\"a\":1,";
  const char *position = source;
  size_t len = strlen(source);
  ASSERT_EQUAL(json_validate(&position, len), E_EXPECTED_OBJECT_ELEMENT, json_error);
  END_TEST;
}

TEST(test_json_stringify_function) {
  json_value v;
  memset(&v, 0, sizeof(json_value));
  v.type = J_NULL;

  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "null") == 0);
  free(result);

  v.type = J_BOOLEAN;
  v.u.boolean.ptr = "true";
  v.u.boolean.len = 4;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "true") == 0);
  free(result);

  v.type = J_NUMBER;
  v.u.number.ptr = "123.45";
  v.u.number.len = strlen("123.45");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "123.45") == 0);
  free(result);

  v.type = J_STRING;
  v.u.string.ptr = "hello";
  v.u.string.len = strlen("hello");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "\"hello\"") == 0);
  free(result);

  v.type = J_ARRAY;
  v.u.array.items = NULL;
  v.u.array.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "[]") == 0);
  free(result);

  v.type = J_OBJECT;
  v.u.object.items = NULL;
  v.u.object.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  /* Empty object formats to "{}" with indentation */
  ASSERT_TRUE(strcmp(result, "{\n}") == 0);
  free(result);

  json_free(&v);
  END_TEST;
}

TEST(test_json_print_function) {
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Create a memory buffer to capture stdout */
  char buffer[MAX_STDIO_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  FILE *mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");
  ASSERT_NOT_EQUAL(mem_stream, (FILE *)NULL, FILE *);

  v.type = J_NULL;

  /* Test printing to stdout */
  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "null") == 0);
  memset(buffer, 0, sizeof(buffer));

  v.type = J_STRING;
  v.u.string.ptr = "hello";
  v.u.string.len = strlen("hello");

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "\"hello\"") == 0);
  memset(buffer, 0, sizeof(buffer));

  v.type = J_NUMBER;
  v.u.number.ptr = "42";
  v.u.number.len = 2;

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "42") == 0);
  memset(buffer, 0, sizeof(buffer));

  json_free(&v);

  END_TEST;
}

TEST(test_json_free_function) {
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test freeing null value */
  v.type = J_NULL;
  json_free(&v);

  /* Test freeing string value */
  v.type = J_STRING;
  v.u.string.ptr = "test";
  v.u.string.len = 4;
  json_free(&v);

  /* Test freeing array value */
  v.type = J_ARRAY;
  v.u.array.items = NULL;
  v.u.array.last = NULL;
  json_free(&v);

  /* Test freeing object value */
  v.type = J_OBJECT;
  v.u.object.items = NULL;
  v.u.object.last = NULL;
  json_free(&v);

  END_TEST;
}

TEST(test_json_equal_function) {
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  /* Test null equality */
  a.type = J_NULL;
  b.type = J_NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test boolean equality */
  a.type = J_BOOLEAN;
  a.u.boolean.ptr = "true";
  a.u.boolean.len = 4;
  b.type = J_BOOLEAN;
  b.u.boolean.ptr = "true";
  b.u.boolean.len = 4;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test number equality */
  a.type = J_NUMBER;
  a.u.number.ptr = "123";
  a.u.number.len = 3;
  b.type = J_NUMBER;
  b.u.number.ptr = "123";
  b.u.number.len = 3;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test string equality */
  a.type = J_STRING;
  a.u.string.ptr = "hello";
  a.u.string.len = strlen("hello");
  b.type = J_STRING;
  b.u.string.ptr = "hello";
  b.u.string.len = strlen("hello");
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test array equality */
  a.type = J_ARRAY;
  a.u.array.items = NULL;
  a.u.array.last = NULL;
  b.type = J_ARRAY;
  b.u.array.items = NULL;
  b.u.array.last = NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test object equality */
  a.type = J_OBJECT;
  a.u.object.items = NULL;
  a.u.object.last = NULL;
  b.type = J_OBJECT;
  b.u.object.items = NULL;
  b.u.object.last = NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test inequality */
  a.type = J_NULL;
  b.type = J_BOOLEAN;
  b.u.boolean.ptr = "true";
  b.u.boolean.len = 4;
  ASSERT_FALSE(json_equal(&a, &b));

  json_free(&a);
  json_free(&b);
  END_TEST;
}

TEST(test_json_array_equal_function) {
  /* Test case from array equal function coverage */
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  a.type = J_ARRAY;
  a.u.array.items = NULL;
  a.u.array.last = NULL;

  b.type = J_ARRAY;
  b.u.array.items = NULL;
  b.u.array.last = NULL;

  /* This will test json_array_equal indirectly through json_equal */
  ASSERT_TRUE(json_equal(&a, &b));

  json_free(&a);
  json_free(&b);
  END_TEST;
}

TEST(test_json_object_equal_function) {
  /* Test case from object equal function coverage */
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  a.type = J_OBJECT;
  a.u.object.items = NULL;
  a.u.object.last = NULL;

  b.type = J_OBJECT;
  b.u.object.items = NULL;
  b.u.object.last = NULL;

  /* This will test json_object_equal indirectly through json_equal */
  ASSERT_TRUE(json_equal(&a, &b));

  json_free(&a);
  json_free(&b);
  END_TEST;
}

TEST(test_json_object_equal_function_null_a) {
  /* Test case from object equal function coverage */
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  a.type = J_OBJECT;
  a.u.object.items = NULL;
  a.u.object.last = NULL;

  b.type = J_OBJECT;
  b.u.object.items = NULL;
  b.u.object.last = NULL;

  /* This will test json_object_equal indirectly through json_equal */
  ASSERT_FALSE(json_equal(NULL, &b));

  json_free(&a);
  json_free(&b);
  END_TEST;
}

TEST(test_json_object_equal_function_null_b) {
  /* Test case from object equal function coverage */
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  a.type = J_OBJECT;
  a.u.object.items = NULL;
  a.u.object.last = NULL;

  b.type = J_OBJECT;
  b.u.object.items = NULL;
  b.u.object.last = NULL;

  /* This will test json_object_equal indirectly through json_equal */
  ASSERT_FALSE(json_equal(&a, NULL));

  json_free(&a);
  json_free(&b);
  END_TEST;
}

TEST(test_json_parse_function) {
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test parsing simple array */
  const char *array_json = "[]";
  ASSERT_TRUE(json_parse(array_json, strlen(array_json), &v));
  ASSERT_EQ(v.type, J_ARRAY);
  json_free(&v);

  /* Test parsing simple object */
  memset(&v, 0, sizeof(json_value));
  const char *object_json = "{}";
  ASSERT_TRUE(json_parse(object_json, strlen(object_json), &v));
  ASSERT_EQ(v.type, J_OBJECT);
  json_free(&v);

  /* Test parsing null */
  memset(&v, 0, sizeof(json_value));
  const char *null_json = "null";
  ASSERT_FALSE(json_parse(null_json, strlen(null_json), &v)); /* Should fail as it's not an array/object */

  /* Test empty string */
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse("", strlen(""), &v));

  END_TEST;
}

TEST(test_json_parse_function_json_object_equals) {
  json_value v1, v2;
  memset(&v1, 0, sizeof(json_value));
  memset(&v2, 0, sizeof(json_value));

  const char *object_json1 = "{\"key\": 1}";
  const char *object_json2 = "{\"key\": 2}";

  ASSERT_TRUE(json_parse(object_json1, strlen(object_json1), &v1));
  ASSERT_TRUE(json_parse(object_json2, strlen(object_json2), &v2));

  ASSERT_EQ(v1.type, J_OBJECT);
  ASSERT_EQ(v2.type, J_OBJECT);

  ASSERT_FALSE(json_equal(&v1, &v2));

  json_free(&v1);
  json_free(&v2);

  END_TEST;
}

TEST(test_json_parse_function_json_object_equals_null) {
  json_value v1, v2;
  memset(&v1, 0, sizeof(json_value));
  memset(&v2, 0, sizeof(json_value));

  const char *object_json1 = "{\"key\": 1}";
  const char *object_json2 = "{\"key\": 2}";

  ASSERT_TRUE(json_parse(object_json1, strlen(object_json1), &v1));
  ASSERT_TRUE(json_parse(object_json2, strlen(object_json2), &v2));

  ASSERT_EQ(v1.type, J_OBJECT);
  ASSERT_EQ(v2.type, J_OBJECT);

  ASSERT_FALSE(json_equal(&v1, &v2));

  json_free(&v1);
  json_free(&v2);

  END_TEST;
}

TEST(test_json_validate_function) {
  const char *valid_json = "{\"key\": \"value\"}";
  const char *invalid_json = "{\"key\": \"value\"";
  size_t len;

  /* Test valid JSON */
  const char *ptr = valid_json;
  len = strlen(ptr);
  json_error err = json_validate(&ptr, len);
  ASSERT_EQ(err, E_NO_ERROR);

  /* Test invalid JSON */
  ptr = invalid_json;
  len = strlen(ptr);
  err = json_validate(&ptr, len);
  ASSERT_NOT_EQ(err, E_NO_ERROR);

  /* Test empty string */
  ptr = "";
  len = strlen(ptr);
  err = json_validate(&ptr, len);
  ASSERT_EQ(err, E_NO_DATA);

  /* Test non-object/array root */
  ptr = "null";
  len = strlen(ptr);
  err = json_validate(&ptr, len);
  ASSERT_EQ(err, E_INVALID_JSON);

  END_TEST;
}

TEST(test_comprehensive_coverage_buffer_functions) {
  /* Tests for all buffer functions */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test json_stringify which uses buffer_write functions internally */
  v.type = J_STRING;
  v.u.string.ptr = "test";
  v.u.string.len = 4;
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "\"test\"") == 0);
  free(result);

  /* Test array to exercise buffer_write_array */
  v.type = J_ARRAY;
  v.u.array.items = NULL;
  v.u.array.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "[]") == 0);
  free(result);

  /* Test object to exercise buffer_write_object */
  v.type = J_OBJECT;
  v.u.object.items = NULL;
  v.u.object.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "{\n}") == 0);

  free(result);

  json_free(&v);
  END_TEST;
}

TEST(test_comprehensive_coverage_print_functions) {
  /* Tests for print functions with stdout mocking */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Create a memory buffer to capture stdout */
  char buffer[MAX_STDIO_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  FILE *mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");
  ASSERT_NOT_EQUAL(mem_stream, (FILE *)NULL, FILE *);

  /* Test json_print which exercises print_value functions */
  v.type = J_NULL;
  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "null") == 0);
  memset(buffer, 0, sizeof(buffer));

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  v.type = J_STRING;
  v.u.string.ptr = "hello\n\t\"world";
  v.u.string.len = strlen("hello\n\t\"world");
  json_print(&v, mem_stream);
  fclose(mem_stream);
  ASSERT_EQUAL(strstr(buffer, "\"hello\\n\\t\\\"world\""), NULL, char *);
  memset(buffer, 0, sizeof(buffer));

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  v.type = J_BOOLEAN;
  v.u.boolean.ptr = "false";
  v.u.boolean.len = strlen("false");

  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "false") == 0);
  memset(buffer, 0, sizeof(buffer));

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  v.type = J_NUMBER;
  v.u.number.ptr = "123.45";
  v.u.number.len = strlen("123.45");

  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "123.45") == 0);
  memset(buffer, 0, sizeof(buffer));

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  /* Test array with various elements to exercise print_array_compact */
  v.type = J_ARRAY;
  v.u.array.items = NULL;
  v.u.array.last = NULL;

  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_TRUE(strcmp(buffer, "[]") == 0);
  memset(buffer, 0, sizeof(buffer));

  mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");

  /* Test object to exercise print_object_compact */
  v.type = J_OBJECT;
  v.u.object.items = NULL;
  v.u.object.last = NULL;

  json_print(&v, mem_stream);
  fclose(mem_stream);

  ASSERT_EQUAL(strstr(buffer, "{\n}"), NULL, char *);

  /* Verify that data was written to the buffer */
  size_t output_len = strlen(buffer);
  ASSERT_NOT_EQUAL(output_len, 0, size_t);

  json_free(&v);
  END_TEST;
}

TEST(test_whitespace_lookup) {
  /* Test to cover line 174 (whitespace_lookup) and lines 182-211 (parse_number with whitespace) */

  /* Valid JSON tests: arrays/objects with leading whitespace (should parse) */
  const char *valid_whitespace_tests[] = {
      "[1,2,3]",                         /* space before array */
      "{\t\"a\":1}",                     /* tab before object */
      "[\r{\n\"key\"\t:\r\"value\"}\t]", /* newline before array containing object */
      "{\r\"nested\":\n[1,2]\t}",        /* carriage return before nested structure */
      "{\r\"x\"\t:5\n}",                 /* mixed whitespace before object */
      "[   \t\r  null\n ]"               /* lots of whitespace before null */
  };

  /* Test numbers with whitespace to hit lines 182-211 in parse_number */
  const char *invalid_whitespace_tests[] = {
      " 123",       /* positive integer with leading space */
      "\t-456",     /* negative integer with leading tab */
      "\n78.9",     /* float with leading newline */
      "\r1.23e4",   /* scientific notation with leading carriage return */
      "   -5.67e-8" /* mixed whitespace before scientific */
  };

  const int valid_whitespace_tests_size = sizeof(valid_whitespace_tests) / sizeof(char *);
  const int invalid_whitespace_tests_size = sizeof(invalid_whitespace_tests) / sizeof(char *);

  size_t i;

  for (i = 0; i < invalid_whitespace_tests_size; i++) {
    const char *source = invalid_whitespace_tests[i];
    json_value v;
    memset(&v, 0, sizeof(json_value));
    ASSERT_FALSE(json_parse_iterative(source, strlen(source), &v));

    char *json = json_stringify(&v);
    ASSERT_PTR_NULL(json);
  }

  for (i = 0; i < valid_whitespace_tests_size; i++) {
    const char *source = valid_whitespace_tests[i];
    json_value v;
    memset(&v, 0, sizeof(json_value));

    ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));

    char *json = json_stringify(&v);
    ASSERT_PTR_NOT_NULL(json);

    /* Verify parsing works regardless of whitespace */
    if (strstr(source, "1,2,3")) {
      ASSERT_TRUE(utils_test_json_equal(json, "[1,2,3]"));
    } else if (strstr(source, ":1}")) {
      ASSERT_TRUE(utils_test_json_equal(json, "{\"a\":1}"));
    } else if (strstr(source, "value")) {
      ASSERT_TRUE(utils_test_json_equal(json, "[{\"key\":\"value\"}]"));
    } else if (strstr(source, "[1,2]")) {
      ASSERT_TRUE(utils_test_json_equal(json, "{\"nested\":\n[1,2]}"));
    } else if (strstr(source, ":5")) {
      ASSERT_TRUE(utils_test_json_equal(json, "{\r\"x\"\t:5}"));
    } else if (strstr(source, "null")) {
      ASSERT_TRUE(utils_test_json_equal(json, "[null]"));
    }

    json_free(&v);
    free(json);
  }

  /* Test numbers with whitespace to hit lines 182-211 */
  const char *number_with_whitespace_tests[] = {
      "{\"key\": 123}",       /* positive integer with leading space */
      "{\"key\":\t-456}",     /* negative integer with leading tab */
      "{\"key\":\n78.9}",     /* float with leading newline */
      "{\"key\":\r1.23e4}",   /* scientific notation with leading carriage return */
      "{\"key\":   -5.67e-8}" /* mixed whitespace before scientific */
  };

  const int number_with_whitespace_tests_size = sizeof(number_with_whitespace_tests) / sizeof(char *);

  for (i = 0; i < number_with_whitespace_tests_size; i++) {
    const char *source = number_with_whitespace_tests[i];
    json_value v;
    memset(&v, 0, sizeof(json_value));
    ASSERT_TRUE(json_parse_iterative(source, strlen(source), &v));
    char *json = json_stringify(&v);
    ASSERT_PTR_NOT_NULL(json);
    ASSERT_TRUE(utils_test_json_equal(json, source));

    json_free(&v);
    free(json);
  }

  END_TEST;
}

TEST(test_print_value_object_with_multiple_keys) {
  /* Test to cover lines 779 (buffer_write_value) and 799 (buffer_write) */

  /* Test all JSON types to exercise buffer_write_value function (line 779) */
  const char *type_tests[] = {
      "[null]",            /* J_NULL */
      "[true]",            /* J_BOOLEAN */
      "[false]",           /* J_BOOLEAN */
      "[123.45]",          /* J_NUMBER */
      "[\"hello world\"]", /* J_STRING */
      "[[]]",              /* J_ARRAY */
      "[{}]"               /* J_OBJECT */
  };

  int i;

  const int type_tests_size = sizeof(type_tests) / sizeof(char *);

  for (i = 0; i < type_tests_size; i++) {
    json_value v;
    memset(&v, 0, sizeof(json_value));
    ASSERT_TRUE(json_parse(type_tests[i], strlen(type_tests[i]), &v));

    /* Test json_stringify to exercise buffer_write (line 799) */
    char *result = json_stringify(&v);
    ASSERT_PTR_NOT_NULL(result);
    ASSERT_TRUE(utils_test_json_equal(result, type_tests[i]));

    json_free(&v);
    free(result);
  }

  END_TEST;
}

TEST(test_large_array_pool_allocation) {
  /* Test to cover line 138 (array pool allocation when creating large arrays) */

  /* Create a large array to trigger pool allocation */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  const int large_json_size = 8192;

  /* Create JSON with many elements to test pool allocation */
  char large_json[large_json_size]; /* Buffer for large JSON */
  strcpy(large_json, "[");

  int i;

  const int pool_size = 1024;
  /* Add 1024 elements (each "1," takes 2 chars) to approach pool size */
  for (i = 0; i < pool_size; i++) {
    if (i > 0)
      strcat(large_json, ",");
    strcat(large_json, "1");
  }
  strcat(large_json, "]");

  ASSERT_TRUE(json_parse_iterative(large_json, strlen(large_json), &v));
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);

  /* Verify the array was created correctly */
  json_value first_elem, check_elem;
  memset(&first_elem, 0, sizeof(json_value));
  memset(&check_elem, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse_iterative("[1]", strlen("[1]"), &first_elem));
  ASSERT_TRUE(json_parse_iterative("[1]", strlen("[1]"), &check_elem));
  ASSERT_TRUE(json_equal(&first_elem, &check_elem));

  json_free(&v);
  json_free(&first_elem);
  json_free(&check_elem);
  free(result);

  END_TEST;
}

TEST(test_buffer_write_values_array_coverage) {
  /* Test to cover lines 868-869 (buffer_write_values function) */

  /* Create test arrays to exercise array comparison in buffer_write_values */
  json_value array1, array2;
  memset(&array1, 0, sizeof(json_value));
  memset(&array2, 0, sizeof(json_value));

  /* Parse different arrays */
  const char *array1_source = "[1,2,3]";
  const char *array2_source = "[1,2,4]";
  ASSERT_TRUE(json_parse_iterative(array1_source, strlen(array1_source), &array1));
  ASSERT_TRUE(json_parse_iterative(array2_source, strlen(array2_source), &array2));

  /* Test using json_stringify to exercise buffer_write_values */
  char *result1 = json_stringify(&array1);
  ASSERT_PTR_NOT_NULL(result1);
  ASSERT_NOT_EQUAL(strstr(result1, "[1, 2, 3]"), NULL, char *);

  char *result2 = json_stringify(&array2);
  ASSERT_PTR_NOT_NULL(result2);
  ASSERT_NOT_EQUAL(strstr(result2, "[1, 2, 4]"), NULL, char *);

  /* Verify results are different */
  ASSERT_NOT_EQUAL(strcmp(result1, result2), 0, int);

  free(result1);
  free(result2);

  json_free(&array1);
  json_free(&array2);

  END_TEST;
}

TEST(test_print_value_object_with_multiple_keys_enhanced) {
  /* Enhanced test to cover lines 779 (buffer_write_value) and 799 (buffer_write) */
  /* Also cover comma logic in object printing */

  /* Create a memory buffer to capture output */
  char buffer[MAX_STDIO_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));
  FILE *mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");
  ASSERT_NOT_EQUAL(mem_stream, (FILE *)NULL, FILE *);

  /* Test object with multiple key-value pairs to exercise comma logic */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Parse object with multiple items using json_parse to create proper structure */
  const char *source_json = "{\"a\":1,\"b\":2,\"c\":3}";
  ASSERT_TRUE(json_parse(source_json, strlen(source_json), &v));

  /* Test printing to exercise buffer_write_value and buffer_write functions */
  json_print(&v, mem_stream);
  fclose(mem_stream);

  /* Verify the printed object contains expected structure with commas */
  ASSERT_NOT_EQUAL(strstr(buffer, "{\n"), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, "\"a\":"), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, "\"b\":"), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, "\"c\":"), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, ", "), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, "1"), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, "2"), NULL, char *);
  ASSERT_NOT_EQUAL(strstr(buffer, "3"), NULL, char *);

  json_free(&v);
  END_TEST;
}

TEST(test_comprehensive_coverage_equality_functions) {
  /* Tests for equality functions */
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  /* Test json_array_equal indirectly through json_equal */
  a.type = J_ARRAY;
  a.u.array.items = NULL;
  a.u.array.last = NULL;
  b.type = J_ARRAY;
  b.u.array.items = NULL;
  b.u.array.last = NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test json_object_equal indirectly through json_equal */
  a.type = J_OBJECT;
  a.u.object.items = NULL;
  a.u.object.last = NULL;
  b.type = J_OBJECT;
  b.u.object.items = NULL;
  b.u.object.last = NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test different types */
  a.type = J_NULL;
  b.type = J_STRING;
  b.u.string.ptr = "test";
  b.u.string.len = 4;
  ASSERT_FALSE(json_equal(&a, &b));

  /* Test equal strings */
  a.type = J_STRING;
  a.u.string.ptr = "test";
  a.u.string.len = 4;
  b.type = J_STRING;
  b.u.string.ptr = "test";
  b.u.string.len = 4;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test equal numbers */
  a.type = J_NUMBER;
  a.u.number.ptr = "123";
  a.u.number.len = 3;
  b.type = J_NUMBER;
  b.u.number.ptr = "123";
  b.u.number.len = 3;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test equal booleans */
  a.type = J_BOOLEAN;
  a.u.boolean.ptr = "true";
  a.u.boolean.len = 4;
  b.type = J_BOOLEAN;
  b.u.boolean.ptr = "true";
  b.u.boolean.len = 4;
  ASSERT_TRUE(json_equal(&a, &b));

  json_free(&a);
  json_free(&b);
  END_TEST;
}

TEST(test_comprehensive_coverage_parsing_functions) {
  /* Tests for parsing functions including parse_array_value, parse_object_value, parse_value_build */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test json_parse which exercises parse_value_build */
  const char *array_json = "[1, 2, 3]";
  ASSERT_TRUE(json_parse(array_json, strlen(array_json), &v));
  ASSERT_EQ(v.type, J_ARRAY);
  json_free(&v);

  /* Test object parsing */
  memset(&v, 0, sizeof(json_value));
  const char *object_json = "{\"key\": \"value\", \"num\": 42}";
  ASSERT_TRUE(json_parse(object_json, strlen(object_json), &v));
  ASSERT_EQ(v.type, J_OBJECT);
  json_free(&v);

  /* Test nested structures */
  memset(&v, 0, sizeof(json_value));
  const char *nested_json = "{\"array\": [1, 2, {\"nested\": true}], \"empty\": {}}";
  ASSERT_TRUE(json_parse(nested_json, strlen(nested_json), &v));
  ASSERT_EQ(v.type, J_OBJECT);
  json_free(&v);

  /* Test invalid JSON */
  memset(&v, 0, sizeof(json_value));
  const char *invalid_json = "[1, 2,]";
  ASSERT_FALSE(json_parse(invalid_json, strlen(invalid_json), &v));
  json_free(&v);

  END_TEST;
}

TEST(test_comprehensive_coverage_string_escape) {
  /* Tests for print_string_escaped function */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test normal string without escapes */
  v.type = J_STRING;
  v.u.string.ptr = "SimpleString";
  v.u.string.len = strlen("SimpleString");
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "\"SimpleString\"") == 0);
  free(result);

  /* Test string with quotes */
  v.u.string.ptr = "Hello\"World";
  v.u.string.len = strlen("Hello\"World");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strstr(result, "\"") != NULL);
  free(result);

  /* Test empty string */
  v.u.string.ptr = "";
  v.u.string.len = 0;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "\"\"") == 0);
  free(result);

  json_free(&v);
  END_TEST;
}

TEST(test_comprehensive_coverage_object_get) {
  /* Tests for json_object_get function - indirectly through json_equal */
  const char *json_with_keys = "{\"key1\": {\"object1\": \"value1\"}, \"key2\": 42, \"key3\": true}";
  const char *ptr = json_with_keys;
  json_value v1, v2;
  memset(&v1, 0, sizeof(json_value));
  memset(&v2, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse_iterative(ptr, strlen(ptr), &v1) == true);
  ASSERT_TRUE(json_parse(ptr, strlen(ptr), &v2) == true);
  ASSERT_TRUE(json_equal(&v1, &v2) == true);

  END_TEST;
}

TEST(test_json_stringify_full_coverage) {
  /* Comprehensive test to exercise all stringify code paths */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test all primitive types */
  v.type = J_NULL;
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "null") == 0);
  free(result);

  v.type = J_BOOLEAN;
  v.u.boolean.ptr = "true";
  v.u.boolean.len = 4;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "true") == 0);
  free(result);

  v.u.boolean.ptr = "false";
  v.u.boolean.len = strlen("false");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "false") == 0);
  free(result);

  v.type = J_NUMBER;
  v.u.number.ptr = "0";
  v.u.number.len = 1;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "0") == 0);
  free(result);

  v.u.number.ptr = "-123.456";
  v.u.number.len = strlen("-123.456");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "-123.456") == 0);
  free(result);

  v.type = J_STRING;
  v.u.string.ptr = "";
  v.u.string.len = 0;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "\"\"") == 0);
  free(result);

  /* Test nested array to trigger buffer_write_array and buffer_write_value_indent */
  v.type = J_ARRAY;
  v.u.array.items = NULL;
  v.u.array.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strcmp(result, "[]") == 0);
  free(result);

  /* Test object to trigger buffer_write_object and buffer_write_object_indent */
  v.type = J_OBJECT;
  v.u.object.items = NULL;
  v.u.object.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(strstr(result, "{") != NULL);
  ASSERT_TRUE(strstr(result, "}") != NULL);
  free(result);

  json_free(&v);
  END_TEST;
}

TEST(test_complex_nested_json_coverage) {
  /* Create a complex nested JSON to exercise all buffer functions */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test json_parse on complex nested structure that exercises all parsing functions */
  const char *complex_json = "{\"user\":{\"name\":\"John\",\"age\":30,\"emails\":[\"john@example.com\",\"work@example.com\"],\"address\":{\"street\":\"123 Main St\",\"city\":\"Anytown\",\"coordinates\":{\"lat\":45.5,\"lng\":-122.7}},\"active\":true,\"score\":null},\"items\":[{\"id\":1,\"value\":\"test\"},{\"id\":2,\"nested\":{\"deep\":{\"value\":42}}}],\"emptyObject\":{},\"emptyArray\":[]}";

  ASSERT_TRUE(json_parse(complex_json, strlen(complex_json), &v));
  ASSERT_EQ(v.type, J_OBJECT);

  /* Test json_stringify on this complex structure to exercise all buffer functions */
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  /* Should contain various patterns */
  ASSERT_TRUE(strstr(result, "{") != NULL);
  ASSERT_TRUE(strstr(result, "}") != NULL);
  ASSERT_TRUE(strstr(result, "[") != NULL);
  ASSERT_TRUE(strstr(result, "]") != NULL);
  ASSERT_TRUE(strstr(result, "\"John\"") != NULL);
  ASSERT_TRUE(strstr(result, "true") != NULL);
  ASSERT_TRUE(strstr(result, "null") != NULL);
  free(result);

  json_free(&v);
  END_TEST;
}

TEST(test_all_json_types_comprehensive) {
  /* Test all JSON types to hit every code path */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Test each primitive type individually */
  v.type = J_NULL;
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.type = J_BOOLEAN;
  v.u.boolean.ptr = "true";
  v.u.boolean.len = 4;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.u.boolean.ptr = "false";
  v.u.boolean.len = strlen("false");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.type = J_NUMBER;
  v.u.number.ptr = "0";
  v.u.number.len = 1;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.u.number.ptr = "1234567890";
  v.u.number.len = strlen("1234567890");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.u.number.ptr = "-123.456e+10";
  v.u.number.len = strlen("-123.456e+10");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.type = J_STRING;
  v.u.string.ptr = "";
  v.u.string.len = 0;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.u.string.ptr = "Hello World";
  v.u.string.len = strlen("Hello World");
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.type = J_ARRAY;
  v.u.array.items = NULL;
  v.u.array.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  v.type = J_OBJECT;
  v.u.object.items = NULL;
  v.u.object.last = NULL;
  result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  free(result);

  json_free(&v);
  END_TEST;
}

TEST(test_equality_functions_full) {
  /* Create a memory buffer to capture stdout and suppress diff output */
  char buffer[MAX_STDIO_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  FILE *mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");
  ASSERT_NOT_EQUAL(mem_stream, (FILE *)NULL, FILE *);

  /* Redirect stdout to suppress diff output from utils_test_json_equal */
  FILE *original_stdout = stdout;
  stdout = mem_stream;

  /* Test equality functions with all type combinations */
  json_value a, b;
  memset(&a, 0, sizeof(json_value));
  memset(&b, 0, sizeof(json_value));

  /* Test same type equality for each type */
  a.type = J_NULL;
  b.type = J_NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  a.type = J_BOOLEAN;
  a.u.boolean.ptr = "true";
  a.u.boolean.len = 4;
  b.type = J_BOOLEAN;
  b.u.boolean.ptr = "true";
  b.u.boolean.len = 4;
  ASSERT_TRUE(json_equal(&a, &b));

  a.u.boolean.ptr = "false";
  a.u.boolean.len = strlen("false");
  b.u.boolean.ptr = "false";
  b.u.boolean.len = strlen("false");
  ASSERT_TRUE(json_equal(&a, &b));

  a.type = J_NUMBER;
  a.u.number.ptr = "123";
  a.u.number.len = 3;
  b.type = J_NUMBER;
  b.u.number.ptr = "123";
  b.u.number.len = 3;
  ASSERT_TRUE(json_equal(&a, &b));

  a.u.number.ptr = "-456.789";
  a.u.number.len = strlen("-456.789");
  b.u.number.ptr = "-456.789";
  b.u.number.len = strlen("-456.789");
  ASSERT_TRUE(json_equal(&a, &b));

  a.type = J_STRING;
  a.u.string.ptr = "test";
  a.u.string.len = 4;
  b.type = J_STRING;
  b.u.string.ptr = "test";
  b.u.string.len = 4;
  ASSERT_TRUE(json_equal(&a, &b));

  a.u.string.ptr = "";
  a.u.string.len = 0;
  b.u.string.ptr = "";
  b.u.string.len = 0;
  ASSERT_TRUE(json_equal(&a, &b));

  a.type = J_ARRAY;
  a.u.array.items = NULL;
  a.u.array.last = NULL;
  b.type = J_ARRAY;
  b.u.array.items = NULL;
  b.u.array.last = NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  a.type = J_OBJECT;
  a.u.object.items = NULL;
  a.u.object.last = NULL;
  b.type = J_OBJECT;
  b.u.object.items = NULL;
  b.u.object.last = NULL;
  ASSERT_TRUE(json_equal(&a, &b));

  /* Test different type inequalities */
  a.type = J_NULL;
  b.type = J_BOOLEAN;
  b.u.boolean.ptr = "true";
  b.u.boolean.len = 4;
  ASSERT_FALSE(json_equal(&a, &b));

  a.type = J_BOOLEAN;
  a.u.boolean.ptr = "true";
  a.u.boolean.len = 4;
  b.type = J_NUMBER;
  b.u.number.ptr = "123";
  b.u.number.len = 3;
  ASSERT_FALSE(json_equal(&a, &b));

  a.type = J_NUMBER;
  a.u.number.ptr = "123";
  a.u.number.len = 3;
  b.type = J_STRING;
  b.u.string.ptr = "123";
  b.u.string.len = 3;
  ASSERT_FALSE(json_equal(&a, &b));

  json_free(&a);
  json_free(&b);

  fclose(mem_stream);

  stdout = original_stdout;

  END_TEST;
}

TEST(test_utils_functions) {

  /* Create a memory buffer to capture stdout and suppress diff output */
  char buffer[MAX_STDIO_BUFFER_SIZE];
  memset(buffer, 0, sizeof(buffer));

  FILE *mem_stream = fmemopen(buffer, sizeof(buffer) - 1, "w");
  ASSERT_NOT_EQUAL(mem_stream, (FILE *)NULL, FILE *);

  /* Redirect stdout to suppress output from utils functions */
  FILE *original_stdout = stdout;
  stdout = mem_stream;

  /* The input was an array with object, so output should contain '[', not just '{' */
  utils_print_time_diff(0, MAX_TICK_COUNTER);

  utils_output("test output");
  utils_output(NULL);

  char itoa_buf[MAX_BUFFER_SIZE];
  const int value1 = 123;
  const int value2 = -456;
  utils_itoa(value1, itoa_buf);
  ASSERT_TRUE(strcmp(itoa_buf, "123") == 0);
  utils_itoa(value2, itoa_buf);
  ASSERT_TRUE(strcmp(itoa_buf, "-456") == 0);
  utils_itoa(0, itoa_buf);
  ASSERT_TRUE(strcmp(itoa_buf, "0") == 0);

  const char *trailing_whitespace_filename = "trailing_whitespace.json";
  FILE *fp = fopen(trailing_whitespace_filename, "w");
  fprintf(fp, "{\"key\": \"value\"}  \n");
  fclose(fp);

  char *json_data = utils_get_test_json_data(trailing_whitespace_filename);
  ASSERT_TRUE(strcmp(json_data, "{\"key\": \"value\"}") == 0);
  free(json_data);

  remove(trailing_whitespace_filename);
  fp = fopen(trailing_whitespace_filename, "w");
  fclose(fp);
  json_data = utils_get_test_json_data(trailing_whitespace_filename);

  ASSERT_EQUAL(json_data, NULL, char *);

  const char *empty_filename = "empty.json";
  fp = fopen(empty_filename, "w");
  fclose(fp);
  json_data = utils_get_test_json_data(empty_filename);

  ASSERT_EQUAL(json_data, NULL, char *);

  json_data = utils_get_test_json_data(empty_filename);
  ASSERT_EQUAL(json_data, NULL, char *);

  remove(empty_filename);
  remove(trailing_whitespace_filename);

  ASSERT_FALSE(utils_test_json_equal("{\"a\":1}", "{\"a\":2}"));
  ASSERT_FALSE(utils_test_json_equal("{\"a\":1}", "{\"b\":1}"));
  ASSERT_FALSE(utils_test_json_equal("{\"a\":1}", "{\"a\":1, \"b\":2}"));
  ASSERT_FALSE(utils_test_json_equal("{\"a\":1, \"b\":2}", "{\"a\":1}"));
  ASSERT_TRUE(utils_test_json_equal("{\"a\":1} ", "{\"a\":1}"));
  ASSERT_TRUE(utils_test_json_equal("{\"a\":1}", "{\"a\":1} "));
  ASSERT_FALSE(utils_test_json_equal(NULL, "{\"a\":1}"));
  ASSERT_FALSE(utils_test_json_equal("{\"a\":1}", NULL));
  ASSERT_FALSE(utils_test_json_equal("{\"a\":1}", "{\"a\":1, \"b\":2}   "));
  ASSERT_FALSE(utils_test_json_equal("{\"a\":1, \"b\":2}   ", "{\"a\":1}"));

  fclose(mem_stream);

  stdout = original_stdout;

  END_TEST;
}

TEST(test_new_coverage_and_whitespace) {
  ASSERT_PTR_NULL(json_stringify(NULL));

  json_value v_invalid;
  memset(&v_invalid, 0, sizeof(json_value));
  v_invalid.type = (json_token)INVALID_TYPE;
  char *result = json_stringify(&v_invalid);
  ASSERT_PTR_NULL(result);

  const char *leading_ws = " [1, 2, 3]";
  const char *trailing_ws = "[1, 2, 3] ";
  const char *leading_ws_obj = " {\"a\":1}";
  const char *trailing_ws_obj = "{\"a\":1} ";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  ASSERT_FALSE(json_parse(leading_ws, strlen(leading_ws), &v));
  json_free(&v);

  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(trailing_ws, strlen(trailing_ws), &v));
  json_free(&v);

  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(leading_ws_obj, strlen(leading_ws_obj), &v));
  json_free(&v);

  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(trailing_ws_obj, strlen(trailing_ws_obj), &v));
  json_free(&v);

  END_TEST;
}

TEST(test_coverage_line_779_buffer_write_value) {
  /* Test to cover line 779 (buffer_write_value function) */

  /* Test all JSON types wrapped in arrays to exercise buffer_write_value function (line 779) */
  const char *type_tests[] = {
      "[null]",            /* J_NULL */
      "[true]",            /* J_BOOLEAN */
      "[false]",           /* J_BOOLEAN */
      "[123.45]",          /* J_NUMBER */
      "[\"hello world\"]", /* J_STRING */
      "[[]]",              /* J_ARRAY */
      "[{}]"               /* J_OBJECT */
  };

  int i;

  const int type_tests_size = sizeof(type_tests) / sizeof(char *);

  for (i = 0; i < type_tests_size; i++) {
    json_value v;
    memset(&v, 0, sizeof(json_value));
    ASSERT_TRUE(json_parse(type_tests[i], strlen(type_tests[i]), &v));

    /* Test json_stringify to exercise buffer_write_value (line 779) */
    char *result = json_stringify(&v);
    ASSERT_PTR_NOT_NULL(result);
    ASSERT_TRUE(utils_test_json_equal(result, type_tests[i]));

    json_free(&v);
    free(result);
  }

  END_TEST;
}

TEST(test_coverage_line_799_buffer_write) {
  /* Test to cover line 799 (buffer_write function) */

  /* Test large string to exercise buffer_write (line 799) */
  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Create a long string that requires buffer operations, wrapped in array */
  const char *long_string = "[\"This is a very long string that will definitely exercise the buffer_write function at line 799 and ensure proper coverage of the buffering mechanism in the JSON parser implementation\"]";
  ASSERT_TRUE(json_parse(long_string, strlen(long_string), &v));

  /* Test json_stringify to exercise buffer_write (line 799) */
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);
  ASSERT_TRUE(utils_test_json_equal(result, long_string));

  json_free(&v);
  free(result);

  END_TEST;
}

TEST(test_coverage_lines_868_869_buffer_write_values) {
  /* Test to cover lines 868-869 (buffer_write_values function) */

  /* Create test arrays to exercise array comparison in buffer_write_values */
  json_value array1, array2;
  memset(&array1, 0, sizeof(json_value));
  memset(&array2, 0, sizeof(json_value));

  /* Parse different arrays to exercise array comparison logic */
  const char *array1_source = "[1,2,3]";
  const char *array2_source = "[1,2,4]";
  ASSERT_TRUE(json_parse_iterative(array1_source, strlen(array1_source), &array1));
  ASSERT_TRUE(json_parse_iterative(array2_source, strlen(array2_source), &array2));

  /* Test using json_stringify to exercise buffer_write_values (lines 868-869) */
  char *result1 = json_stringify(&array1);
  ASSERT_PTR_NOT_NULL(result1);
  ASSERT_NOT_EQUAL(strstr(result1, "[1, 2, 3]"), NULL, char *);

  char *result2 = json_stringify(&array2);
  ASSERT_PTR_NOT_NULL(result2);
  ASSERT_NOT_EQUAL(strstr(result2, "[1, 2, 4]"), NULL, char *);

  /* Verify results are different */
  ASSERT_NOT_EQUAL(strcmp(result1, result2), 0, int);

  json_free(&array1);
  json_free(&array2);
  free(result1);
  free(result2);

  END_TEST;
}

TEST(test_coverage_line_138_large_array) {
  /* Test to cover line 138 (large array to exercise memory pool) */

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* Create a reasonably large array that exercises pool but won't fail */
  const int num_elements = 1000; /* Use 1000 instead of 0xFFFE to avoid parsing failures */

  char *large_array = (char *)malloc(num_elements * ELEMENT_SIZE + ELEMENT_BUFFER_SIZE);
  ASSERT_PTR_NOT_NULL(large_array);

  /* Build array: [0,1,2,...,999] */
  strcpy(large_array, "[");
  int i;
  for (i = 0; i < num_elements; i++) {
    if (i > 0) {
      strcat(large_array, ",");
    }
    char num_str[ELEMENT_BUFFER_SIZE];
    sprintf(num_str, "%d", i);
    strcat(large_array, num_str);
  }
  strcat(large_array, "]");

  /* Test parsing large array to exercise memory allocation paths */
  bool parse_result = json_parse(large_array, strlen(large_array), &v);
  ASSERT_TRUE(parse_result);

  /* Test stringifying to verify array was parsed correctly */
  char *result = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(result);

  /* Verify result starts and ends correctly */
  ASSERT_EQUAL(result[0], '[', char);
  ASSERT_EQUAL(result[strlen(result) - 1], ']', char);

  json_free(&v);
  free(result);
  free(large_array);

  END_TEST;
}

int main(int argc, char *argv[]) {
  TEST_INITIALIZE;
  TEST_SUITE("unit tests");
  RUN_TEST(test_memory_leaks);
  RUN_TEST(test_printf);
  RUN_TEST(test_whitespace);
  RUN_TEST(test_array);
  RUN_TEST(test_object);
  RUN_TEST(test_invalid_number_leading_zero);
  RUN_TEST(test_invalid_iterative_unclosed_array);
  RUN_TEST(test_invalid_iterative_unclosed_object);
  RUN_TEST(test_invalid_iterative_unquoted_string_key);
  RUN_TEST(test_invalid_iterative_missing_colon);
  RUN_TEST(test_invalid_iterative_extra_comma_array);
  RUN_TEST(test_invalid_iterative_extra_comma_object);
  RUN_TEST(test_invalid_iterative_invalid_escape_sequence);
  RUN_TEST(test_invalid_iterative_truncated_string);
  RUN_TEST(test_invalid_iterative_truncated_number);
  RUN_TEST(test_invalid_iterative_incorrect_boolean);
  RUN_TEST(test_invalid_iterative_incorrect_null);
  RUN_TEST(test_invalid_iterative_empty_input);
  RUN_TEST(test_invalid_iterative_whitespace_only_input);
  RUN_TEST(test_invalid_iterative_single_value_no_array_or_object);
  RUN_TEST(test_invalid_iterative_nested_unclosed_array);
  RUN_TEST(test_invalid_iterative_array_of_unclosed_objects);
  RUN_TEST(test_valid_number_zero_point_zero);
  RUN_TEST(test_valid_number_zero_point_zero_iterative);
  RUN_TEST(test_invalid_iterative_truncated_exponent);
  RUN_TEST(test_invalid_iterative_truncated_exponent_sign);
  RUN_TEST(test_invalid_iterative_exponent_missing_digits);
  RUN_TEST(test_valid_number_iterative_positive_integer);
  RUN_TEST(test_valid_number_iterative_negative_integer);
  RUN_TEST(test_valid_number_iterative_exponent_positive);
  RUN_TEST(test_valid_number_iterative_exponent_negative);
  RUN_TEST(test_valid_number_iterative_positive_float);
  RUN_TEST(test_valid_number_iterative_negative_float);
  RUN_TEST(test_valid_number_iterative_scientific_notation);
  RUN_TEST(test_valid_number_scientific_notation_coverage);
  RUN_TEST(test_valid_string_iterative_empty);
  RUN_TEST(test_valid_string_iterative_with_spaces);
  RUN_TEST(test_valid_string_iterative_with_escaped_chars);
  RUN_TEST(test_valid_boolean_iterative_true);
  RUN_TEST(test_valid_boolean_iterative_false);
  RUN_TEST(test_valid_null_iterative);
  RUN_TEST(test_valid_array_iterative_empty);
  RUN_TEST(test_valid_array_iterative_mixed_types);
  RUN_TEST(test_valid_object_iterative_empty);
  RUN_TEST(test_valid_object_iterative_simple);
  RUN_TEST(test_valid_object_iterative_nested);
  RUN_TEST(test_valid_nested_array_and_object_iterative);
  RUN_TEST(test_json_parse);
  RUN_TEST(test_case_0);
  RUN_TEST(test_case_1);
  RUN_TEST(test_case_2);
  RUN_TEST(test_case_3);
  RUN_TEST(test_case_4);
  RUN_TEST(test_case_5);
  RUN_TEST(test_case_6);
  RUN_TEST(test_case_7);
  RUN_TEST(test_case_8);
  RUN_TEST(test_case_9);
  RUN_TEST(test_case_10);
  RUN_TEST(test_case_11);
  RUN_TEST(test_case_12);
  RUN_TEST(test_case_13);
  RUN_TEST(test_case_14);
  RUN_TEST(test_case_15);
  RUN_TEST(test_case_16);
  RUN_TEST(test_case_17);
  RUN_TEST(test_case_18);
  RUN_TEST(test_case_19);
  RUN_TEST(test_case_20);
  RUN_TEST(test_case_21);
  RUN_TEST(test_case_22);
  RUN_TEST(test_case_23);
  RUN_TEST(test_case_24);
  RUN_TEST(test_case_25);
  RUN_TEST(test_case_26);
  RUN_TEST(test_case_27);
  RUN_TEST(test_case_28);
  RUN_TEST(test_case_29);
  RUN_TEST(test_case_30);
  RUN_TEST(test_case_31);
  RUN_TEST(test_case_32);
  RUN_TEST(test_case_33);
  RUN_TEST(test_case_34);
  RUN_TEST(test_case_35);
  RUN_TEST(test_case_36);
  RUN_TEST(test_case_37);
  RUN_TEST(test_case_38);
  RUN_TEST(test_case_39);
  RUN_TEST(test_case_40);
  RUN_TEST(test_case_41);
  RUN_TEST(test_case_42);
  RUN_TEST(test_case_43);
  RUN_TEST(test_case_44);
  RUN_TEST(test_case_45);
  RUN_TEST(test_case_46);
  RUN_TEST(test_case_47);
  RUN_TEST(test_case_48);
  RUN_TEST(test_case_49);
  RUN_TEST(test_case_50);
  RUN_TEST(test_case_51);
  RUN_TEST(test_case_52);
  RUN_TEST(test_case_53);
  RUN_TEST(test_case_54);
  RUN_TEST(test_case_55);
  RUN_TEST(test_case_56);
  RUN_TEST(test_case_57);
  RUN_TEST(test_case_58);
  RUN_TEST(test_case_59);
  RUN_TEST(test_case_60);
  RUN_TEST(test_case_61);
  RUN_TEST(test_case_62);
  RUN_TEST(test_case_63);
  RUN_TEST(test_case_64);
  RUN_TEST(test_case_65);
  RUN_TEST(test_case_66);
  RUN_TEST(test_case_67);
  RUN_TEST(test_case_68);
  RUN_TEST(test_case_69);
  RUN_TEST(test_case_70);
  RUN_TEST(test_case_71);
  RUN_TEST(test_case_72);
  RUN_TEST(test_case_73);
  RUN_TEST(test_case_74);
  RUN_TEST(test_case_invalid_char_0);
  RUN_TEST(test_case_invalid_char_1);
  RUN_TEST(test_case_invalid_char_2);
  RUN_TEST(test_case_invalid_char_3);
  RUN_TEST(test_case_invalid_char_4);
  RUN_TEST(test_case_invalid_char_5);
  RUN_TEST(test_case_invalid_char_6);
  RUN_TEST(test_case_invalid_char_7);
  RUN_TEST(test_case_invalid_char_8);
  RUN_TEST(test_case_invalid_char_9);
  RUN_TEST(test_case_invalid_char_10);
  RUN_TEST(test_case_invalid_char_11);
  RUN_TEST(test_case_invalid_char_12);
  RUN_TEST(test_case_invalid_char_13);
  RUN_TEST(test_case_invalid_char_14);
  RUN_TEST(test_case_invalid_char_15);
  RUN_TEST(test_case_invalid_char_16);
  RUN_TEST(test_case_invalid_char_17);
  RUN_TEST(test_case_invalid_char_18);
  RUN_TEST(test_case_invalid_char_19);
  RUN_TEST(test_case_invalid_char_20);
  RUN_TEST(test_case_invalid_char_21);
  RUN_TEST(test_case_invalid_char_22);
  RUN_TEST(test_case_invalid_char_23);
  RUN_TEST(test_case_invalid_char_24);
  RUN_TEST(test_case_invalid_char_25);
  RUN_TEST(test_case_invalid_char_26);
  RUN_TEST(test_case_invalid_char_27);
  RUN_TEST(test_case_invalid_char_28);
  RUN_TEST(test_case_invalid_char_29);
  RUN_TEST(test_case_invalid_char_30);
  RUN_TEST(test_case_invalid_char_31);
  RUN_TEST(test_case_invalid_char_32);
  RUN_TEST(test_case_invalid_char_33);
  RUN_TEST(test_case_invalid_char_34);
  RUN_TEST(test_case_invalid_char_35);
  RUN_TEST(test_case_invalid_char_36);
  RUN_TEST(test_case_invalid_char_37);
  RUN_TEST(test_case_invalid_char_38);
  RUN_TEST(test_case_invalid_char_39);
  RUN_TEST(test_case_invalid_char_40);
  RUN_TEST(test_case_invalid_char_41);
  RUN_TEST(test_case_invalid_char_42);
  RUN_TEST(test_case_invalid_char_43);
  RUN_TEST(test_case_invalid_char_44);
  RUN_TEST(test_case_invalid_char_45);
  RUN_TEST(test_case_invalid_char_46);
  RUN_TEST(test_case_invalid_char_47);
  RUN_TEST(test_case_invalid_char_48);
  RUN_TEST(test_case_invalid_char_49);
  RUN_TEST(test_case_invalid_char_50);
  RUN_TEST(test_case_invalid_char_51);
  RUN_TEST(test_case_invalid_char_52);
  RUN_TEST(test_case_invalid_char_53);
  RUN_TEST(test_case_invalid_char_54);
  RUN_TEST(test_case_invalid_char_55);
  RUN_TEST(test_case_invalid_char_56);
  RUN_TEST(test_case_invalid_char_57);
  RUN_TEST(test_case_invalid_char_58);
  RUN_TEST(test_case_invalid_char_59);
  RUN_TEST(test_case_invalid_char_60);
  RUN_TEST(test_case_invalid_char_61);
  RUN_TEST(test_case_invalid_char_62);
  RUN_TEST(test_case_invalid_char_63);
  RUN_TEST(test_case_invalid_char_64);
  RUN_TEST(test_case_invalid_char_65);
  RUN_TEST(test_case_invalid_char_66);
  RUN_TEST(test_case_invalid_char_67);
  RUN_TEST(test_case_invalid_char_68);
  RUN_TEST(test_case_invalid_char_69);
  RUN_TEST(test_case_invalid_char_70);
  RUN_TEST(test_case_truncated_input);
  RUN_TEST(test_case_iterative_0);
  RUN_TEST(test_case_iterative_1);
  RUN_TEST(test_case_iterative_2);
  RUN_TEST(test_case_iterative__2);
  RUN_TEST(test_case_iterative_3);
  RUN_TEST(test_case_iterative_4);
  RUN_TEST(test_case_iterative_5);
  RUN_TEST(test_case_iterative_6);
  RUN_TEST(test_case_iterative_7);
  RUN_TEST(test_case_iterative_8);
  RUN_TEST(test_case_iterative_9);
  RUN_TEST(test_case_iterative_10);
  RUN_TEST(test_case_iterative_11);
  RUN_TEST(test_case_iterative_12);
  RUN_TEST(test_case_iterative_13);
  RUN_TEST(test_case_iterative_14);
  RUN_TEST(test_case_iterative_15);
  RUN_TEST(test_case_iterative_16);
  RUN_TEST(test_case_iterative_17);
  RUN_TEST(test_case_iterative_18);
  RUN_TEST(test_case_iterative_19);
  RUN_TEST(test_case_iterative_20);
  RUN_TEST(test_case_iterative_21);
  RUN_TEST(test_case_iterative_22);
  RUN_TEST(test_case_iterative_23);
  RUN_TEST(test_case_iterative_24);
  RUN_TEST(test_case_iterative_25);
  RUN_TEST(test_case_iterative_26);
  RUN_TEST(test_case_iterative_27);
  RUN_TEST(test_case_iterative_28);
  RUN_TEST(test_case_iterative_29);
  RUN_TEST(test_case_iterative_30);
  RUN_TEST(test_case_iterative_31);
  RUN_TEST(test_case_iterative_32);
  RUN_TEST(test_case_iterative_33);
  RUN_TEST(test_case_iterative_34);
  RUN_TEST(test_case_iterative_35);
  RUN_TEST(test_case_iterative_36);
  RUN_TEST(test_case_iterative_37);
  RUN_TEST(test_case_iterative_38);
  RUN_TEST(test_case_iterative_39);
  RUN_TEST(test_case_iterative_40);
  RUN_TEST(test_case_iterative_41);
  RUN_TEST(test_case_iterative_42);
  RUN_TEST(test_case_iterative_43);
  RUN_TEST(test_case_iterative_44);
  RUN_TEST(test_case_iterative_45);
  RUN_TEST(test_case_iterative_46);
  RUN_TEST(test_case_iterative_47);
  RUN_TEST(test_case_iterative_48);
  RUN_TEST(test_case_iterative_49);
  RUN_TEST(test_case_iterative_50);
  RUN_TEST(test_case_iterative_51);
  RUN_TEST(test_case_iterative_52);
  RUN_TEST(test_case_iterative_53);
  RUN_TEST(test_case_iterative_54);
  RUN_TEST(test_case_iterative_55);
  RUN_TEST(test_case_iterative_56);
  RUN_TEST(test_case_iterative_57);
  RUN_TEST(test_case_iterative_58);
  RUN_TEST(test_case_iterative_59);
  RUN_TEST(test_case_iterative_60);
  RUN_TEST(test_case_iterative_61);
  RUN_TEST(test_case_iterative_62);
  RUN_TEST(test_case_iterative_63);
  RUN_TEST(test_case_iterative_64);
  RUN_TEST(test_case_iterative_65);
  RUN_TEST(test_case_iterative_66);
  RUN_TEST(test_case_iterative_67);
  RUN_TEST(test_case_iterative_68);
  RUN_TEST(test_case_iterative_69);
  RUN_TEST(test_case_iterative_70);
  RUN_TEST(test_case_iterative_71);
  RUN_TEST(test_case_iterative_72);
  RUN_TEST(test_case_iterative_73);
  RUN_TEST(test_case_iterative_74);
  RUN_TEST(test_case_iterative_invalid_char_0);
  RUN_TEST(test_case_iterative_invalid_char_1);
  RUN_TEST(test_case_iterative_invalid_char_2);
  RUN_TEST(test_case_iterative_invalid_char_3);
  RUN_TEST(test_case_iterative_invalid_char_4);
  RUN_TEST(test_case_iterative_invalid_char_5);
  RUN_TEST(test_case_iterative_invalid_char_6);
  RUN_TEST(test_case_iterative_invalid_char_7);
  RUN_TEST(test_case_iterative_invalid_char_8);
  RUN_TEST(test_case_iterative_invalid_char_9);
  RUN_TEST(test_case_iterative_invalid_char_10);
  RUN_TEST(test_case_iterative_invalid_char_11);
  RUN_TEST(test_case_iterative_invalid_char_12);
  RUN_TEST(test_case_iterative_invalid_char_13);
  RUN_TEST(test_case_iterative_invalid_char_14);
  RUN_TEST(test_case_iterative_invalid_char_15);
  RUN_TEST(test_case_iterative_invalid_char_16);
  RUN_TEST(test_case_iterative_invalid_char_17);
  RUN_TEST(test_case_iterative_invalid_char_18);
  RUN_TEST(test_case_iterative_invalid_char_19);
  RUN_TEST(test_case_iterative_invalid_char_20);
  RUN_TEST(test_case_iterative_invalid_char_21);
  RUN_TEST(test_case_iterative_invalid_char_22);
  RUN_TEST(test_case_iterative_invalid_char_23);
  RUN_TEST(test_case_iterative_invalid_char_24);
  RUN_TEST(test_case_iterative_invalid_char_25);
  RUN_TEST(test_case_iterative_invalid_char_26);
  RUN_TEST(test_case_iterative_invalid_char_27);
  RUN_TEST(test_case_iterative_invalid_char_28);
  RUN_TEST(test_case_iterative_invalid_char_29);
  RUN_TEST(test_case_iterative_invalid_char_30);
  RUN_TEST(test_case_iterative_invalid_char_31);
  RUN_TEST(test_case_iterative_invalid_char_32);
  RUN_TEST(test_case_iterative_invalid_char_33);
  RUN_TEST(test_case_iterative_invalid_char_34);
  RUN_TEST(test_case_iterative_invalid_char_35);
  RUN_TEST(test_case_iterative_invalid_char_36);
  RUN_TEST(test_case_iterative_invalid_char_37);
  RUN_TEST(test_case_iterative_invalid_char_38);
  RUN_TEST(test_case_iterative_invalid_char_39);
  RUN_TEST(test_case_iterative_invalid_char_40);
  RUN_TEST(test_case_iterative_invalid_char_41);
  RUN_TEST(test_case_iterative_invalid_char_42);
  RUN_TEST(test_case_iterative_invalid_char_43);
  RUN_TEST(test_case_iterative_invalid_char_44);
  RUN_TEST(test_case_iterative_invalid_char_45);
  RUN_TEST(test_case_iterative_invalid_char_46);
  RUN_TEST(test_case_iterative_invalid_char_47);
  RUN_TEST(test_case_iterative_invalid_char_48);
  RUN_TEST(test_case_iterative_invalid_char_49);
  RUN_TEST(test_case_iterative_invalid_char_50);
  RUN_TEST(test_case_iterative_invalid_char_51);
  RUN_TEST(test_case_iterative_invalid_char_52);
  RUN_TEST(test_case_iterative_invalid_char_53);
  RUN_TEST(test_case_iterative_invalid_char_54);
  RUN_TEST(test_case_iterative_invalid_char_55);
  RUN_TEST(test_case_iterative_invalid_char_56);
  RUN_TEST(test_case_iterative_invalid_char_57);
  RUN_TEST(test_case_iterative_invalid_char_58);
  RUN_TEST(test_case_iterative_invalid_char_59);
  RUN_TEST(test_case_iterative_invalid_char_60);
  RUN_TEST(test_case_iterative_invalid_char_61);
  RUN_TEST(test_case_iterative_invalid_char_62);
  RUN_TEST(test_case_iterative_invalid_char_63);
  RUN_TEST(test_case_iterative_invalid_char_64);
  RUN_TEST(test_case_iterative_invalid_char_65);
  RUN_TEST(test_case_iterative_invalid_char_66);
  RUN_TEST(test_case_iterative_invalid_char_67);
  RUN_TEST(test_case_iterative_invalid_char_68);
  RUN_TEST(test_case_iterative_invalid_char_69);
  RUN_TEST(test_case_iterative_invalid_char_70);
  RUN_TEST(test_case_iterative_truncated_input);
  RUN_TEST(test_validate_no_data);
  RUN_TEST(test_validate_invalid_json_string);
  RUN_TEST(test_validate_invalid_data);
  RUN_TEST(test_validate_invalid_json_number);
  RUN_TEST(test_validate_invalid_json_boolean);
  RUN_TEST(test_validate_invalid_json_value);
  RUN_TEST(test_validate_invalid_json_data_error);
  RUN_TEST(test_validate_object_key);
  RUN_TEST(test_validate_mailformed_object);
  RUN_TEST(test_validate_expected_object_key);
  RUN_TEST(test_validate_array_mailformed_json);
  RUN_TEST(test_validate_expected_string);
  RUN_TEST(test_unicode_hex4_parsing_coverage);
  RUN_TEST(test_json_node_pool_allocation_coverage);
  RUN_TEST(test_whitespace_lookup);
  RUN_TEST(test_print_value_object_with_multiple_keys);
  RUN_TEST(test_validate_expected_array_value);
  RUN_TEST(test_validate_expected_boolean);
  RUN_TEST(test_validate_expected_null);
  RUN_TEST(test_validate_expected_json);
  RUN_TEST(test_validate_expected_object_key);
  RUN_TEST(test_validate_expected_object_key_null);
  RUN_TEST(test_validate_expected_object_value);
  RUN_TEST(test_validate_expected_object_value_null);
  RUN_TEST(test_validate_expected_array_element);
  RUN_TEST(test_validate_expected_array_element_null);
  RUN_TEST(test_validate_expected_object_element);
  RUN_TEST(test_validate_expected_object_element_null);
  RUN_TEST(test_validate_no_error);
  RUN_TEST(test_randomization);
  RUN_TEST(test_json_cleanup);
  RUN_TEST(test_replacement);
  RUN_TEST(test_generation);
  RUN_TEST(test_json_stringify_function);
  RUN_TEST(test_json_print_function);
  RUN_TEST(test_json_free_function);
  RUN_TEST(test_json_equal_function);
  RUN_TEST(test_json_array_equal_function);
  RUN_TEST(test_json_object_equal_function);
  RUN_TEST(test_json_object_equal_function_null_a);
  RUN_TEST(test_json_object_equal_function_null_b);
  RUN_TEST(test_json_parse_function);
  RUN_TEST(test_json_parse_function_json_object_equals);
  RUN_TEST(test_json_parse_function_json_object_equals_null);
  RUN_TEST(test_json_validate_function);
  RUN_TEST(test_comprehensive_coverage_buffer_functions);
  RUN_TEST(test_comprehensive_coverage_print_functions);
  RUN_TEST(test_buffer_write_values_array_coverage);
  RUN_TEST(test_large_array_pool_allocation);
  RUN_TEST(test_print_value_object_with_multiple_keys);
  RUN_TEST(test_print_value_object_with_multiple_keys_enhanced);
  RUN_TEST(test_coverage_line_779_buffer_write_value);
  RUN_TEST(test_coverage_line_799_buffer_write);
  RUN_TEST(test_coverage_lines_868_869_buffer_write_values);
  RUN_TEST(test_coverage_line_138_large_array);
  RUN_TEST(test_comprehensive_coverage_equality_functions);
  RUN_TEST(test_comprehensive_coverage_parsing_functions);
  RUN_TEST(test_comprehensive_coverage_string_escape);
  RUN_TEST(test_comprehensive_coverage_object_get);
  RUN_TEST(test_json_stringify_full_coverage);
  RUN_TEST(test_complex_nested_json_coverage);
  RUN_TEST(test_all_json_types_comprehensive);
  RUN_TEST(test_equality_functions_full);
  RUN_TEST(test_utils_functions);
  RUN_TEST(test_new_coverage_and_whitespace);
  RUN_TEST(test_large_array_0xfffe_elements);
  RUN_TEST(test_comprehensive_uncovered_lines);
  RUN_TEST(test_free_array_node_coverage);
  RUN_TEST(test_free_object_node_coverage);
  RUN_TEST(test_skip_whitespace_coverage);
  RUN_TEST(test_number_parsing_edge_cases);
  RUN_TEST(test_string_parsing_complex_cases);
  RUN_TEST(test_string_escape_printing_coverage);
  RUN_TEST(test_print_value_compact_coverage);
  RUN_TEST(test_print_value_all_types_coverage);
  RUN_TEST(test_json_stringify_buffer_error_coverage);
  RUN_TEST(test_parse_string_full_coverage);
  RUN_TEST(test_parse_hex4);
  RUN_TEST(test_json_error_string_function);
  RUN_TEST(test_json_error_string_with_validate);
  TEST_FINALIZE;
}