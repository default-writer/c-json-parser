#include "../test/test.h"

#include <stdio.h>
#define printf printf

// TEST_DEFINITION(test_simple_json_parsing);
TEST_DEFINITION(test_json_parsing);
TEST_DEFINITION(test_c_json_parser);

#ifdef USE_JSON_C

TEST_DEFINITION(test_json_c_parser);

#endif

int main(void) {

  TEST_INITIALIZE;

#ifndef USE_PERFORMANCE_TESTS

  TEST_SUITE("unit tests");
  // test_simple_json_parsing();
  test_json_parsing();

#else

  TEST_SUITE("performance tests");

#ifdef USE_JSON_C

  test_json_c_parser();

#else

  test_c_json_parser();

#endif

#endif

  TEST_FINALIZE;

  return 0;
}