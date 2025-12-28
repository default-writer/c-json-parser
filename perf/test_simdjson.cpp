#include "../libs/simdjson/simdjson.h"
#include "../test/test.h"

TEST(test_simdjson_parser) {
  simdjson::dom::parser parser;
  simdjson::dom::element doc;

  /* parse into internal json_value* */
  long long start_time = utils_get_time();
  unsigned long i;
  for (i = 0; i < TEST_COUNT; i++) {
    simdjson::dom::element doc;
    auto error = parser.load("data/test.json").get(doc);
    if (error) {
      break;
    }
  }

  long long end_time = utils_get_time();

  ASSERT_EQUAL(TEST_COUNT, i, unsigned long);

  utils_print_time_diff(start_time, end_time);

  END_TEST;
}

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("performance tests");
  test_simdjson_parser();
  TEST_FINALIZE;
}