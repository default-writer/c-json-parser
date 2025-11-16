#ifdef USE_JSON_C

#include "../test/test.h"

#include "../libs/include/json-c/json.h"

#define TEST_COUNT 100000UL

TEST_SETUP();

TEST(test_json_c_parser) {
  long long start_time;
  long long end_time;

  const char *filename = "test/test.json";
  FILE *fp = fopen(filename, "r");
  ASSERT_PTR_NOT_NULL(fp);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (size == -1) {
    fclose(fp);
    return;
  }

  char *json = calloc(1, size + 1);
  ASSERT_PTR_NOT_NULL(json);

  fread(json, 1, size, fp);
  json[size] = '\0';
  fclose(fp);

  /* parse into internal json_value* */
  start_time = get_time_ns();
  for (size_t i = 0; i < TEST_COUNT; i++) {
    struct json_object *jobj = json_tokener_parse(json);
    json_object_put(jobj);
  }
  end_time = get_time_ns();
  print_time_diff("test_json_c_parser", start_time, end_time);

  /* cleanup */
  free(json);

  END_TEST;
}

#endif

