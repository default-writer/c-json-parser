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
  ASSERT_TRUE(result9); /* Lone low surrogate is valid according to ECMA-404 */
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  /* Test lone high surrogate */
  const char *test10 = "[\"\\uD83D\"]";
  size_t test10_len = strlen(test10);
  memset(&test_val, 0, sizeof(json_value));
  bool result10 = json_parse(test10, test10_len, &test_val);
  ASSERT_TRUE(result10); /* Lone high surrogate is valid according to ECMA-404 */
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  /* Test valid surrogate pair */
  const char *test11 = "[\"\\uD83D\\uDE00\"]";
  size_t test11_len = strlen(test11);
  memset(&test_val, 0, sizeof(json_value));
  bool result11 = json_parse(test11, test11_len, &test_val);
  ASSERT_TRUE(result11); /* Valid surrogate pair */
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  /* Test invalid surrogate pair (low followed by high) */
  const char *test12 = "[\"\\uDC00\\uD83D\"]";
  size_t test12_len = strlen(test12);
  memset(&test_val, 0, sizeof(json_value));
  bool result12 = json_parse(test12, test12_len, &test_val);
  ASSERT_TRUE(result12); /* Both surrogates individually valid, order doesn't matter */
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  /* Test edge of BMP Unicode characters */
  const char *test13 = "[\"\\uFFFF\"]";
  size_t test13_len = strlen(test13);
  memset(&test_val, 0, sizeof(json_value));
  bool result13 = json_parse(test13, test13_len, &test_val);
  ASSERT_TRUE(result13); /* Edge of BMP character (should be valid) */
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  /* Test basic Latin characters */
  const char *test14 = "[\"\\u0041\\u0042\\u0043\"]";
  size_t test14_len = strlen(test14);
  memset(&test_val, 0, sizeof(json_value));
  bool result14 = json_parse(test14, test14_len, &test_val);
  ASSERT_TRUE(result14); /* Basic Latin 'ABC' */
  ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
  json_free(&test_val);

  END_TEST;
}