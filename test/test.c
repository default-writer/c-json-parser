#include "../test/test.h"
#include "../src/json.h"

#define PREFIX_CHAR_OFFSET 10
#define POSTFIX_CHAR_OFFSET 10

int tests_run = 0;
int tests_passed = 0;

const char *GREEN = "";
const char *RED = "";
const char *RESET = "";

void test_initialize() {
#ifdef _WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      if (SetConsoleMode(hOut, dwMode)) {
        GREEN = "\033[0;32m";
        RED = "\033[0;31m";
        RESET = "\033[0m";
      }
    }
  }
#else
  GREEN = "\033[0;32m";
  RED = "\033[0;31m";
  RESET = "\033[0m";
#endif
}

static void skip_ws(const char **s) {
  while (isspace((unsigned char)**s))
    (*s)++;
}

static bool test_json_equal(const char *a, const char *b) {
  if (!a || !b)
    return false;

  const char *pa = a;
  const char *pb = b;

  if (!a && !b) {
    return false;
  }

  skip_ws(&pa);
  skip_ws(&pb);

  json_pool_reset();
  json_value *va = json_parse(pa);
  json_value *vb = json_parse(pb);

  if (!va || !vb) {
    return false;
  }

  if (va->type != vb->type) {
    json_free(va);
    json_free(vb);
    return false;
  }

  bool eq = json_equal(va, vb);

  if (!eq) {
    /* find first differing position in the original inputs (skip whitespace) */
    const char *xa = pa;
    const char *xb = pb;
    while (*xa || *xb) {
      while (isspace((unsigned char)*xa))
        xa++;
      while (isspace((unsigned char)*xb))
        xb++;
      if (*xa != *xb)
        break;
      xa++;
      xb++;
    }
    size_t off_a = (size_t)(xa - pa);
    size_t off_b = (size_t)(xb - pb);
    /* print brief context */
    size_t ctx_before = PREFIX_CHAR_OFFSET;
    size_t ctx_after = POSTFIX_CHAR_OFFSET;
    size_t start_a = (off_a > ctx_before) ? off_a - ctx_before : 0;
    size_t start_b = (off_b > ctx_before) ? off_b - ctx_before : 0;
    fprintf(stderr, "JSON mismatch: first differing byte offsets a=%zu b=%zu\n", off_a, off_b);
    fprintf(stderr, "a context: \"");
    for (size_t i = start_a; i < off_a + ctx_after && pa[i] != '\0'; ++i) {
      char c = pa[i];
      printf("a:%x\n", c);
      fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
    fprintf(stderr, "b context: \"");
    for (size_t i = start_b; i < off_b + ctx_after && pb[i] != '\0'; ++i) {
      char c = pb[i];
      printf("b:%x\n", c);
      fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
  }

  json_free((json_value *)va);
  json_free((json_value *)vb);
  return eq;
}

TEST(test_simple_json_parsing) {
  json_pool_reset();
  const char *filename = "test/test-simple.json";
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
  json_value *v = json_parse(json);
  ASSERT_PTR_NOT_NULL(v);

  /* render json_value back to string */
  char *out = json_stringify(v);
  ASSERT_PTR_NOT_NULL(out);
  json_free((json_value *)v);

  /* compare structurally (order-insensitive) */
  ASSERT(test_json_equal(json, out));

  /* cleanup */
  free(json);
  free(out);

  END_TEST;
}

TEST(test_json_parsing) {
  json_pool_reset();
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
  json_value *v = json_parse(json);
  ASSERT_PTR_NOT_NULL(v);

  /* render json_value back to string */
  char *out = json_stringify(v);
  ASSERT_PTR_NOT_NULL(out);
  json_free((json_value *)v);

  /* compare structurally (order-insensitive) */
  ASSERT(test_json_equal(json, out));

  /* cleanup */
  free(json);
  free(out);

  END_TEST;
}