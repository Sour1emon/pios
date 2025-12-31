#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT 0 // offset of cpu_context in task_struct
#define THREAD_FPSIMD_CONTEXT                                                  \
  (14 * 64 / 8) // 14 = 13 registers of cpu_context + 1 to point to the next
                // free position. each register (Xn) 64 bits and 8 bits/byte =>
                // 14 * 64 / 8 = offset of struct

#define PID_MAX 65535

#ifndef __ASSEMBLER__

#define THREAD_SIZE 4096

#define TASK_RUNNING 0
#define TASK_ZOMBIE 1

#define PF_KTHREAD 0x00000002

extern struct task_struct *current;
extern struct task_struct *initial_task;

// Save the FP/SIMD registers
struct fpsimd_context {
  __uint128_t vregs[32];
  unsigned int fpsr;
  unsigned int fpcr;
};

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
  struct fpsimd_context fpsimd_context;
  long state;
  long counter;
  long priority;
  long preempt_count;
  long pid;
  unsigned long stack;
  unsigned long flags;
  struct task_struct *next_task;
};

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct task_struct *next);
extern void cpu_switch_to(struct task_struct *prev, struct task_struct *next);
extern void exit_process(void);

#define INIT_TASK {{0}, {{0}, 0, 0}, 0, 0, 1, 0, 0, 0, PF_KTHREAD, 0}

#endif
#endif
