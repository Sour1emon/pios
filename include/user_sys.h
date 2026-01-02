#ifndef _USER_SYS_H
#define _USER_SYS_H

#define SYS_WRITE_NUMBER 0
#define SYS_FORK_NUMBER 1
#define SYS_EXIT_NUMBER 2
#define SYS_GETPID_NUMBER 3
#define SYS_PRIORITY_NUMBER 4

#ifndef __ASSEMBLER__

void call_sys_write(char *buf);
int call_sys_fork();
void call_sys_exit();
long call_sys_getpid();
void call_sys_priority(long priority);

extern void user_delay(unsigned long);
extern unsigned long get_sp(void);
extern unsigned long get_pc(void);

#endif

#endif /*_USER_SYS_H */
