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
#include "keys.h"

int isKeyPressed(Key key)
{

    // charge cap briefly
    // set pin to source current
    BITSET(*key.dataReg, key.dataBit);
    // set pin to input (hi z)
    BITCLR(*key.dirReg, key.dirBit);
    // set pin to sink current
    BITCLR(*key.dataReg, key.dataBit);

    // equalise charges
    _delay_us(50);

    asm volatile ("nop");
    const int pressed = ISCLR(*key.readReg, key.readBit);

    // reset pin to default, disharged state

    // reset to default state (key cap -> 0V)
    // set pin to sink current
    BITCLR(*key.dataReg, key.dataBit);
    // set key 0 pin to output
    BITSET(*key.dirReg, key.dirBit);

    return pressed;
}


int main (void)
{
    // set led pin to output
    DDRB |= _BV(DDRB2); 

    Key keys[] = {
        (Key){&DDRC, DDRC0, &PORTC, PORTC0, &PINC, PINC0},
        (Key){&DDRC, DDRC1, &PORTC, PORTC1, &PINC, PINC1},
        (Key){&DDRC, DDRC2, &PORTC, PORTC2, &PINC, PINC2},
        (Key){&DDRC, DDRC3, &PORTC, PORTC3, &PINC, PINC3},
        (Key){&DDRC, DDRC4, &PORTC, PORTC4, &PINC, PINC4},
        (Key){&DDRC, DDRC5, &PORTC, PORTC5, &PINC, PINC5},

        (Key){&DDRD, DDRD0, &PORTD, PORTD0, &PIND, PIND0},
        (Key){&DDRD, DDRD1, &PORTD, PORTD1, &PIND, PIND1},
        (Key){&DDRD, DDRD2, &PORTD, PORTD2, &PIND, PIND2},
        (Key){&DDRD, DDRD3, &PORTD, PORTD3, &PIND, PIND3},
        (Key){&DDRD, DDRD4, &PORTD, PORTD4, &PIND, PIND4},

        (Key){&DDRE, DDRE0, &PORTE, PORTE0, &PINE, PINE0},
    };

    int anyPressed = 0;

    while(1) 
    {
        _delay_ms(200);
        anyPressed = 0;
        for (int i = 0; i < sizeof(keys)/sizeof(Key); i++)
        {
            anyPressed |= isKeyPressed(keys[i]);
        }

        if (anyPressed)
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