#ifndef _TEST_H
#define _TEST_H

/* Test result codes */
#define TEST_PASS 0
#define TEST_FAIL 1
#define TEST_SKIP 2

/* Test framework macros */
#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      test_fail(__func__, __LINE__, #cond);                                    \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_EQ(expected, actual)                                       \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      test_fail_eq(__func__, __LINE__, #expected, #actual,                     \
                   (unsigned long)(expected), (unsigned long)(actual));        \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_NEQ(not_expected, actual)                                  \
  do {                                                                         \
    if ((not_expected) == (actual)) {                                          \
      test_fail_neq(__func__, __LINE__, #not_expected, #actual,                \
                    (unsigned long)(actual));                                  \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_GT(val, threshold)                                         \
  do {                                                                         \
    if (!((val) > (threshold))) {                                              \
      test_fail_cmp(__func__, __LINE__, #val, ">", #threshold,                 \
                    (unsigned long)(val), (unsigned long)(threshold));         \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_GTE(val, threshold)                                        \
  do {                                                                         \
    if (!((val) >= (threshold))) {                                             \
      test_fail_cmp(__func__, __LINE__, #val, ">=", #threshold,                \
                    (unsigned long)(val), (unsigned long)(threshold));         \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_LT(val, threshold)                                         \
  do {                                                                         \
    if (!((val) < (threshold))) {                                              \
      test_fail_cmp(__func__, __LINE__, #val, "<", #threshold,                 \
                    (unsigned long)(val), (unsigned long)(threshold));         \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_LTE(val, threshold)                                        \
  do {                                                                         \
    if (!((val) <= (threshold))) {                                             \
      test_fail_cmp(__func__, __LINE__, #val, "<=", #threshold,                \
                    (unsigned long)(val), (unsigned long)(threshold));         \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_NULL(ptr)                                                  \
  do {                                                                         \
    if ((ptr) != 0) {                                                          \
      test_fail_null(__func__, __LINE__, #ptr, (unsigned long)(ptr), 1);       \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_NOT_NULL(ptr)                                              \
  do {                                                                         \
    if ((ptr) == 0) {                                                          \
      test_fail_null(__func__, __LINE__, #ptr, 0, 0);                          \
      return TEST_FAIL;                                                        \
    }                                                                          \
  } while (0)

/* Test registration and execution */
typedef int (*test_func_t)(void);

struct test_case {
  const char *name;
  const char *suite;
  test_func_t func;
};

/* Maximum number of tests that can be registered */
#define MAX_TESTS 128

/* Test suite structure for grouping tests */
struct test_suite {
  const char *name;
  void (*setup)(void);
  void (*teardown)(void);
};

/* Test framework functions */
void test_init(void);
void test_register(const char *name, const char *suite, test_func_t func);
void test_run_all(void);
void test_run_suite(const char *suite_name);

/* Test result reporting */
void test_fail(const char *func, int line, const char *condition);
void test_fail_eq(const char *func, int line, const char *expected_str,
                  const char *actual_str, unsigned long expected,
                  unsigned long actual);
void test_fail_neq(const char *func, int line, const char *not_expected_str,
                   const char *actual_str, unsigned long actual);
void test_fail_cmp(const char *func, int line, const char *val_str,
                   const char *op, const char *threshold_str, unsigned long val,
                   unsigned long threshold);
void test_fail_null(const char *func, int line, const char *ptr_str,
                    unsigned long ptr_val, int expected_null);

/* Test summary */
void test_print_summary(void);
int test_get_pass_count(void);
int test_get_fail_count(void);
int test_get_skip_count(void);

/* Convenience macro to define and register a test */
#define DEFINE_TEST(suite, name)                                               \
  static int test_##suite##_##name(void);                                      \
  __attribute__((constructor)) static void register_##suite##_##name(void) {   \
    test_register(#name, #suite, test_##suite##_##name);                       \
  }                                                                            \
  static int test_##suite##_##name(void)

/* Alternative registration for bare metal (no constructor support) */
#define TEST_FUNC(suite, name) test_##suite##_##name
#define TEST_REGISTER(suite, name)                                             \
  test_register(#name, #suite, test_##suite##_##name)

#endif /* _TEST_H */
