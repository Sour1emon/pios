/*
 * Fork and Process Creation Tests
 *
 * Tests for:
 * - Process creation with copy_process
 * - Kernel thread creation
 * - User mode transition
 * - pt_regs structure
 * - Task stack setup
 * - Process flags
 * - Child process initialization
 */

#include "entry.h"
#include "fork.h"
#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "test.h"

/* Forward declarations for test functions */
static int test_fork_copy_process_returns_pid(void);
static int test_fork_kthread_flag(void);
static int test_fork_task_pt_regs_location(void);
static int test_fork_pt_regs_size(void);
static int test_fork_child_state_running(void);
static int test_fork_child_preempt_disabled(void);
static int test_fork_child_in_task_list(void);
static int test_fork_child_has_stack(void);
static int test_fork_psr_mode_constants(void);
static int test_fork_multiple_processes(void);
static int test_fork_child_priority(void);
static int test_fork_child_counter(void);
static int test_fork_cpu_context_setup(void);
static int test_fork_different_pids(void);
static int test_fork_task_list_grows(void);

/* Dummy function for kernel thread testing - must call exit_process() */
static void test_kernel_func(void) {
  exit_process(); /* Properly terminate instead of returning */
}

/* Helper to count tasks in task list */
static int count_tasks(void) {
  int count = 0;
  struct task_struct *p = initial_task;
  while (p && count < 1000) { /* Limit to prevent infinite loop */
    count++;
    p = p->next_task;
  }
  return count;
}

/* Test: copy_process returns a valid PID */
static int test_fork_copy_process_returns_pid(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);

  TEST_ASSERT_GTE(pid, 0);
  TEST_ASSERT_LTE(pid, PID_MAX);

  return TEST_PASS;
}

/* Test: Kernel thread has PF_KTHREAD flag set */
static int test_fork_kthread_flag(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
  TEST_ASSERT_GTE(pid, 0);

  /* Find the task */
  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);
  TEST_ASSERT_EQ(PF_KTHREAD, p->flags);

  return TEST_PASS;
}

/* Test: task_pt_regs returns correct location */
static int test_fork_task_pt_regs_location(void) {
  /* pt_regs should be at the top of the task's stack page */
  struct pt_regs *regs = task_pt_regs(current);

  TEST_ASSERT_NOT_NULL(regs);

  /* Calculate expected location */
  unsigned long task_base = (unsigned long)current;
  unsigned long expected = task_base + THREAD_SIZE - sizeof(struct pt_regs);

  TEST_ASSERT_EQ(expected, (unsigned long)regs);

  return TEST_PASS;
}

/* Test: pt_regs structure size */
static int test_fork_pt_regs_size(void) {
  /* pt_regs should contain:
   * - 31 general purpose registers (regs[31])
   * - sp
   * - pc
   * - pstate
   * Total: 34 unsigned longs = 34 * 8 = 272 bytes */
  unsigned long expected_size = 34 * sizeof(unsigned long);

  TEST_ASSERT_EQ(expected_size, sizeof(struct pt_regs));

  return TEST_PASS;
}

/* Test: Child process starts in TASK_RUNNING state */
static int test_fork_child_state_running(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
  TEST_ASSERT_GTE(pid, 0);

  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);
  TEST_ASSERT_EQ(TASK_RUNNING, p->state);

  return TEST_PASS;
}

/* Test: Child process has preemption disabled initially */
static int test_fork_child_preempt_disabled(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
  TEST_ASSERT_GTE(pid, 0);

  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);
  /* Preempt count should be 1 until schedule_tail is called */
  TEST_ASSERT_EQ(1, p->preempt_count);

  return TEST_PASS;
}

/* Test: Child is added to task list */
static int test_fork_child_in_task_list(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
  TEST_ASSERT_GTE(pid, 0);

  /* Search for the child in task list */
  struct task_struct *p = initial_task;
  int found = 0;
  int count = 0;

  while (p && count < 1000) {
    if (p->pid == pid) {
      found = 1;
      break;
    }
    p = p->next_task;
    count++;
  }

  TEST_ASSERT(found);

  return TEST_PASS;
}

/* Test: Child has valid stack pointer */
static int test_fork_child_has_stack(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
  TEST_ASSERT_GTE(pid, 0);

  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);

  /* Stack pointer should be set (non-zero) */
  TEST_ASSERT_NEQ(0, p->cpu_context.sp);

  /* Stack should be within the task's page */
  unsigned long task_base = (unsigned long)p;
  unsigned long task_end = task_base + THREAD_SIZE;

  TEST_ASSERT_GTE(p->cpu_context.sp, task_base);
  TEST_ASSERT_LTE(p->cpu_context.sp, task_end);

  return TEST_PASS;
}

/* Test: PSR mode constants are correct */
static int test_fork_psr_mode_constants(void) {
  /* Verify PSR mode constants for ARM64 */
  TEST_ASSERT_EQ(0x00000000, PSR_MODE_EL0t);
  TEST_ASSERT_EQ(0x00000004, PSR_MODE_EL1t);
  TEST_ASSERT_EQ(0x00000005, PSR_MODE_EL1h);

  return TEST_PASS;
}

/* Test: Multiple processes can be created */
static int test_fork_multiple_processes(void) {
  int pids[5];

  for (int i = 0; i < 5; i++) {
    pids[i] =
        copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, i + 1);
    TEST_ASSERT_GTE(pids[i], 0);
  }

  /* All PIDs should be different */
  for (int i = 0; i < 5; i++) {
    for (int j = i + 1; j < 5; j++) {
      TEST_ASSERT_NEQ(pids[i], pids[j]);
    }
  }

  return TEST_PASS;
}

/* Test: Child inherits priority from argument */
static int test_fork_child_priority(void) {
  int test_priority = 7;
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0,
                         test_priority);
  TEST_ASSERT_GTE(pid, 0);

  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);
  TEST_ASSERT_EQ(test_priority, p->priority);

  return TEST_PASS;
}

/* Test: Child counter initialized from priority */
static int test_fork_child_counter(void) {
  int test_priority = 8;
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0,
                         test_priority);
  TEST_ASSERT_GTE(pid, 0);

  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);
  /* Counter should be initialized to priority */
  TEST_ASSERT_EQ(test_priority, p->counter);

  return TEST_PASS;
}

/* Test: CPU context is properly set up for kernel thread */
static int test_fork_cpu_context_setup(void) {
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 42, 5);
  TEST_ASSERT_GTE(pid, 0);

  struct task_struct *p = initial_task;
  while (p && p->pid != pid) {
    p = p->next_task;
  }

  TEST_ASSERT_NOT_NULL(p);

  /* For kernel threads, x19 holds the function pointer */
  TEST_ASSERT_EQ((unsigned long)&test_kernel_func, p->cpu_context.x19);

  /* x20 holds the argument */
  TEST_ASSERT_EQ(42, p->cpu_context.x20);

  /* PC should point to ret_from_fork */
  extern void ret_from_fork(void);
  TEST_ASSERT_EQ((unsigned long)ret_from_fork, p->cpu_context.pc);

  return TEST_PASS;
}

/* Test: Each new process gets a different PID */
static int test_fork_different_pids(void) {
  int prev_pid = -1;

  for (int i = 0; i < 10; i++) {
    int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
    TEST_ASSERT_GTE(pid, 0);
    TEST_ASSERT_NEQ(prev_pid, pid);
    prev_pid = pid;
  }

  return TEST_PASS;
}

/* Test: Task list grows with each new process */
static int test_fork_task_list_grows(void) {
  int initial_count = count_tasks();

  /* Create a new process */
  int pid = copy_process(PF_KTHREAD, (unsigned long)&test_kernel_func, 0, 5);
  TEST_ASSERT_GTE(pid, 0);

  int new_count = count_tasks();

  /* Should have one more task */
  TEST_ASSERT_EQ(initial_count + 1, new_count);

  return TEST_PASS;
}

/* Register all fork tests */
void register_fork_tests(void) {
  TEST_REGISTER(fork, copy_process_returns_pid);
  TEST_REGISTER(fork, kthread_flag);
  TEST_REGISTER(fork, task_pt_regs_location);
  TEST_REGISTER(fork, pt_regs_size);
  TEST_REGISTER(fork, child_state_running);
  TEST_REGISTER(fork, child_preempt_disabled);
  TEST_REGISTER(fork, child_in_task_list);
  TEST_REGISTER(fork, child_has_stack);
  TEST_REGISTER(fork, psr_mode_constants);
  TEST_REGISTER(fork, multiple_processes);
  TEST_REGISTER(fork, child_priority);
  TEST_REGISTER(fork, child_counter);
  TEST_REGISTER(fork, cpu_context_setup);
  TEST_REGISTER(fork, different_pids);
  TEST_REGISTER(fork, task_list_grows);
}
