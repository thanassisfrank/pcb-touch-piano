#pragma once

typedef struct {
    volatile uint8_t* dirReg;
    int dirBit;
    volatile uint8_t* dataReg;
    int dataBit;
    volatile uint8_t* readReg;
    int readBit;
    unsigned int val;
} Key;