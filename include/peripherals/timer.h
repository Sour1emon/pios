#ifndef _P_TIMER_H
#define _P_TIMER_H

#include "peripherals/base.h"

// https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf#page=172
#define SYS_TIMER_BASE (PBASE + 0x3000)
#define TIMER_CS (SYS_TIMER_BASE + 0x0)
#define TIMER_CLO (SYS_TIMER_BASE + 0x4)
#define TIMER_CHI (SYS_TIMER_BASE + 0x8)
#define TIMER_C0 (SYS_TIMER_BASE + 0xC)
#define TIMER_C1 (SYS_TIMER_BASE + 0x10)
#define TIMER_C2 (SYS_TIMER_BASE + 0x14)
#define TIMER_C3 (SYS_TIMER_BASE + 0x18)

#define TIMER_CS_M0 (1 << 0)
#define TIMER_CS_M1 (1 << 1)
#define TIMER_CS_M2 (1 << 2)
#define TIMER_CS_M3 (1 << 3)

#endif
