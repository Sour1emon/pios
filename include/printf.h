#ifndef __TFP_PRINTF__
#define __TFP_PRINTF__

#include <stdarg.h>

void init_printf(void *putp, void (*putf)(void *, char));

void tfp_printf(char *fmt, ...) __attribute__((format(printf, 1, 2)));
void tfp_sprintf(char *s, char *fmt, ...) __attribute__((format(printf, 2, 3)));

void tfp_format(void *putp, void (*putf)(void *, char), char *fmt, va_list va);

#define printf tfp_printf
#define sprintf tfp_sprintf

// Enable long support
#define PRINTF_LONG_SUPPORT

#endif
