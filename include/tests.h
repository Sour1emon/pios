#ifndef _TESTS_H
#define _TESTS_H

/*
 * PIOS Test Suite Header
 *
 * This header provides declarations for the test framework
 * and all test suite registration functions.
 */

/* Main test runner functions */
void run_all_tests(void);
void run_test_suite(const char *suite_name);
void run_smoke_tests(void);
void test_kernel_main(void);

/* Test suite registration functions */
void register_mm_tests(void);
void register_sched_tests(void);
void register_timer_tests(void);
void register_uart_tests(void);
void register_syscall_tests(void);
void register_irq_tests(void);
void register_fork_tests(void);
void register_printf_tests(void);
void register_utils_tests(void);

#endif /* _TESTS_H */
