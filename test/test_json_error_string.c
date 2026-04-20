#include "../src/json.h"
#include "../test/test.h"

TEST(test_json_error_string_function) {
  ASSERT(strcmp(json_error_string(E_OK), "OK") == 0);
  ASSERT(strcmp(json_error_string(E_INVALID_JSON), "Invalid JSON") == 0);
  ASSERT(strcmp(json_error_string(E_OBJECT_KEY), "Invalid object key") == 0);
  ASSERT(strcmp(json_error_string(E_OBJECT_VALUE), "Invalid object value") == 0);
  ASSERT(strcmp(json_error_string(E_OBJECT), "Invalid object") == 0);
  ASSERT(strcmp(json_error_string(E_ARRAY), "Invalid array") == 0);
  ASSERT(strcmp(json_error_string(E_STRING), "Invalid string") == 0);
  ASSERT(strcmp(json_error_string(E_CONSTANT), "Invalid constant") == 0);
  ASSERT(strcmp(json_error_string(E_NUMBER), "Invalid number") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT_KEY), "Expected object key") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT_VALUE), "Expected object value") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT), "Expected object element") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_ARRAY), "Expected array") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_STRING), "Expected string") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_CONSTANT), "Expected constant (true/false/null)") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_NUMBER), "Expected number") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_OBJECT_ELEMENT), "Expected object element") == 0);
  ASSERT(strcmp(json_error_string(E_EXPECTED_ARRAY_ELEMENT), "Expected array element") == 0);
  ASSERT(strcmp(json_error_string(E_NO_MEMORY_OBJECT), "Out of memory while parsing object") == 0);
  ASSERT(strcmp(json_error_string(E_NO_MEMORY_ARRAY), "Out of memory while parsing parsing array") == 0);
  ASSERT(strcmp(json_error_string((json_error)0xFE), "Unknown error code") == 0);
  ASSERT_PTR_NOT_NULL(json_error_string(E_OK));
  ASSERT_PTR_NOT_NULL(json_error_string((json_error)999));
  END_TEST;
}

TEST(test_json_error_string_with_validate) {
  const char *empty_input = "";
  json_error error = json_validate(empty_input, empty_input);
  ASSERT(strcmp(json_error_string(error), "Invalid JSON") == 0);

  const char *invalid_start = "invalid";
  const char *invalid_start_ptr = invalid_start;
  error = json_validate(invalid_start_ptr, invalid_start_ptr + strlen(invalid_start));
  ASSERT(strcmp(json_error_string(error), "Invalid JSON") == 0);

  END_TEST;
}