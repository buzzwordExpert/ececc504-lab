#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_putchar_polling(char ch);
void uart_putchar_interrupt(char ch);

#endif      // UART_H
