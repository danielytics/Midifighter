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
#include "adc.h"
#include "constants.h"
#include "expansion.h"

// This file contains only one of may ways of using the expansion port. In
// this case we are simply treating each of the digital ports as input keys.
//
// There are many other ways of using these pins as they are connected to
// the UART which can also be used as an SPI master (pins D2,D3 and D5).


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

    // Setup the digital pins as inputs and turn on the pull-up resistor.
    DDRD &= ~(EXP_DIGITAL0 + EXP_DIGITAL1 + EXP_DIGITAL2 + EXP_DIGITAL3);
    PORTD |= EXP_DIGITAL0 + EXP_DIGITAL1 + EXP_DIGITAL2 + EXP_DIGITAL3;

    // Read the four analog ports to generate an initial set of "previous"
    // values that we will be testing against.
    for (uint8_t i=0; i<NUM_ANALOG; ++i) {
        g_exp_analog_prev[i] = adc_read(i);
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

// ---------------------------------------------------------------------------
