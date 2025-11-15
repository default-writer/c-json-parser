#include "../test/test.h"

TEST_SETUP();

extern void test_simple_json_parsing(void);
extern void test_json_parsing(void);
extern void test_json_perf_test(void);

#ifdef USE_JSON_C
extern void test_json_c_parser(void);
#endif

int main(void) {

  TEST_INITIALIZE();

  test_simple_json_parsing();
  test_json_parsing();

  printf("===============================================================================\n");
  printf("running performance tests\n");
  printf("===============================================================================\n");

  test_json_perf_test();
#ifdef USE_JSON_C
  test_json_c_parser();
#endif

  TEST_FINALIZE();

  return 0;
}