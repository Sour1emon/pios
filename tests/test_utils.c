/*
 * Utility Function Tests
 *
 * Tests for:
 * - delay function
 * - put32/get32 memory operations
 * - memzero functionality
 * - memcpy functionality
 * - get_el (exception level)
 * - set_pgd/get_pgd (page directory)
 */

#include "mm.h"
#include "peripherals/base.h"
#include "printf.h"
#include "sched.h"
#include "test.h"
#include "utils.h"

/* Forward declarations for test functions */
static int test_utils_delay_returns(void);
static int test_utils_get32_put32(void);
static int test_utils_get_el_returns_valid(void);
static int test_utils_memzero_clears(void);
static int test_utils_memcpy_copies(void);
static int test_utils_memcpy_preserves_src(void);
static int test_utils_memzero_boundary(void);
static int test_utils_get_pgd_returns_value(void);
static int test_utils_const_div_ceil_macro(void);
static int test_utils_va_start_constant(void);

/* Test buffer for memory operations */
static unsigned char test_buffer[256];
static unsigned char src_buffer[256];

/* Test: delay function returns (doesn't hang) */
static int test_utils_delay_returns(void) {
  /* Test delay with various values including 0 */
  delay(0); /* Should return immediately, not infinite loop */
  delay(1);
  delay(10);
  delay(100);

  /* If we get here, delay returns properly */
  return TEST_PASS;
}

/* Test: get32/put32 work with memory-mapped registers */
static int test_utils_get32_put32(void) {
  /* We can't safely test arbitrary memory locations, but we can
   * verify that reading from known safe locations works */

  /* Read a known peripheral register (UART flag register) */
  /* This is a read-only register, so we just verify we can read it */
  unsigned int val = get32(VA_START + 0x3F201018); /* UART0_FR */

  /* Value should be some valid flag combination */
  /* We don't check specific bits as they depend on UART state */
  (void)val;

  return TEST_PASS;
}

/* Test: get_el returns valid exception level */
static int test_utils_get_el_returns_valid(void) {
  int el = get_el();

  /* Kernel runs at EL1 */
  TEST_ASSERT_EQ(1, el);

  /* Call multiple times to ensure consistency */
  TEST_ASSERT_EQ(el, get_el());
  TEST_ASSERT_EQ(el, get_el());

  return TEST_PASS;
}

/* Test: memzero clears memory */
static int test_utils_memzero_clears(void) {
  /* Fill buffer with non-zero data */
  for (int i = 0; i < 256; i++) {
    test_buffer[i] = 0xAA;
  }

  /* Zero it out */
  memzero((unsigned long)test_buffer, 256);

  /* Verify all bytes are zero */
  for (int i = 0; i < 256; i++) {
    TEST_ASSERT_EQ(0, test_buffer[i]);
  }

  return TEST_PASS;
}

/* Test: memcpy copies data correctly */
static int test_utils_memcpy_copies(void) {
  /* Initialize source buffer with pattern */
  for (int i = 0; i < 256; i++) {
    src_buffer[i] = (unsigned char)i;
  }

  /* Clear destination */
  for (int i = 0; i < 256; i++) {
    test_buffer[i] = 0;
  }

  /* Copy */
  memcpy((unsigned long)test_buffer, (unsigned long)src_buffer, 256);

  /* Verify copy */
  for (int i = 0; i < 256; i++) {
    TEST_ASSERT_EQ(src_buffer[i], test_buffer[i]);
  }

  return TEST_PASS;
}

/* Test: memcpy preserves source data */
static int test_utils_memcpy_preserves_src(void) {
  /* Initialize source buffer */
  for (int i = 0; i < 128; i++) {
    src_buffer[i] = (unsigned char)(255 - i);
  }

  /* Copy to destination */
  memcpy((unsigned long)test_buffer, (unsigned long)src_buffer, 128);

  /* Verify source is unchanged */
  for (int i = 0; i < 128; i++) {
    TEST_ASSERT_EQ((unsigned char)(255 - i), src_buffer[i]);
  }

  return TEST_PASS;
}

/* Test: memzero handles boundary conditions */
static int test_utils_memzero_boundary(void) {
  /* Test single byte */
  test_buffer[0] = 0xFF;
  test_buffer[1] = 0xFF;
  memzero((unsigned long)test_buffer, 1);
  TEST_ASSERT_EQ(0, test_buffer[0]);
  /* Byte after should be unchanged */
  TEST_ASSERT_EQ(0xFF, test_buffer[1]);

  /* Test 8-byte aligned length (common case) */
  for (int i = 0; i < 16; i++) {
    test_buffer[i] = 0xFF;
  }
  memzero((unsigned long)test_buffer, 8);
  for (int i = 0; i < 8; i++) {
    TEST_ASSERT_EQ(0, test_buffer[i]);
  }
  /* Bytes beyond should be unchanged */
  TEST_ASSERT_EQ(0xFF, test_buffer[8]);
  TEST_ASSERT_EQ(0xFF, test_buffer[9]);

  return TEST_PASS;
}

/* Test: get_pgd returns a value */
static int test_utils_get_pgd_returns_value(void) {
  unsigned long pgd = get_pgd();

  /* PGD should be a valid address or 0 */
  /* We just verify the function can be called */
  (void)pgd;

  return TEST_PASS;
}

/* Test: CONST_DIV_CEIL macro */
static int test_utils_const_div_ceil_macro(void) {
  /* Test ceiling division: (a + b - 1) / b */
  TEST_ASSERT_EQ(1, CONST_DIV_CEIL(1, 1));
  TEST_ASSERT_EQ(1, CONST_DIV_CEIL(1, 2));
  TEST_ASSERT_EQ(2, CONST_DIV_CEIL(3, 2));
  TEST_ASSERT_EQ(2, CONST_DIV_CEIL(4, 2));
  TEST_ASSERT_EQ(3, CONST_DIV_CEIL(5, 2));
  TEST_ASSERT_EQ(10, CONST_DIV_CEIL(100, 10));
  TEST_ASSERT_EQ(11, CONST_DIV_CEIL(101, 10));
  TEST_ASSERT_EQ(1024, CONST_DIV_CEIL(65535, 64));
  TEST_ASSERT_EQ(1024,
                 CONST_DIV_CEIL(65536, 64)); /* 65536 is exactly divisible */
  TEST_ASSERT_EQ(1025, CONST_DIV_CEIL(65537, 64)); /* 65537 needs one more */

  return TEST_PASS;
}

/* Test: VA_START constant is correct */
static int test_utils_va_start_constant(void) {
  /* VA_START should be the kernel virtual address base */
  TEST_ASSERT_EQ(0xffff000000000000UL, VA_START);

  /* DEVICE_BASE should be the peripheral base */
  TEST_ASSERT_EQ(0x3F000000, DEVICE_BASE);

  /* PBASE should be VA_START + DEVICE_BASE */
  TEST_ASSERT_EQ(VA_START + DEVICE_BASE, PBASE);

  return TEST_PASS;
}

/* Register all utility tests */
void register_utils_tests(void) {
  TEST_REGISTER(utils, delay_returns);
  TEST_REGISTER(utils, get32_put32);
  TEST_REGISTER(utils, get_el_returns_valid);
  TEST_REGISTER(utils, memzero_clears);
  TEST_REGISTER(utils, memcpy_copies);
  TEST_REGISTER(utils, memcpy_preserves_src);
  TEST_REGISTER(utils, memzero_boundary);
  TEST_REGISTER(utils, get_pgd_returns_value);
  TEST_REGISTER(utils, const_div_ceil_macro);
  TEST_REGISTER(utils, va_start_constant);
}
