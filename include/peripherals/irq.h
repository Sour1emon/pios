#ifndef _P_IRQ_H
#define _P_IRQ_H

#include "base.h"

// https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf#page=112
#define INTERRUPT_REG_BASE (PBASE + 0xB000)

#define IRQ_BASIC_PENDING (INTERRUPT_REG_BASE + 0x200)
#define IRQ_PENDING_1 (INTERRUPT_REG_BASE + 0x204)
#define IRQ_PENDING_2 (INTERRUPT_REG_BASE + 0x208)
#define FIQ_CONTROL (INTERRUPT_REG_BASE + 0x20C)
#define ENABLE_IRQS_1 (INTERRUPT_REG_BASE + 0x210)
#define ENABLE_IRQS_2 (INTERRUPT_REG_BASE + 0x214)
#define ENABLE_BASIC_IRQS (INTERRUPT_REG_BASE + 0x218)
#define DISABLE_IRQS_1 (INTERRUPT_REG_BASE + 0x21C)
#define DISABLE_IRQS_2 (INTERRUPT_REG_BASE + 0x220)
#define DISABLE_BASIC_IRQS (INTERRUPT_REG_BASE + 0x224)

// https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf#page=113

// Bits 0-3 of the first interrupt register
#define SYSTEM_TIMER_IRQ_0 (1 << 0)
#define SYSTEM_TIMER_IRQ_1 (1 << 1)
#define SYSTEM_TIMER_IRQ_2 (1 << 2)
#define SYSTEM_TIMER_IRQ_3 (1 << 3)

// Bit 57 (bit 25 of second interrupt register)
#define UART0_IRQ (1 << 25)

#endif
