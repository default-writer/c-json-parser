#include "../src/json.h"

#define PREFIX_CHAR_OFFSET 10
#define POSTFIX_CHAR_OFFSET 10

int tests_run = 0;
int tests_passed = 0;

const char *GREEN = "";
const char *RED = "";
const char *RESET = "";

void setup_console() {
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

#define TEST(name)                                                                                                     \
  do {                                                                                                                 \
    tests_run++;                                                                                                       \
    printf("running test: %s...", #name);                                                                              \
    int passed = 1;                                                                                                    \
    do

#define END_TEST                                                                                                       \
  while (0)                                                                                                            \
    ;                                                                                                                  \
  if (passed) {                                                                                                        \
    tests_passed++;                                                                                                    \
    printf("%s  PASSED%s\n", GREEN, RESET);                                                                            \
  } else {                                                                                                             \
    printf("%s  FAILED%s\n", RED, RESET);                                                                              \
  }                                                                                                                    \
  }                                                                                                                    \
  while (0)

#define ASSERT(condition)                                                                                              \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      printf("  assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition);                                     \
      passed = 0;                                                                                                      \
    }                                                                                                                  \
  } while (0)

#define ASSERT_EQ(expected, actual)                                                                                    \
  do {                                                                                                                 \
    if ((expected) != (actual)) {                                                                                      \
      printf("  assertion failed at %s:%d: Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected),             \
             (long)(actual));                                                                                          \
      passed = 0;                                                                                                      \
    }                                                                                                                  \
  } while (0)

#define ASSERT_PTR_EQ(expected, actual)                                                                                \
  do {                                                                                                                 \
    if ((expected) != (actual)) {                                                                                      \
      printf("  assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual));          \
      passed = 0;                                                                                                      \
    }                                                                                                                  \
  } while (0)

#define ASSERT_PTR_NOT_EQ(expected, actual)                                                                            \
  do {                                                                                                                 \
    if ((expected) == (actual)) {                                                                                      \
      printf("  assertion failed at %s:%d: Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual));          \
      passed = 0;                                                                                                      \
    }                                                                                                                  \
  } while (0)

#define ASSERT_PTR_NULL(actual)                                                                                        \
  do {                                                                                                                 \
    if (NULL != (actual)) {                                                                                            \
      printf("  assertion failed at %s:%d: Expected NULL, got %p\n", __FILE__, __LINE__, (actual));                    \
      passed = 0;                                                                                                      \
    }                                                                                                                  \
  } while (0)

#define ASSERT_PTR_NOT_NULL(actual)                                                                                    \
  do {                                                                                                                 \
    if (NULL == (actual)) {                                                                                            \
      printf("  assertion failed at %s:%d: Expected non-NULL, got NULL\n", __FILE__, __LINE__);                        \
      passed = 0;                                                                                                      \
    }                                                                                                                  \
  } while (0)

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

  const json_value *va = json_parse(pa);
  const json_value *vb = json_parse(pb);

  if (!va || !vb) {
    return false;
  }

  if (va->type != vb->type) {
    json_free(va);
    json_free(vb);
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

  json_free(va);
  json_free(vb);
  return eq;
}

void test_json_parsing() {
  TEST(test_json_parsing) {

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
    const json_value *v = json_parse(json);
    ASSERT_PTR_NOT_NULL(v);
    json_print(v, stdout);

    /* render json_value back to string */
    char *out = json_stringify(v);
    ASSERT_PTR_NOT_NULL(out);

    /* compare structurally (order-insensitive) */
    ASSERT(test_json_equal(json, out));

    /* cleanup */
    free(out);
    json_free(v);
    free(json);
  }
  END_TEST;
}
