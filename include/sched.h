#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT 0 // offset of cpu_context in task_struct

#ifndef __ASSEMBLER__

#define THREAD_SIZE 4096

#define NR_TASKS 64

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS - 1]

#define TASK_RUNNING 0

extern struct task_struct *current;
extern struct task_struct *task[NR_TASKS];
extern int nr_tasks;

struct cpu_context {
  unsigned long x19;
  unsigned long x20;
  unsigned long x21;
  unsigned long x22;
  unsigned long x23;
  unsigned long x24;
  unsigned long x25;
  unsigned long x26;
  unsigned long x27;
  unsigned long x28;
  unsigned long fp;
  unsigned long sp;
  unsigned long pc;
};

struct task_struct {
  struct cpu_context cpu_context;
  long state;
  long counter;
  long priority;
  long preempt_count;
};

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct task_struct *next);
extern void cpu_switch_to(struct task_struct *prev, struct task_struct *next);

#define INIT_TASK                                                              \
  ((struct task_struct){.cpu_context = {.x19 = 0UL,                            \
                                        .x20 = 0UL,                            \
                                        .x21 = 0UL,                            \
                                        .x22 = 0UL,                            \
                                        .x23 = 0UL,                            \
                                        .x24 = 0UL,                            \
                                        .x25 = 0UL,                            \
                                        .x26 = 0UL,                            \
                                        .x27 = 0UL,                            \
                                        .x28 = 0UL,                            \
                                        .fp = 0UL,                             \
                                        .sp = 0UL,                             \
                                        .pc = 0UL},                            \
                        .state = 0L,                                           \
                        .counter = 0L,                                         \
                        .priority = 1L,                                        \
                        .preempt_count = 0L})

#endif
#endif
