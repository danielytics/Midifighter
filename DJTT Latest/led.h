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

#ifndef _LED_H_INCLUDED
#define _LED_H_INCLUDED

#include <stdbool.h>

// Import globals -------------------

extern bool g_led_keypress_enable;   // Light the LED when a key is pressed?
extern uint16_t g_led_state;         // Copy of the last led state set
extern uint16_t g_led_groundfx_counter;   // Ground fx MIDI clock counter.

// Basic functions ------------------

void led_setup(void);
void led_set_state(uint16_t new_state);
void led_groundfx_state(bool state);

// Lightshow effects ----------------

void led_count_all_leds(void);

#endif // _LED_H_INCLUDED
