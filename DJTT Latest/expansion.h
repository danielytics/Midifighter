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

#ifndef _EXPANSION_H_INCLUDED
#define _EXPANSION_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

// global values -------------------------------------------------------------

#ifdef MULTIPLEX_ANALOG
   #define NUM_ANALOG 11
#else
   #define NUM_ANALOG 4
#endif

extern uint8_t g_exp_digital_read;
extern uint8_t g_exp_analog_read;

// Key states for the expansion port inputs.
extern uint8_t g_exp_key_debounce_buffer[];
extern uint8_t g_exp_key_state;
extern uint8_t g_exp_key_prev_state;
extern uint8_t g_exp_key_down;
extern uint8_t g_exp_key_up;

// Array of previous ADC values for the analog reads, each one the full
// 10-bit range.
extern uint16_t g_exp_analog_prev[NUM_ANALOG];

// functions -----------------------------------------------------------------

void exp_setup(void);

void exp_buffer_digital_inputs(void);
uint8_t exp_key_read(void);
void exp_key_calc(void);

uint16_t exp_adc_read(uint8_t channel);

// ---------------------------------------------------------------------------

#endif // _EXPANSION_H_INCLUDED
