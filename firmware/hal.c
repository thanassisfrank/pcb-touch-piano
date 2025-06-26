#include "hal.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// collections of registers for each io port
io_port_t PORT_B = {&DDRB, &PORTB, &PINB};
io_port_t PORT_C = {&DDRC, &PORTC, &PINC};
io_port_t PORT_D = {&DDRD, &PORTD, &PIND};
io_port_t PORT_E = {&DDRE, &PORTE, &PINE};


void chargePinAndTransfer(const io_pin_t* pin, const unsigned int waitCycles)
{
    // alias needed registers
    uint8_t* dataReg = pin->port->dataReg;
    uint8_t* dirReg = pin->port->dirReg;

    // create the new reg vals
    uint8_t dataRegCharge = *dataReg;
    BITSET(dataRegCharge, pin->num);

    uint8_t dirRegHiZ = *dirReg;
    BITCLR(dirRegHiZ, pin->num);

    uint8_t dataRegTransfer = *dataReg;
    BITCLR(dataRegTransfer, pin->num);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // charge external pin
        *dataReg = dataRegCharge;
        *dirReg = dirRegHiZ;
    
        // wait a number of cycles
        for (unsigned int i = 0; i < waitCycles; i++)
        {
            asm("nop");
        }
    
        // transfer charge gathered to pin capacitance
        *dataReg = dataRegTransfer;
    }
}

void enablePinInterupt(const uint8_t num)
{
    if (num <= 7)
    {
        BITSET(PCICR, PCIE0);
        BITSET(PCMSK0, num);
    }
    else if (num >= 8 && num <= 14)
    {
        BITSET(PCICR, PCIE1);
        BITSET(PCMSK1, num - 8);
    }
    else if (num >= 16 && num <= 23)
    {
        BITSET(PCICR, PCIE2);
        BITSET(PCMSK2, num - 16);
    }
}