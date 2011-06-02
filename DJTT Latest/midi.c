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

#include <string.h>  // for memset()
#include <avr/pgmspace.h>

#include "constants.h"
#include "usb_descriptors.h"
#include "key.h"
#include "midi.h"

// Global variables ------------------------------------------------------------

// Interface object for the high level LUFA MIDI Class Drivers. This gets
// passed into every MIDI call so it can potentially keep track of many
// interfaces. The Midifighter only needs the one.
static USB_ClassInfo_MIDI_Device_t s_midi_interface = {
    .Config = {
        .StreamingInterfaceNumber = 1,
        .DataINEndpointNumber      = MIDI_STREAM_IN_EPNUM,
        .DataINEndpointSize        = MIDI_STREAM_EPSIZE,
        .DataINEndpointDoubleBank  = false,
        .DataOUTEndpointNumber     = MIDI_STREAM_OUT_EPNUM,
        .DataOUTEndpointSize       = MIDI_STREAM_EPSIZE,
        .DataOUTEndpointDoubleBank = false,
    },
};

USB_ClassInfo_MIDI_Device_t* g_midi_interface_info;

// This table maps key numbers to midi note offsets, to match
// up the notes with Ableton Live drum racks, e.g. key 5 will initially
// send NoteOn G#3 = 44, with the rest of the keypad sending:
//
//     C4  C#4 D4  D#4
//     G#3 A3  A#3 B3
//     E3  F3  F#3 G3
//     C3  C#3 D3  D#3
//
// The table is coded as note offsets so we can re-base the pad at another
// MIDI note, with the default offset being C3 = 36. The "PROGMEM" setting
// forces the table into program memory so it won't take up precious RAM.
//
uint8_t kNoteMap[16] PROGMEM = {
    12, 13, 14, 15,
     8,  9, 10, 11,
     4,  5,  6,  7,
     0,  1,  2,  3,
};


// NOTE(rgreen): These assignments here are debug MIDI values, you should
// never see these, the actual values will be read and set from the EEPROM
// table during startup.
//
uint8_t g_midi_channel = 14;      // MIDI channel to listen and send on (0..15)
uint8_t g_midi_velocity = 74;     // Default velocity for NoteOn (0..127)

// A copy of the most recent velocity for each MIDI note.
uint8_t g_midi_note_state[MIDI_MAX_NOTES];

// Save a little storage by preallocating and reusing space for the MIDI
// event packet.
static MIDI_EventPacket_t midi_event;


// MIDI functions -------------------------------------------------------------

// Initialize the MIDI key state.
void midi_setup(void)
{
    // Set up the global LUFA MIDI class interface pointer.
    g_midi_interface_info = &s_midi_interface;

    // basenote, expnote, channel and velocity have already been set up via
    // the EEPROM settings. Clear the MIDI keystate.
    memset(g_midi_note_state, 0, MIDI_MAX_NOTES);
}

// Append a MIDI note change event (note on or off) to the currently
// selected USB endpoint. If the endpoint is full it will be flushed.
//
//  pitch    Pitch of the note to turn on or off.
//  onoff    True for a NoteOn, false for a NoteOff.
//
// NOTE: The endpoint can contain 64 bytes and each MIDI-USB message is 4
// bytes giving us just enough space to fit in, for example, 16 keydown
// messages.
//
void midi_stream_note(const uint8_t pitch, const bool onoff)
{
    // Each USB-MIDI endpoint can have up to 16 virtual cables each
    // with 16 MIDI channels. Assign everything to cable 0 for now.
    const uint8_t midi_virtual_cable = 0;

    // Check if the message should be a NoteOn or NoteOff event.
    uint8_t command = ((onoff)? 0x90 : 0x80);

    // Assemble a USB-MIDI event packet, remembering to mask off the values
    // to the correct bit fields.
    midi_event.CableNumber = midi_virtual_cable & 0x0f;  //0..15
    midi_event.Command     = command >> 4;   // 0..15
    midi_event.Data1       = command | (g_midi_channel & 0x0f);  // 0..15
    midi_event.Data2       = pitch & 0x7f;   // 0..127
    midi_event.Data3       = g_midi_velocity & 0x7f; // 0..127

    MIDI_Device_SendEventPacket(g_midi_interface_info, &midi_event);
}

// Append a Control Change Event to the currently selected USB Endpoint. If
// the endpoint is full it will be flushed.
//
//  controller   Number of the controller to alter.
//  value        Value to send to the CC.
//
void midi_stream_cc(const uint8_t controller, const uint8_t value)
{
    //  Assign this MIDI event to cable 0.
    const uint8_t midi_virtual_cable = 0;
    const uint8_t command = 0xb0;  // the Channel Change command.

    midi_event.CableNumber = midi_virtual_cable << 4;
    midi_event.Command     = command >> 4;
    midi_event.Data1       = command | (g_midi_channel & 0x0f); // 0..15
    midi_event.Data2       = controller & 0x7f;   // 0..127
    midi_event.Data3       = value & 0x7f;  // 0..127

    MIDI_Device_SendEventPacket(g_midi_interface_info, &midi_event);
}

// Convert a note number (relative to the basenote) to an LED number,
// returning 0xff (high bit set) if the midi note doesn't map to an LED
// number.
//
uint8_t midi_note_to_key(const uint8_t note)
{
    uint8_t relative_note = note - MIDI_BASE_NOTE;
    if (relative_note >= 16) {
        // note is outside of the key range.
        return 0xff;
    }
    return pgm_read_byte(&kNoteMap[relative_note]);
}

// Convert a key number to a note number using the note mapping
// table. Entries in the table are read from Program Memory using the AVR
// pgm_read_byte() function.
// The MIDI basenote will be added later.
//
uint8_t midi_key_to_note(const uint8_t keynum)
{
    return pgm_read_byte(&kNoteMap[keynum]) + MIDI_BASE_NOTE;
}

// Convert a key number to a note number in fourbanks mode.
//
uint8_t midi_fourbanks_key_to_note(const uint8_t keynum)
{
    uint8_t banksize = 0;
    switch(g_key_fourbanks_mode) {
    case FOURBANKS_INTERNAL:
        banksize = 12;
        break;
    case FOURBANKS_EXTERNAL:
        banksize = 16;
        break;
    default:
        banksize = 0;
    }
    uint8_t offset = MIDI_BASE_NOTE + g_key_bank_selected * banksize;
    return pgm_read_byte(&kNoteMap[keynum]) + offset;
}

// Convert a fourbanks note to a key number, taking into account whether we
// have internal or external fourbanks mode.
//
// NOTE(rgreen): this function has no safeguard as to whether the note being
// requested is actually visible on the selected bank. Use with care.
//
uint8_t midi_fourbanks_note_to_key(const uint8_t note)
{
    uint8_t banksize = 0;
    switch(g_key_fourbanks_mode) {
    case FOURBANKS_INTERNAL:
        banksize = 12;
        break;
    case FOURBANKS_EXTERNAL:
        banksize = 16;
        break;
    default:
        banksize = 0;
    }
    uint8_t keynum = note - MIDI_BASE_NOTE - (g_key_bank_selected * banksize);
    return pgm_read_byte(&kNoteMap[keynum]);
}
