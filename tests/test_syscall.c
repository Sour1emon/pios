/*
 * System Call Tests
 *
 * Tests for:
 * - Syscall table initialization
 * - Syscall number definitions
 * - Syscall handler functions
 * - sys_write functionality
 * - sys_fork functionality
 * - sys_getpid functionality
 * - sys_priority functionality
 */

#include "peripherals/base.h"
#include "printf.h"
#include "sched.h"
#include "sys.h"
#include "test.h"
#include "user_sys.h"

/* External syscall table */
extern void *const sys_call_table[];

/* Forward declarations for test functions */
static int test_syscall_table_exists(void);
static int test_syscall_numbers_defined(void);
static int test_syscall_nr_count(void);
static int test_syscall_table_populated(void);
static int test_syscall_sys_write_exists(void);
static int test_syscall_sys_fork_exists(void);
static int test_syscall_sys_getpid_direct(void);
static int test_syscall_sys_priority_direct(void);
static int test_syscall_getpid_returns_current_pid(void);
static int test_syscall_priority_changes_priority(void);
static int test_syscall_priority_ignores_invalid(void);
static int test_syscall_table_no_null_entries(void);

/* Test: Syscall table exists */
static int test_syscall_table_exists(void) {
  /* sys_call_table should be accessible */
  TEST_ASSERT_NOT_NULL(sys_call_table);

  return TEST_PASS;
}

/* Test: Syscall numbers are properly defined */
static int test_syscall_numbers_defined(void) {
  /* Verify syscall number definitions */
  TEST_ASSERT_EQ(0, SYS_WRITE_NUMBER);
  TEST_ASSERT_EQ(1, SYS_FORK_NUMBER);
  TEST_ASSERT_EQ(2, SYS_EXIT_NUMBER);
  TEST_ASSERT_EQ(3, SYS_GETPID_NUMBER);
  TEST_ASSERT_EQ(4, SYS_PRIORITY_NUMBER);

  return TEST_PASS;
}

/* Test: __NR_syscalls count is correct */
static int test_syscall_nr_count(void) {
  /* Should have 5 syscalls defined */
  TEST_ASSERT_EQ(5, __NR_syscalls);

  /* Syscall numbers should be less than __NR_syscalls */
  TEST_ASSERT_LT(SYS_WRITE_NUMBER, __NR_syscalls);
  TEST_ASSERT_LT(SYS_FORK_NUMBER, __NR_syscalls);
  TEST_ASSERT_LT(SYS_EXIT_NUMBER, __NR_syscalls);
  TEST_ASSERT_LT(SYS_GETPID_NUMBER, __NR_syscalls);
  TEST_ASSERT_LT(SYS_PRIORITY_NUMBER, __NR_syscalls);

  return TEST_PASS;
}

/* Test: Syscall table is populated */
static int test_syscall_table_populated(void) {
  /* All entries should be non-null */
  for (int i = 0; i < __NR_syscalls; i++) {
    TEST_ASSERT_NOT_NULL(sys_call_table[i]);
  }

  return TEST_PASS;
}

/* Test: sys_write entry exists in table */
static int test_syscall_sys_write_exists(void) {
  void *handler = sys_call_table[SYS_WRITE_NUMBER];
  TEST_ASSERT_NOT_NULL(handler);

  /* Handler should be in kernel virtual address space */
  TEST_ASSERT_GTE((unsigned long)handler, VA_START);

  return TEST_PASS;
}

/* Test: sys_fork entry exists in table */
static int test_syscall_sys_fork_exists(void) {
  void *handler = sys_call_table[SYS_FORK_NUMBER];
  TEST_ASSERT_NOT_NULL(handler);

  /* Handler should be in kernel virtual address space */
  TEST_ASSERT_GTE((unsigned long)handler, VA_START);

  return TEST_PASS;
}

/* Test: sys_getpid can be called directly */
static int test_syscall_sys_getpid_direct(void) {
  /* Call sys_getpid directly (not via syscall mechanism) */
  long pid = sys_getpid();

  /* Should return current process PID */
  TEST_ASSERT_EQ(current->pid, pid);

  return TEST_PASS;
}

/* Test: sys_priority can be called directly */
static int test_syscall_sys_priority_direct(void) {
  long original_priority = current->priority;

  /* Set a new priority */
  sys_priority(10);
  TEST_ASSERT_EQ(10, current->priority);

  /* Restore original */
  sys_priority(original_priority);
  TEST_ASSERT_EQ(original_priority, current->priority);

  return TEST_PASS;
}

/* Test: getpid returns current task's PID */
static int test_syscall_getpid_returns_current_pid(void) {
  long expected_pid = current->pid;
  long returned_pid = sys_getpid();

  TEST_ASSERT_EQ(expected_pid, returned_pid);

  return TEST_PASS;
}

/* Test: priority changes current task's priority */
static int test_syscall_priority_changes_priority(void) {
  long original = current->priority;

  /* Test various priority values */
  sys_priority(5);
  TEST_ASSERT_EQ(5, current->priority);

  sys_priority(20);
  TEST_ASSERT_EQ(20, current->priority);

  sys_priority(1);
  TEST_ASSERT_EQ(1, current->priority);

  /* Restore */
  sys_priority(original);

  return TEST_PASS;
}

/* Test: priority ignores invalid values */
static int test_syscall_priority_ignores_invalid(void) {
  long original = current->priority;

  /* Set a known value */
  sys_priority(10);
  TEST_ASSERT_EQ(10, current->priority);

  /* Try to set invalid priority (0 or negative) */
  sys_priority(0);
  /* Priority should remain unchanged */
  TEST_ASSERT_EQ(10, current->priority);

  sys_priority(-5);
  /* Priority should remain unchanged */
  TEST_ASSERT_EQ(10, current->priority);

  /* Restore */
  sys_priority(original);

  return TEST_PASS;
}

/* Test: No NULL entries in syscall table */
static int test_syscall_table_no_null_entries(void) {
  for (int i = 0; i < __NR_syscalls; i++) {
    if (sys_call_table[i] == 0) {
      printf("    NULL entry at syscall %d\r\n", i);
      return TEST_FAIL;
    }
  }

  return TEST_PASS;
}

/* Register all syscall tests */
void register_syscall_tests(void) {
  TEST_REGISTER(syscall, table_exists);
  TEST_REGISTER(syscall, numbers_defined);
  TEST_REGISTER(syscall, nr_count);
  TEST_REGISTER(syscall, table_populated);
  TEST_REGISTER(syscall, sys_write_exists);
  TEST_REGISTER(syscall, sys_fork_exists);
  TEST_REGISTER(syscall, sys_getpid_direct);
  TEST_REGISTER(syscall, sys_priority_direct);
  TEST_REGISTER(syscall, getpid_returns_current_pid);
  TEST_REGISTER(syscall, priority_changes_priority);
  TEST_REGISTER(syscall, priority_ignores_invalid);
  TEST_REGISTER(syscall, table_no_null_entries);
}
