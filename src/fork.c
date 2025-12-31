#include "fork.h"
#include "entry.h"
#include "mm.h"
#include "printf.h"
#include "sched.h"

int copy_process(unsigned long clone_flags, unsigned long fn,
                 unsigned long args, unsigned long stack, long pri) {
  // We don't want to reschedule to a new task in the middle of the copy_process
  // function
  preempt_disable();
  struct task_struct *p, *previous_task;

  // Allocate a new page. The task_struct will be at the bottom of the page and
  // the rest of it will be used for the stack;
  p = (struct task_struct *)get_free_page();

  if (!p)
    return -1;

  struct pt_regs *childregs = task_pt_regs(p);
  memzero((unsigned long)childregs, sizeof(struct pt_regs));
  memzero((unsigned long)&p->cpu_context, sizeof(struct cpu_context));

  if (clone_flags & PF_KTHREAD) {
    p->cpu_context.x19 = fn;
    p->cpu_context.x20 = args;
  } else {
    struct pt_regs *cur_regs = task_pt_regs(current);
    *childregs = *cur_regs;
    childregs->regs[0] = 0;
    childregs->sp = stack + PAGE_SIZE;
    p->stack = stack;
  }

  p->flags = clone_flags;
  p->priority = pri;
  p->state = TASK_RUNNING;
  p->counter = p->priority;
  p->preempt_count = 1; // disable preemtion until schedule_tail

  p->cpu_context.pc = (unsigned long)ret_from_fork;
  p->cpu_context.sp = (unsigned long)childregs;

  p->next_task = 0;

  previous_task = initial_task;

  while (previous_task->next_task)
    previous_task = previous_task->next_task;

  previous_task->next_task = p;

  tfp_printf("%d", (int)(unsigned long)p);

  preempt_enable();
  return (int)(unsigned long)p;
}

int move_to_user_mode(unsigned long pc) {
  struct pt_regs *regs = task_pt_regs(current);
  memzero((unsigned long)regs, sizeof(*regs));
  regs->pc = pc;
  regs->pstate = PSR_MODE_EL0t;
  unsigned long stack =
      get_free_page(); // allocate some memory for the user stack
  if (!stack) {
    return -1;
  }
  regs->sp = stack + PAGE_SIZE;
  current->stack = stack;
  return 0;
}

struct pt_regs *task_pt_regs(struct task_struct *tsk) {
  unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}
