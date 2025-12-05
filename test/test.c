#include "../test/test.h"

#ifdef LONG_TEST
#define TEST_COUNT 1000000UL
#else
#define TEST_COUNT 100000UL
#endif

TEST(test_json_parse) {
  char *source = utils_get_test_json_data("test/test.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  utils_output(json);

  /* cleanup */
  json_free(&v);
  free(json);
  free(source);

  END_TEST;
}

TEST(test_c_json_parser) {
  char *json = utils_get_test_json_data("test/test.json");
  ASSERT_PTR_NOT_NULL(json);

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  long long start_time = utils_get_time();
  unsigned long i;
  for (i = 0; i < TEST_COUNT; i++) {
    memset(&v, 0, sizeof(json_value));
    json_parse(json, &v);
    json_free(&v);
  }
  long long end_time = utils_get_time();
  utils_print_time_diff(start_time, end_time);

  /* cleanup */
  free(json);

  END_TEST;
}