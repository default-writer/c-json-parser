#include "../src/json.h"
#include "../test/test.h"

TEST(test_parse_string_full_coverage) {
  json_value test_val;

  memset(&test_val, 0, sizeof(json_value));
  bool result1 = json_parse("[\"simple\"]", strlen("[\"simple\"]"), &test_val);
  if (result1) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result2 = json_parse("[\"\\b\\f\\r\\t\"]", strlen("[\"\\b\\f\\r\\t\"]"), &test_val);
  if (result2) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result3 = json_parse("[\"\\\"\"]", strlen("[\"\\\"\"]"), &test_val);
  if (result3) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result4 = json_parse("[\"\\\\\"]", strlen("[\"\\\\\"]"), &test_val);
  if (result4) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result5 = json_parse("[\"\\/bfnrt\"]", strlen("[\"\\/bfnrt\"]"), &test_val);
  if (result5) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result6 = json_parse("\"\\\"", strlen("\"\\\""), &test_val);
  ASSERT_FALSE(result6);

  memset(&test_val, 0, sizeof(json_value));
  bool result7 = json_parse("\"\\ug\"", strlen("\"\\ug\""), &test_val);
  ASSERT_FALSE(result7);

  memset(&test_val, 0, sizeof(json_value));
  bool result8 = json_parse("[\"\\uD83D\\uDE00\"]", strlen("[\"\\uD83D\\uDE00\"]"), &test_val);
  if (result8) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result9 = json_parse("[\"\\uDC00\"]", strlen("[\"\\uDC00\"]"), &test_val);
  ASSERT_FALSE(result9);

  END_TEST;
}