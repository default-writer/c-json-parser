#include "../test/test.h"
#include "../src/json.h"

TEST(test_memory_leaks, char *json) {
  const char *source = "[{\"key\": \"value\"}]";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_free(&v);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  utils_output(json);

  /* cleanup */
  json_free(&v);
  free(json);
  free(out);

  END_TEST;
}

TEST(test_printf, char *json) {
  const char *source = "[{\"key\": \"value\"}]";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_print(&v, stdout);
  fputc('\n', stdout);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  /* cleanup */
  json_free(&v);
  free(json);
  free(out);

  END_TEST;
}

TEST(test_whitespace) {
  const char *source = " { \t \"key\" \n : \r \"value\" } ";
  const char *expected = "{\n    \"key\": \"value\"\n}";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, expected));

  utils_output(json);

  /* cleanup */
  json_free(&v);
  free(json);

  END_TEST;
}

TEST(test_array) {
  char *source = utils_get_test_json_data("data/array.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  char *json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  char *json_iterative = json_stringify(&v_iterative);
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
  char *source = utils_get_test_json_data("data/object.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  char *json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  char *json_iterative = json_stringify(&v_iterative);
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

TEST(test_json_parse) {
  char *source = utils_get_test_json_data("data/test.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  char *json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  char *json_iterative = json_stringify(&v_iterative);
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

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("unit tests");
  test_memory_leaks();
  test_printf();
  test_whitespace();
  test_array();
  test_object();
  test_json_parse();
  TEST_FINALIZE;
}
