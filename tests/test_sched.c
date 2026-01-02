/*
 * Scheduler and Process Tests
 *
 * Tests for:
 * - Task structure initialization
 * - PID allocation and deallocation
 * - Process creation (fork/copy_process)
 * - Preemption enable/disable
 * - Process state transitions
 * - Task list management
 * - Priority handling
 * - Counter management
 */

#include "fork.h"
#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "test.h"
#include <limits.h>

/* External declarations for testing internal functions */
extern long alloc_pid(void);
extern void free_pid(long pid);

/* Forward declarations for test functions */
static int test_sched_init_task_state(void);
static int test_sched_current_exists(void);
static int test_sched_initial_task_exists(void);
static int test_sched_preempt_disable(void);
static int test_sched_preempt_enable(void);
static int test_sched_preempt_nesting(void);
static int test_sched_pid_alloc(void);
static int test_sched_pid_alloc_multiple(void);
static int test_sched_pid_free(void);
static int test_sched_pid_reuse(void);
static int test_sched_copy_process_kthread(void);
static int test_sched_task_struct_size(void);
static int test_sched_task_state_running(void);
static int test_sched_counter_decrement(void);
static int test_sched_priority_assignment(void);
static int test_sched_task_list_traversal(void);
static int test_sched_cpu_context_offset(void);
static int test_sched_fpsimd_context_offset(void);

/* Dummy kernel function for testing */
static void dummy_kernel_func(void) {
  exit_process(); /* Properly terminate instead of returning */
}

/* Test: Initial task state is correct */
static int test_sched_init_task_state(void) {
  /* The init task should be in running state */
  TEST_ASSERT_EQ(TASK_RUNNING, initial_task->state);

  /* Init task should have PID 0 */
  TEST_ASSERT_EQ(0, initial_task->pid);

  /* Init task should be a kernel thread */
  TEST_ASSERT_EQ(PF_KTHREAD, initial_task->flags);

  return TEST_PASS;
}

/* Test: Current task pointer exists */
static int test_sched_current_exists(void) {
  TEST_ASSERT_NOT_NULL(current);

  /* Current should point to a valid task */
  TEST_ASSERT_GTE((unsigned long)current, VA_START);

  return TEST_PASS;
}

/* Test: Initial task pointer exists */
static int test_sched_initial_task_exists(void) {
  TEST_ASSERT_NOT_NULL(initial_task);

  /* Initial task should be valid */
  TEST_ASSERT_GTE((unsigned long)initial_task, VA_START);

  return TEST_PASS;
}

/* Test: Preempt disable increases count */
static int test_sched_preempt_disable(void) {
  long initial_count = current->preempt_count;

  preempt_disable();

  TEST_ASSERT_EQ(initial_count + 1, current->preempt_count);

  /* Clean up - restore count */
  preempt_enable();

  return TEST_PASS;
}

/* Test: Preempt enable decreases count */
static int test_sched_preempt_enable(void) {
  /* First disable to ensure we can safely enable */
  preempt_disable();
  long count_after_disable = current->preempt_count;

  preempt_enable();

  TEST_ASSERT_EQ(count_after_disable - 1, current->preempt_count);

  return TEST_PASS;
}

/* Test: Preempt nesting works correctly */
static int test_sched_preempt_nesting(void) {
  long initial_count = current->preempt_count;

  preempt_disable();
  TEST_ASSERT_EQ(initial_count + 1, current->preempt_count);

  preempt_disable();
  TEST_ASSERT_EQ(initial_count + 2, current->preempt_count);

  preempt_disable();
  TEST_ASSERT_EQ(initial_count + 3, current->preempt_count);

  preempt_enable();
  TEST_ASSERT_EQ(initial_count + 2, current->preempt_count);

  preempt_enable();
  TEST_ASSERT_EQ(initial_count + 1, current->preempt_count);

  preempt_enable();
  TEST_ASSERT_EQ(initial_count, current->preempt_count);

  return TEST_PASS;
}

/* Test: PID allocation returns valid PID */
static int test_sched_pid_alloc(void) {
  long pid = alloc_pid();

  /* PID should be non-negative and within bounds */
  TEST_ASSERT_GTE(pid, 0);
  TEST_ASSERT_LTE(pid, PID_MAX);

  /* Clean up */
  free_pid(pid);

  return TEST_PASS;
}

/* Test: Multiple PID allocations return different PIDs */
static int test_sched_pid_alloc_multiple(void) {
  long pid1 = alloc_pid();
  long pid2 = alloc_pid();
  long pid3 = alloc_pid();

  TEST_ASSERT_GTE(pid1, 0);
  TEST_ASSERT_GTE(pid2, 0);
  TEST_ASSERT_GTE(pid3, 0);

  /* All PIDs should be different */
  TEST_ASSERT_NEQ(pid1, pid2);
  TEST_ASSERT_NEQ(pid2, pid3);
  TEST_ASSERT_NEQ(pid1, pid3);

  /* Clean up */
  free_pid(pid1);
  free_pid(pid2);
  free_pid(pid3);

  return TEST_PASS;
}

/* Test: PID free works */
static int test_sched_pid_free(void) {
  long pid = alloc_pid();
  TEST_ASSERT_GTE(pid, 0);

  /* Free should not crash */
  free_pid(pid);

  /* Freeing invalid PIDs should be safe */
  free_pid(-1);

  return TEST_PASS;
}

/* Test: PID reuse after free */
static int test_sched_pid_reuse(void) {
  /* Allocate several PIDs */
  long pids[5];
  for (int i = 0; i < 5; i++) {
    pids[i] = alloc_pid();
    TEST_ASSERT_GTE(pids[i], 0);
  }

  /* Free them all */
  for (int i = 0; i < 5; i++) {
    free_pid(pids[i]);
  }

  /* Allocate again - should succeed */
  long new_pids[5];
  for (int i = 0; i < 5; i++) {
    new_pids[i] = alloc_pid();
    TEST_ASSERT_GTE(new_pids[i], 0);
  }

  /* Clean up */
  for (int i = 0; i < 5; i++) {
    free_pid(new_pids[i]);
  }

  return TEST_PASS;
}

/* Test: copy_process creates kernel thread */
static int test_sched_copy_process_kthread(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&dummy_kernel_func, 0, 5);

  /* Should return valid PID */
  TEST_ASSERT_GTE(pid, 0);

  /* Find the new task in the task list */
  struct task_struct *p = initial_task;
  struct task_struct *new_task = 0;
  while (p) {
    if (p->pid == pid) {
      new_task = p;
      break;
    }
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(new_task);
  TEST_ASSERT_EQ(pid, new_task->pid);
  TEST_ASSERT_EQ(TASK_RUNNING, new_task->state);
  TEST_ASSERT_EQ(PF_KTHREAD, new_task->flags);
  TEST_ASSERT_EQ(5, new_task->priority);
  TEST_ASSERT_EQ(1, new_task->preempt_count); /* Preemption disabled until
                                                 schedule_tail */

  return TEST_PASS;
}

/* Test: Task struct fits in a page */
static int test_sched_task_struct_size(void) {
  /* Task struct + pt_regs must fit in THREAD_SIZE */
  unsigned long task_size = sizeof(struct task_struct);
  unsigned long pt_regs_size = sizeof(struct pt_regs);

  TEST_ASSERT_LT(task_size + pt_regs_size, THREAD_SIZE);

  return TEST_PASS;
}

/* Test: Task state constants */
static int test_sched_task_state_running(void) {
  TEST_ASSERT_EQ(0, TASK_RUNNING);
  TEST_ASSERT_EQ(1, TASK_ZOMBIE);

  return TEST_PASS;
}

/* Test: Counter can be modified */
static int test_sched_counter_decrement(void) {
  long original = current->counter;

  current->counter = 100;
  TEST_ASSERT_EQ(100, current->counter);

  current->counter--;
  TEST_ASSERT_EQ(99, current->counter);

  /* Restore */
  current->counter = original;

  return TEST_PASS;
}

/* Test: Priority assignment */
static int test_sched_priority_assignment(void) {
  long original = current->priority;

  current->priority = 10;
  TEST_ASSERT_EQ(10, current->priority);

  current->priority = 1;
  TEST_ASSERT_EQ(1, current->priority);

  current->priority = 100;
  TEST_ASSERT_EQ(100, current->priority);

  /* Restore */
  current->priority = original;

  return TEST_PASS;
}

/* Test: Task list traversal */
static int test_sched_task_list_traversal(void) {
  /* Should be able to traverse from initial_task */
  struct task_struct *p = initial_task;
  int count = 0;

  while (p && count < 100) { /* Limit to prevent infinite loop */
    count++;
    p = p->next_task;
  }

  /* Should have at least one task (initial_task) */
  TEST_ASSERT_GTE(count, 1);

  /* Should not have hit infinite loop limit */
  TEST_ASSERT_LT(count, 100);

  return TEST_PASS;
}

/* Test: CPU context offset is at start of task struct */
static int test_sched_cpu_context_offset(void) {
  /* THREAD_CPU_CONTEXT should be 0 (cpu_context is first member) */
  TEST_ASSERT_EQ(0, THREAD_CPU_CONTEXT);

  /* Verify by checking actual offset */
  struct task_struct test_task;
  unsigned long offset =
      (unsigned long)&test_task.cpu_context - (unsigned long)&test_task;
  TEST_ASSERT_EQ(0, offset);

  return TEST_PASS;
}

/* Test: FPSIMD context offset calculation */
static int test_sched_fpsimd_context_offset(void) {
  /* The THREAD_FPSIMD_CONTEXT macro calculates: 14 * 64 / 8 = 112 bytes */
  /* This accounts for 13 registers in cpu_context + padding for alignment */
  unsigned long expected_offset = THREAD_FPSIMD_CONTEXT;

  struct task_struct test_task;
  unsigned long actual_offset =
      (unsigned long)&test_task.fpsimd_context - (unsigned long)&test_task;

  /* Verify struct layout matches the assembly constant */
  TEST_ASSERT_EQ(expected_offset, actual_offset);

  return TEST_PASS;
}

/* Register all scheduler tests */
void register_sched_tests(void) {
  TEST_REGISTER(sched, init_task_state);
  TEST_REGISTER(sched, current_exists);
  TEST_REGISTER(sched, initial_task_exists);
  TEST_REGISTER(sched, preempt_disable);
  TEST_REGISTER(sched, preempt_enable);
  TEST_REGISTER(sched, preempt_nesting);
  TEST_REGISTER(sched, pid_alloc);
  TEST_REGISTER(sched, pid_alloc_multiple);
  TEST_REGISTER(sched, pid_free);
  TEST_REGISTER(sched, pid_reuse);
  TEST_REGISTER(sched, copy_process_kthread);
  TEST_REGISTER(sched, task_struct_size);
  TEST_REGISTER(sched, task_state_running);
  TEST_REGISTER(sched, counter_decrement);
  TEST_REGISTER(sched, priority_assignment);
  TEST_REGISTER(sched, task_list_traversal);
  TEST_REGISTER(sched, cpu_context_offset);
  TEST_REGISTER(sched, fpsimd_context_offset);
}
