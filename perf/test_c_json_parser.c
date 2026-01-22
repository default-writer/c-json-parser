#include "../src/json.h"
#include "../test/test.h"
#include <sys/types.h>

TEST(test_c_json_parser) {
  char *json = utils_get_test_json_data("data/test.json");
  ASSERT_PTR_NOT_NULL(json);

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  long long start_time = utils_get_time();
  unsigned long i;
  for (i = 0; i < TEST_COUNT; i++) {
    if (!json_parse_iterative(json, &v)) {
      break;
    }
    json_reset();
  }

  json_cleanup();
  long long end_time = utils_get_time();

  ASSERT_EQUAL(TEST_COUNT, i, uint32_t);

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
}