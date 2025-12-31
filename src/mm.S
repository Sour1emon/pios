.globl memcpy
memcpy:
    mov    x9, x0           // save original dest pointer for return
    cbz    x2, memcpy_done  // if length == 0, done

    lsr    x3, x2, #3       // x3 = n / 8 (number of 8-byte blocks)
    cbz    x3, memcpy_tail  // if no 8-byte blocks, go to tail

memcpy_loop:
    ldr    x4, [x1], #8
    str    x4, [x0], #8
    subs   x3, x3, #1
    b.ne   memcpy_loop

memcpy_tail:
    and    x2, x2, #7       // x2 = n % 8 (remaining bytes)
    cbz    x2, memcpy_done

memcpy_byte_loop:
    ldrb   w4, [x1], #1
    strb   w4, [x0], #1
    subs   x2, x2, #1
    b.ne   memcpy_byte_loop

memcpy_done:
    mov    x0, x9           // return original dest
    ret


.globl memzero
memzero:
    cbz x1, memzero_done        // if n == 0 return
memzero_loop:
    cmp x1, #8
    blt memzero_tail
    str xzr, [x0], #8
    sub x1, x1, #8
    b memzero_loop
memzero_tail:
    cbz x1, memzero_done
memzero_tail_loop:
    strb wzr, [x0], #1
    sub x1, x1, #1
    b.gt memzero_tail_loop
memzero_done:
    ret
