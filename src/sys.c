#include "sys.h"
#include "fork.h"
#include "printf.h"
#include "sched.h"

void sys_write(char *buf) { printf("%s", buf); }

int sys_fork(void) { return copy_process(0, 0, 0, current->priority); }

void sys_exit() { exit_process(); }

long sys_getpid() { return current->pid; }

void sys_priority(long priority) {
  if (priority > 0) {
    current->priority = priority;
  }
}

void *const sys_call_table[__NR_syscalls] = {sys_write, sys_fork, sys_exit,
                                             sys_getpid, sys_priority};
