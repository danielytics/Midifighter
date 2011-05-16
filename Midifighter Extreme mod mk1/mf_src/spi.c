// SPI (Serial Peripheral Interface) routines for DJTechTools Midifighter
//
//   Copyright (C) 2009 Robin Green
//
//   This file is part of the Midifighter Firmware.
//
//   The Midifighter Firmware is free software: you can redistribute it
//   and/or modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation, either version 3 of the
//   License, or (at your option) any later version.
//
//   The Midifighter Firmware is distributed in the hope that it will be
//   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License along
//   with the Midifighter Firmware.  If not, see
//   <http://www.gnu.org/licenses/>.
//
// rgreen 2009-05-22

#include <avr/io.h>
#include "spi.h"
#include "constants.h"


// SPI functions ---------------------------------------------------------------

// Set up the output pins that feed the TLC5924 LED driver, setting the Data
// and Clock line to be driven by the SPI unit. This means the CPU doesn't
// have to bit-bang the clock lines setting the LEDs, we have a dedicated
// unit to drive that for us. We still have to set the chip mode and latch
// the results manually.
//
// TODO(rgreen): Add SPI bus discovery to find out which peripherals have
// been attached.
//
void spi_setup(void)
{
    // Data Direction Register B: enable as output
    //    (PB0) = LED latch (pulse high to latch)
    //    (PB1) SCLK = SPI clock
    //    (PB2) MOSI = SPI data output
    //    (PB5) = ADC chip select (low to enable)
	//	  EXP_DIGITAL3 = PIC chip select (low to enable)
    //
    // All others are input.
    //    (PB3) MISO - SPI data input
    //
    DDRB |= SPI_CLOCK + SPI_MOSI + LED_LATCH;

    // Turn on the select pins for the SPI devices
    //DDRB |= ADC_SELECT;

    // Start with the ADC chip disabled (pin high)
    //PORTB |= ADC_SELECT;
	
	// Start with the PIC chip disabled (pin high)
	//PORTD |= EXP_DIGITAL3
	spi_select_none();

    // SPI Control Register:
    //    SPE  (bit7) = SPI Enable on.
    //    MSTR (bit4) = function as Master.
    //    SPR1 (bit1) = clock rate hi bit (=0)
    //    SPR0 (bit0) = clock rate lo bit to fck/16 (=1)
    SPCR = _BV(SPE) + _BV(MSTR) + _BV(SPR0);
}

// Send an 8-bit value over the SPI bus.
//
// The SPI master will take the contents of the SPDR byte and clock it out
// through the MOSI (Master Out Slave In) pin to the selected device. It
// will then read the state of the MISO pin (Master In Slave Out) and shift
// that into the bottom bit of the SPDR register. After 8 bits of clocking,
// we will have sent one byte and received one byte.
//
uint8_t spi_transmit(uint8_t byte)
{
    // Set SPI Data Register to the value to be transmitted.
    SPDR = byte;
    // While the SPI Status Register doesn't have the SPI Interrupt
    // Flag set, wait.
    while (!(SPSR & _BV(SPIF)));
    // Transmit completed, your input data is now in SPDR
    return SPDR;
}

// Slave map
typedef struct {
	uint8_t enabled : 1;
	uint8_t ss_value : 1;
	uint8_t port : 4;
	uint8_t pin;
} slave_t;
#define MAX_SLAVES  8
slave_t slave_map[MAX_SLAVES] = {{0,0},}; // 8 SPI slaves ought to be enough for everyone ;-)
uint8_t currently_selected = 0xff;

// Install a new SPI slave by assigning a chip select pin
void spi_install_slave (uint8_t id, uint8_t port, uint8_t pin, uint8_t select_with)
{
	slave_map[id].enabled = 1;
	slave_map[id].port = port & 0x7;
	slave_map[id].ss_value = select_with & 0x1;
	slave_map[id].pin = pin;
}

uint8_t spi_is_selected (uint8_t id)
{
	return currently_selected == id;
}

// Deselect all installed SPI slaves by setting their chip selects high
void spi_select_none ()
{
	uint8_t pin;
	uint8_t port_a_set = 0x0;
	uint8_t port_b_set = 0x0;
	uint8_t port_c_set = 0x0;
	uint8_t port_d_set = 0x0;
	
	uint8_t port_a_clr = 0x0;
	uint8_t port_b_clr = 0x0;
	uint8_t port_c_clr = 0x0;
	uint8_t port_d_clr = 0x0;

	for (uint8_t i=0; i < MAX_SLAVES; ++i) {
		if (slave_map[i].enabled) { // Slave is enabled
			pin = slave_map[i].pin;
			switch (slave_map[i].port) {
				case SPI_PORT_A:
					if (slave_map[i].ss_value) {
						port_a_clr |= pin;
					} else {
						port_a_set |= pin;
					}
					break;
				case SPI_PORT_B:
					if (slave_map[i].ss_value) {
						port_b_clr |= pin;
					} else {
						port_b_set |= pin;
					}
					break;
				case SPI_PORT_C:
					if (slave_map[i].ss_value) {
						port_c_clr |= pin;
					} else {
						port_c_set |= pin;
					}
					break;
				case SPI_PORT_D:
					if (slave_map[i].ss_value) {
						port_d_clr |= pin;
					} else {
						port_d_set |= pin;
					}
					break;
				default:
					break;
			};
		}
	}
	
	// Disable SPI slaves
	//PORTA = (PORTA | port_a_set) & ~port_a_clr;
	PORTB = (PORTB | port_b_set) & ~port_b_clr;
	PORTC = (PORTC | port_c_set) & ~port_c_clr;
	PORTD = (PORTD | port_d_set) & ~port_d_clr;
	
	currently_selected = 0xff;
}

// Select the SPI slave by setting its chip select low and making sure all other
// SPI slaves are disabled (chip select high)
void spi_select (uint8_t slave)
{	
	// Deselect all slaves first
	spi_select_none();
	
	// Select desired slave
	// Only select slave if it is enabled in the slave map
	if (slave_map[slave].enabled) {
		uint8_t pin = slave_map[slave].pin;
		switch (slave_map[slave].port) {
			case SPI_PORT_A:
				if (slave_map[slave].ss_value) {
					//PORTA |= pin;
				} else {
					//PORTA &= ~pin;
				}
				break;
			case SPI_PORT_B:
				if (slave_map[slave].ss_value) {
					PORTB |= pin;
				} else {
					PORTB &= ~pin;
				}
				break;
			case SPI_PORT_C:
				if (slave_map[slave].ss_value) {
					PORTC |= pin;
				} else {
					PORTC &= ~pin;
				}
				break;
			case SPI_PORT_D:
				if (slave_map[slave].ss_value) {
					PORTD |= pin;
				} else {
					PORTD &= ~pin;
				}
				break;
			default:
				break;
		};
	}
	currently_selected = slave;
}

// -----------------------------------------------------------------------------
