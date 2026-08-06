#include <msp430.h>
#include <uart.h>

static void io_init(void) 
{
    // P1.1 UART RX, P1.2 UART TX
    P1SEL  |= BIT1 | BIT2;
    P1SEL2 |= BIT1 | BIT2;
}

static void mcu_init(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // stop the watchdog timer

    // Configure SMCLK to run at 16 MHz
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL  = CALDCO_16MHZ;

    io_init();
    _enable_interrupts();
}

int main(void)
{
    mcu_init();
    uart_init();

    while(1) {
        uart_putchar_interrupt('h');
        uart_putchar_interrupt('e');
        uart_putchar_interrupt('l');
        uart_putchar_interrupt('l');
        uart_putchar_interrupt('o');
        uart_putchar_interrupt('\n');
        __delay_cycles(1000000);
    }
}
