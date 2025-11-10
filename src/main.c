#include "../test/test.h"

TEST_SETUP();
TEST_DEFINITION(test_json_parsing);
TEST_DEFINITION(test_simple_json_parsing);

int main(void) {

  TEST_INITIALIZE();

  test_json_parsing();
  test_simple_json_parsing();

  TEST_FINALIZE();

  return 0;
}