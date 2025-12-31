#include "peripherals/uart.h"
#include "peripherals/gpio.h"
#include "utils.h"

void uart_init(void) {
  unsigned int selector;

  put32(UART0_CR, 0); // turn off UART0

  selector = get32(GPFSEL1);
  selector &= ~(7 << 12); // clean gpio14
  selector |= 4 << 12;    // ALT0 for TX
  selector &= ~(7 << 15); // clean gpio15
  selector |= 4 << 15;    // ALT0 for RX
  put32(GPFSEL1, selector);

  put32(GPPUD, 0);
  delay(150);
  put32(GPPUDCLK0, (1 << 14) | (1 << 15));
  delay(150);
  put32(GPPUDCLK0, 0);

  put32(UART0_ICR, 0x7FF); // clear interrupts
  put32(UART0_IBRD, 26);   // 115200 baud
  put32(UART0_FBRD, 3);
  put32(UART0_LCRH,
        (1 << 4) | (1 << 5) | (1 << 6)); // 8 bit word length, enable FIFO
  put32(UART0_IMSC,
        (1 << 4)); // Mask all interrupts except for the RXIM (UART Receive
                   // Interrupt)

  put32(UART0_CR,
        (1 << 0) | (1 << 8) |
            (1 << 9)); // enable UART0, receive enable, transmit enable
}

void uart_send(char c) {
  while (get32(UART0_FR) & 0x20) {
  }
  put32(UART0_DR, c);
}

char uart_recv() {
  while (get32(UART0_FR) & 0x10) {
  }
  return get32(UART0_DR);
}

void uart_send_string(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    uart_send(str[i]);
  }
}

void uart_putc(void *_p __attribute__((unused)), char c) { uart_send(c); }

void handle_uart_irq(void) {
  // Echo back all the characters we have received
  while (!(get32(UART0_FR) & 0x10)) { // RXFE bit - RX FIFO not empty
    char c = get32(UART0_DR);
    uart_send(c);
  }

  put32(UART0_ICR, (1 << 4)); // Clear the interrupt just in case
}
