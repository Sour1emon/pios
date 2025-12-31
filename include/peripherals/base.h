#ifndef _P_BASE_H
#define _P_BASE_H

// https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf#page=186
// NOTE: The document says the physical address starts at 0x20000000. This is
// for the Raspberry Pi 1 and not the Raspberry Pi 3, which we are targeting.
#define PBASE 0x3F000000

#endif /*_P_BASE_H */
