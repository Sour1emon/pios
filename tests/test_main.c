/*
 * PIOS Test Suite - Main Test Runner
 *
 * This file provides the main entry point for running all tests
 * on the actual hardware. It initializes the test framework,
 * registers all test suites, and runs them.
 *
 * Usage:
 *   Replace kernel_main() call or add a test mode to run tests
 *   on boot.
 */

#include "printf.h"
#include "test.h"
#include "timer.h"
#include "uart.h"

/* External test registration functions from each test file */
extern void register_mm_tests(void);
extern void register_sched_tests(void);
extern void register_timer_tests(void);
extern void register_uart_tests(void);
extern void register_syscall_tests(void);
extern void register_irq_tests(void);
extern void register_fork_tests(void);
extern void register_printf_tests(void);
extern void register_utils_tests(void);

/*
 * Register all test suites
 *
 * Add new test registration calls here as you create new test files.
 */
static void register_all_tests(void) {
  /* Core utilities - test these first as other tests depend on them */
  register_utils_tests();
  register_printf_tests();
  register_uart_tests();

  /* Memory management */
  register_mm_tests();

  /* Process and scheduling */
  register_sched_tests();
  register_fork_tests();

  /* Interrupts and timer */
  register_irq_tests();
  register_timer_tests();

  /* System calls */
  register_syscall_tests();
}

/*
 * Run all tests
 *
 * This is the main entry point for the test suite.
 * It initializes the framework, registers all tests, and runs them.
 */
void run_all_tests(void) {
  unsigned long start_time = time_since_boot();

  printf("\r\n");
  printf("****************************************\r\n");
  printf("*         PIOS TEST SUITE              *\r\n");
  printf("*    Running on Actual Hardware        *\r\n");
  printf("****************************************\r\n");
  printf("\r\n");

  /* Initialize the test framework */
  test_init();

  /* Register all test suites */
  register_all_tests();

  /* Run all registered tests */
  test_run_all();

  unsigned long end_time = time_since_boot();
  unsigned long elapsed_us = end_time - start_time;
  unsigned long elapsed_ms = elapsed_us / 1000;

  printf("Test execution time: %lu ms\r\n\r\n", elapsed_ms);

  /* Final status */
  if (test_get_fail_count() == 0) {
    printf("========================================\r\n");
    printf("   ALL TESTS PASSED - System OK!\r\n");
    printf("========================================\r\n");
  } else {
    printf("========================================\r\n");
    printf("   WARNING: %d TEST(S) FAILED!\r\n", test_get_fail_count());
    printf("========================================\r\n");
  }
}

/*
 * Run a specific test suite
 *
 * Useful for debugging specific components.
 */
void run_test_suite(const char *suite_name) {
  printf("\r\n");
  printf("Running test suite: %s\r\n", suite_name);
  printf("\r\n");

  /* Initialize the test framework */
  test_init();

  /* Register all test suites (we need them all registered to find the suite) */
  register_all_tests();

  /* Run only the specified suite */
  test_run_suite(suite_name);
}

/*
 * Quick smoke test
 *
 * Runs a minimal set of tests to verify basic functionality.
 * Useful for quick sanity checks during development.
 */
void run_smoke_tests(void) {
  printf("\r\n");
  printf("****************************************\r\n");
  printf("*         PIOS SMOKE TESTS             *\r\n");
  printf("****************************************\r\n");
  printf("\r\n");

  test_init();

  /* Only register essential tests */
  register_utils_tests();
  register_printf_tests();

  test_run_all();
}

/*
 * Test mode kernel main
 *
 * This can be used as an alternative kernel_main that runs
 * tests instead of the normal kernel functionality.
 *
 * To use:
 *   1. In kernel.c, add: extern void test_kernel_main(void);
 *   2. Replace kernel_main() call with test_kernel_main()
 *   Or use conditional compilation with a TEST_MODE define.
 */
void test_kernel_main(void) {
  /* UART should already be initialized by boot code */
  /* If not, uncomment the following: */
  /* uart_init(); */
  /* init_printf(NULL, uart_putc); */

  printf("\r\n");
  printf("========================================\r\n");
  printf("     PIOS Test Mode Boot\r\n");
  printf("========================================\r\n");
  printf("\r\n");

  /* Run all tests */
  run_all_tests();

  printf("\r\n");
  printf("Tests complete. System halted.\r\n");

  /* Halt the system */
  while (1) {
    /* Infinite loop - tests are done */
  }
}
