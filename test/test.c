#include "../test/test.h"
#include "../src/json.h"

TEST(test_memory_leaks, char *json) {
  const char *source = "[{\"key\": \"value\"}]";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_free(&v);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  utils_output(json);

  /* cleanup */
  json_free(&v);
  free(json);
  free(out);

  END_TEST;
}

TEST(test_printf, char *json) {
  const char *source = "[{\"key\": \"value\"}]";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *out = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(out);

  /* render json_value back to string */
  json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  json_print(&v, stdout);
  fputc('\n', stdout);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, source));

  /* cleanup */
  json_free(&v);
  free(json);
  free(out);

  END_TEST;
}

TEST(test_whitespace) {
  const char *source = " { \t \"key\" \n : \r \"value\" } ";
  const char *expected = "{\n    \"key\": \"value\"\n}";

  json_value v;
  memset(&v, 0, sizeof(json_value));

  /* parse into internal json_value* */
  json_parse(source, &v);
  ASSERT_PTR_NOT_NULL(&v);

  /* render json_value back to string */
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);

  /* compare structurally (order-insensitive) */
  ASSERT_TRUE(utils_test_json_equal(json, expected));

  utils_output(json);

  /* cleanup */
  json_free(&v);
  free(json);

  END_TEST;
}

TEST(test_array) {
  char *source = utils_get_test_json_data("data/array.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  char *json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  char *json_iterative = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative);

  ASSERT_TRUE(utils_test_json_equal(json_recursive, json_iterative));
  ASSERT_TRUE(strcmp(json_recursive, json_iterative) == 0);

  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive);
  free(json_iterative);
  free(source);

  END_TEST;
}

TEST(test_object) {
  char *source = utils_get_test_json_data("data/object.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  char *json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  char *json_iterative = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative);

  ASSERT_TRUE(utils_test_json_equal(json_recursive, json_iterative));
  ASSERT_TRUE(strcmp(json_recursive, json_iterative) == 0);

  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive);
  free(json_iterative);
  free(source);

  END_TEST;
}

TEST(test_invalid_number_leading_zero) {
  const char *source = "[01]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_valid_number_zero_point_zero) {
  const char *source = "[0.0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_json_parse) {
  char *source = utils_get_test_json_data("data/test.json");
  ASSERT_PTR_NOT_NULL(source);

  json_value v_recursive;
  memset(&v_recursive, 0, sizeof(json_value));

  json_value v_iterative;
  memset(&v_iterative, 0, sizeof(json_value));

  ASSERT_TRUE(json_parse(source, &v_recursive));
  ASSERT_TRUE(json_parse_iterative(source, &v_iterative));

  char *json_recursive = json_stringify(&v_recursive);
  ASSERT_PTR_NOT_NULL(json_recursive);

  char *json_iterative = json_stringify(&v_iterative);
  ASSERT_PTR_NOT_NULL(json_iterative);

  ASSERT_TRUE(utils_test_json_equal(json_recursive, json_iterative));
  ASSERT_TRUE(strcmp(json_recursive, json_iterative) == 0);

  json_free(&v_recursive);
  json_free(&v_iterative);
  free(json_recursive);
  free(json_iterative);
  free(source);

  END_TEST;
}

TEST(test_case_0) {
  const char *source = "[]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_1) {
  const char *source = "[null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_2) {
  const char *source = "[null, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_3) {
  const char *source = "[true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_4) {
  const char *source = "[true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_5) {
  const char *source = "[false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_6) {
  const char *source = "[false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_7) {
  const char *source = "[true, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_8) {
  const char *source = "[0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_9) {
  const char *source = "[0, 0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_10) {
  const char *source = "[1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_11) {
  const char *source = "[1, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_12) {
  const char *source = "[0, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_13) {
  const char *source = "[0, 1, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_14) {
  const char *source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_15) {
  const char *source = "[{}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_16) {
  const char *source = "[{\"key\": null}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_17) {
  const char *source = "[{\"key\": []}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_18) {
  const char *source = "[{\"key\": [null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_19) {
  const char *source = "[{\"key\": [null, null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_20) {
  const char *source = "[{\"key1\": null, \"key2\":[]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_21) {
  const char *source = "[{\"key1\": null, \"key2\":[null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_22) {
  const char *source = "[{\"key1\": null, \"key2\":[null,null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_23) {
  const char *source = "{}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_24) {
  const char *source = "{\"key\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_25) {
  const char *source = "{\"key1\": null, \"key2\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_26) {
  const char *source = "{\"key\": []}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_27) {
  const char *source = "{\"key\": [null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_28) {
  const char *source = "{\"key\": [null, null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_29) {
  const char *source = "{\"key\": true}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_30) {
  const char *source = "{\"key\": false}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_31) {
  const char *source = "{\"key\": 0}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_32) {
  const char *source = "{\"key\": 1}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_33) {
  const char *source = "{\"key\": 0.5}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_34) {
  const char *source = "{\"key\": \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_35) {
  const char *source = "{\"key\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_36) {
  const char *source = "{\"key1\": {}, \"key2\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_37) {
  const char *source = "{\"key\": [{\"subkey\": []}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_38) {
  const char *source = "{\"key\": [{\"subkey\": [null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_39) {
  const char *source = "{\"key\": [{\"subkey\": [null, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_40) {
  const char *source = "{\"key\": [{\"subkey\": null\"}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_41) {
  const char *source = "{\"key\": [{\"subkey\": [true]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_42) {
  const char *source = "{\"key\": [{\"subkey\": [true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_43) {
  const char *source = "{\"key\": [{\"subkey\": [false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_44) {
  const char *source = "{\"key\": [{\"subkey\": [false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_45) {
  const char *source = "{\"key\": [{\"subkey\": [true, false]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_46) {
  const char *source = "{\"key\": [{\"subkey\": [0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_47) {
  const char *source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_48) {
  const char *source = "{\"key\": [{\"subkey\": [1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_49) {
  const char *source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_50) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_51) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_52) {
  const char *source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_53) {
  const char *source = "{\"key\": [{\"subkey\": [{}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_54) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_55) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_56) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_57) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_58) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_59) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_60) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_61) {
  const char *source = "{\"key\": [{\"subkey\": {}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_62) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_63) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_64) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_65) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_66) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_67) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_68) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_69) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_70) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_71) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_72) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_73) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_74) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_invalid_char_case_0) {
  const char *original_source = "[]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_1) {
  const char *original_source = "[null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_2) {
  const char *original_source = "[null, null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_3) {
  const char *original_source = "[true]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_4) {
  const char *original_source = "[true, true]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_5) {
  const char *original_source = "[false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_6) {
  const char *original_source = "[false, false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_7) {
  const char *original_source = "[true, false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_8) {
  const char *original_source = "[0]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_9) {
  const char *original_source = "[0, 0]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_10) {
  const char *original_source = "[1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_11) {
  const char *original_source = "[1, 1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_12) {
  const char *original_source = "[0, 1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_13) {
  const char *original_source = "[0, 1, null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_14) {
  const char *original_source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if ((i > 0 && original_source[i - 1] == '1' && original_source[i] == '2') || (i < len - 1 && original_source[i] == '1' && original_source[i + 1] == '2') || (i < len - 1 && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_15) {
  const char *original_source = "[{}]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_16) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_17) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_18) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_19) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_20) {
  const char *original_source = "{\"key1\": null, \"key2\":[]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_21) {
  const char *original_source = "{\"key1\": null, \"key2\":[null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_22) {
  const char *original_source = "{\"key1\": null, \"key2\":[null,null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_23) {
  const char *original_source = "{}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_24) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_25) {
  const char *original_source = "{\"key1\": null, \"key2\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_26) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_27) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_28) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_29) {
  const char *original_source = "{\"key\": true}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_30) {
  const char *original_source = "{\"key\": false}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_31) {
  const char *original_source = "{\"key\": 0}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_32) {
  const char *original_source = "{\"key\": 1}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_33) {
  const char *original_source = "{\"key\": 0.5}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_34) {
  const char *original_source = "{\"key\": \"value\"}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_35) {
  const char *original_source = "{\"key\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_36) {
  const char *original_source = "{\"key1\": {}, \"key2\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_37) {
  const char *original_source = "{\"key\": [{\"subkey\": []}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_38) {
  const char *original_source = "{\"key\": [{\"subkey\": [null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_39) {
  const char *original_source = "{\"key\": [{\"subkey\": [null, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_40) {
  const char *original_source = "{\"key\": [{\"subkey\": [true]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_41) {
  const char *original_source = "{\"key\": [{\"subkey\": [true, false]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_42) {
  const char *original_source = "{\"key\": [{\"subkey\": [0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '0')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_43) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_44) {
  const char *original_source = "{\"key\": [{\"subkey\": [1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_45) {
  const char *original_source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_46) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_47) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_48) {
  const char *original_source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || (original_source[i] == '2' && original_source[i + 1] == ',') || (i > 0 && original_source[i - 1] == ' ' && original_source[i] == '1') || (i < len && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_49) {
  const char *original_source = "{\"key\": [{\"subkey\": [{}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_50) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_51) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_52) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_53) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_54) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_55) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_56) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_57) {
  const char *original_source = "{\"key\": [{\"subkey\": {}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_58) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_59) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_60) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_61) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_62) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_63) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_64) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_65) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_66) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_67) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_68) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_69) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_70) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_error_case_truncated_input) {
  const char *test_cases[] = {
      "[]",
      "[null]",
      "[null, null]",
      "[true]",
      "[true, true]",
      "[false]",
      "[false, false]",
      "[true, false]",
      "[0]",
      "[0, 0]",
      "[1]",
      "[1, 1]",
      "[0, 1]",
      "[0, 1, null]",
      "[0.0, 0.1, 2.1, 1e12, 1234567890]",
      "[{}]",
      "[{\"key\": null}]",
      "[{\"key\": []}]",
      "[{\"key\": [null]}]",
      "[{\"key\": [null, null]}]",
      "[{\"key1\": null, \"key2\":[]}]",
      "[{\"key1\": null, \"key2\":[null]}]",
      "[{\"key1\": null, \"key2\":[null,null]}]",
      "{}",
      "{\"key\": null}",
      "{\"key1\": null, \"key2\": null}",
      "{\"key\": []}",
      "{\"key\": [null]}",
      "{\"key\": [null, null]}",
      "{\"key\": true}",
      "{\"key\": false}",
      "{\"key\": 0}",
      "{\"key\": 1}",
      "{\"key\": 0.5}",
      "{\"key\": \"value\"}",
      "{\"key\": {}}",
      "{\"key1\": {}, \"key2\": {}}",
      "{\"key\": [{\"subkey\": []}]}",
      "{\"key\": [{\"subkey\": [null]}]}",
      "{\"key\": [{\"subkey\": [null, null]}]}",
      "{\"key\": [{\"subkey\": [true]}]}",
      "{\"key\": [{\"subkey\": [true, false]}]}",
      "{\"key\": [{\"subkey\": [0]}]}",
      "{\"key\": [{\"subkey\": [0, 0]}]}",
      "{\"key\": [{\"subkey\": [1]}]}",
      "{\"key\": [{\"subkey\": [1, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1, null]}]}",
      "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}",
      "{\"key\": [{\"subkey\": [{}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": null}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": []}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}",
      "{\"key\": [{\"subkey\": {}}]}",
      "{\"key\": [{\"subkey\": {\"key\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key\": []}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": true}}]}",
      "{\"key\": [{\"subkey\": {\"key\": false}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 1}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}",
      "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}",
      "{\"key\": [{\"subkey\": {\"key\": {}}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}",
  };
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int j = 0; j < num_tests; j++) {
    const char *original_source = test_cases[j];
    size_t len = strlen(original_source);
    char *source = (char *)calloc(1, len + 1);
    json_value v;
    for (size_t i = 0; i < len; i++) {
      memcpy(source, original_source, len + 1);
      source[i] = '\0';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse(source, &v));
      json_free(&v);
    }
    free(source);
  }
  END_TEST;
}

TEST(test_case_iterative_0) {
  const char *source = "[]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_1) {
  const char *source = "[null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_2) {
  const char *source = "[null, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_3) {
  const char *source = "[true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_4) {
  const char *source = "[true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_5) {
  const char *source = "[false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_6) {
  const char *source = "[false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_7) {
  const char *source = "[true, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_8) {
  const char *source = "[0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_9) {
  const char *source = "[0, 0]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_10) {
  const char *source = "[1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_11) {
  const char *source = "[1, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_12) {
  const char *source = "[0, 1]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_13) {
  const char *source = "[0, 1, null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_14) {
  const char *source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_15) {
  const char *source = "[{}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_16) {
  const char *source = "[{\"key\": null}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_17) {
  const char *source = "[{\"key\": []}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_18) {
  const char *source = "[{\"key\": [null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_19) {
  const char *source = "[{\"key\": [null, null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_20) {
  const char *source = "[{\"key1\": null, \"key2\":[]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_21) {
  const char *source = "[{\"key1\": null, \"key2\":[null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_22) {
  const char *source = "[{\"key1\": null, \"key2\":[null,null]}]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_23) {
  const char *source = "{}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_24) {
  const char *source = "{\"key\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_25) {
  const char *source = "{\"key1\": null, \"key2\": null}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_26) {
  const char *source = "{\"key\": []}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_27) {
  const char *source = "{\"key\": [null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_28) {
  const char *source = "{\"key\": [null, null]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_29) {
  const char *source = "{\"key\": true}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_30) {
  const char *source = "{\"key\": false}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_31) {
  const char *source = "{\"key\": 0}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_32) {
  const char *source = "{\"key\": 1}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_33) {
  const char *source = "{\"key\": 0.5}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_34) {
  const char *source = "{\"key\": \"value\"}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_35) {
  const char *source = "{\"key\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_36) {
  const char *source = "{\"key1\": {}, \"key2\": {}}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_37) {
  const char *source = "{\"key\": [{\"subkey\": []}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_38) {
  const char *source = "{\"key\": [{\"subkey\": [null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_39) {
  const char *source = "{\"key\": [{\"subkey\": [null, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_40) {
  const char *source = "{\"key\": [{\"subkey\": null\"}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_41) {
  const char *source = "{\"key\": [{\"subkey\": [true]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_42) {
  const char *source = "{\"key\": [{\"subkey\": [true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_43) {
  const char *source = "{\"key\": [{\"subkey\": [false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_44) {
  const char *source = "{\"key\": [{\"subkey\": [false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_case_iterative_45) {
  const char *source = "{\"key\": [{\"subkey\": [true, false]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_46) {
  const char *source = "{\"key\": [{\"subkey\": [0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_47) {
  const char *source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_48) {
  const char *source = "{\"key\": [{\"subkey\": [1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_49) {
  const char *source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_50) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_51) {
  const char *source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_52) {
  const char *source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_53) {
  const char *source = "{\"key\": [{\"subkey\": [{}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_54) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_55) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_56) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_57) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_58) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_59) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_60) {
  const char *source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_61) {
  const char *source = "{\"key\": [{\"subkey\": {}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_62) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_63) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_64) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_65) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_66) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_67) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_68) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_69) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_70) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_71) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_72) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_73) {
  const char *source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_case_iterative_74) {
  const char *source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_TRUE(json_parse_iterative(source, &v));
  char *json = json_stringify(&v);
  ASSERT_PTR_NOT_NULL(json);
  ASSERT_TRUE(utils_test_json_equal(json, source));
  json_free(&v);
  free(json);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_0) {
  const char *original_source = "[]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_1) {
  const char *original_source = "[null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_2) {
  const char *original_source = "[null, null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_3) {
  const char *original_source = "[true]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_4) {
  const char *original_source = "[true, true]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_5) {
  const char *original_source = "[false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_6) {
  const char *original_source = "[false, false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_7) {
  const char *original_source = "[true, false]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_8) {
  const char *original_source = "[0]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_9) {
  const char *original_source = "[0, 0]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_10) {
  const char *original_source = "[1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i += 2) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_11) {
  const char *original_source = "[1, 1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_12) {
  const char *original_source = "[0, 1]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_13) {
  const char *original_source = "[0, 1, null]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_14) {
  const char *original_source = "[0.0, 0.1, 2.1, 1e12, 1234567890]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if ((i > 0 && original_source[i - 1] == '1' && original_source[i] == '2') || (i < len - 1 && original_source[i] == '1' && original_source[i + 1] == '2') || (i < len - 1 && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_15) {
  const char *original_source = "[{}]";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_16) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_17) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_18) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_19) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_20) {
  const char *original_source = "{\"key1\": null, \"key2\":[]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_21) {
  const char *original_source = "{\"key1\": null, \"key2\":[null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_22) {
  const char *original_source = "{\"key1\": null, \"key2\":[null,null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_23) {
  const char *original_source = "{}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_24) {
  const char *original_source = "{\"key\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_25) {
  const char *original_source = "{\"key1\": null, \"key2\": null}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_26) {
  const char *original_source = "{\"key\": []}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_27) {
  const char *original_source = "{\"key\": [null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_28) {
  const char *original_source = "{\"key\": [null, null]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_29) {
  const char *original_source = "{\"key\": true}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_30) {
  const char *original_source = "{\"key\": false}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_31) {
  const char *original_source = "{\"key\": 0}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_32) {
  const char *original_source = "{\"key\": 1}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_33) {
  const char *original_source = "{\"key\": 0.5}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_34) {
  const char *original_source = "{\"key\": \"value\"}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_35) {
  const char *original_source = "{\"key\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_36) {
  const char *original_source = "{\"key1\": {}, \"key2\": {}}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_37) {
  const char *original_source = "{\"key\": [{\"subkey\": []}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_38) {
  const char *original_source = "{\"key\": [{\"subkey\": [null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_39) {
  const char *original_source = "{\"key\": [{\"subkey\": [null, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_40) {
  const char *original_source = "{\"key\": [{\"subkey\": [true]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_41) {
  const char *original_source = "{\"key\": [{\"subkey\": [true, false]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_42) {
  const char *original_source = "{\"key\": [{\"subkey\": [0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '0')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_43) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 0]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_44) {
  const char *original_source = "{\"key\": [{\"subkey\": [1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_45) {
  const char *original_source = "{\"key\": [{\"subkey\": [1, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_46) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_47) {
  const char *original_source = "{\"key\": [{\"subkey\": [0, 1, null]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_48) {
  const char *original_source = "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || (original_source[i] == '2' && original_source[i + 1] == ',') || (i > 0 && original_source[i - 1] == ' ' && original_source[i] == '1') || (i < len && original_source[i] == '0' && original_source[i + 1] == ']'))
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_49) {
  const char *original_source = "{\"key\": [{\"subkey\": [{}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_50) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": null}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_51) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": []}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_52) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_53) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_54) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_55) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_56) {
  const char *original_source = "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_57) {
  const char *original_source = "{\"key\": [{\"subkey\": {}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_58) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_59) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_60) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": []}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_61) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_62) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_63) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": true}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_64) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": false}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_65) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_66) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 1}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_67) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_68) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == 'v' || original_source[i] == 'a' || original_source[i] == 'l' || original_source[i] == 'u' || original_source[i] == 'e')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_69) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_invalid_char_case_iterative_70) {
  const char *original_source = "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}";
  size_t len = strlen(original_source);
  char *source = (char *)calloc(1, len + 1);
  json_value v;
  for (size_t i = 0; i < len; i++) {
    if (original_source[i] == 'k' || original_source[i] == 'e' || original_source[i] == 'y' || original_source[i] == 's' || original_source[i] == 'u' || original_source[i] == 'b' || original_source[i] == '1' || original_source[i] == '2')
      continue;
    if (!isspace(original_source[i])) {
      memcpy(source, original_source, len + 1);
      source[i] = ' ';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
  }
  free(source);
  END_TEST;
}

TEST(test_error_case_iterative_truncated_input) {
  const char *test_cases[] = {
      "[]",
      "[null]",
      "[null, null]",
      "[true]",
      "[true, true]",
      "[false]",
      "[false, false]",
      "[true, false]",
      "[0]",
      "[0, 0]",
      "[1]",
      "[1, 1]",
      "[0, 1]",
      "[0, 1, null]",
      "[0.0, 0.1, 2.1, 1e12, 1234567890]",
      "[{}]",
      "[{\"key\": null}]",
      "[{\"key\": []}]",
      "[{\"key\": [null]}]",
      "[{\"key\": [null, null]}]",
      "[{\"key1\": null, \"key2\":[]}]",
      "[{\"key1\": null, \"key2\":[null]}]",
      "[{\"key1\": null, \"key2\":[null,null]}]",
      "{}",
      "{\"key\": null}",
      "{\"key1\": null, \"key2\": null}",
      "{\"key\": []}",
      "{\"key\": [null]}",
      "{\"key\": [null, null]}",
      "{\"key\": true}",
      "{\"key\": false}",
      "{\"key\": 0}",
      "{\"key\": 1}",
      "{\"key\": 0.5}",
      "{\"key\": \"value\"}",
      "{\"key\": {}}",
      "{\"key1\": {}, \"key2\": {}}",
      "{\"key\": [{\"subkey\": []}]}",
      "{\"key\": [{\"subkey\": [null]}]}",
      "{\"key\": [{\"subkey\": [null, null]}]}",
      "{\"key\": [{\"subkey\": [true]}]}",
      "{\"key\": [{\"subkey\": [true, false]}]}",
      "{\"key\": [{\"subkey\": [0]}]}",
      "{\"key\": [{\"subkey\": [0, 0]}]}",
      "{\"key\": [{\"subkey\": [1]}]}",
      "{\"key\": [{\"subkey\": [1, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1]}]}",
      "{\"key\": [{\"subkey\": [0, 1, null]}]}",
      "{\"key\": [{\"subkey\": [0.0, 0.1, 2.1, 1e12, 1234567890]}]}",
      "{\"key\": [{\"subkey\": [{}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": null}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": []}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key\": [null, null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null]}]}]}",
      "{\"key\": [{\"subkey\": [{\"key1\": null, \"key2\":[null,null]}]}]}",
      "{\"key\": [{\"subkey\": {}}]}",
      "{\"key\": [{\"subkey\": {\"key\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": null, \"key2\": null}}]}",
      "{\"key\": [{\"subkey\": {\"key\": []}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": [null, null]}}]}",
      "{\"key\": [{\"subkey\": {\"key\": true}}]}",
      "{\"key\": [{\"subkey\": {\"key\": false}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 1}}]}",
      "{\"key\": [{\"subkey\": {\"key\": 0.5}}]}",
      "{\"key\": [{\"subkey\": {\"key\": \"value\"}}]}",
      "{\"key\": [{\"subkey\": {\"key\": {}}}]}",
      "{\"key\": [{\"subkey\": {\"key1\": {}, \"key2\": {}}}]}",
  };
  int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int j = 0; j < num_tests; j++) {
    const char *original_source = test_cases[j];
    size_t len = strlen(original_source);
    char *source = (char *)calloc(1, len + 1);
    json_value v;
    for (size_t i = 0; i < len; i++) {
      memcpy(source, original_source, len + 1);
      source[i] = '\0';
      memset(&v, 0, sizeof(json_value));
      ASSERT_FALSE(json_parse_iterative(source, &v));
      json_free(&v);
    }
    free(source);
  }
  END_TEST;
}

TEST(test_case_iterative__2) {
  const char *source = "[null null]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse_iterative(source, &v));
  json_free(&v);
  END_TEST;
}

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("unit tests");
  test_memory_leaks();
  test_printf();
  test_whitespace();
  test_array();
  test_object();
  test_invalid_number_leading_zero();
  test_valid_number_zero_point_zero();
  test_json_parse();
  test_case_0();
  test_case_1();
  test_case_2();
  test_case_3();
  test_case_4();
  test_case_5();
  test_case_6();
  test_case_7();
  test_case_8();
  test_case_9();
  test_case_10();
  test_case_11();
  test_case_12();
  test_case_13();
  test_case_14();
  test_case_15();
  test_case_16();
  test_case_17();
  test_case_18();
  test_case_19();
  test_case_20();
  test_case_21();
  test_case_22();
  test_case_23();
  test_case_24();
  test_case_25();
  test_case_26();
  test_case_27();
  test_case_28();
  test_case_29();
  test_case_30();
  test_case_31();
  test_case_32();
  test_case_33();
  test_case_34();
  test_case_35();
  test_case_36();
  test_case_37();
  test_case_38();
  test_case_39();
  test_case_40();
  test_case_41();
  test_case_42();
  test_case_43();
  test_case_44();
  test_case_45();
  test_case_46();
  test_case_47();
  test_case_48();
  test_case_49();
  test_case_50();
  test_case_51();
  test_case_52();
  test_case_53();
  test_case_54();
  test_case_55();
  test_case_56();
  test_case_57();
  test_case_58();
  test_case_59();
  test_case_60();
  test_case_61();
  test_case_62();
  test_case_63();
  test_case_64();
  test_case_65();
  test_case_66();
  test_case_67();
  test_case_68();
  test_case_69();
  test_case_70();
  test_case_71();
  test_case_72();
  test_case_73();
  test_case_74();
  test_invalid_char_case_0();
  test_invalid_char_case_1();
  test_invalid_char_case_2();
  test_invalid_char_case_3();
  test_invalid_char_case_4();
  test_invalid_char_case_5();
  test_invalid_char_case_6();
  test_invalid_char_case_7();
  test_invalid_char_case_8();
  test_invalid_char_case_9();
  test_invalid_char_case_10();
  test_invalid_char_case_11();
  test_invalid_char_case_12();
  test_invalid_char_case_13();
  test_invalid_char_case_14();
  test_invalid_char_case_15();
  test_invalid_char_case_16();
  test_invalid_char_case_17();
  test_invalid_char_case_18();
  test_invalid_char_case_19();
  test_invalid_char_case_20();
  test_invalid_char_case_21();
  test_invalid_char_case_22();
  test_invalid_char_case_23();
  test_invalid_char_case_24();
  test_invalid_char_case_25();
  test_invalid_char_case_26();
  test_invalid_char_case_27();
  test_invalid_char_case_28();
  test_invalid_char_case_29();
  test_invalid_char_case_30();
  test_invalid_char_case_31();
  test_invalid_char_case_32();
  test_invalid_char_case_33();
  test_invalid_char_case_34();
  test_invalid_char_case_35();
  test_invalid_char_case_36();
  test_invalid_char_case_37();
  test_invalid_char_case_38();
  test_invalid_char_case_39();
  test_invalid_char_case_40();
  test_invalid_char_case_41();
  test_invalid_char_case_42();
  test_invalid_char_case_43();
  test_invalid_char_case_44();
  test_invalid_char_case_45();
  test_invalid_char_case_46();
  test_invalid_char_case_47();
  test_invalid_char_case_48();
  test_invalid_char_case_49();
  test_invalid_char_case_50();
  test_invalid_char_case_51();
  test_invalid_char_case_52();
  test_invalid_char_case_53();
  test_invalid_char_case_54();
  test_invalid_char_case_55();
  test_invalid_char_case_56();
  test_invalid_char_case_57();
  test_invalid_char_case_58();
  test_invalid_char_case_59();
  test_invalid_char_case_60();
  test_invalid_char_case_61();
  test_invalid_char_case_62();
  test_invalid_char_case_63();
  test_invalid_char_case_64();
  test_invalid_char_case_65();
  test_invalid_char_case_66();
  test_invalid_char_case_67();
  test_invalid_char_case_68();
  test_invalid_char_case_69();
  test_invalid_char_case_70();
  test_error_case_truncated_input();
  test_case_iterative_0();
  test_case_iterative_1();
  test_case_iterative_2();
  test_case_iterative__2();
  test_case_iterative_3();
  test_case_iterative_4();
  test_case_iterative_5();
  test_case_iterative_6();
  test_case_iterative_7();
  test_case_iterative_8();
  test_case_iterative_9();
  test_case_iterative_10();
  test_case_iterative_11();
  test_case_iterative_12();
  test_case_iterative_13();
  test_case_iterative_14();
  test_case_iterative_15();
  test_case_iterative_16();
  test_case_iterative_17();
  test_case_iterative_18();
  test_case_iterative_19();
  test_case_iterative_20();
  test_case_iterative_21();
  test_case_iterative_22();
  test_case_iterative_23();
  test_case_iterative_24();
  test_case_iterative_25();
  test_case_iterative_26();
  test_case_iterative_27();
  test_case_iterative_28();
  test_case_iterative_29();
  test_case_iterative_30();
  test_case_iterative_31();
  test_case_iterative_32();
  test_case_iterative_33();
  test_case_iterative_34();
  test_case_iterative_35();
  test_case_iterative_36();
  test_case_iterative_37();
  test_case_iterative_38();
  test_case_iterative_39();
  test_case_iterative_40();
  test_case_iterative_41();
  test_case_iterative_42();
  test_case_iterative_43();
  test_case_iterative_44();
  test_case_iterative_45();
  test_case_iterative_46();
  test_case_iterative_47();
  test_case_iterative_48();
  test_case_iterative_49();
  test_case_iterative_50();
  test_case_iterative_51();
  test_case_iterative_52();
  test_case_iterative_53();
  test_case_iterative_54();
  test_case_iterative_55();
  test_case_iterative_56();
  test_case_iterative_57();
  test_case_iterative_58();
  test_case_iterative_59();
  test_case_iterative_60();
  test_case_iterative_61();
  test_case_iterative_62();
  test_case_iterative_63();
  test_case_iterative_64();
  test_case_iterative_65();
  test_case_iterative_66();
  test_case_iterative_67();
  test_case_iterative_68();
  test_case_iterative_69();
  test_case_iterative_70();
  test_case_iterative_71();
  test_case_iterative_72();
  test_case_iterative_73();
  test_case_iterative_74();
  test_invalid_char_case_iterative_0();
  test_invalid_char_case_iterative_1();
  test_invalid_char_case_iterative_2();
  test_invalid_char_case_iterative_3();
  test_invalid_char_case_iterative_4();
  test_invalid_char_case_iterative_5();
  test_invalid_char_case_iterative_6();
  test_invalid_char_case_iterative_7();
  test_invalid_char_case_iterative_8();
  test_invalid_char_case_iterative_9();
  test_invalid_char_case_iterative_10();
  test_invalid_char_case_iterative_11();
  test_invalid_char_case_iterative_12();
  test_invalid_char_case_iterative_13();
  test_invalid_char_case_iterative_14();
  test_invalid_char_case_iterative_15();
  test_invalid_char_case_iterative_16();
  test_invalid_char_case_iterative_17();
  test_invalid_char_case_iterative_18();
  test_invalid_char_case_iterative_19();
  test_invalid_char_case_iterative_20();
  test_invalid_char_case_iterative_21();
  test_invalid_char_case_iterative_22();
  test_invalid_char_case_iterative_23();
  test_invalid_char_case_iterative_24();
  test_invalid_char_case_iterative_25();
  test_invalid_char_case_iterative_26();
  test_invalid_char_case_iterative_27();
  test_invalid_char_case_iterative_28();
  test_invalid_char_case_iterative_29();
  test_invalid_char_case_iterative_30();
  test_invalid_char_case_iterative_31();
  test_invalid_char_case_iterative_32();
  test_invalid_char_case_iterative_33();
  test_invalid_char_case_iterative_34();
  test_invalid_char_case_iterative_35();
  test_invalid_char_case_iterative_36();
  test_invalid_char_case_iterative_37();
  test_invalid_char_case_iterative_38();
  test_invalid_char_case_iterative_39();
  test_invalid_char_case_iterative_40();
  test_invalid_char_case_iterative_41();
  test_invalid_char_case_iterative_42();
  test_invalid_char_case_iterative_43();
  test_invalid_char_case_iterative_44();
  test_invalid_char_case_iterative_45();
  test_invalid_char_case_iterative_46();
  test_invalid_char_case_iterative_47();
  test_invalid_char_case_iterative_48();
  test_invalid_char_case_iterative_49();
  test_invalid_char_case_iterative_50();
  test_invalid_char_case_iterative_51();
  test_invalid_char_case_iterative_52();
  test_invalid_char_case_iterative_53();
  test_invalid_char_case_iterative_54();
  test_invalid_char_case_iterative_55();
  test_invalid_char_case_iterative_56();
  test_invalid_char_case_iterative_57();
  test_invalid_char_case_iterative_58();
  test_invalid_char_case_iterative_59();
  test_invalid_char_case_iterative_60();
  test_invalid_char_case_iterative_61();
  test_invalid_char_case_iterative_62();
  test_invalid_char_case_iterative_63();
  test_invalid_char_case_iterative_64();
  test_invalid_char_case_iterative_65();
  test_invalid_char_case_iterative_66();
  test_invalid_char_case_iterative_67();
  test_invalid_char_case_iterative_68();
  test_invalid_char_case_iterative_69();
  test_invalid_char_case_iterative_70();
  test_error_case_iterative_truncated_input();  
  TEST_FINALIZE;
}
