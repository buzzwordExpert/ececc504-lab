#include <uart.h>
#include <msp430.h>

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
}

int main(void)
{
    mcu_init();
    uart_init();
    
    while (1) 
    {
        uart_putchar_polling('h');
        uart_putchar_polling('e');
        uart_putchar_polling('l');
        uart_putchar_polling('l');
        uart_putchar_polling('o');
        uart_putchar_polling('\n');
        __delay_cycles(1000000);
    }
}
