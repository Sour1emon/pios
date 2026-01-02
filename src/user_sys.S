#include "user_sys.h"

.macro syscall num
mov x8, #\num
svc #0
.endm

.globl user_delay
user_delay:
	subs x0, x0, #1
	bne user_delay
	ret

.globl call_sys_write
call_sys_write:
    syscall SYS_WRITE_NUMBER
    ret


.globl call_sys_fork
call_sys_fork:
    syscall SYS_FORK_NUMBER
    ret

.globl call_sys_malloc
call_sys_malloc:
    syscall SYS_MALLOC_NUMBER
    ret

.globl call_sys_exit
call_sys_exit:
    syscall SYS_EXIT_NUMBER
    ret

.globl call_sys_getpid
call_sys_getpid:
    syscall SYS_GETPID_NUMBER
    ret

.globl call_sys_priority
call_sys_priority:
    syscall SYS_PRIORITY_NUMBER
    ret
