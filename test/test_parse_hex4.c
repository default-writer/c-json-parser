#include "../src/json.h"
#include "../test/test.h"

TEST(test_parse_hex4) {
  json_value test_val;

  memset(&test_val, 0, sizeof(json_value));
  bool result1 = json_parse("[\"\\u0000\"]", strlen("[\"\\u0000\"]"), &test_val);
  if (result1) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result2 = json_parse("[\"\\u1234\"]", strlen("[\"\\u1234\"]"), &test_val);
  if (result2) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result3 = json_parse("[\"\\uABCD\"]", strlen("[\"\\uABCD\"]"), &test_val);
  if (result3) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result4 = json_parse("[\"\\uFFFF\"]", strlen("[\"\\uFFFF\"]"), &test_val);
  if (result4) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  memset(&test_val, 0, sizeof(json_value));
  bool result5 = json_parse("[\"\\u123\"]", strlen("[\"\\u123\"]"), &test_val);
  ASSERT_FALSE(result5);

  memset(&test_val, 0, sizeof(json_value));
  bool result6 = json_parse("[\"\\u12G4\"]", strlen("[\"\\u12G4\"]"), &test_val);
  ASSERT_FALSE(result6);

  memset(&test_val, 0, sizeof(json_value));
  bool result7 = json_parse("[\"\\uXYZ\"]", strlen("[\"\\uXYZ\"]"), &test_val);
  ASSERT_FALSE(result7);

  memset(&test_val, 0, sizeof(json_value));
  bool result8 = json_parse("[\"\\u12345\"]", strlen("[\"\\u12345\"]"), &test_val);
  ASSERT_FALSE(result8);

  memset(&test_val, 0, sizeof(json_value));
  bool result9 = json_parse("[\"\\u@123\"]", strlen("[\"\\u@123\"]"), &test_val);
  ASSERT_FALSE(result9);

  END_TEST;
}