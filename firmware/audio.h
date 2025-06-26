#pragma once

#include <stdint.h>

typedef enum {
    SQUARE_WAVE=0,
    SAW_WAVE,
    SINE_WAVE
} wave_t;

extern const wave_t waves[3];

// sets the current wave
void setWave(wave_t);

// sets the currently played note
void setNote(uint8_t, uint8_t);

// stop playing note
void clearNotes();