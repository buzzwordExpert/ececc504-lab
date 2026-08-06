#include <uart.h>
#include <ring_buffer.h>
#include <msp430.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define UART_BUFFER_SIZE (16)
static uint8_t buffer[UART_BUFFER_SIZE];
static struct ring_buffer tx_buffer = { .buffer = buffer, .size = sizeof(buffer) };

/*
    Calculate the integer and fractional part of the divisor
    N = (Clock Source / Desired baud rate)
    for low-frequency baude rate mode
    these are used to configure the desired baude rate
*/

#define SMCLK (16000000u)
#define BRCLK (SMCLK)
#define UART_BAUD_RATE (115200u)
static_assert(UART_BAUD_RATE < (BRCLK / 3.0f), "Baudrate must be smaller than 1/3 of input clock in Low-Frequency mode");

#define UART_DIVISOR ((float)BRCLK / UART_BAUD_RATE)
static_assert(UART_DIVISOR < 0xFFFFu, "To check if divisor fits in the 16-bit register");

#define UART_DIVISOR_INT_16BIT ((uint16_t)UART_DIVISOR)
#define UART_DIVISOR_INT_LOW_BYTE (UART_DIVISOR_INT_16BIT & 0xFF)
#define UART_DIVISOR_INT_HIGH_BYTE (UART_DIVISOR_INT_16BIT >> 8)
#define UART_DIVISOR_FRACTIONAL (UART_DIVISOR - UART_DIVISOR_INT_16BIT)
#define UART_UCBRS ((uint8_t)(8 * UART_DIVISOR_FRACTIONAL))
#define UART_UCBRF (0)
#define UART_UC0S16 (0)
static_assert(UART_UCBRS < 8, "modulation register is 3 bit");

static inline void uart_tx_enable_interrupt(void)
{
    UC0IE |= UCA0TXIE;
}

static inline void uart_tx_disable_interrupt(void)
{
    UC0IE &= ~UCA0TXIE;
}

static void uart_tx_start(void)
{
    if (!ring_buffer_empty(&tx_buffer)) {
        UCA0TXBUF = ring_buffer_peek(&tx_buffer);
    }
}

__attribute__((interrupt(USCIAB0TX_VECTOR))) void isr_uart_tx(void)
{
    if (ring_buffer_empty(&tx_buffer)) {
        uart_tx_disable_interrupt();
        return;
    }

    // remove the transmitted data byte from the buffer
    ring_buffer_get(&tx_buffer);

    if (!ring_buffer_empty(&tx_buffer)) {
        uart_tx_start();
    } else {
        uart_tx_disable_interrupt();
    }
}

void uart_init(void)
{
    // the module should be in reset condition while configured
    UCA0CTL1 |= UCSWRST;

    // default uart -> [ Start (1 bit) | Data (8 bits) | Stop (1 bit)]
    UCA0CTL0 = 0;

    // Set SMCLK as the clock source
    UCA0CTL1 |= UCSSEL_2;

    // Set clock prescaler to the integer part of divisor N
    UCA0BR0 = UART_DIVISOR_INT_LOW_BYTE;
    UCA0BR1 = UART_DIVISOR_INT_HIGH_BYTE;
    
    // Set modulation control register for the fractional part of divisor N
    // UCA0MCTL = [UCBRF (4 bits) | UCBRS (3 bits) | UC0S16 (1 bit)]
    UCA0MCTL = (UART_UCBRF << 4) | (UART_UCBRS << 1) | UART_UC0S16; 

    // clear reset to release the module for operation
    UCA0CTL1 &= ~UCSWRST;

    // Enable TX interrupts only when data is queued.
    uart_tx_disable_interrupt();
}

void uart_putchar_polling(char ch)
{
    // wait for any ongoing transmission to finish    
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = ch;

    // some terminals expect carriage return (\r) after line-feed (\n) for proper new line
    if (ch == '\n')
    {
        uart_putchar_polling('\r');
    }
}

void uart_putchar_interrupt(char ch)
{
    // Poll if full
    while(ring_buffer_full(&tx_buffer));

    uart_tx_disable_interrupt();
    const bool tx_ongoing = !ring_buffer_empty(&tx_buffer);
    ring_buffer_put(&tx_buffer, ch);
    if (!tx_ongoing) {
        uart_tx_start();
    }
    uart_tx_enable_interrupt();

    // some terminals expect carriage return (\r) after line-feed (\n) for proper new line
    if (ch == '\n')
    {
        uart_putchar_interrupt('\r');
    }
}
