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
    // setup pwm ==============================================
    // set pin to output
    BITSET(DDRB, DDRB1);

    // set clock to clkio/8 (125 KHz)
    BITSET(TCCR1B, CS11);
    
    // setup waveform generation mode (mode 4, CTC)
    BITSET(TCCR1B, WGM12);

    // set led pin to output =================================
    BITSET(DDRB, DDRB2); 

    Key keys[] = {
        (Key){&DDRC, DDRC0, &PORTC, PORTC0, &PINC, PINC0, 120},
        (Key){&DDRC, DDRC1, &PORTC, PORTC1, &PINC, PINC1, 113},
        (Key){&DDRC, DDRC2, &PORTC, PORTC2, &PINC, PINC2, 106},
        (Key){&DDRC, DDRC3, &PORTC, PORTC3, &PINC, PINC3, 100},
        (Key){&DDRC, DDRC4, &PORTC, PORTC4, &PINC, PINC4, 95},
        (Key){&DDRC, DDRC5, &PORTC, PORTC5, &PINC, PINC5, 89},

        (Key){&DDRD, DDRD0, &PORTD, PORTD0, &PIND, PIND0, 84},
        (Key){&DDRD, DDRD1, &PORTD, PORTD1, &PIND, PIND1, 80},
        (Key){&DDRD, DDRD2, &PORTD, PORTD2, &PIND, PIND2, 75},
        (Key){&DDRD, DDRD3, &PORTD, PORTD3, &PIND, PIND3, 71},
        (Key){&DDRD, DDRD4, &PORTD, PORTD4, &PIND, PIND4, 67},

        (Key){&DDRE, DDRE0, &PORTE, PORTE0, &PINE, PINE0, 63},
    };

    int anyPressed = 0;

    while(1) 
    {
        _delay_ms(200);
        anyPressed = 0;
        for (unsigned int i = 0; i < sizeof(keys)/sizeof(Key); i++)
        {
            if(isKeyPressed(keys[i]))
            {
                // key pressed, show light
                BITSET(PORTB, PORTB2);

                // set output compare value to half max
                OCR1AL = keys[i].val;

                // enable sound out (toggle when reaching ocr1a val)
                BITSET(TCCR1A, COM1A0);

                anyPressed = 1;
                break;
            }
        }

        if (!anyPressed)
        {
            // key not pressed, no led
            BITCLR(PORTB, PORTB2);

            // disable sound
            BITCLR(TCCR1A, COM1A0);
        }
    }
}