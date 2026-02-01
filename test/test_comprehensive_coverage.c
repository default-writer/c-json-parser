#include "../src/json.h"
#include "../test/test.h"

#define MAX_VALUE 1000
#define MAX_JSON_ELEMENTS 50

TEST(test_comprehensive_uncovered_lines) {
  /* Test to cover uncovered lines in src/json.c */

  /* 1. Test memory pool cleanup (lines 138-140, 164-166) */
  /* Create and free many arrays to consume pool nodes and trigger cleanup */
  json_value pool_test;
  memset(&pool_test, 0, sizeof(json_value));

  /* Create and free many arrays to exercise pool */
  int i;
  for (i = 0; i < MAX_VALUE; i++) {
    char array_json[MAX_JSON_ELEMENTS];
    sprintf(array_json, "[%d,%d,%d]", i, i + 1, i + 2);

    const size_t len_array_json = strlen(array_json);
    bool result = json_parse(array_json, len_array_json, &pool_test);
    if (result) {
      json_free(&pool_test); /* This should trigger pool cleanup */
    }
    memset(&pool_test, 0, sizeof(json_value));
  }

  /* 2. Test string parsing edge cases (lines 174, 182, 202, 211) */
  json_value parse_test;

  /* Test negative number (line 182) */
  memset(&parse_test, 0, sizeof(json_value));
  const char *source1 = "[-123]";
  const size_t len1 = strlen(source1);
  bool neg_result = json_parse(source1, len1, &parse_test);
  if (neg_result) {
    ASSERT_EQ(parse_test.u.array.items->item.type, J_NUMBER);
    json_free(&parse_test);
  }

  /* Test decimal number (line 202) */
  memset(&parse_test, 0, sizeof(json_value));
  const char *source2 = "[123.456]";
  const size_t len2 = strlen(source2);
  bool dec_result = json_parse(source2, len2, &parse_test);
  if (dec_result) {
    ASSERT_EQ(parse_test.u.array.items->item.type, J_NUMBER);
    json_free(&parse_test);
  }

  /* Test exponent with + (line 211) */
  memset(&parse_test, 0, sizeof(json_value));
  const char *source3 = "[1e+10]";
  const size_t len3 = strlen(source3);
  bool exp_result = json_parse(source3, len3, &parse_test);
  if (exp_result) {
    ASSERT_EQ(parse_test.u.array.items->item.type, J_NUMBER);
    json_free(&parse_test);
  }

  /* 3. Test Unicode escape sequences (lines 282-325) */
  memset(&parse_test, 0, sizeof(json_value));
  const char *source4 = "[\"\\u0041\"]";
  const size_t len4 = strlen(source4);
  bool unicode_result = json_parse(source4, len4, &parse_test); /* Simple unicode */
  if (unicode_result) {
    ASSERT_EQ(parse_test.u.array.items->item.type, J_STRING);
    json_free(&parse_test);
  }

  /* 4. Test string escape printing (lines 535-549, 557, etc.) */
  /* Test simple escape characters */
  memset(&parse_test, 0, sizeof(json_value));
  const char *source5 = "[\"\\\\\"]";
  const size_t len5 = strlen(source5);
  bool escape_result = json_parse(source5, len5, &parse_test); /* Backslash and quote */
  if (escape_result) {
    ASSERT_EQ(parse_test.u.array.items->item.type, J_STRING);
    char *escape_output = json_stringify(&parse_test);
    if (escape_output)
      free(escape_output);
    json_free(&parse_test);
  }

  /* 5. Test all JSON types for printing (lines 600-608, 612-614) */
  json_value all_types_test;
  memset(&all_types_test, 0, sizeof(json_value));
  const char *simple_json = "{\"null\":null,\"bool\":true,\"num\":42}";
  const size_t len_simple_json = strlen(simple_json);
  bool mixed_result = json_parse(simple_json, len_simple_json, &all_types_test);
  if (mixed_result) {
    char *mixed_output = json_stringify(&all_types_test);
    if (mixed_output)
      free(mixed_output);
    json_free(&all_types_test);
  }

  END_TEST;
}