#include "hal.h"

// collections of registers for each io port
io_port_t PORT_B = {&DDRB, &PORTB, &PINB};
io_port_t PORT_C = {&DDRC, &PORTC, &PINC};
io_port_t PORT_D = {&DDRD, &PORTD, &PIND};
io_port_t PORT_E = {&DDRE, &PORTE, &PINE};