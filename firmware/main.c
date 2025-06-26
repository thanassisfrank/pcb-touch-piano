#define __AVR_ATmega328PB__
// tell delay.h the frequency of the CPU
// this is the defualt value from the internal RC osc
#define F_CPU 1000000UL

#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))

#include <stddef.h>
#include <stdbool.h>


#include <util/delay.h>
#include "hal.h"
#include "audio.h"

const io_pin_t keyPins[12] = {
    (io_pin_t){&PORT_C, 0},
    (io_pin_t){&PORT_C, 1},
    (io_pin_t){&PORT_C, 2},
    (io_pin_t){&PORT_C, 3},
    (io_pin_t){&PORT_C, 4},
    (io_pin_t){&PORT_C, 5},

    (io_pin_t){&PORT_D, 0},
    (io_pin_t){&PORT_D, 1},
    (io_pin_t){&PORT_D, 2},
    (io_pin_t){&PORT_D, 3},
    (io_pin_t){&PORT_D, 4},

    (io_pin_t){&PORT_E, 0},
};

volatile uint8_t octave = 4;
volatile uint8_t waveIndex = 0;


const io_pin_t octDownPin    = {&PORT_D, 5};
const io_pin_t octUpPin      = {&PORT_D, 6};
const io_pin_t waveChangePin = {&PORT_D, 7};

// track bits of pins that have been pressed
uint8_t pressedBits = 0;
// initialise to all ones
uint8_t lastPortDBits = 0xff;
ISR (PCINT2_vect)
{
    // 1 if bits different
    uint8_t changedBits = PIND ^ lastPortDBits;
    // 1 if bits different and currently low
    pressedBits |= changedBits & ~PIND;
    // save this bits state
    lastPortDBits = PIND;
}


bool checkKeyStates(io_pin_t* keys, size_t pinCount, size_t* pressedIndex)
{
    for (size_t i = 0; i < pinCount; i++)
    {
        if(!isKeyPressed(&(keys[i]))) continue;
        
        // this key is pressed
        *pressedIndex = i;
        return true;
    }

    return false;
}

int isKeyPressed(const io_pin_t* key)
{

    // charge key
    chargePinAndTransfer(key, 0);

    // equalise charges for short period
    _delay_us(70);

    // check internal cap state
    const int pressed = GET_PIN_DATA(key);

    // reset pin to default, disharged state

    // reset to default state (key cap -> 0V)
    // set pin to sink current
    SET_PIN_DATA(key, LOW);
    // set key 0 pin to output
    SET_PIN_DIR(key, OUTPUT);

    return pressed;
}


int main (void)
{
    // setup pwm ==============================================
    io_pin_t soundPin = {&PORT_B, 1};
    // set pin to output
    SET_PIN_DIR(&soundPin, OUTPUT);

    // setup waveform generator
    setWave(waves[waveIndex]);

    // set led pin to output =================================
    io_pin_t LEDPin = {&PORT_B, 2};
    SET_PIN_DIR(&LEDPin, OUTPUT);

    // enable buttons ========================================
    // configure as inputs
    SET_PIN_DIR(&octDownPin, INPUT);
    SET_PIN_DIR(&octUpPin, INPUT);
    SET_PIN_DIR(&waveChangePin, INPUT);

    // enable pullups
    SET_PIN_DATA(&octDownPin, HIGH);
    SET_PIN_DATA(&octUpPin, HIGH);
    SET_PIN_DATA(&waveChangePin, HIGH);

    // setup interrupts
    enablePinInterupt(21);
    enablePinInterupt(22);
    enablePinInterupt(23);

    // enable interrupts globally ============================
    sei();

    // begin loop ============================================

    size_t pressedIndex;

    while(1) 
    {
        _delay_ms(100);

        // handle button interrupts that have happened
        if(ISSET(pressedBits, octDownPin.num))
        {
            if (octave > 0) octave--;
        }

        if(ISSET(pressedBits, octUpPin.num))
        {
            octave++;
        }

        if(ISSET(pressedBits, waveChangePin.num))
        {
            waveIndex = (waveIndex + 1) % ARRAY_LEN(waves);

            // do any setup needed for new wavform generator
            setWave(waves[waveIndex]);
        }

        pressedBits = 0;

        if (checkKeyStates(keyPins, ARRAY_LEN(keyPins), &pressedIndex))
        {
            // key pressed, show light
            SET_PIN_DATA(&LEDPin, HIGH);

            // play the correct pitch
            setNote(pressedIndex, octave);
        }
        else
        {
            // key not pressed, no led
            SET_PIN_DATA(&LEDPin, LOW);

            clearNotes();
        }
    }
}