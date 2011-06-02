// LED functions for DJTechTools Midifighter
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

#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "led.h"
#include "spi.h"
#include "constants.h"

// Global variables ------------------------------------------------------------

bool g_led_keypress_enable = false;  // light the LED when a key is pressed?
uint16_t g_led_midi_state = 0x0000;  // Persistent LED state from midi commands.
uint16_t g_led_state = 0x0000;       // Current LED state, copied from last
                                     // call to led_set_state()
uint16_t g_led_groundfx_counter = 0; // Counter for the ground FX MIDI clock.

// Basic functions -------------------------------------------------------------

// setup the LEDS for writing.
//
void led_setup(void)
{
    // Yes, this inits the SPI master device again. We made sure it is safe
    // to do so. just in case you're only playing with the lights.
    spi_setup();
    // Initialize the LED state tracking, so we only send LED updates when
    // something has changed.
    g_led_state = 0x0000;
    g_led_midi_state = 0x0000;
    // Set the LED_BLANK pin to be an output.
    DDRB |= LED_BLANK;
    // Set LED_BLANK low and keep it there.
    PORTB &= ~LED_BLANK;

    // set up the Ground Effects pin to output.
    DDRD |= _BV(PD0);
}

// Set each LEDs on or off state from a 16-bit value.
//
//    LED d1 = bit 1
//    LED d2 = bit 2
//
// etc, in the pattern:
//
//    1  2  3  4
//    5  6  7  8
//    9 10 11 12
//   13 14 15 16
//
void led_set_state(uint16_t new_state)
{
    // If no lights have changed, transmit nothing. This saves bandwidth on
    // the SPI bus for more important things.
    if (g_led_state == new_state) return;
    // Transmit Most Significant Byte first.
    spi_transmit(new_state >> 8);
    spi_transmit(new_state & 0xff);
    // Latch the result to the LEDs by pulsing high.
    PORTB |= LED_LATCH;
    PORTB &= ~LED_LATCH;
    // record the state.
    g_led_state = new_state;
}

// Turn on or off the Ground Effects LED.
void led_groundfx_state(bool state)
{
    if (state) {
        PORTD |= _BV(PD0);
    } else {
        PORTD &= ~_BV(PD0);
    }
}

// Lightshow effects -----------------------------------------------------------

// Turn each LED on for a short time, one by one.  Note the blocking
// delays. Not designed to be used in any situation that requires immediate
// response.
//
void led_count_all_leds(void)
{
    const int kDelay = 60;
    // Run through the LEDs turning them on one by one.
    uint16_t value = 0;
    for(uint8_t i=0; i<16; ++i) {
        value = 1<<i;
        led_set_state(value);
        // This is a blocking wait, so don't use this function if you need
        // timely USB service. It's purely decorative.
        _delay_ms(kDelay);
    }
}

// -----------------------------------------------------------------------------
