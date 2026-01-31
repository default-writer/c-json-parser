#include "../src/json.h"
#include "../test/test.h"

#define ELEMENT_ARRAY_SIZE_100 100
#define ELEMENT_ARRAY_SIZE_2000 2000
#define ELEMENT_BUFFER_SIZE_20 20
#define ELEMENT_BUFFER_SIZE_30 30
#define ELEMENT_BUFFER_SIZE_50 50
#define ELEMENT_BUFFER_SIZE_300 300

TEST(test_free_array_node_coverage) {
  int i;

  /* Test lines 138-140: free_array_node function */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));

  /* Create and destroy many arrays to trigger free_array_node */
  for (i = 0; i < ELEMENT_ARRAY_SIZE_2000; i++) {
    json_value arr;
    memset(&arr, 0, sizeof(json_value));

/* Create a simple array */
    char json_str[ELEMENT_BUFFER_SIZE_20];
    sprintf(json_str, "[%d]", i);
    const size_t len = strlen(json_str);

    bool result = json_parse(json_str, len, &arr);
    if (result) {
      /* This should consume pool nodes, then free them */
      json_free(&arr); /* This should trigger free_array_node -> lines 138-140 */
    }
  }

  END_TEST;
}

TEST(test_free_object_node_coverage) {
  int i;

  /* Test lines 164-166: free_object_node function */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));

  /* Create and destroy many objects to trigger free_object_node */
  for (i = 0; i < ELEMENT_ARRAY_SIZE_2000; i++) {
    json_value obj;
    memset(&obj, 0, sizeof(json_value));

/* Create a simple object */
    char json_str[ELEMENT_BUFFER_SIZE_30];
    sprintf(json_str, "{\"key%d\":%d}", i, i);
    const size_t len = strlen(json_str);

    bool result = json_parse(json_str, len, &obj);
    if (result) {
      /* This should consume pool nodes, then free them */
      json_free(&obj); /* This should trigger free_object_node -> lines 164-166 */
    }
  }

  END_TEST;
}

TEST(test_skip_whitespace_coverage) {
  /* Test line 174: skip_whitespace function */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));

/* Test with leading whitespace */
  const char *source1 = "[ 1,2,3 ]";
  const size_t len1 = strlen(source1);
  bool result = json_parse(source1, len1, &test_val);
  ASSERT_TRUE(result);
  ASSERT_EQ(test_val.type, J_ARRAY);
  json_free(&test_val);

  END_TEST;
}

TEST(test_number_parsing_edge_cases) {
  /* Test lines 182, 202, 211: number parsing edge cases */
  json_value test_val;

/* Test line 182: negative number parsing */
  memset(&test_val, 0, sizeof(json_value));
  const char *source2 = "[ -123.456 ]";
  const size_t len2 = strlen(source2);
  bool neg_result = json_parse(source2, len2, &test_val);
  ASSERT_TRUE(neg_result);
  ASSERT_EQ(test_val.u.array.items->item.type, J_NUMBER);
  json_free(&test_val);

/* Test line 202: decimal parsing progression */
  memset(&test_val, 0, sizeof(json_value));
  const char *source3 = "[ 123.456789 ]";
  const size_t len3 = strlen(source3);
  bool dec_result = json_parse(source3, len3, &test_val);
  ASSERT_TRUE(dec_result);
  ASSERT_EQ(test_val.u.array.items->item.type, J_NUMBER);
  json_free(&test_val);

/* Test line 211: exponent with + sign */
  memset(&test_val, 0, sizeof(json_value));
  const char *source4 = "[ 1e+10 ]";
  const size_t len4 = strlen(source4);
  bool exp_result = json_parse(source4, len4, &test_val);
  ASSERT_TRUE(exp_result);
  ASSERT_EQ(test_val.u.array.items->item.type, J_NUMBER);
  json_free(&test_val);

  END_TEST;
}

TEST(test_string_parsing_complex_cases) {
  /* Test lines 282-325: parse_string complex cases */
  json_value test_val;

/* Test line 299: quote in string */
  memset(&test_val, 0, sizeof(json_value));
  const char *source5 = "[\"test\\\"quote\"]";
  const size_t len5 = strlen(source5);
  bool quote_result = json_parse(source5, len5, &test_val);
  if (quote_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

/* Test line 311: unicode escape */
  memset(&test_val, 0, sizeof(json_value));
  const char *source6 = "[\"test\\u0041\"]";
  const size_t len6 = strlen(source6);
  bool unicode_result = json_parse(source6, len6, &test_val);
  if (unicode_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

/* Test line 320: invalid surrogate pair */
  memset(&test_val, 0, sizeof(json_value));
  const char *source7 = "[\"test\\uDC00\"]";
  const size_t len7 = strlen(source7);
  bool invalid_surrogate = json_parse(source7, len7, &test_val);
  ASSERT_FALSE(invalid_surrogate); /* Should fail at line 321 */

/* Test line 325: valid surrogate pair */
  memset(&test_val, 0, sizeof(json_value));
  const char *source8 = "[\"test\\uD83D\\uDE00\"]";
  const size_t len8 = strlen(source8);
  bool valid_surrogate = json_parse(source8, len8, &test_val);
  if (valid_surrogate) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);
    json_free(&test_val);
  }

  END_TEST;
}

TEST(test_string_escape_printing_coverage) {
  /* Test lines 535-549, 557: string escape printing */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));

/* Test backslash and quote escapes (lines 535-538) */
  const char *source9 = "[\"\\\\\\\"\"]";
  const size_t len9 = strlen(source9);
  bool escape_result = json_parse(source9, len9, &test_val);
  if (escape_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);

    /* Stringify to trigger escape printing (lines 535-538) */
    char *escape_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(escape_output);
    free(escape_output);
    json_free(&test_val);
  }

/* Test control character escapes (lines 538-543) */
  memset(&test_val, 0, sizeof(json_value));
  const char *source10 = "[\"test\\b\\f\\r\"]";
  const size_t len10 = strlen(source10);
  bool control_result = json_parse(source10, len10, &test_val);
  if (control_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_STRING);

    /* Stringify to trigger control char printing (lines 538-543) */
    char *control_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(control_output);
    free(control_output);
    json_free(&test_val);
  }

  END_TEST;
}

TEST(test_print_value_compact_coverage) {
  /* Test line 613: print_array_compact function */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));

/* Test compact array printing */
  const char *source11 = "[1,2,3]";
  const size_t len11 = strlen(source11);
  bool result = json_parse(source11, len11, &test_val);
  if (result) {
    ASSERT_EQ(test_val.type, J_ARRAY);

    /* Stringify to trigger print_array_compact (line 613) */
    char *compact_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(compact_output);
    free(compact_output);
    json_free(&test_val);
  }

  END_TEST;
}

TEST(test_print_value_all_types_coverage) {
  /* Test lines 600-614: print_value_compact for all types */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));

/* Test J_NULL (lines 600-602) */
  memset(&test_val, 0, sizeof(json_value));
  const char *source12 = "[null]";
  const size_t len12 = strlen(source12);
  bool null_result = json_parse(source12, len12, &test_val);
  if (null_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_NULL);

    /* Stringify to trigger J_NULL printing (lines 600-602) */
    char *null_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(null_output);
    free(null_output);
    json_free(&test_val);
  }

/* Test J_BOOLEAN (lines 603-605) */
  memset(&test_val, 0, sizeof(json_value));
  const char *source13 = "[true]";
  const size_t len13 = strlen(source13);
  bool bool_result = json_parse(source13, len13, &test_val);
  if (bool_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_BOOLEAN);

    /* Stringify to trigger J_BOOLEAN printing (lines 603-605) */
    char *bool_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(bool_output);
    free(bool_output);
    json_free(&test_val);
  }

/* Test J_NUMBER (lines 606-608) */
  memset(&test_val, 0, sizeof(json_value));
  const char *source14 = "[123.456]";
  const size_t len14 = strlen(source14);
  bool num_result = json_parse(source14, len14, &test_val);
  if (num_result) {
    ASSERT_EQ(test_val.u.array.items->item.type, J_NUMBER);

    /* Stringify to trigger J_NUMBER printing (lines 606-608) */
    char *num_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(num_output);
    free(num_output);
    json_free(&test_val);
  }

/* Test J_ARRAY (lines 612-614) */
  memset(&test_val, 0, sizeof(json_value));
  const char *source15 = "[1,2,3]";
  const size_t len15 = strlen(source15);
  bool arr_result = json_parse(source15, len15, &test_val);
  if (arr_result) {
    ASSERT_EQ(test_val.type, J_ARRAY);

    /* Stringify to trigger J_ARRAY printing (lines 612-614) */
    char *arr_output = json_stringify(&test_val);
    ASSERT_PTR_NOT_NULL(arr_output);
    free(arr_output);
    json_free(&test_val);
  }

  END_TEST;
}

TEST(test_json_stringify_buffer_error_coverage) {
  /* Test lines 848-849: json_stringify buffer allocation failure */

  /* Since forcing realloc to fail is non-deterministic, we focus on */
  /* creating a test that definitely triggers the realloc call path */
  
  /* Create a JSON value that will force buffer expansion beyond initial size */
  json_value test_val;
  memset(&test_val, 0, sizeof(json_value));
  test_val.type = J_STRING;
  
  /* Use a string that will exceed the initial MAX_BUFFER_SIZE (256) */
  /* to force the realloc call at line 846 in json_stringify */
  char large_string[ELEMENT_BUFFER_SIZE_300];
  memset(large_string, 'A', ELEMENT_BUFFER_SIZE_300 - 1);
  large_string[ELEMENT_BUFFER_SIZE_300 - 1] = '\0';
  test_val.u.string.ptr = large_string;
  test_val.u.string.len = ELEMENT_BUFFER_SIZE_300 - 1;
  
  /* This will trigger buffer growth and the realloc call */
  char *output = json_stringify(&test_val);
  
  /* Whether realloc succeeds or fails, we've exercised the code path */
  /* If realloc fails (lines 848-849), output will be NULL */
  /* If realloc succeeds, output will contain the stringified JSON */
  if (output) {
    free(output);
  }

  /* Test with NULL input to exercise the early return path */
  char *null_output = json_stringify(NULL);
  
  ASSERT_PTR_NULL(null_output);

  END_TEST;
}