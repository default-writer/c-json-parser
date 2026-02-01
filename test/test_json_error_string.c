#include "../src/json.h"
#include "../test/test.h"

TEST(test_json_error_string_function) {
  ASSERT(strcmp(json_error_string(E_NO_ERROR), "No error occurred") == 0);
  ASSERT(strcmp(json_error_string(E_NO_DATA), "No data provided or empty input") == 0);
  ASSERT(strcmp(json_error_string(E_INVALID_JSON), "Invalid JSON structure") == 0);
  ASSERT(strcmp(json_error_string(E_INVALID_JSON_DATA), "Invalid data within JSON structure") == 0);
  ASSERT(strcmp(json_error_string(E_STACK_OVERFLOW_OBJECT), "Stack overflow while parsing object") == 0);
  ASSERT(strcmp(json_error_string(E_STACK_OVERFLOW_ARRAY), "Stack overflow while parsing array") == 0);
  ASSERT(strcmp(json_error_string(E_OBJECT_KEY), "Invalid object key format") == 0);
  ASSERT(strcmp(json_error_string(E_OBJECT_VALUE), "Invalid object value") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT), "Expected object but found different type") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_ARRAY), "Expected array but found different type") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_STRING), "Expected string but found different type") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_BOOLEAN), "Expected boolean but found different type") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_NULL), "Expected null but found different type") == 0);
  ASSERT(strcmp(json_error_string(E_INVALID_DATA), "Invalid data format") == 0);
  ASSERT(strcmp(json_error_string(E_MAILFORMED_JSON), "Malformed JSON structure") == 0);
  ASSERT(strcmp(json_error_string(E_UNKNOWN_ERROR), "Unknown or unexpected error") == 0);
  ASSERT(strcmp(json_error_string(E_NULL), "Null pointer encountered") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT_KEY), "Object key with null pointer") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT_VALUE), "Object value with null pointer") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_ARRAY_ELEMENT), "Array element with null pointer") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT_ELEMENT), "Object element with null pointer") == 0);
  ASSERT(strcmp(json_error_string((json_error)255), "Invalid error code") == 0);
  ASSERT_PTR_NOT_NULL(json_error_string(E_NO_ERROR));
  ASSERT_PTR_NOT_NULL(json_error_string((json_error)999));
  END_TEST;
}

TEST(test_json_error_string_with_validate) {
  const char *empty_input = "";
  json_error error = json_validate((const char **)&empty_input, 0);
  ASSERT(strcmp(json_error_string(error), "No data provided or empty input") == 0);

  const char *invalid_start = "invalid";
  const char *invalid_start_ptr = invalid_start;
  error = json_validate(&invalid_start_ptr, strlen(invalid_start));
  ASSERT(strcmp(json_error_string(error), "Invalid JSON structure") == 0);

  END_TEST;
}