#include "../test/test.h"

#include <stdio.h>

#define printf printf

#ifdef LONG_TEST
#define TEST_COUNT 1000000UL
#else
#define TEST_COUNT 100000UL
#endif

TEST(test_c_json_parser) {
  char *json = utils_get_test_json_data("test/test.json");
  ASSERT_PTR_NOT_NULL(json);

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  long long start_time = utils_get_time();
  for (size_t i = 0; i < TEST_COUNT; i++) {
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

int main(void) {
  TEST_INITIALIZE;

  TEST_SUITE("performance tests");

  test_c_json_parser();

  TEST_FINALIZE;

  return 0;
}