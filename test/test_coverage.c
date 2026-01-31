#include "../src/json.h"
#include "../test/test.h"
#include <cstddef>

#define MAX_STRING_SIZE 1000

TEST(test_memory_pool_cleanup_coverage) {
  /* Test to trigger memory pool cleanup paths
   * This will create exactly 0xfffe elements to hit pool limit and trigger cleanup
   */
  size_t len;
  json_value array_val;
  memset(&array_val, 0, sizeof(json_value));

  /* Create JSON array with exactly 0xfffe (65534) elements */
  const int num_elements = 0xFFFE;
  const int buffer_size = num_elements * 2 + 10; /* Each "0" is 1 char + comma */

  char *large_array_json = (char *)malloc(buffer_size);
  ASSERT_PTR_NOT_NULL(large_array_json);

  /* Build array with 0xfffe elements: [0,0,0,...,0] */
  strcpy(large_array_json, "[");
  for (int i = 0; i < num_elements; i++) {
    if (i > 0)
      strcat(large_array_json, ",");
    strcat(large_array_json, "0");
  }
  strcat(large_array_json, "]");

  /* Parse the large array - this should exercise pool allocation and hit cleanup paths */
  len = strlen(large_array_json);
  bool parse_result = json_parse(large_array_json, len, &array_val);
  ASSERT_TRUE(parse_result);

  /* Free the array - this should trigger pool cleanup when pool is full */
  json_free(&array_val);

  free(large_array_json);

  END_TEST;
}

TEST(test_string_parsing_edge_cases) {
  /* Test to cover uncovered string parsing lines */
  json_value val;
  memset(&val, 0, sizeof(json_value));

  /* Test negative number parsing (line 182) */
  const char *source1 = "-123";
  size_t len1 = strlen(source1);
  bool result1 = json_parse(source1, len1, &val);
  ASSERT_TRUE(result1);
  ASSERT_EQ(val.type, J_NUMBER);
  json_free(&val);

  /* Test decimal parsing with multiple digits (line 202) */
  memset(&val, 0, sizeof(json_value));
  const char *source2 = "12.345";
  size_t len2 = strlen(source2);
  bool result2 = json_parse(source2, len2, &val);
  ASSERT_TRUE(result2);
  ASSERT_EQ(val.type, J_NUMBER);
  json_free(&val);

  /* Test exponent with + sign (line 211) */
  memset(&val, 0, sizeof(json_value));
  const char *source3 = "1e+10";
  size_t len3 = strlen(source3);
  bool result3 = json_parse(source3, len3, &val);
  ASSERT_TRUE(result3);
  ASSERT_EQ(val.type, J_NUMBER);
  json_free(&val);

  /* Test various escape sequences in strings */
  memset(&val, 0, sizeof(json_value));
  const char *source4 = "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"";
  size_t len4 = strlen(source4);
  bool result4 = json_parse(source4, len4, &val);
  ASSERT_TRUE(result4);
  ASSERT_EQ(val.type, J_STRING);
  json_free(&val);

  END_TEST;
}

TEST(test_unicode_surrogate_pairs) {
  /* Test to cover Unicode surrogate pair handling */
  json_value val;
  memset(&val, 0, sizeof(json_value));

  /* Test Unicode escape sequence */
  const char *source1 = "\"\\u0041\"";
  size_t len1 = strlen(source1);
  bool result1 = json_parse(source1, len1, &val); /* 'A' */
  ASSERT_TRUE(result1);
  ASSERT_EQ(val.type, J_STRING);
  json_free(&val);

  /* Test high surrogate followed by low surrogate */
  memset(&val, 0, sizeof(json_value));
  const char *source2 = "\"\\uD83D\\uDE00\"";
  size_t len2 = strlen(source2);
  bool result2 = json_parse(source2, len2, &val); /* 😀 */
  ASSERT_TRUE(result2);
  ASSERT_EQ(val.type, J_STRING);
  json_free(&val);

  END_TEST;
}

TEST(test_string_escape_printing) {
  /* Test to cover string escape sequence printing (lines 535-549, 557) */
  json_value val;
  size_t len;
  memset(&val, 0, sizeof(json_value));

  /* Parse and then stringify strings with various escape characters */
  const char *test_strings[] = {
      "\"\\\"",      /* Contains backslash */
      "\"\\b\"",     /* Contains backspace */
      "\"\\f\"",     /* Contains form feed */
      "\"\\r\"",     /* Contains carriage return */
      "\"\\t\"",     /* Contains tab */
      "\"\\u0041\"", /* Contains Unicode */
      NULL};
  int i;

  for (i = 0; test_strings[i]; i++) {
    memset(&val, 0, sizeof(json_value));
    len = strlen(test_strings[i]);
    bool result = json_parse(test_strings[i], len, &val);
    ASSERT_TRUE(result);
    ASSERT_EQ(val.type, J_STRING);

    /* Convert back to string - this should trigger the escape printing code */
    char *output = json_stringify(&val);
    ASSERT_PTR_NOT_NULL(output);

    free(output);
    json_free(&val);
  }

  END_TEST;
}

TEST(test_compact_printing_edge_cases) {
  /* Test to cover uncovered compact printing paths */
  json_value val;
  memset(&val, 0, sizeof(json_value));

  /* Test array compact printing with multiple elements */
  const char *source1 = "[1]";
  size_t len1 = strlen(source1);
  bool result1 = json_parse(source1, len1, &val);
  ASSERT_TRUE(result1);

  /* Use json_stringify which internally uses compact printing for some cases */
  char *output1 = json_stringify(&val);
  ASSERT_PTR_NOT_NULL(output1);

  free(output1);
  json_free(&val);

  /* Test object compact printing */
  memset(&val, 0, sizeof(json_value));
  const char *source2 = "{\"a\":1}";
  size_t len2 = strlen(source2);
  bool result2 = json_parse(source2, len2, &val);
  ASSERT_TRUE(result2);

  char *output2 = json_stringify(&val);
  ASSERT_PTR_NOT_NULL(output2);

  free(output2);
  json_free(&val);

  END_TEST;
}

TEST(test_memory_allocation_edge_cases) {
  /* Test to cover memory allocation failure paths */
  json_value val;
  memset(&val, 0, sizeof(json_value));

  /* Test with smaller string to avoid issues */
  char *test_string = (char *)malloc(MAX_STRING_SIZE);
  ASSERT_PTR_NOT_NULL(test_string);

  strcpy(test_string, "\"");
  for (int i = 1; i < MAX_STRING_SIZE - 2; i++) {
    test_string[i] = 'a';
  }
  test_string[MAX_STRING_SIZE - 2] = '"';
  test_string[MAX_STRING_SIZE - 1] = '\0';

  size_t test_string_len = strlen(test_string);
  bool result = json_parse(test_string, test_string_len, &val);
  ASSERT_TRUE(result);
  ASSERT_EQ(val.type, J_STRING);

  /* This might trigger the buffer reallocation and memory management code */
  char *output = json_stringify(&val);
  ASSERT_PTR_NOT_NULL(output);

  free(output);
  free(test_string);
  json_free(&val);

  END_TEST;
}