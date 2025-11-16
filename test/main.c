#include "../test/test.h"

TEST_SETUP()
TEST_DEFINITION(test_simple_json_parsing)
TEST_DEFINITION(test_json_parsing)
TEST_DEFINITION(test_json_perf_test)

#ifdef USE_JSON_C

TEST_DEFINITION(test_json_c_parser);

#endif

int main(void) {

  TEST_INITIALIZE();

  TEST_SUITE("unit tests");
  test_simple_json_parsing();
  test_json_parsing();

  TEST_SUITE("performance tests");
  test_json_perf_test();

#ifdef USE_JSON_C

  test_json_c_parser();

#endif

  TEST_FINALIZE();

  return 0;
}