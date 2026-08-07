#include <msp430.h>

#define SW_IO    BIT3
#define LED_IO   BIT0

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED_IO;
    P1DIR &= ~SW_IO;

    while (1)
    {
        if (!(P1IN & SW_IO))
        {
            while (!(P1IN & SW_IO));
            P1OUT ^= LED_IO;
        }
    }
}
