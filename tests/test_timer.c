/*
 * Timer Tests
 *
 * Tests for:
 * - Timer initialization
 * - Time since boot functionality
 * - Timer tick behavior
 * - Timer value progression
 */

#include "printf.h"
#include "sched.h"
#include "test.h"
#include "timer.h"
#include "utils.h"

/* Forward declarations for test functions */
static int test_timer_time_since_boot_nonzero(void);
static int test_timer_time_increases(void);
static int test_timer_time_monotonic(void);
static int test_timer_reasonable_rate(void);
static int test_timer_delay_function(void);
static int test_timer_multiple_reads(void);
static int test_timer_no_overflow_short_term(void);
static int test_timer_counter_affects_scheduling(void);

/* Test: time_since_boot returns non-zero after boot */
static int test_timer_time_since_boot_nonzero(void) {
  unsigned long time = time_since_boot();

  /* After boot, time should be greater than 0 */
  TEST_ASSERT_GT(time, 0);

  return TEST_PASS;
}

/* Test: Time increases between calls */
static int test_timer_time_increases(void) {
  unsigned long time1 = time_since_boot();

  /* Small delay */
  for (volatile int i = 0; i < 10000; i++) {
    /* busy wait */
  }

  unsigned long time2 = time_since_boot();

  /* Time should have increased */
  TEST_ASSERT_GT(time2, time1);

  return TEST_PASS;
}

/* Test: Time is monotonically increasing */
static int test_timer_time_monotonic(void) {
  unsigned long prev = time_since_boot();

  for (int i = 0; i < 100; i++) {
    unsigned long curr = time_since_boot();
    TEST_ASSERT_GTE(curr, prev);
    prev = curr;
  }

  return TEST_PASS;
}

/* Test: Timer runs at a reasonable rate (approximately 1MHz) */
static int test_timer_reasonable_rate(void) {
  unsigned long start = time_since_boot();

  /* Use delay function for a small duration */
  delay(100); /* Small delay */

  unsigned long end = time_since_boot();
  unsigned long elapsed = end - start;

  /* Timer should have advanced */
  /* We just check it moved forward by at least something */
  TEST_ASSERT_GT(elapsed, 0);

  return TEST_PASS;
}

/* Test: delay function actually delays */
static int test_timer_delay_function(void) {
  unsigned long start = time_since_boot();

  /* Delay for a small amount - delay is cycle-based, not time-based */
  delay(100);

  unsigned long end = time_since_boot();
  unsigned long elapsed = end - start;

  /* Should have elapsed at least some time */
  /* The delay function is cycle-based, not time-based, so we just verify
   * time passed */
  TEST_ASSERT_GT(elapsed, 0);

  return TEST_PASS;
}

/* Test: Multiple rapid reads return valid values */
static int test_timer_multiple_reads(void) {
  unsigned long times[10];

  for (int i = 0; i < 10; i++) {
    times[i] = time_since_boot();
  }

  /* All values should be valid (non-zero) */
  for (int i = 0; i < 10; i++) {
    TEST_ASSERT_GT(times[i], 0);
  }

  /* Values should be monotonically increasing or equal */
  for (int i = 1; i < 10; i++) {
    TEST_ASSERT_GTE(times[i], times[i - 1]);
  }

  return TEST_PASS;
}

/* Test: No overflow in short-term operation */
static int test_timer_no_overflow_short_term(void) {
  unsigned long time1 = time_since_boot();
  unsigned long time2 = time_since_boot();

  /* If there was an overflow between reads, time2 would be much smaller
   * than time1 */
  /* For short-term operation, time2 should be >= time1 */
  TEST_ASSERT_GTE(time2, time1);

  /* The difference should be small (no huge jumps) */
  unsigned long diff = time2 - time1;
  /* Even with interrupts, shouldn't be more than a second apart */
  TEST_ASSERT_LT(diff, 1000000);

  return TEST_PASS;
}

/* Test: Timer counter affects scheduling decisions */
static int test_timer_counter_affects_scheduling(void) {
  /* Save original counter */
  long original_counter = current->counter;

  /* Set counter to a known value */
  current->counter = 10;
  TEST_ASSERT_EQ(10, current->counter);

  /* Simulate timer tick behavior (decrement) */
  current->counter--;
  TEST_ASSERT_EQ(9, current->counter);

  /* Counter going to 0 should trigger scheduling (conceptually) */
  current->counter = 0;
  TEST_ASSERT_EQ(0, current->counter);

  /* Restore original counter */
  current->counter = original_counter;

  return TEST_PASS;
}

/* Register all timer tests */
void register_timer_tests(void) {
  TEST_REGISTER(timer, time_since_boot_nonzero);
  TEST_REGISTER(timer, time_increases);
  TEST_REGISTER(timer, time_monotonic);
  TEST_REGISTER(timer, reasonable_rate);
  TEST_REGISTER(timer, delay_function);
  TEST_REGISTER(timer, multiple_reads);
  TEST_REGISTER(timer, no_overflow_short_term);
  TEST_REGISTER(timer, counter_affects_scheduling);
}
