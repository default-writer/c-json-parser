#include "../src/json.h"
#include "../test/test.h"

TEST(test_json_parse) {
  char *source = utils_get_test_json_data("data/test.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v;
  memset(&v, 0, sizeof(json_value));

  unsigned long i;
  for (i = 0; i < TEST_COUNT; i++) {
    if (!json_parse_iterative(source, &v)) {
      break;
    }
    json_free(&v);
  }

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  utils_output(json);

  /* cleanup */
  json_free(&v);
  free(json);
  free(source);

  END_TEST;
}

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("unit tests");
  test_json_parse();
  TEST_FINALIZE;
}