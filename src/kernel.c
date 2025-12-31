#include "irq.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"

void kernel_main(void) {
  uart_init();
  init_printf(0, uart_putc);
  irq_vector_init();
  timer_init();
  enable_interrupt_controller();
  enable_irq();

  while (1) {
  }
}
