/*
 * Printf Tests
 *
 * Tests for:
 * - Basic string output
 * - Integer formatting (%d, %u)
 * - Hexadecimal formatting (%x, %X)
 * - Long integer formatting (%ld, %lu, %lx)
 * - String formatting (%s)
 * - Character formatting (%c)
 * - Width and padding specifiers
 * - sprintf functionality
 * - Edge cases (negative numbers, zero, max values)
 */

#include "printf.h"
#include "test.h"
#include <stddef.h>

/* Buffer for sprintf testing */
static char sprintf_buf[256];

/* Helper to check string equality */
static int str_eq(const char *a, const char *b) {
  if (a == 0 || b == 0)
    return a == b;
  while (*a && *b) {
    if (*a != *b)
      return 0;
    a++;
    b++;
  }
  return *a == *b;
}

/* Helper to get string length */
static int str_len(const char *s) {
  int len = 0;
  while (s && *s) {
    len++;
    s++;
  }
  return len;
}

/* Forward declarations for test functions */
static int test_printf_sprintf_basic(void);
static int test_printf_sprintf_integer(void);
static int test_printf_sprintf_negative(void);
static int test_printf_sprintf_unsigned(void);
static int test_printf_sprintf_hex_lower(void);
static int test_printf_sprintf_hex_upper(void);
static int test_printf_sprintf_string(void);
static int test_printf_sprintf_char(void);
static int test_printf_sprintf_percent(void);
static int test_printf_sprintf_width(void);
static int test_printf_sprintf_zero_pad(void);
static int test_printf_sprintf_long(void);
static int test_printf_sprintf_long_hex(void);
static int test_printf_sprintf_multiple(void);
static int test_printf_sprintf_empty_string(void);
static int test_printf_sprintf_zero_int(void);
static int test_printf_sprintf_large_int(void);

/* Test: Basic sprintf with plain string */
static int test_printf_sprintf_basic(void) {
  tfp_sprintf(sprintf_buf, "hello");

  TEST_ASSERT(str_eq(sprintf_buf, "hello"));
  TEST_ASSERT_EQ(5, str_len(sprintf_buf));

  return TEST_PASS;
}

/* Test: sprintf with integer formatting */
static int test_printf_sprintf_integer(void) {
  tfp_sprintf(sprintf_buf, "%d", 42);
  TEST_ASSERT(str_eq(sprintf_buf, "42"));

  tfp_sprintf(sprintf_buf, "%d", 0);
  TEST_ASSERT(str_eq(sprintf_buf, "0"));

  tfp_sprintf(sprintf_buf, "%d", 123456);
  TEST_ASSERT(str_eq(sprintf_buf, "123456"));

  return TEST_PASS;
}

/* Test: sprintf with negative integers */
static int test_printf_sprintf_negative(void) {
  tfp_sprintf(sprintf_buf, "%d", -1);
  TEST_ASSERT(str_eq(sprintf_buf, "-1"));

  tfp_sprintf(sprintf_buf, "%d", -42);
  TEST_ASSERT(str_eq(sprintf_buf, "-42"));

  tfp_sprintf(sprintf_buf, "%d", -999);
  TEST_ASSERT(str_eq(sprintf_buf, "-999"));

  return TEST_PASS;
}

/* Test: sprintf with unsigned integers */
static int test_printf_sprintf_unsigned(void) {
  tfp_sprintf(sprintf_buf, "%u", 42u);
  TEST_ASSERT(str_eq(sprintf_buf, "42"));

  tfp_sprintf(sprintf_buf, "%u", 0u);
  TEST_ASSERT(str_eq(sprintf_buf, "0"));

  tfp_sprintf(sprintf_buf, "%u", 4294967295u);
  TEST_ASSERT(str_eq(sprintf_buf, "4294967295"));

  return TEST_PASS;
}

/* Test: sprintf with lowercase hex */
static int test_printf_sprintf_hex_lower(void) {
  tfp_sprintf(sprintf_buf, "%x", 0);
  TEST_ASSERT(str_eq(sprintf_buf, "0"));

  tfp_sprintf(sprintf_buf, "%x", 255);
  TEST_ASSERT(str_eq(sprintf_buf, "ff"));

  tfp_sprintf(sprintf_buf, "%x", 0xdeadbeef);
  TEST_ASSERT(str_eq(sprintf_buf, "deadbeef"));

  tfp_sprintf(sprintf_buf, "%x", 16);
  TEST_ASSERT(str_eq(sprintf_buf, "10"));

  return TEST_PASS;
}

/* Test: sprintf with uppercase hex */
static int test_printf_sprintf_hex_upper(void) {
  tfp_sprintf(sprintf_buf, "%X", 255);
  TEST_ASSERT(str_eq(sprintf_buf, "FF"));

  tfp_sprintf(sprintf_buf, "%X", 0xABCD);
  TEST_ASSERT(str_eq(sprintf_buf, "ABCD"));

  tfp_sprintf(sprintf_buf, "%X", 0xDEADBEEF);
  TEST_ASSERT(str_eq(sprintf_buf, "DEADBEEF"));

  return TEST_PASS;
}

/* Test: sprintf with string formatting */
static int test_printf_sprintf_string(void) {
  tfp_sprintf(sprintf_buf, "%s", "world");
  TEST_ASSERT(str_eq(sprintf_buf, "world"));

  tfp_sprintf(sprintf_buf, "hello %s", "world");
  TEST_ASSERT(str_eq(sprintf_buf, "hello world"));

  tfp_sprintf(sprintf_buf, "%s %s", "foo", "bar");
  TEST_ASSERT(str_eq(sprintf_buf, "foo bar"));

  return TEST_PASS;
}

/* Test: sprintf with character formatting */
static int test_printf_sprintf_char(void) {
  tfp_sprintf(sprintf_buf, "%c", 'A');
  TEST_ASSERT(str_eq(sprintf_buf, "A"));

  tfp_sprintf(sprintf_buf, "%c%c%c", 'X', 'Y', 'Z');
  TEST_ASSERT(str_eq(sprintf_buf, "XYZ"));

  tfp_sprintf(sprintf_buf, "[%c]", '!');
  TEST_ASSERT(str_eq(sprintf_buf, "[!]"));

  return TEST_PASS;
}

/* Test: sprintf with percent escape */
static int test_printf_sprintf_percent(void) {
  tfp_sprintf(sprintf_buf, "100%%");
  TEST_ASSERT(str_eq(sprintf_buf, "100%"));

  tfp_sprintf(sprintf_buf, "%%");
  TEST_ASSERT(str_eq(sprintf_buf, "%"));

  tfp_sprintf(sprintf_buf, "a%%b%%c");
  TEST_ASSERT(str_eq(sprintf_buf, "a%b%c"));

  return TEST_PASS;
}

/* Test: sprintf with width specifier */
static int test_printf_sprintf_width(void) {
  tfp_sprintf(sprintf_buf, "%5d", 42);
  TEST_ASSERT(str_eq(sprintf_buf, "   42"));

  tfp_sprintf(sprintf_buf, "%3d", 42);
  TEST_ASSERT(str_eq(sprintf_buf, " 42"));

  tfp_sprintf(sprintf_buf, "%1d", 42);
  TEST_ASSERT(str_eq(sprintf_buf, "42")); /* Width less than digits */

  return TEST_PASS;
}

/* Test: sprintf with zero padding */
static int test_printf_sprintf_zero_pad(void) {
  tfp_sprintf(sprintf_buf, "%05d", 42);
  TEST_ASSERT(str_eq(sprintf_buf, "00042"));

  tfp_sprintf(sprintf_buf, "%08x", 0xff);
  TEST_ASSERT(str_eq(sprintf_buf, "000000ff"));

  tfp_sprintf(sprintf_buf, "%03d", 7);
  TEST_ASSERT(str_eq(sprintf_buf, "007"));

  return TEST_PASS;
}

/* Test: sprintf with long integers */
static int test_printf_sprintf_long(void) {
  tfp_sprintf(sprintf_buf, "%ld", 123456789L);
  TEST_ASSERT(str_eq(sprintf_buf, "123456789"));

  tfp_sprintf(sprintf_buf, "%ld", -123456789L);
  TEST_ASSERT(str_eq(sprintf_buf, "-123456789"));

  tfp_sprintf(sprintf_buf, "%lu", 4294967295UL);
  TEST_ASSERT(str_eq(sprintf_buf, "4294967295"));

  return TEST_PASS;
}

/* Test: sprintf with long hex */
static int test_printf_sprintf_long_hex(void) {
  tfp_sprintf(sprintf_buf, "%lx", 0xFFFFFFFFUL);
  TEST_ASSERT(str_eq(sprintf_buf, "ffffffff"));

  tfp_sprintf(sprintf_buf, "%lX", 0xABCDEF01UL);
  TEST_ASSERT(str_eq(sprintf_buf, "ABCDEF01"));

  return TEST_PASS;
}

/* Test: sprintf with multiple format specifiers */
static int test_printf_sprintf_multiple(void) {
  tfp_sprintf(sprintf_buf, "%d + %d = %d", 1, 2, 3);
  TEST_ASSERT(str_eq(sprintf_buf, "1 + 2 = 3"));

  tfp_sprintf(sprintf_buf, "0x%x is %d in decimal", 255, 255);
  TEST_ASSERT(str_eq(sprintf_buf, "0xff is 255 in decimal"));

  tfp_sprintf(sprintf_buf, "%s: %d (0x%X)", "value", 42, 42);
  TEST_ASSERT(str_eq(sprintf_buf, "value: 42 (0x2A)"));

  return TEST_PASS;
}

/* Test: sprintf with empty string */
static int test_printf_sprintf_empty_string(void) {
  tfp_sprintf(sprintf_buf, "%s", "");
  TEST_ASSERT(str_eq(sprintf_buf, ""));
  TEST_ASSERT_EQ(0, str_len(sprintf_buf));

  tfp_sprintf(sprintf_buf, "a%sb", "");
  TEST_ASSERT(str_eq(sprintf_buf, "ab"));

  return TEST_PASS;
}

/* Test: sprintf with zero */
static int test_printf_sprintf_zero_int(void) {
  tfp_sprintf(sprintf_buf, "%d", 0);
  TEST_ASSERT(str_eq(sprintf_buf, "0"));

  tfp_sprintf(sprintf_buf, "%x", 0);
  TEST_ASSERT(str_eq(sprintf_buf, "0"));

  tfp_sprintf(sprintf_buf, "%u", 0);
  TEST_ASSERT(str_eq(sprintf_buf, "0"));

  tfp_sprintf(sprintf_buf, "%05d", 0);
  TEST_ASSERT(str_eq(sprintf_buf, "00000"));

  return TEST_PASS;
}

/* Test: sprintf with large integers */
static int test_printf_sprintf_large_int(void) {
  tfp_sprintf(sprintf_buf, "%u", 2147483647u);
  TEST_ASSERT(str_eq(sprintf_buf, "2147483647"));

  tfp_sprintf(sprintf_buf, "%d", 2147483647);
  TEST_ASSERT(str_eq(sprintf_buf, "2147483647"));

  return TEST_PASS;
}

/* Register all printf tests */
void register_printf_tests(void) {
  TEST_REGISTER(printf, sprintf_basic);
  TEST_REGISTER(printf, sprintf_integer);
  TEST_REGISTER(printf, sprintf_negative);
  TEST_REGISTER(printf, sprintf_unsigned);
  TEST_REGISTER(printf, sprintf_hex_lower);
  TEST_REGISTER(printf, sprintf_hex_upper);
  TEST_REGISTER(printf, sprintf_string);
  TEST_REGISTER(printf, sprintf_char);
  TEST_REGISTER(printf, sprintf_percent);
  TEST_REGISTER(printf, sprintf_width);
  TEST_REGISTER(printf, sprintf_zero_pad);
  TEST_REGISTER(printf, sprintf_long);
  TEST_REGISTER(printf, sprintf_long_hex);
  TEST_REGISTER(printf, sprintf_multiple);
  TEST_REGISTER(printf, sprintf_empty_string);
  TEST_REGISTER(printf, sprintf_zero_int);
  TEST_REGISTER(printf, sprintf_large_int);
}
