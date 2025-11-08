#include <stdbool.h>
#include <stdio.h>

extern void test_json_parsing(void);
extern void setup_console(void);
extern int tests_run;
extern int tests_passed;

extern const char *GREEN;
extern char *RED;
extern char *RESET;

int main(void) {

  setup_console();
  printf("running unit tests\n");
  printf("==========================================\n\n");

  test_json_parsing();

  printf("\n==========================================\n");
  printf("tests run: %d\n", tests_run);
  printf("tests passed: %d\n", tests_passed);
  if (tests_run == tests_passed) {
    printf("all tests %sPASSED%s\n", GREEN, RESET);
    return 0;
  } else {
    printf("some tests %sFAILED%s\n", RED, RESET);
    return 1;
  }

  return 0;
}