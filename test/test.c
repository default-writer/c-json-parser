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

TEST(test_simple_case_0) {
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

TEST(test_simple_case_1) {
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

TEST(test_simple_case_2) {
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

TEST(test_simple_case_3) {
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

TEST(test_simple_case_4) {
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

TEST(test_simple_case_5) {
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

TEST(test_simple_case_6) {
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

TEST(test_simple_case_7) {
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

TEST(test_simple_case_8) {
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

TEST(test_simple_case_9) {
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

TEST(test_simple_case_10) {
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

TEST(test_simple_case_11) {
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

TEST(test_simple_case_12) {
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

TEST(test_simple_case_13) {
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

TEST(test_simple_case_14) {
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

TEST(test_simple_case_15) {
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

TEST(test_simple_case_16) {
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

TEST(test_simple_case_17) {
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

TEST(test_simple_case_18) {
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

TEST(test_simple_case_19) {
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

TEST(test_simple_case_20) {
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

TEST(test_simple_case_21) {
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

TEST(test_simple_case_22) {
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

TEST(test_simple_case_23) {
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

TEST(test_simple_case_24) {
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

TEST(test_simple_case_25) {
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

TEST(test_simple_case_26) {
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

TEST(test_simple_case_27) {
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

TEST(test_simple_case_28) {
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

TEST(test_simple_case_29) {
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

TEST(test_simple_case_30) {
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

TEST(test_simple_case_31) {
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

TEST(test_simple_case_32) {
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

TEST(test_simple_case_33) {
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

TEST(test_simple_case_34) {
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

TEST(test_simple_case_35) {
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

TEST(test_simple_case_36) {
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

TEST(test_simple_case_37) {
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

TEST(test_simple_case_38) {
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

TEST(test_simple_case_39) {
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

TEST(test_simple_case_40) {
  const char *source = "{\"key\": [{\"subkey\": null\"}]}";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_simple_case_41) {
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

TEST(test_simple_case_42) {
  const char *source = "{\"key\": [{\"subkey\": [true, true]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_simple_case_43) {
  const char *source = "{\"key\": [{\"subkey\": [false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_simple_case_44) {
  const char *source = "{\"key\": [{\"subkey\": [false, false]";
  json_value v;
  memset(&v, 0, sizeof(json_value));
  ASSERT_FALSE(json_parse(source, &v));
  json_free(&v);
  END_TEST;
}

TEST(test_simple_case_45) {
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

TEST(test_simple_case_46) {
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

TEST(test_simple_case_47) {
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

TEST(test_simple_case_48) {
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

TEST(test_simple_case_49) {
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

TEST(test_simple_case_50) {
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

TEST(test_simple_case_51) {
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

TEST(test_simple_case_52) {
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

TEST(test_simple_case_53) {
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

TEST(test_simple_case_54) {
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

TEST(test_simple_case_55) {
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

TEST(test_simple_case_56) {
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

TEST(test_simple_case_57) {
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

TEST(test_simple_case_58) {
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

TEST(test_simple_case_59) {
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

TEST(test_simple_case_60) {
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

TEST(test_simple_case_61) {
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

TEST(test_simple_case_62) {
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

TEST(test_simple_case_63) {
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

TEST(test_simple_case_64) {
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

TEST(test_simple_case_65) {
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

TEST(test_simple_case_66) {
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

TEST(test_simple_case_67) {
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

TEST(test_simple_case_68) {
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

TEST(test_simple_case_69) {
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

TEST(test_simple_case_70) {
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

TEST(test_simple_case_71) {
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

TEST(test_simple_case_72) {
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

TEST(test_simple_case_73) {
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

TEST(test_simple_case_74) {
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

int main(void) {
  TEST_INITIALIZE;
  TEST_SUITE("unit tests");
  test_memory_leaks();
  test_printf();
  test_whitespace();
  test_array();
  test_object();
  test_json_parse();
  test_simple_case_0();
  test_simple_case_1();
  test_simple_case_2();
  test_simple_case_3();
  test_simple_case_4();
  test_simple_case_5();
  test_simple_case_6();
  test_simple_case_7();
  test_simple_case_8();
  test_simple_case_9();
  test_simple_case_10();
  test_simple_case_11();
  test_simple_case_12();
  test_simple_case_13();
  test_simple_case_14();
  test_simple_case_15();
  test_simple_case_16();
  test_simple_case_17();
  test_simple_case_18();
  test_simple_case_19();
  test_simple_case_20();
  test_simple_case_21();
  test_simple_case_22();
  test_simple_case_23();
  test_simple_case_24();
  test_simple_case_25();
  test_simple_case_26();
  test_simple_case_27();
  test_simple_case_28();
  test_simple_case_29();
  test_simple_case_30();
  test_simple_case_31();
  test_simple_case_32();
  test_simple_case_33();
  test_simple_case_34();
  test_simple_case_35();
  test_simple_case_36();
  test_simple_case_37();
  test_simple_case_38();
  test_simple_case_39();
  test_simple_case_40();
  test_simple_case_41();
  test_simple_case_42();
  test_simple_case_43();
  test_simple_case_44();
  test_simple_case_45();
  test_simple_case_46();
  test_simple_case_47();
  test_simple_case_48();
  test_simple_case_49();
  test_simple_case_50();
  test_simple_case_51();
  test_simple_case_52();
  test_simple_case_53();
  test_simple_case_54();
  test_simple_case_55();
  test_simple_case_56();
  test_simple_case_57();
  test_simple_case_58();
  test_simple_case_59();
  test_simple_case_60();
  test_simple_case_61();
  test_simple_case_62();
  test_simple_case_63();
  test_simple_case_64();
  test_simple_case_65();
  test_simple_case_66();
  test_simple_case_67();
  test_simple_case_68();
  test_simple_case_69();
  test_simple_case_70();
  test_simple_case_71();
  test_simple_case_72();
  test_simple_case_73();
  test_simple_case_74();
  TEST_FINALIZE;
}
