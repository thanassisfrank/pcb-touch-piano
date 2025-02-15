#define __AVR_ATmega328P__
#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int
main (void)
{
    DDRB |= _BV(DDB2); 
    
    while(1) 
    {
        PORTB ^= _BV(PB2);
        _delay_ms(500);
    }
}