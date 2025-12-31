#include "sys.h"
#include "fork.h"
#include "mm.h"
#include "printf.h"
#include "sched.h"

void sys_write(char *buf) { printf("%s", buf); }

int sys_clone(unsigned long stack) {
  return copy_process(0, 0, 0, stack, current->priority);
}

unsigned long sys_malloc() {
  unsigned long page_addr = get_free_page();
  if (page_addr == 0) {
    return -1;
  }
  return page_addr;
}

void sys_exit() { exit_process(); }

void *const sys_call_table[__NR_syscalls] = {sys_write, sys_clone, sys_malloc,
                                             sys_exit};
