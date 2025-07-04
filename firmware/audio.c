#include "audio.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "hal.h"

#define PWM_BITS 6
#define PWM_TOP (2 << PWM_BITS)

// external constants
const wave_t waves[3] = {SQUARE_WAVE, SAW_WAVE, SINE_WAVE};

static wave_t currWave = SQUARE_WAVE;
static volatile uint8_t currNote = 0;
static volatile uint8_t currOctave = 0;

// the current phase when using DDS
// the top 8 bits are used for the LUT
static uint16_t phase = 0;

#if PWM_BITS == 8

static const uint8_t sineWaveTable[] = {
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124,
};

#elif PWM_BITS == 7

static const uint8_t sineWaveTable[] = {
    64, 67, 70, 73, 76, 80, 83, 86, 88, 91, 94, 97, 100, 102, 105, 107, 109, 111, 113, 115, 117, 119, 120, 122, 123, 124, 125, 126, 127, 127, 128, 128, 128, 128, 128, 127, 127, 126, 125, 124, 123, 122, 120, 119, 117, 115, 113, 111, 109, 107, 105, 102, 100, 97, 94, 91, 88, 86, 83, 80, 76, 73, 70, 67, 64, 61, 58, 55, 52, 48, 45, 42, 40, 37, 34, 31, 28, 26, 23, 21, 19, 17, 15, 13, 11, 9, 8, 6, 5, 4, 3, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4, 5, 6, 8, 9, 11, 13, 15, 17, 19, 21, 23, 26, 28, 31, 34, 37, 40, 42, 45, 48, 52, 55, 58, 61
};

#elif PWM_BITS == 6

static const uint8_t sineWaveTable[] = {
    32, 35, 38, 41, 44, 47, 50, 52, 55, 57, 59, 60, 62, 63, 63, 64, 64, 64, 63, 63, 62, 60, 59, 57, 55, 52, 50, 47, 44, 41, 38, 35, 32, 29, 26, 23, 20, 17, 14, 12, 9, 7, 5, 4, 2, 1, 1, 0, 0, 0, 1, 1, 2, 4, 5, 7, 9, 12, 14, 17, 20, 23, 26, 29
};

#endif

// tuning words for 
static const uint16_t tuningWords[12] = {
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
    phase += tuningWords[currNote] >> ((14 - PWM_BITS) - currOctave);

    const uint8_t upperPhase = phase >> (16 - PWM_BITS);

    // nothing needed for square wave
    switch (currWave)
    {
        case SAW_WAVE:
        OCR1A = upperPhase;
        break;

        case SINE_WAVE:
        OCR1A = sineWaveTable[upperPhase];
        break;
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
        BITCLR(TIMSK1, TOIE1);
        break;
        
        case SAW_WAVE:
        case SINE_WAVE:
        // set clock to clkio (1 MHz)
        BITSET(TCCR1B, CS10);
        BITCLR(TCCR1B, CS11);
        BITCLR(TCCR1B, CS12);

        // setup waveform generation mode (mode 5, 8 bit fast PWM)
        // BITSET(TCCR1A, WGM10);
        // BITCLR(TCCR1A, WGM11);
        // BITSET(TCCR1B, WGM12);
        // BITCLR(TCCR1B, WGM13);

        // setup waveform generation mode (mode 1, 8 bit phase correct)
        // BITSET(TCCR1A, WGM10);
        // BITCLR(TCCR1A, WGM11);
        // BITSET(TCCR1B, WGM12);
        // BITCLR(TCCR1B, WGM13);

        // setup waveform generation mode (mode 14, n bit fast PWM)
        BITCLR(TCCR1A, WGM10);
        BITSET(TCCR1A, WGM11);
        BITSET(TCCR1B, WGM12);
        BITSET(TCCR1B, WGM13);

        ICR1 = PWM_TOP;

        // enable timer overflow interrupt
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
    else if (currWave == SAW_WAVE || currWave == SINE_WAVE)
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