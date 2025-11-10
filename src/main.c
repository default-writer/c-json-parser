#include "../test/test.h"

TEST_SETUP();

extern void test_simple_json_parsing(void);
extern void test_json_parsing(void);

int main(void) {

  TEST_INITIALIZE();

  test_simple_json_parsing();
  test_json_parsing();

  TEST_FINALIZE();

  return 0;
}