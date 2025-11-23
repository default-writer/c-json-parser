#include "../test/test.h"

TEST(test_memory_leaks) {
  const char *json = "[{\"key\": \"value\"}]";

  json_value v; // = new_json_value();
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(json, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);
  json_print(&v, stdout);
  fputs("\n", stdout);
  json_free(&v);

  /* compare structurally (order-insensitive) */
  ASSERT(utils_test_json_equal(json, out));

  /* cleanup */
  free(out);

  json_free(&v);
  END_TEST;
}

int main(void) {

  UTILS_INITIALIZE();

  TEST_SUITE("unit tests");
  test_memory_leaks();

  TEST_FINALIZE();

  return 0;
}