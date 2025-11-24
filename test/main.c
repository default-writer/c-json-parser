#include "../test/test.h"

#include <stdio.h>

#define printf printf

#ifndef USE_PERFORMANCE_TESTS
TEST_DEFINITION(test_json_parse);
#else
TEST_DEFINITION(test_c_json_parser);
#endif

int main(void) {

  TEST_INITIALIZE;

#ifndef USE_PERFORMANCE_TESTS

  TEST_SUITE("unit tests");

  test_json_parse();

#else

  TEST_SUITE("performance tests");

  test_c_json_parser();

#endif

  TEST_FINALIZE;

  return 0;
}