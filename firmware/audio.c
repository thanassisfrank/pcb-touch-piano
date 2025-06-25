#include "audio.h"

// external constants

const wave_t waves[2] = {SQUARE_WAVE, SAW_WAVE};



static wave_t currWave = SQUARE_WAVE;
static volatile uint8_t currNote = 0;
static volatile uint8_t currOctave = 0;

// the current phase when using DDS
// the top 8 bits are used for the LUT
static uint16_t phase = 0;

// tuning words for 
static const uint16_t tuningWordsOct6[12] = {
    17557,
    18601,
    19708,
    20879,
    22121,
    23436,
    24830,
    26306,
    27871,
    29528,
    31284,
    33144,
};

static const uint16_t pitchesOct0[12] = {
    30578,
    28861,
    27241,
    25712,
    24269,
    22907,
    21621,
    20408,
    19262,
    18181,
    17161,
    16198,
};

// run whenever the timer overflows
ISR (TIMER1_OVF_vect)
{
    // nothing needed for square wave
    if (currWave == SQUARE_WAVE) return;

    if (currWave == SAW_WAVE)
    {
        // increment the phase
        phase += tuningWordsOct6[currNote] >> (6 - currOctave);
        // get the next compare value
        OCR1A = phase >> 8;
    }
}


void setWave(wave_t wave)
{
    // uses timer/counter 1 and output compare unit A 
    switch (wave)
    {
        case SQUARE_WAVE:
        default:
        // set clock to clkio (1 MHz)
        BITSET(TCCR1B, CS10);
        BITCLR(TCCR1B, CS11);
        BITCLR(TCCR1B, CS12);
        
        // setup waveform generation mode (mode 4, CTC)
        BITCLR(TCCR1A, WGM10);
        BITCLR(TCCR1A, WGM11);
        BITSET(TCCR1B, WGM12);
        BITCLR(TCCR1B, WGM13);
        
        // disable timer overflow interrupt
        BITCLR(SREG, SREG_I);
        BITCLR(TIMSK1, TOIE1);
        break;
        
        case SAW_WAVE:
        // set clock to clkio (1 MHz)
        BITSET(TCCR1B, CS10);
        BITCLR(TCCR1B, CS11);
        BITCLR(TCCR1B, CS12);

        // setup waveform generation mode (mode 5, 8 bit fast PWM)
        BITSET(TCCR1A, WGM10);
        BITCLR(TCCR1A, WGM11);
        BITSET(TCCR1B, WGM12);
        BITCLR(TCCR1B, WGM13);

        // enable timer overflow interrupt
        BITSET(SREG, SREG_I);
        BITSET(TIMSK1, TOIE1);
        break;
    }

    // set current wave
    currWave = wave;
}

void setNote(uint8_t note, uint8_t octave)
{
    if (currWave == SQUARE_WAVE)
    {
        OCR1A = pitchesOct0[note] >> octave;
        // set compare output mode (toggle when reaching ocr1a val)
        BITSET(TCCR1A, COM1A0);
        BITCLR(TCCR1A, COM1A1);
    }
    else if (currWave == SAW_WAVE)
    {
        // set compare output mode (non-inverting mode)
        BITCLR(TCCR1A, COM1A0);
        BITSET(TCCR1A, COM1A1);
    }

    currNote = note;
    currOctave = octave;
}

void clearNotes()
{
    // disable output
    BITCLR(TCCR1A, COM1A0);
    BITCLR(TCCR1A, COM1A1);
}