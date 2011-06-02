// Definitions and function for running the expansion port.
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
// rgreen 2009-11-24

#include <avr/io.h>
#include "constants.h"
#include "expansion.h"

// TRAKTOR_H
// ---------
//
//                  USB
//                   |
//  +----------------#---------------+
//  |                   sw0          |
//  |  (R7)  (R8)  [s1]  O  O  O  O  |
//  |                        ^       |
//  |   |     |    [s2]  O  O| O  O  |
//  |   =     =              |       |
//  |   |     |    [s3]  O  O| O  O  |
//  |   R5    R6             |   sw16|
//  |   |     |    [s4]  O  O  O  O  |
//  |                                |
//  +--------------------------------+

// TRAKTOR_V
// ---------
//
//      +-----------------+
//      |  sw0            |
//      |   O  O  O  O    |
//      |                 |
//      |   O  O  O  O    |
//      |     <------     |
//      |   O  O  O  O    |
//  U   |           sw16  |
//  S ==#   O  O  O  O    |
//  B   |                 |
//      | [1] [2] [3] [4] |
//      |                 |
//      | (8)  --|---R6-  |
//      |                 |
//      | (7)  --|---R5-  |
//      |                 |
//      +-----------------+

// SERATO
// ------
//
//     +-----------------+
//     | R   sw   sw  R  |
//     |(12)[11] [09](16)|
//     |                 |
//     |                 |
//     | R   sw   sw  R  |
//     |(14)[12] [10](15)|
//     |                 |
//     |  sw0            |   U
//     |   O  O  O  O    #== S
//     |                 |   B
//     |   O  O  O  O    |
//     |     ------->    |
//     |   O  O  O  O    |
//     |           sw16  |
//     |   O  O  O  O    |
//     |                 |
//     +-----------------+

// BEATS
// -----
//
//     +-----------------+
//     |  R              |
//     | (12) [7][5]  |  |
//     |      [8][6]  =  |
//     |              |  |
//     |             R10 |
//     |  -|---R11-   |  |
//     |                 |
//     |  sw0            |   U
//     |   O  O  O  O    #== S
//     |                 |   B
//     |   O  O  O  O    |
//     |     ------->    |
//     |   O  O  O  O    |
//     |           sw16  |
//     |   O  O  O  O    |
//     |                 |
//     +-----------------+


// Global values -------------------------------------------------------------

uint8_t g_exp_digital_read;   // 4-bits of "enabled" flags, one for each pin.
uint8_t g_exp_analog_read;    // 4-bits of "enabled" flags, one for each pin.

// Key states for the expansion port inputs.
uint8_t g_exp_key_debounce_buffer[DEBOUNCE_BUFFER_SIZE];
uint8_t g_exp_key_state;
uint8_t g_exp_key_prev_state;
uint8_t g_exp_key_down;
uint8_t g_exp_key_up;

// Array of previous ADC values for the analog reads, each one the full
// 10-bit range so we can track the lower bits and add hysteresis into
// the value changes.
uint16_t g_exp_analog_prev[NUM_ANALOG];


// Functions -----------------------------------------------------------------

// Setup the expansion port for reading.
void exp_setup()
{
    // NOTE: This function assumes that SPI has already been setup.

    // Setup Digital
    // -------------
    // digital pins to inputs and turn on the pull-up resistor.
    DDRD &= ~(EXP_DIGITAL0 + EXP_DIGITAL1 + EXP_DIGITAL2 + EXP_DIGITAL3);
    PORTD |= EXP_DIGITAL0 + EXP_DIGITAL1 + EXP_DIGITAL2 + EXP_DIGITAL3;

    // Setup ADC
    // ---------
    // Turn on the select pins for output to the SPI devices
    DDRB |= ADC_SELECT;
    // Start with the ADC chip disabled (pin high)
    PORTB |= ADC_SELECT;
    // Read the four analog ports to generate an initial set of "previous"
    // values that we will be testing against.
    for (uint8_t i=0; i<NUM_ANALOG; ++i) {
        g_exp_analog_prev[i] = exp_adc_read(i);
    }

}

// This function is designed to be used inside the key-read interrupt
// service routine, so it has to be as fast as possible and make no
// assumptions about the state of any hardware it uses.
//
void exp_buffer_digital_inputs(void)
{
    // Where to write the next value in the ring buffer.
    static uint8_t ext_buffer_pos = 0;
    // Read the input from port D. As there is a single bit for each input,
    // a single read will give us all the bits we need. We shift the bits
    // down to the bottom of the byte and invert them, as an open key should
    // read as a "1" (just like on the main keyboard).
    uint8_t value = (PIND >> 2) ^ 0x0f;
    // Store the new value in our ring buffer and increment the ring buffer
    // offset, wrapping the write position to a point inside the buffer.
    g_exp_key_debounce_buffer[ext_buffer_pos] = value;
    ext_buffer_pos = (ext_buffer_pos + 1) % DEBOUNCE_BUFFER_SIZE;
}

// Generate a debounced read of the digital input ports. For more on how
// this works see comments in "key.c"
//
uint8_t exp_key_read(void)
{
    g_exp_key_state = 0xff;
    for(uint8_t i=0; i<DEBOUNCE_BUFFER_SIZE; ++i) {
        g_exp_key_state &= g_exp_key_debounce_buffer[i];
    }
    return g_exp_key_state;
}

// Calculate the keydown and keyup states for the digital inputs.
//
void exp_key_calc(void)
{
    // Something is different, and it's set in the keystate.
    g_exp_key_down = (g_exp_key_prev_state ^ g_exp_key_state) &
                     g_exp_key_state;
    // Something is different, and it used to be set in the prev state.
    g_exp_key_up = (g_exp_key_prev_state ^ g_exp_key_state) &
                   g_exp_key_prev_state;
    g_exp_key_prev_state = g_exp_key_state;
}


// Analog ---------------------------------------------------------------------

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
uint16_t exp_adc_read(uint8_t channel)
{
    // Disable the LED controller by making sure it's latch is low. The LED
    // and ADC chips share the same SPI data and clock lines, so it's
    // essential to make sure the LED driver is not listening to the data
    // flowing down the SPI bus.
    PORTB &= ~LED_LATCH;

    // single channel reads only (no differential) and we select the ADC
    // channel here.
    uint8_t byte2 = 0b10000000 | ((channel & 0x03) << 4);

    // Enable the ADC chip by bringing the select line low.
    PORTB &= ~ADC_SELECT;
    // First byte wakes up the chip.
    spi_transmit(0b00000001);
    // Second byte sets up single-channel-read the channel.
    uint8_t topbyte = spi_transmit(byte2);
    // Third byte shifts in the remaining values.
    uint8_t lowbyte = spi_transmit(0b00000000);
    // Put the ADC chip back into hibernation by pulling the select pin high.
    PORTB |= ADC_SELECT;
    // Mask out the "don't care" bits and return the 10-bit value including
    // the leading zero at bit 11.
    return ((topbyte & 0x07) << 8) | lowbyte;
}

// ---------------------------------------------------------------------------

#ifdef MIDIFIGHTER_PRO
#warning Setting up Midifighter Pro expansion board LEDs

    // EXTERNAL BANK LEDs
    // ------------------
    // Turn off interrupts because the LEDs and the Key Read lines share the
    // same clock lines. An interrupt in the middle of this sequence will
    // give you corrupt LEDs.
    cli();

    // Latch low
    PORTD &= ~EXP_DIGITAL2;
    // Start with the clock high.
    PORTD &= ~EXP_DIGITAL3;
    // loop over external keys setting their color.
    uint8_t bit = 1;
    for (uint8_t i=0; i<8; ++i) {
        // set a blue LED.
        if (!(g_exp_key_state & bit)) {
            PORTD |= EXP_DIGITAL4;
        } else {
            PORTD &= ~EXP_DIGITAL4;
        }
        // clock the bit into the LED controller.
        PORTD |= EXP_DIGITAL3;
        PORTD &= ~EXP_DIGITAL3;

        // set a white LED
        if (g_exp_key_state & bit) {
            PORTD |= EXP_DIGITAL4;
        } else {
            PORTD &= ~EXP_DIGITAL4;
        }
        // clock the bit into the LED controller.
        PORTD |= EXP_DIGITAL3;
        PORTD &= ~EXP_DIGITAL3;

        // next bit
        bit <<= 1;
    }
    // latch the result with a rising edge
    PORTD |= EXP_DIGITAL2;
    PORTD &= ~EXP_DIGITAL2;

    // Turn interrupts back on now we're leaving the critical section.
    sei();

#endif

// ----------------------------------------------------------------------------
