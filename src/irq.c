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

void enable_interrupt_controller(void) {
  put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
  put32(ENABLE_IRQS_2, UART0_IRQ);
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long elr,
                                unsigned long far) {
  size_t msg_count =
      sizeof(entry_error_messages) / sizeof(entry_error_messages[0]);
  const char *msg = (type >= 0 && (size_t)type < msg_count)
                        ? entry_error_messages[type]
                        : "UNKNOWN";
  printf("%s, ELR: 0x%lx, FAR: 0x%lx, ESR: 0x%lx\r\n", msg, elr, far, esr);

  unsigned long ec = (esr >> 26) & 0x3f;
  unsigned long fsc = esr & 0x3f;
  unsigned long il = (esr >> 25) & 1;
  unsigned long isv = (esr >> 24) & 1;
  unsigned long wnr = (esr >> 6) & 1;
  printf("  EC=0x%02lx, FSC=0x%02lx, IL=%lu, ISV=%lu, WnR=%lu\r\n", ec, fsc, il,
         isv, wnr);

  if (ec == 0x24 && fsc == 0x21)
    printf("  -> alignment fault (likely unaligned %s access)\r\n",
           wnr ? "write" : "read");
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
