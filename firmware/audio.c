#include "audio.h"

static const uint16_t pitches[12] = {
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

void setWave(wave_t wave)
{
    switch (wave)
    {
        case SQUARE_WAVE:
        default:
        // set clock to clkio (1 MHz)
        BITSET(TCCR1B, CS10);
        
        // setup waveform generation mode (mode 4, CTC)
        BITSET(TCCR1B, WGM12);
        break;
        
        case SAW_WAVE:
        // set clock to clkio (1 MHz)
        BITSET(TCCR1B, CS10);
        // setup waveform generation mode (mode 5, 8 bit fast PWM)
        BITSET(TCCR1A, WGM10);
        BITSET(TCCR1B, WGM12);
    }
}

void setNote(uint8_t note, uint8_t octave)
{
    OCR1A = pitches[note] >> octave;

    // enable sound out (toggle when reaching ocr1a val)
    BITSET(TCCR1A, COM1A0);
}

void clearNotes()
{
    // disable waveform generator
    BITCLR(TCCR1A, COM1A0);
}