#include "fork.h"
#include "entry.h"
#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "utils.h"
#include <limits.h>

#define ULONG_BITS (sizeof(unsigned long) * 8)
#define PID_BITMAP_LENGTH CONST_DIV_CEIL(PID_MAX, ULONG_BITS)

// Bitmap where each long = 64 pids (assuming 64 bit cpu)
// The first value will be one and the rest will be zero. This is to account for
// the init_task which has a pid of 0
static unsigned long pid_bitmap[PID_BITMAP_LENGTH] = {1};

long alloc_pid(void) {
  for (unsigned long i = 0; i < PID_BITMAP_LENGTH; i++) {
    unsigned long part = pid_bitmap[i];
    if (part == ULONG_MAX)
      continue;
    unsigned long zero_idx = __builtin_ctzl(~part);
    unsigned long pid = i * ULONG_BITS + zero_idx;
    if (pid > PID_MAX)
      continue; // validate bounds
    pid_bitmap[i] |= (1UL << zero_idx);
    return (long)pid;
  }
  return -1;
}

void free_pid(long pid) {
  if (pid < 0)
    return;
  unsigned long idx = (unsigned long)pid / ULONG_BITS;
  unsigned long bit = 1UL << ((unsigned long)pid % ULONG_BITS);
  pid_bitmap[idx] &= ~bit;
}

int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg,
                 long pri) {
  preempt_disable();
  struct task_struct *p, *previous_task;

  long pid = alloc_pid();
  if (pid == -1) {
    // No more pids left
    // TODO: Debug print that it couldn't find an available PID
    preempt_enable();
    return -1;
  }

  unsigned long page = allocate_kernel_page();
  p = (struct task_struct *)page;
  struct pt_regs *childregs = task_pt_regs(p);

  if (!p)
    return -1;

  if (clone_flags & PF_KTHREAD) {
    p->cpu_context.x19 = fn;
    p->cpu_context.x20 = arg;
  } else {
    struct pt_regs *cur_regs = task_pt_regs(current);
    *childregs = *cur_regs;
    childregs->regs[0] = 0;
    copy_virt_memory(p);
  }
  p->flags = clone_flags;
  p->priority = pri;
  p->state = TASK_RUNNING;
  p->counter = p->priority;
  p->preempt_count = 1; // disable preemtion until schedule_tail
  p->pid = pid;

  p->cpu_context.pc = (unsigned long)ret_from_fork;
  p->cpu_context.sp = (unsigned long)childregs;

  p->next_task = 0;

  previous_task = initial_task;

  while (previous_task->next_task)
    previous_task = previous_task->next_task;

  previous_task->next_task = p;

  preempt_enable();
  return pid;
}

int move_to_user_mode(unsigned long start, unsigned long size,
                      unsigned long pc) {
  struct pt_regs *regs = task_pt_regs(current);
  regs->pstate = PSR_MODE_EL0t;
  regs->pc = pc;
  regs->sp = 2 * PAGE_SIZE;
  unsigned long code_page = allocate_user_page(current, 0);
  if (code_page == 0) {
    return -1;
  }
  memcpy(code_page, start, size);
  set_pgd(current->mm.pgd);
  return 0;
}

struct pt_regs *task_pt_regs(struct task_struct *tsk) {
  unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}
