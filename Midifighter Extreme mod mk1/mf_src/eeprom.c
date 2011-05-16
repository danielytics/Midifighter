// EEPROM functions for DJTechTools Midifighter
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
// rjgreen 2009-05-22

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "led.h"
#include "key.h"
#include "midi.h"
#include "eeprom.h"
#include "selftest.h"
#include "expansion.h"
#include "constants.h"

// EEPROM functions ------------------------------------------------------------

// Write an 8-bit value to EEPROM memory.
//
void eeprom_write(uint16_t address, uint8_t data)
{
    // Wait for completion of previous write
    while(EECR & (1<<EEPE)) {}
    // Set up Address and Data Registers
    EEAR = address & 0x0fff; // mask out 512 bytes
    EEDR = data;
    // This next critical section cannot have an interrupt occur between the
    // two writes or the write will fail and we will lose the information,
    // so disable all interrupts for a moment.
    cli();
    // Write logical one to EEMPE (Master Program Enable) to allow us to
    // write.
    EECR |= (1<<EEMPE);
    // Then within 4 cycles, initiate the eeprom write by writing to the
    // EEPE (Program Enable) strobe.
    EECR |= (1<<EEPE);
    // Reenable interrupts.
    sei();
}

// Read an 8-bit value from EEPROM memory.
//
uint8_t eeprom_read(uint16_t address)
{
    // Wait for completion of previous write
    while(EECR & (1<<EEPE)) {}
    // Set up address register
    EEAR = address;
    // Start eeprom read by writing EERE (Read Enable)
    EECR |= (1<<EERE);
    // Return data from Data Register
    return EEDR;
}


// System functions -----------------------------------------------------------

// Set up the EEPROM system for use and read out the settings into the
// global values.
//
// This includes checking the layout version and, if the version tag written
// to the EEPROM doesn't match this software version we're running, we reset
// the EEPROM values to their default settings.
//
void eeprom_setup(void)
{
    // If our EEPROM layout has changed, reset everything.
    if (eeprom_read(EE_EEPROM_VERSION) != EEPROM_VERSION) {
        eeprom_factory_reset();
    }

    // Read the EEPROM into the global settings.
    g_self_test_passed = eeprom_read(EE_FIRST_BOOT_CHECK);
    g_midi_channel = eeprom_read(EE_MIDI_CHANNEL);
    g_midi_velocity = eeprom_read(EE_MIDI_VELOCITY);
    g_led_keypress_enable = eeprom_read(EE_KEY_KEYPRESS_LED);
    g_key_fourbanks_mode = eeprom_read(EE_KEY_FOURBANKS);
    g_exp_digital_read = eeprom_read(EE_EXP_DIGITAL_ENABLED);
    g_exp_analog_read = eeprom_read(EE_EXP_ANALOG_ENABLED);
}

// Used by the menu system, if we have edited any of the global values then
// save them off to the EEPROM.
//
void eeprom_save_edits(void)
{
    eeprom_write(EE_MIDI_CHANNEL, g_midi_channel);
    eeprom_write(EE_MIDI_VELOCITY, g_midi_velocity);
    eeprom_write(EE_KEY_KEYPRESS_LED, g_led_keypress_enable);
    eeprom_write(EE_KEY_FOURBANKS, g_key_fourbanks_mode);
    eeprom_write(EE_EXP_DIGITAL_ENABLED, g_exp_digital_read);
    eeprom_write(EE_EXP_ANALOG_ENABLED, g_exp_analog_read);
}

// Return the EEPROM values to their factory default values, erasing any
// customizations you may have made. Sorry dude!
//
void eeprom_factory_reset(void)
{
    // NOTE: hardware check on first boot only occurs if the eeprom was zeroed.
    // and not every time we reflash an eeprom. That keeps it a rare event.

    eeprom_write(EE_EEPROM_VERSION,      EEPROM_VERSION); // This layout version
    eeprom_write(EE_FIRST_BOOT_CHECK,    0xff); // No h/w check on first boot
    eeprom_write(EE_MIDI_CHANNEL,        2);    // MIDI channel (3)
    eeprom_write(EE_MIDI_VELOCITY,       127);  // MIDI velocity (127)
    eeprom_write(EE_KEY_KEYPRESS_LED ,   1);    // Light LED of pressed key (on)
    eeprom_write(EE_KEY_FOURBANKS,       FOURBANKS_OFF); // Fourbanks mode (off)
    eeprom_write(EE_EXP_DIGITAL_ENABLED, 0);    // Read from digital (all off)
    eeprom_write(EE_EXP_ANALOG_ENABLED,  0);    // Read from analog (all off)

    // Reset the global variables to their default versions, as they were
    // read with their old values before the factory reset happened and they
    // could get written back if the user leaves menu mode through the exit
    // button.
    g_self_test_passed = 0xff;
    g_midi_channel = 2;
    g_midi_velocity = 127;
    g_led_keypress_enable = 1;
    g_key_fourbanks_mode = FOURBANKS_OFF;
    g_exp_digital_read = 0;
    g_exp_analog_read = 0;

    // Flash to signal success.
    led_set_state(0xffff);
    _delay_ms(100);
    led_set_state(0x0000);
    _delay_ms(100);
    led_set_state(0xffff);
    _delay_ms(100);
}

// -----------------------------------------------------------------------------
