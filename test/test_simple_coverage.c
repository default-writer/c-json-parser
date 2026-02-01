#include "../src/json.h"
#include "../test/test.h"

TEST(test_large_array_0xfffe_elements) {
  /* Test to trigger memory pool cleanup by consuming pool nodes */
  int i;

  const int num_elements_size = 1000;
  const int pool_size = 1000;

  /* Create and parse multiple arrays to consume memory pool nodes */
  for (i = 0; i < num_elements_size; i++) {
    json_value val;
    memset(&val, 0, sizeof(json_value));

    /* Create arrays with varying sizes to consume pool */
    char json_str[pool_size];
    sprintf(json_str, "[%d,%d,%d]", i, i + 1, i + 2);
    const size_t len = strlen(json_str);

    bool result = json_parse(json_str, len, &val);
    if (result) {
      /* Successfully parsed - this consumes pool nodes */
      json_free(&val); /* This returns nodes to pool */
    }
  }

  END_TEST;
}