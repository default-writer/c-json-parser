#include "../src/json.h"
#include "../test/test.h"

TEST(test_parse_string_full_coverage) {
  json_value test_val;

  const char *test1 = "[\"simple\"]";
  size_t test1_len = strlen(test1);
  memset(&test_val, 0, sizeof(json_value));
  bool result1 = json_parse(test1, test1_len, &test_val);
  ASSERT_TRUE(result1);
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  const char *test2 = "[\"\\b\\f\\r\\t\"]";
  size_t test2_len = strlen(test2);
  memset(&test_val, 0, sizeof(json_value));
  bool result2 = json_parse(test2, test2_len, &test_val);
  ASSERT_TRUE(result2);
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  const char *test3 = "[\"\\\"\"]";
  size_t test3_len = strlen(test3);
  memset(&test_val, 0, sizeof(json_value));
  bool result3 = json_parse(test3, test3_len, &test_val);
  ASSERT_TRUE(result3);
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  const char *test4 = "[\"\\\\\"]";
  size_t test4_len = strlen(test4);
  memset(&test_val, 0, sizeof(json_value));
  bool result4 = json_parse(test4, test4_len, &test_val);
  ASSERT_TRUE(result4);
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  const char *test5 = "[\"\\/bfnrt\"]";
  size_t test5_len = strlen(test5);
  memset(&test_val, 0, sizeof(json_value));
  bool result5 = json_parse(test5, test5_len, &test_val);
  ASSERT_TRUE(result5);
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  const char *test6 = "\"\\\"";
  size_t test6_len = strlen(test6);
  memset(&test_val, 0, sizeof(json_value));
  bool result6 = json_parse(test6, test6_len, &test_val);
  ASSERT_FALSE(result6);

  const char *test7 = "\"\\ug\"";
  size_t test7_len = strlen(test7);
  memset(&test_val, 0, sizeof(json_value));
  bool result7 = json_parse(test7, test7_len, &test_val);
  ASSERT_FALSE(result7);

  const char *test8 = "[\"\\uD83D\\uDE00\"]";
  size_t test8_len = strlen(test8);
  memset(&test_val, 0, sizeof(json_value));
  bool result8 = json_parse(test8, test8_len, &test_val);
  ASSERT_TRUE(result8);
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  const char *test9 = "[\"\\uDC00\"]";
  size_t test9_len = strlen(test9);
  memset(&test_val, 0, sizeof(json_value));
  bool result9 = json_parse(test9, test9_len, &test_val);
  ASSERT_FALSE(result9);

  END_TEST;
}