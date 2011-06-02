// MIDI send, receive and decode functions for DJTechTools MidiFighter
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

#ifndef _MIDI_H_INCLUDED
#define _MIDI_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/USB/Class/MIDI.h>
#include "constants.h"

// MIDI types ------------------------------------------------------------------

// Type define for a USB-MIDI event packet.
typedef struct
{
    unsigned char Command     : 4; // MIDI command number
    unsigned char CableNumber : 4; // Virtual cable number
    uint8_t Data1; // First byte of data in the MIDI event
    uint8_t Data2; // Second byte of data in the MIDI event
    uint8_t Data3; // Third byte of data in the MIDI event
} USB_MIDI_EventPacket_t;

// MIDI global variables -------------------------------------------------------

extern USB_ClassInfo_MIDI_Device_t* g_midi_interface_info;

extern uint8_t g_midi_channel;
extern uint8_t g_midi_velocity;

// a copy of the most recent velocity for each MIDI note.
extern uint8_t g_midi_note_state[MIDI_MAX_NOTES];

// MIDI function prototypes ----------------------------------------------------

void midi_setup(void);
void midi_stream_note(const uint8_t pitch, const bool onoff);
void midi_stream_cc(const uint8_t controller, const uint8_t value);
uint8_t midi_note_to_key(const uint8_t notenum);
uint8_t midi_key_to_note(const uint8_t keynum);
uint8_t midi_fourbanks_key_to_note(const uint8_t keynum);
uint8_t midi_fourbanks_note_to_key(const uint8_t note);

#endif // _MIDI_H_INCLUDED
