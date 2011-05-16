// Combo key functions for DJTechTools Midifighter
//
//   Copyright (C) 2011 Robin Green
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
// rgreen 2011-03-09

#ifndef _COMBO_H_INCLUDED
#define _COMBO_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

// Types ----------------------------------------------------------------------

// event keyups are DOWN + 5
typedef enum combo_action {
    COMBO_NONE,
    COMBO_A_DOWN = 1,
    COMBO_B_DOWN,
    COMBO_C_DOWN,
    COMBO_D_DOWN,
    COMBO_E_DOWN,
    COMBO_A_RELEASE = 6,
    COMBO_B_RELEASE,
    COMBO_C_RELEASE,
    COMBO_D_RELEASE,
    COMBO_E_RELEASE,
} combo_action_t;

// Functions -------------------------------------------------------------------

void combo_setup(void);
combo_action_t combo_recognize(const uint16_t keydown,
                               const uint16_t keyup,
                               const uint16_t keystate);

// ----------------------------------------------------------------------------

#endif // _COMBO_H_INCLUDED
