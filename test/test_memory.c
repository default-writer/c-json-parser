#include "../test/test.h"

TEST(test_memory_leaks, char *json) {
  const char *source = "[{\"key\": \"value\"}]";

  json_value v; // = new_json_value();
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_free(&v);

  /* compare structurally (order-insensitive) */
  ASSERT(utils_test_json_equal(json, out));

  /* cleanup */
  free(out);

  json_free(&v);

  fputs(json, stdout);
  free(json);

  END_TEST;
}

int main(void) {

  TEST_INITIALIZE;

  TEST_SUITE("unit tests");
  test_memory_leaks();

  TEST_FINALIZE;

  return 0;
}