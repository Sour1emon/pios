#include "sys.h"

.macro syscall num
mov x8, #\num
svc #0
.endm

.globl call_sys_write
call_sys_write:
    syscall SYS_WRITE_NUMBER
    ret

.globl call_sys_clone
call_sys_clone:
    /* Save args for the child.  */
    mov    x10, x0                    /*fn*/
    mov    x11, x1                    /*arg*/
    mov    x12, x2                    /*stack*/

    /* Do the system call.  */
    mov    x0, x2                     /* stack  */
    syscall SYS_CLONE_NUMBER

    cmp x0, #0
    beq thread_start
    ret

thread_start:
    mov x29, #0

    /* Pick the function arg and execute.  */
    mov x0, x11
    blr x10

    /* We are done, pass the return value through x0.  */
    syscall SYS_EXIT_NUMBER

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
