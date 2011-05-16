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

#ifndef _KEY_H_INCLUDED
#define _KEY_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "constants.h"

// Extern Globals --------------------------------------------------------------

// Which fourbanks mode - internal, external or off?
extern uint8_t g_key_fourbanks_mode;

// Which bank is currently active (if we're in fourbanks mode)?
// Range is 0..3
extern uint8_t g_key_bank_selected;

// The key debounce buffer.
extern uint16_t g_key_debounce_buffer[DEBOUNCE_BUFFER_SIZE];

// The key states (after debounce).
extern uint16_t g_key_state;      // Current state of the keys.
extern uint16_t g_key_prev_state; // State of the keys when last polled.
extern uint16_t g_key_up;         // Key was released since last poll.
extern uint16_t g_key_down;       // Key was pressed since last poll.

// Interrupt service routine ---------------------------------------------------

ISR(TIMER0_OVF_vect);

// Key functions ---------------------------------------------------------------

void key_setup(void);
void key_disable(void);
uint16_t key_read(void);
void key_calc(void);

#endif // _KEY_H_INCLUDED
