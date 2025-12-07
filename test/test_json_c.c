#include "../test/test.h"

#include "../libs/include/json-c/json_tokener.h"

#ifdef LONG_TEST
#define TEST_COUNT 1000000UL
#else
#define TEST_COUNT 100000UL
#endif

TEST(test_json_c) {
  char *json = utils_get_test_json_data("test/test.json");
  ASSERT_PTR_NOT_NULL(json);

  /* parse into internal json_value* */
  long long start_time = utils_get_time();
  unsigned long i;
  for (i = 0; i < TEST_COUNT; i++) {
    struct json_object *jobj = json_tokener_parse(json);
    json_object_put(jobj);
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

  test_json_c();

  TEST_FINALIZE;
}