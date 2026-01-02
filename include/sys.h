#ifndef _SYS_H
#define _SYS_H

#define __NR_syscalls 5

#ifndef __ASSEMBLER__

void sys_write(char *buf);
int sys_fork(void);
void sys_exit(void);
long sys_getpid(void);
void sys_priority(long priority);

#endif
#endif
