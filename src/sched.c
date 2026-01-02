#include "sched.h"
#include "irq.h"
#include "utils.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct *initial_task = &(init_task);

void preempt_disable(void) { current->preempt_count++; }

void preempt_enable(void) { current->preempt_count--; }

void _schedule(void) {
  preempt_disable();
  int c;
  struct task_struct *p, *next_task;
  while (1) {
    c = -1;
    for (p = initial_task; p; p = p->next_task) {
      if (p && p->state == TASK_RUNNING && p->counter > c) {
        c = p->counter;
        next_task = p;
      }
    }
    if (c) {
      break;
    }
    for (p = initial_task; p; p = p->next_task) {
      if (p) {
        p->counter = (p->counter >> 1) + p->priority;
      }
    }
  }
  switch_to(next_task);
  preempt_enable();
}

void schedule(void) {
  current->counter = 0;
  _schedule();
}

void switch_to(struct task_struct *next) {
  if (current == next) {
    return;
  }
  struct task_struct *prev = current;
  current = next;
  set_pgd(next->mm.pgd);
  cpu_switch_to(prev, next);
}

void schedule_tail(void) { preempt_enable(); }

void timer_tick(void) {
  --current->counter;

  if (current->counter > 0 || current->preempt_count > 0) {
    return;
  }

  current->counter = 0;
  enable_irq();
  _schedule();
  disable_irq();
}

void exit_process() {
  preempt_disable();
  current->state = TASK_ZOMBIE;
  preempt_enable();
  schedule();
}
