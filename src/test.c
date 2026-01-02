#include "test.h"
#include "printf.h"

/* Test storage */
static struct test_case tests[MAX_TESTS];
static int test_count = 0;

/* Test statistics */
static int pass_count = 0;
static int fail_count = 0;
static int skip_count = 0;

/* Current test info for reporting */
static const char *current_test_name = 0;
static const char *current_suite_name = 0;

void test_init(void) {
  test_count = 0;
  pass_count = 0;
  fail_count = 0;
  skip_count = 0;
}

void test_register(const char *name, const char *suite, test_func_t func) {
  if (test_count >= MAX_TESTS) {
    printf("[TEST] ERROR: Max test count (%d) exceeded!\r\n", MAX_TESTS);
    return;
  }
  tests[test_count].name = name;
  tests[test_count].suite = suite;
  tests[test_count].func = func;
  test_count++;
}

static int str_equals(const char *a, const char *b) {
  if (a == 0 || b == 0)
    return a == b;
  while (*a && *b) {
    if (*a != *b)
      return 0;
    a++;
    b++;
  }
  return *a == *b;
}

static void run_single_test(struct test_case *tc) {
  current_test_name = tc->name;
  current_suite_name = tc->suite;

  printf("  [%s::%s] ", tc->suite, tc->name);

  int result = tc->func();

  switch (result) {
  case TEST_PASS:
    printf("PASS\r\n");
    pass_count++;
    break;
  case TEST_FAIL:
    /* Failure message already printed by assertion macro */
    fail_count++;
    break;
  case TEST_SKIP:
    printf("SKIP\r\n");
    skip_count++;
    break;
  default:
    printf("UNKNOWN RESULT (%d)\r\n", result);
    fail_count++;
    break;
  }
}

void test_run_all(void) {
  printf("\r\n");
  printf("========================================\r\n");
  printf("       PIOS TEST SUITE\r\n");
  printf("========================================\r\n");
  printf("Running %d tests...\r\n\r\n", test_count);

  const char *last_suite = 0;

  for (int i = 0; i < test_count; i++) {
    /* Print suite header when suite changes */
    if (!str_equals(tests[i].suite, last_suite)) {
      if (last_suite != 0) {
        printf("\r\n");
      }
      printf("[Suite: %s]\r\n", tests[i].suite);
      last_suite = tests[i].suite;
    }
    run_single_test(&tests[i]);
  }

  printf("\r\n");
  test_print_summary();
}

void test_run_suite(const char *suite_name) {
  printf("\r\n");
  printf("========================================\r\n");
  printf("  Running Suite: %s\r\n", suite_name);
  printf("========================================\r\n\r\n");

  int suite_tests = 0;
  for (int i = 0; i < test_count; i++) {
    if (str_equals(tests[i].suite, suite_name)) {
      run_single_test(&tests[i]);
      suite_tests++;
    }
  }

  if (suite_tests == 0) {
    printf("  No tests found in suite '%s'\r\n", suite_name);
  }

  printf("\r\n");
  test_print_summary();
}

void test_fail(const char *func, int line, const char *condition) {
  printf("FAIL\r\n");
  printf("    Assertion failed at %s:%d\r\n", func, line);
  printf("    Condition: %s\r\n", condition);
}

void test_fail_eq(const char *func, int line, const char *expected_str,
                  const char *actual_str, unsigned long expected,
                  unsigned long actual) {
  printf("FAIL\r\n");
  printf("    Assertion failed at %s:%d\r\n", func, line);
  printf("    Expected %s == %s\r\n", expected_str, actual_str);
  printf("    Expected: 0x%lx (%lu)\r\n", expected, expected);
  printf("    Actual:   0x%lx (%lu)\r\n", actual, actual);
}

void test_fail_neq(const char *func, int line, const char *not_expected_str,
                   const char *actual_str, unsigned long actual) {
  printf("FAIL\r\n");
  printf("    Assertion failed at %s:%d\r\n", func, line);
  printf("    Expected %s != %s\r\n", not_expected_str, actual_str);
  printf("    Both values: 0x%lx (%lu)\r\n", actual, actual);
}

void test_fail_cmp(const char *func, int line, const char *val_str,
                   const char *op, const char *threshold_str, unsigned long val,
                   unsigned long threshold) {
  printf("FAIL\r\n");
  printf("    Assertion failed at %s:%d\r\n", func, line);
  printf("    Expected: %s %s %s\r\n", val_str, op, threshold_str);
  printf("    Left:  0x%lx (%lu)\r\n", val, val);
  printf("    Right: 0x%lx (%lu)\r\n", threshold, threshold);
}

void test_fail_null(const char *func, int line, const char *ptr_str,
                    unsigned long ptr_val, int expected_null) {
  printf("FAIL\r\n");
  printf("    Assertion failed at %s:%d\r\n", func, line);
  if (expected_null) {
    printf("    Expected %s to be NULL\r\n", ptr_str);
    printf("    Actual: 0x%lx\r\n", ptr_val);
  } else {
    printf("    Expected %s to be non-NULL\r\n", ptr_str);
  }
}

void test_print_summary(void) {
  int total = pass_count + fail_count + skip_count;

  printf("========================================\r\n");
  printf("           TEST SUMMARY\r\n");
  printf("========================================\r\n");
  printf("  Total:  %d\r\n", total);
  printf("  Passed: %d\r\n", pass_count);
  printf("  Failed: %d\r\n", fail_count);
  printf("  Skipped: %d\r\n", skip_count);
  printf("========================================\r\n");

  if (fail_count == 0 && total > 0) {
    printf("  ALL TESTS PASSED!\r\n");
  } else if (fail_count > 0) {
    printf("  SOME TESTS FAILED!\r\n");
  }
  printf("========================================\r\n\r\n");
}

int test_get_pass_count(void) { return pass_count; }

int test_get_fail_count(void) { return fail_count; }

int test_get_skip_count(void) { return skip_count; }
