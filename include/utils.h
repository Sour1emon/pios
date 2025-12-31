#ifndef _UTILS_H
#define _UTILS_H

#define CONST_DIV_CEIL(a, b) ((a + b - 1) / b)

extern void delay(unsigned long);
extern void put32(unsigned long, unsigned int);
extern unsigned int get32(unsigned long);
extern int get_el(void);

#endif
