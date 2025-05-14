#define __AVR_ATmega328PB__
// tell delay.h the frequency of the CPU
// this is the defualt value from the internal RC osc
#define F_CPU 1000000UL

#define BITSET(r, b) r |= _BV(b)
#define BITCLR(r, b) r &= ~_BV(b)

#define ISSET(r, b) (r & _BV(b)) != 0
#define ISCLR(r, b) (r & _BV(b)) == 0

#include <avr/io.h>
#include <util/delay.h>


int main (void)
{
    // set led pin to output
    DDRB |= _BV(DDRB2); 

    // set audio pin to output
    DDRE |= _BV(DDRE3);
    
    // set PWM for 50% duty cycle
    // OCR0A = 128;

    // TCCR2 |= (1 << WGM21) | (1 << WGM20);
    // // set fast PWM Mode
    // TCCR2 |= (1 << CS21);
    // // set prescaler to 8 and starts PWM

    while(1) 
    {
        // PORTB ^= _BV(PB2);

        // disharge system
        // reset to default state (key cap -> 0V)
        // set pin to sink current
        BITCLR(PORTC, PORTC0);
        // set key 0 pin to output
        BITSET(DDRC, DDRC0);

        _delay_ms(100);

        // charge cap briefly
        // set pin to source current
        BITSET(PORTC, PORTC0);

        // set pin to input (hi z)
        BITCLR(DDRC, DDRC0);
        BITCLR(PORTC, PORTC0);

        // equalise charges
        _delay_us(50);

        // need nop to ensure sync
        asm volatile ("nop");
        if (ISCLR(PINC, PINC0))
        {
            // key pressed, show light
            BITSET(PORTB, PORTB2);
        }
        else 
        {
            // key not pressed, no led
            BITCLR(PORTB, PORTB2);
        }
    }
}