#include "../src/json.h"
#include "../test/test.h"

TEST(test_object_equal_order) {
  const char *source1 = "{\"a\": 1, \"b\": 2}";
  const char *source2 = "{\"b\": 2, \"a\": 1}";

  json_value v1, v2;
  memset(&v1, 0, sizeof(json_value));
  memset(&v2, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source1, &v1));
  ASSERT_TRUE(json_parse(source2, &v2));

  ASSERT_TRUE(json_equal(&v1, &v2));

  json_free(&v1);
  json_free(&v2);
  END_TEST;
}

TEST(test_object_equal_different_keys) {
  const char *source1 = "{\"a\": 1, \"b\": 2}";
  const char *source2 = "{\"c\": 1, \"d\": 2}";

  json_value v1, v2;
  memset(&v1, 0, sizeof(json_value));
  memset(&v2, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source1, &v1));
  ASSERT_TRUE(json_parse(source2, &v2));

  ASSERT_FALSE(json_equal(&v1, &v2));

  json_free(&v1);
  json_free(&v2);
  END_TEST;
}

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("equal tests");
  test_object_equal_order();
  test_object_equal_different_keys();
  TEST_FINALIZE;
}
