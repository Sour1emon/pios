#include "peripherals/timer.h"
#include "printf.h"
#include "sched.h"
#include "timer.h"
#include "utils.h"
#include <stdint.h>

const unsigned int interval = 200000;
static unsigned int curVal = 0;

// Return the time since boot in µs
unsigned long time_since_boot() {
  uint32_t hi1, lo, hi2;
  do {
    hi1 = get32(TIMER_CHI);
    lo = get32(TIMER_CLO);
    hi2 = get32(TIMER_CHI);
  } while (hi1 != hi2); // retry if CLO wrapped while reading

  return ((uint64_t)hi1 << 32) | lo;
}

void timer_init(void) {
  curVal = get32(TIMER_CLO) + interval;
  put32(TIMER_C1, curVal);
}

void handle_timer_irq(void) {
  unsigned int now = get32(TIMER_CLO);

  // Schedule the next tick, ensuring it's in the future
  curVal += interval;
  if ((int)(curVal - now) <= 0) {
    // curVal wrapped into the past or missed ticks
    curVal = now + interval;
  }

  put32(TIMER_C1, curVal);      // set next compare
  put32(TIMER_CS, TIMER_CS_M1); // clear interrupt flag

  timer_tick();
}
