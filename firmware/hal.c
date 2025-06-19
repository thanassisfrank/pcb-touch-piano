#include "hal.h"

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