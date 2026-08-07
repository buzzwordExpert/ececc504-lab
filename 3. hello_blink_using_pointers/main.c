#include <msp430.h>

void main(void)
{
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;
    
    (*(volatile unsigned int *) 0x0120) = 0x5A00 | 0x0080;      // WDCTL = WDTPW | WDTHOLD

    (*(volatile unsigned char *) 0x0022) |= 0x0040;             // P1DIR |= BIT6

    volatile unsigned long i;
    while (1)
    {
        (*(volatile unsigned char *) 0x0021) |= 0x0040;         // P1OUT |= BIT6
        for (i = 0; i < 10000; i++);

        (*(volatile unsigned char *) 0x0021) &= ~0x0040;        // P1OUT &= ~BIT6
        for (i = 0; i < 10000; i++);
    }
}
