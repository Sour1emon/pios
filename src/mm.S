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
