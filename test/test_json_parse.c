#include "../src/json.h"
#include "../test/test.h"

#include "../src/json.h"

TEST(test_json_parse_iterative_vs_recursive) {
  char *source = utils_get_test_json_data("data/test.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  /* Parse with recursive parser */
  ASSERT_TRUE(json_parse(source, &v_recursive));

  /* Parse with iterative parser */
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  /* Compare the two JSON objects directly */
  char *json_recursive_str = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive_str);

  char *json_iterative_str = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative_str);
  
  ASSERT_TRUE(utils_test_json_equal(json_recursive_str, json_iterative_str));

  /* Stringify both and compare the strings */
  ASSERT_TRUE(strcmp(json_recursive_str, json_iterative_str) == 0);

  utils_output(json_recursive_str);
  utils_output(json_iterative_str);

  /* Cleanup */
  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive_str);
  free(json_iterative_str);
  free(source);

  END_TEST;
}

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("test_json_parse");
  test_json_parse_iterative_vs_recursive();
  TEST_FINALIZE;
}