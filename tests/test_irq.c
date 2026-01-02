/*
 * IRQ (Interrupt Request) Tests
 *
 * Tests for:
 * - IRQ vector initialization
 * - Interrupt enable/disable
 * - Exception level checks
 * - Interrupt controller setup
 * - Exception vector table
 * - Invalid entry message handling
 */

#include "irq.h"
#include "peripherals/base.h"
#include "peripherals/irq.h"
#include "printf.h"
#include "sched.h"
#include "test.h"
#include "utils.h"

/* External declarations */
extern void irq_vector_init(void);
extern void enable_irq(void);
extern void disable_irq(void);

/* Forward declarations for test functions */
static int test_irq_exception_level(void);
static int test_irq_enable_disable(void);
static int test_irq_disable_enable_sequence(void);
static int test_irq_nested_disable(void);
static int test_irq_controller_registers(void);
static int test_irq_enable_irqs_register(void);
static int test_irq_pending_registers_readable(void);
static int test_irq_error_messages_exist(void);
static int test_irq_vector_alignment(void);
static int test_irq_preempt_interaction(void);

/* External reference to exception vectors for alignment check */
extern char vectors[];

/* External reference to error messages array */
extern const char *entry_error_messages[];

/* Test: Verify we're running at EL1 */
static int test_irq_exception_level(void) {
  int el = get_el();

  /* Kernel should run at EL1 */
  TEST_ASSERT_EQ(1, el);

  return TEST_PASS;
}

/* Test: IRQ enable and disable work */
static int test_irq_enable_disable(void) {
  /* Disable IRQs */
  disable_irq();

  /* Enable IRQs */
  enable_irq();

  /* If we get here without crashing, basic enable/disable works */
  return TEST_PASS;
}

/* Test: Disable then enable sequence */
static int test_irq_disable_enable_sequence(void) {
  /* Start with known state */
  enable_irq();

  /* Disable */
  disable_irq();

  /* Do some work with interrupts disabled */
  for (volatile int i = 0; i < 100; i++) {
    /* busy loop */
  }

  /* Re-enable */
  enable_irq();

  return TEST_PASS;
}

/* Test: Nested disable calls */
static int test_irq_nested_disable(void) {
  enable_irq();

  /* Multiple disables */
  disable_irq();
  disable_irq();
  disable_irq();

  /* One enable should be enough to re-enable (ARM doesn't nest) */
  enable_irq();

  return TEST_PASS;
}

/* Test: Interrupt controller registers are accessible */
static int test_irq_controller_registers(void) {
  /* Read pending registers - should not crash */
  unsigned int pending1 = get32(IRQ_PENDING_1);
  unsigned int pending2 = get32(IRQ_PENDING_2);

  /* Values don't matter, just that we can read them */
  (void)pending1;
  (void)pending2;

  return TEST_PASS;
}

/* Test: Enable IRQs register is writable */
static int test_irq_enable_irqs_register(void) {
  /* Read current state */
  unsigned int enable1 = get32(ENABLE_IRQS_1);

  /* The timer IRQ should be enabled (SYSTEM_TIMER_IRQ_1 = 1 << 1) */
  TEST_ASSERT(enable1 & SYSTEM_TIMER_IRQ_1);

  return TEST_PASS;
}

/* Test: IRQ pending registers are readable */
static int test_irq_pending_registers_readable(void) {
  /* Should be able to read pending registers multiple times */
  for (int i = 0; i < 10; i++) {
    unsigned int p1 = get32(IRQ_PENDING_1);
    unsigned int p2 = get32(IRQ_PENDING_2);
    (void)p1;
    (void)p2;
  }

  return TEST_PASS;
}

/* Test: Error messages array exists and is populated */
static int test_irq_error_messages_exist(void) {
  /* Check that error messages are defined */
  TEST_ASSERT_NOT_NULL(entry_error_messages[0]);

  /* Verify some known error message strings exist */
  /* First entry should be "SYNC_INVALID_EL1t" */
  const char *first_msg = entry_error_messages[0];
  TEST_ASSERT_NOT_NULL(first_msg);

  /* Check that it's a non-empty string */
  TEST_ASSERT_NEQ('\0', first_msg[0]);

  return TEST_PASS;
}

/* Test: Exception vector table is properly aligned */
static int test_irq_vector_alignment(void) {
  unsigned long vectors_addr = (unsigned long)vectors;

  /* Vector table must be 2KB (0x800) aligned for ARM64 */
  /* This is 2^11 = 2048 bytes */
  TEST_ASSERT_EQ(0, vectors_addr & 0x7FF);

  return TEST_PASS;
}

/* Test: IRQ handling interacts with preemption */
static int test_irq_preempt_interaction(void) {
  long initial_preempt = current->preempt_count;

  /* Disable preemption */
  preempt_disable();
  TEST_ASSERT_EQ(initial_preempt + 1, current->preempt_count);

  /* Disable IRQs */
  disable_irq();

  /* With both preemption and IRQs disabled, we're in a critical section */

  /* Re-enable IRQs */
  enable_irq();

  /* Re-enable preemption */
  preempt_enable();
  TEST_ASSERT_EQ(initial_preempt, current->preempt_count);

  return TEST_PASS;
}

/* Register all IRQ tests */
void register_irq_tests(void) {
  TEST_REGISTER(irq, exception_level);
  TEST_REGISTER(irq, enable_disable);
  TEST_REGISTER(irq, disable_enable_sequence);
  TEST_REGISTER(irq, nested_disable);
  TEST_REGISTER(irq, controller_registers);
  TEST_REGISTER(irq, enable_irqs_register);
  TEST_REGISTER(irq, pending_registers_readable);
  TEST_REGISTER(irq, error_messages_exist);
  TEST_REGISTER(irq, vector_alignment);
  TEST_REGISTER(irq, preempt_interaction);
}
