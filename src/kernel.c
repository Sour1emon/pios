#include <stddef.h>
#include <stdint.h>

#include "fork.h"
#include "irq.h"
#include "printf.h"
#include "sched.h"
#include "timer.h"
#include "uart.h"
#include "user.h"
#include "utils.h"

/* Test mode support */
#ifdef TEST_MODE
#include "test.h"
#include "tests.h"
#endif

void kernel_process() {
  printf("Kernel process started. EL %d\r\n", get_el());
  unsigned long begin = (unsigned long)&user_begin;
  unsigned long end = (unsigned long)&user_end;
  unsigned long process = (unsigned long)&user_process;
  int err = move_to_user_mode(begin, end - begin, process - begin);
  if (err < 0) {
    printf("Error while moving process to user mode\n\r");
  }
}

void kernel_main() {
  uart_init();
  init_printf(NULL, uart_putc);
  irq_vector_init();
  timer_init();
  enable_interrupt_controller();
  enable_irq();

#ifdef TEST_MODE
  /* Run tests instead of normal kernel operation */
  run_all_tests();

  printf("\r\n");
  printf("Tests complete. System halted.\r\n");

  /* Halt the system after tests */
  while (1) {
    /* Infinite loop - tests are done */
  }
#else
  /* Normal kernel operation */
  int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0, 1);
  if (res < 0) {
    printf("error while starting kernel process");
    return;
  }

  while (1) {
    schedule();
  }
#endif
}
