#include "../test/utils.h"

#define PREFIX_CHAR_OFFSET 10
#define POSTFIX_CHAR_OFFSET 10

#define TIME_NANOSECONDS 1000000UL
#define TIME_MILLISECONDS 1000UL
#define TIME_SECONDS 60UL
#define TIME_MINUTES 60UL
#define TIME_HOURS 24UL
#define TIME_EPSILON 1000000000UL

#define NEXT_TOKEN(s)        \
  do {                       \
    if (!json_next_token(s)) \
      return false;          \
  } while (0)

int tests_run = 0;
int tests_passed = 0;

const char *GREEN = "";
const char *RED = "";
const char *RESET = "";

void utils_initialize() {
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

long long utils_get_time(void) {
#ifdef _WIN32
  LARGE_INTEGER t, f;
  QueryPerformanceCounter(&t);
  QueryPerformanceFrequency(&f);
  return (long long)(t.QuadPart * TIME_EPSILON / f.QuadPart);
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * TIME_EPSILON + ts.tv_nsec;
#endif
}

void utils_print_time_diff(const char *test, long long start_ns, long long end_ns) {
  long long diff_ns = end_ns - start_ns;
  long long ns_per_ms = TIME_NANOSECONDS;
  long long ns_per_s = TIME_MILLISECONDS * ns_per_ms;
  long long ns_per_m = TIME_SECONDS * ns_per_s;
  long long ns_per_h = TIME_MINUTES * ns_per_m;

  long long hours = diff_ns / ns_per_h;
  diff_ns %= ns_per_h;
  long long minutes = diff_ns / ns_per_m;
  diff_ns %= ns_per_m;
  long long seconds = diff_ns / ns_per_s;
  diff_ns %= ns_per_s;
  long long milliseconds = diff_ns / ns_per_ms;

  printf("%s execution time: %02lld:%02lld:%02lld.%03lld\n", test, hours, minutes, seconds, milliseconds);
}

char *utils_get_test_json_data(const char *filename) {
  FILE *fp = fopen(filename, "r");

  if (fp == NULL) {
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (size == -1) {
    fclose(fp);
    return NULL;
  }

  char *json = calloc(1, size + 1);
  fread(json, 1, size, fp);
  json[size] = '\0';
  fclose(fp);

  return json;
}

bool utils_test_json_equal(const char *a, const char *b) {
  if (!a || !b)
    return false;

  const char *pa = a;
  const char *pb = b;

  if (!a && !b) {
    return false;
  }

  // json_pool_reset();

  json_value va;
  memset(&va, 0, sizeof(json_value));

  json_value vb;
  memset(&vb, 0, sizeof(json_value));

  json_parse(pa, &va);
  json_parse(pb, &vb);

  if (va.type != vb.type) {
    json_free(&va);
    json_free(&vb);
    return false;
  }

  bool eq = json_equal(&va, &vb);

  if (!eq) {
    /* find first differing position in the original inputs (skip whitespace) */
    const char *xa = pa;
    const char *xb = pb;
    while (*xa != '\0' && *xb != '\0') {
      NEXT_TOKEN(&xa);
      NEXT_TOKEN(&xb);
      if (*xa != *xb)
        break;
      xa++;
      xb++;
    }
    if (*xa != '\0')
      while (isspace((unsigned char)*xa))
        xa++;
    if (*xb != '\0')
      while (isspace((unsigned char)*xb))
        xb++;
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
      fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
    fprintf(stderr, "b context: \"");
    for (size_t i = start_b; i < off_b + ctx_after && pb[i] != '\0'; ++i) {
      char c = pb[i];
      fputc(c, stderr);
    }
    fprintf(stderr, "\"\n");
  }

  json_free(&va);
  json_free(&vb);
  return eq;
}
