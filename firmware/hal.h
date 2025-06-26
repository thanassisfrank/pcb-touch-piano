#pragma once

#include <stdint.h>

// macros =======================================

// reg macros
#define BITSET(r, b) (r) |= _BV(b)
#define BITCLR(r, b) (r) &= ~_BV(b)

#define ISSET(r, b) ((r) & _BV(b)) != 0
#define ISCLR(r, b) ((r) & _BV(b)) == 0

// error ========================================

typedef enum {
    ERR_NONE = 0,
    ERR_INVALID_PIN = 1,
} hal_err_t;

// io pins ======================================

typedef struct {
    volatile uint8_t* dirReg;
    volatile uint8_t* dataReg;
    volatile uint8_t* readReg;
} io_port_t;

// pre-defined io ports
extern io_port_t PORT_B;
extern io_port_t PORT_C;
extern io_port_t PORT_D;
extern io_port_t PORT_E;

typedef struct {
    io_port_t* port;
    uint8_t num;
} io_pin_t;

typedef enum {
    LOW=0,
    HIGH=1,
} pin_data_t;

typedef enum {
    INPUT=0,
    OUTPUT=1,
} pin_dir_t;


#define SET_PIN_DATA(pin, val) ( \
    (val == LOW) ? (BITCLR(*((pin)->port->dataReg), (pin)->num)) : (BITSET(*((pin)->port->dataReg), (pin)->num)) \
)

#define SET_PIN_DIR(pin, dir) ( \
    (dir == INPUT) ? (BITCLR(*((pin)->port->dirReg), (pin)->num)) : (BITSET(*((pin)->port->dirReg), (pin)->num)) \
)

#define GET_PIN_DATA(pin) (ISCLR(*((pin)->port->readReg), (pin)->num))


void chargePinAndTransfer(const io_pin_t*, const unsigned int);

void enablePinInterupt(const uint8_t);