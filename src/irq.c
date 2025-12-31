#include "irq.h"
#include "peripherals/irq.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"
#include <stddef.h>

const char *entry_error_messages[] = {
    "SYNC_INVALID_EL1t",   "IRQ_INVALID_EL1t",
    "FIQ_INVALID_EL1t",    "ERROR_INVALID_EL1T",

    "SYNC_INVALID_EL1h",   "IRQ_INVALID_EL1h",
    "FIQ_INVALID_EL1h",    "ERROR_INVALID_EL1h",

    "SYNC_INVALID_EL0_64", "IRQ_INVALID_EL0_64",
    "FIQ_INVALID_EL0_64",  "ERROR_INVALID_EL0_64",

    "SYNC_INVALID_EL0_32", "IRQ_INVALID_EL0_32",
    "FIQ_INVALID_EL0_32",  "ERROR_INVALID_EL0_32",

    "SYNC_ERROR",          "SYSCALL_ERROR"};

void enable_interrupt_controller() {
  put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
  put32(ENABLE_IRQS_2, UART0_IRQ);
}

void show_invalid_entry_message(int type, unsigned long esr,
                                unsigned long address) {
  // Bounds checking to prevent buffer overflow
  size_t msg_count =
      sizeof(entry_error_messages) / sizeof(entry_error_messages[0]);
  const char *msg = (type >= 0 && (size_t)type < msg_count)
                        ? entry_error_messages[type]
                        : "UNKNOWN";
  printf("%s, ESR: %lx, address: %lx\r\n", msg, esr, address);
}

void handle_irq(void) {
  unsigned int irq1 = get32(IRQ_PENDING_1);
  unsigned int irq2 = get32(IRQ_PENDING_2);
  int handled = 0;

  if (irq1 & SYSTEM_TIMER_IRQ_1) {
    handle_timer_irq();
    if (irq2 & UART0_IRQ)
      handled = 1;
  }

  if (irq2 & UART0_IRQ) {
    handle_uart_irq();
    handled = 1;
  }

  unsigned int unhandled_irq1 = irq1 & ~SYSTEM_TIMER_IRQ_1;
  unsigned int unhandled_irq2 = irq2 & ~UART0_IRQ;

  if (!handled || unhandled_irq1 || unhandled_irq2) {
    // TODO: Avoid printf here to prevent blocking in IRQ context
    if (unhandled_irq1) {
      printf("Unhandled IRQ in bank 1: 0x%x\r\n", unhandled_irq1);
    }
    if (unhandled_irq2) {
      printf("Unhandled IRQ in bank 2: 0x%x\r\n", unhandled_irq2);
    }
  }
}
