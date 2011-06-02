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
    //
    // All others are input.
    //    (PB3) MISO - SPI data input
    //
    DDRB |= SPI_CLOCK + SPI_MOSI + LED_LATCH;

    // Turn on the select pins for the SPI devices
    DDRB |= ADC_SELECT;

    // Start with the ADC chip disabled (pin high)
    PORTB |= ADC_SELECT;

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

// -----------------------------------------------------------------------------
