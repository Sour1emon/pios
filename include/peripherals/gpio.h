#ifndef _P_GPIO_H
#define _P_GPIO_H

#include "peripherals/base.h"

// https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf#page=90

#define GPIO_BASE (PBASE + 0x00200000)

#define GPFSEL1 (GPIO_BASE + 0x4)
#define GPSET0 (GPIO_BASE + 0x1C)
#define GPCLR0 (GPIO_BASE + 0x28)
#define GPPUD (GPIO_BASE + 0x94)
#define GPPUDCLK0 (GPIO_BASE + 0x98)

#endif /*_P_GPIO_H */
