#include "fork.h"
#include "entry.h"
#include "mm.h"
#include "sched.h"

int copy_process(unsigned long fn, unsigned long args, long pri) {
  // We don't want to reschedule to a new task in the middle of the copy_process
  // function
  preempt_disable();
  struct task_struct *p, *previous_task;

  // Allocate a new page. The task_struct will be at the bottom of the page and
  // the rest of it will be used for the stack;
  p = (struct task_struct *)get_free_page();

  if (!p)
    return 1;

  // Initialize the task's properties
  p->priority = pri;
  p->state = TASK_RUNNING;
  p->counter = p->priority;
  p->preempt_count = 1; // disable preemtion until schedule_tail

  p->cpu_context.x19 = fn;
  p->cpu_context.x20 = args;
  p->cpu_context.pc = (unsigned long)ret_from_fork;
  p->cpu_context.sp = (unsigned long)p + THREAD_SIZE;

  p->next_task = 0;

  previous_task = initial_task;

  while (previous_task->next_task)
    previous_task = previous_task->next_task;

  previous_task->next_task = p;

  preempt_enable();
  return 0;
}
