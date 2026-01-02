/*
 * UART Tests
 *
 * Tests for:
 * - UART initialization
 * - Character sending
 * - String sending
 * - UART register access
 * - Printf integration
 */

#include "peripherals/uart.h"
#include "printf.h"
#include "test.h"
#include "uart.h"
#include "utils.h"

/* Forward declarations for test functions */
static int test_uart_send_char(void);
static int test_uart_send_string(void);
static int test_uart_send_newline(void);
static int test_uart_send_special_chars(void);
static int test_uart_printf_basic(void);
static int test_uart_printf_integer(void);
static int test_uart_printf_hex(void);
static int test_uart_printf_string(void);
static int test_uart_printf_long(void);
static int test_uart_printf_multiple_args(void);
static int test_uart_putc_callback(void);
static int test_uart_registers_accessible(void);

/* Test: Send a single character */
static int test_uart_send_char(void) {
  /* Send a test character - if we get here without hanging, UART works */
  uart_send('T');
  uart_send('E');
  uart_send('S');
  uart_send('T');
  uart_send(' ');

  return TEST_PASS;
}

/* Test: Send a string */
static int test_uart_send_string(void) {
  uart_send_string("UART_STRING_OK ");

  return TEST_PASS;
}

/* Test: Send newline characters */
static int test_uart_send_newline(void) {
  uart_send('\r');
  uart_send('\n');

  /* Also test the sequence in a string */
  uart_send_string("newline_test\r\n");

  return TEST_PASS;
}

/* Test: Send special characters */
static int test_uart_send_special_chars(void) {
  /* Test various ASCII characters */
  uart_send('[');
  uart_send(']');
  uart_send('{');
  uart_send('}');
  uart_send('<');
  uart_send('>');
  uart_send(' ');

  /* Test digits */
  for (char c = '0'; c <= '9'; c++) {
    uart_send(c);
  }
  uart_send(' ');

  return TEST_PASS;
}

/* Test: Printf basic output */
static int test_uart_printf_basic(void) {
  printf("printf_basic_ok ");

  return TEST_PASS;
}

/* Test: Printf with integer formatting */
static int test_uart_printf_integer(void) {
  int val = 42;
  printf("int=%d ", val);

  int neg = -123;
  printf("neg=%d ", neg);

  unsigned int uval = 255;
  printf("uint=%u ", uval);

  return TEST_PASS;
}

/* Test: Printf with hexadecimal formatting */
static int test_uart_printf_hex(void) {
  unsigned int val = 0xDEAD;
  printf("hex=0x%x ", val);

  unsigned int val2 = 0xBEEF;
  printf("HEX=0x%X ", val2);

  /* Test with width specifier */
  printf("hex8=0x%08x ", 0x42);

  return TEST_PASS;
}

/* Test: Printf with string formatting */
static int test_uart_printf_string(void) {
  const char *str = "hello";
  printf("str=%s ", str);

  printf("inline=%s ", "world");

  return TEST_PASS;
}

/* Test: Printf with long integer formatting */
static int test_uart_printf_long(void) {
  unsigned long lval = 0xFFFFFFFF00000001UL;
  printf("long=0x%lx ", lval);

  long slval = -1L;
  printf("slong=%ld ", slval);

  return TEST_PASS;
}

/* Test: Printf with multiple arguments */
static int test_uart_printf_multiple_args(void) {
  int a = 1, b = 2, c = 3;
  printf("multi: %d %d %d ", a, b, c);

  printf("mixed: %d 0x%x %s ", 42, 0xFF, "end");

  return TEST_PASS;
}

/* Test: uart_putc callback function */
static int test_uart_putc_callback(void) {
  /* uart_putc is used as a callback for printf - test it directly */
  uart_putc(0, 'P');
  uart_putc(0, 'U');
  uart_putc(0, 'T');
  uart_putc(0, 'C');
  uart_putc(0, ' ');

  return TEST_PASS;
}

/* Test: UART registers are accessible */
static int test_uart_registers_accessible(void) {
  /* Read the flag register - should not crash */
  unsigned int fr = get32(UART0_FR);

  /* Flag register should have some bits set (TXFE or similar) */
  /* We don't check specific values as they depend on UART state */
  (void)fr; /* Suppress unused warning */

  /* Read the control register */
  unsigned int cr = get32(UART0_CR);

  /* If UART is initialized, bit 0 (UARTEN) should be set */
  TEST_ASSERT(cr & (1 << 0)); /* UART enabled */
  TEST_ASSERT(cr & (1 << 8)); /* TX enabled */
  TEST_ASSERT(cr & (1 << 9)); /* RX enabled */

  return TEST_PASS;
}

/* Register all UART tests */
void register_uart_tests(void) {
  TEST_REGISTER(uart, send_char);
  TEST_REGISTER(uart, send_string);
  TEST_REGISTER(uart, send_newline);
  TEST_REGISTER(uart, send_special_chars);
  TEST_REGISTER(uart, printf_basic);
  TEST_REGISTER(uart, printf_integer);
  TEST_REGISTER(uart, printf_hex);
  TEST_REGISTER(uart, printf_string);
  TEST_REGISTER(uart, printf_long);
  TEST_REGISTER(uart, printf_multiple_args);
  TEST_REGISTER(uart, putc_callback);
  TEST_REGISTER(uart, registers_accessible);
}
