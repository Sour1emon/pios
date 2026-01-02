#include "irq.h"
#include "arm/sysregs.h"
#include "peripherals/irq.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"
#include <stddef.h>

const char *entry_error_messages[] = {
    "SYNC_INVALID_EL1t",    "IRQ_INVALID_EL1t",   "FIQ_INVALID_EL1t",
    "ERROR_INVALID_EL1T",

    "SYNC_INVALID_EL1h",    "IRQ_INVALID_EL1h",   "FIQ_INVALID_EL1h",
    "ERROR_INVALID_EL1h",

    "SYNC_INVALID_EL0_64",  "IRQ_INVALID_EL0_64", "FIQ_INVALID_EL0_64",
    "ERROR_INVALID_EL0_64",

    "SYNC_INVALID_EL0_32",  "IRQ_INVALID_EL0_32", "FIQ_INVALID_EL0_32",
    "ERROR_INVALID_EL0_32",

    "SYNC_ERROR",           "SYSCALL_ERROR",      "DATA_ABORT_ERROR"};

void enable_interrupt_controller(void) {
  put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1);
  put32(ENABLE_IRQS_2, UART0_IRQ);
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long elr,
                                unsigned long far, unsigned long fp,
                                unsigned long lr) {
#ifndef DEBUG
  // Suppress unused parameter warnings in non-debug builds
  (void)fp;
  (void)lr;
#endif

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

  if (ec == ESR_ELx_EC_SVC64) {
    printf("  -> SVC64 (system call)\r\n");
  } else if (ec == 0x18) {
    printf("  -> trapped system register access from lower EL (MRS/MSR)\r\n");
  } else if (ec == 0x00) {
    printf("  -> Illegal/unknown instruction (likely privileged register "
           "access or unimplemented opcode)\r\n");
    if (elr) {
      unsigned int instr =
          *(unsigned int *)elr; // beware of user pointers in a real kernel
      printf("  instruction @ ELR: 0x%08x\r\n", instr);
    }
  }

  if (ec == 0x24 && fsc == 0x21)
    printf("  -> alignment fault (likely unaligned %s access)\r\n",
           wnr ? "write" : "read");

#ifdef DEBUG
  // Print stack trace (only in debug mode)
  print_stack_trace(fp, lr, elr);
#endif
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

// Add this function to walk the stack frames
void print_stack_trace(unsigned long fp, unsigned long lr, unsigned long elr) {
  printf("\r\nStack trace:\r\n");
  printf("  [0] 0x%lx (exception address)\r\n", elr);

  // Print the link register from the exception context
  if (lr != 0 && lr != elr) {
    printf("  [1] 0x%lx (link register)\r\n", lr);
  }

  int frame = 2;
  unsigned long prev_fp = 0;
  unsigned long prev_lr = lr;

  // Maximum 20 frames to prevent infinite loops
  for (int i = 0; i < 20 && fp != 0; i++) {
    // Detect loops - if FP hasn't changed, we're stuck
    if (fp == prev_fp) {
      printf("  (stack trace stopped: frame pointer loop detected)\r\n");
      break;
    }

    // Frame pointer should be reasonable (in kernel space)
    // and aligned to 16 bytes
    if (fp < 0xffff000000000000 || fp > 0xffff000001000000 || (fp & 0xf) != 0) {
      printf("  (stack trace stopped: invalid frame pointer 0x%lx)\r\n", fp);
      break;
    }

    // On ARM64, the frame record is: [FP, LR]
    // FP points to the previous FP
    // LR is at FP + 8
    unsigned long *frame_ptr = (unsigned long *)fp;
    unsigned long next_fp = frame_ptr[0];
    unsigned long saved_lr = frame_ptr[1];

    // Check if LR is valid and not a repeat
    if (saved_lr == 0 || saved_lr == prev_lr) {
      break;
    }

    // LR should be in kernel text segment
    if (saved_lr < 0xffff000000080000 || saved_lr > 0xffff000000090000) {
      printf("  (stack trace stopped: invalid return address 0x%lx)\r\n",
             saved_lr);
      break;
    }

    printf("  [%d] 0x%lx\r\n", frame, saved_lr);

    prev_fp = fp;
    prev_lr = saved_lr;
    fp = next_fp;
    frame++;
  }
  printf("\r\n");
}
