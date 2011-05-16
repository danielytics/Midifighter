// Analog to Digital Converter routines for DJTechTools MidiFighter.
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
// rgreen 2009-10-17

#include <avr/io.h>
#include "constants.h"
#include "spi.h"
#include "adc.h"

// ADC Functions -----------------------------------------------

// A repetition of the code in spi_setup(), but convenient if you are
// looking to use the ADC stand-alone.
//
void adc_setup(void)
{
    // Turn on the select pins for output to the SPI devices
    DDRB |= ADC_SELECT;

    // Start with the ADC chip disabled (pin high)
    //PORTB |= ADC_SELECT;
	
	spi_install_slave(SPI_SLAVE_ADC, SPI_PORT_B, ADC_SELECT, SPI_LOW_TO_SELECT);
}


// Get the 10-bit value from one of the four analog channels.
// The SPI protocol consists of sending and receiving three bytes:
//
//             S E N D            R E A D
//  byte1  7 6 5 4 3 2 1 0    7 6 5 4 3 2 1 0
//         . . . . . . . 1    . . . . . . . .
//
//  byte2  7 6 5 4 3 2 1 0    7 6 5 4 3 2 1 0
//         S . A B . . . .    . . . . . 0 a b
//
//  byte3  7 6 5 4 3 2 1 0    7 6 5 4 3 2 1 0
//         . . . . . . . .    c d e f g h i j
//
//  S   = select single ended (1) or differential (0) measurement.
//  AB  = the port number, four ports numbered from 0b00 to 0b11.
//  a-j = the 10 bit ADC value, note the leading zero.
//  .   = don't care, do not use, could be anything.
//
uint16_t adc_read(uint8_t channel)
{
    // Disable the LED controller by making sure it's latch is low. The LED
    // and ADC chips share the same SPI data and clock lines, so it's
    // essential to make sure the LED driver is not listening to the data
    // flowing down the SPI bus.
    //PORTB &= ~LED_LATCH;

    // single channel reads only (no differential) and we select the ADC
    // channel here.
    uint8_t byte2 = 0b10000000 | ((channel & 0x03) << 4);

    // Enable the ADC chip by bringing the select line low.
    //PORTB &= ~ADC_SELECT;
	spi_select(SPI_SLAVE_ADC);
	
    // First byte wakes up the chip.
    spi_transmit(0b00000001);
    // Second byte sets up single-channel-read the channel.
    uint8_t topbyte = spi_transmit(byte2);
    // Third byte shifts in the remaining values.
    uint8_t lowbyte = spi_transmit(0b00000000);
    // Put the ADC chip back into hibernation by pulling the select pin high.
    //PORTB |= ADC_SELECT;
	spi_select_none();
	
    // Mask out the "don't care" bits and return the 10-bit value including
    // the leading zero at bit 11.
    return  ((topbyte << 8) | lowbyte) & 0x3ff;
	//return 0;
}
