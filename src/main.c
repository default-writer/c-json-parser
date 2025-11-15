#include "../test/test.h"

TEST_SETUP();

extern void test_simple_json_parsing(void);
extern void test_json_parsing(void);
extern void test_json_perf_test(void);

int main(void) {

  TEST_INITIALIZE();

  test_simple_json_parsing();
  test_json_parsing();

  printf("===============================================================================\n");
  printf("running performance tests\n");
  printf("===============================================================================\n");
  test_json_perf_test();

  TEST_FINALIZE();

  return 0;
}