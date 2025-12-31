#ifndef _UART_H
#define _UART_H

void uart_init(void);
char uart_recv(void);
void uart_send(char c);
void uart_send_string(char *str);

void putc(void *p, char c);

void handle_uart_irq(void);

#endif /*_UART_H */
