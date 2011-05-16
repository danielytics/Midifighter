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

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
#include "led.h"
#include "key.h"
#include "midi.h"
#include "eeprom.h"
#include "expansion.h"
#include "constants.h"

// Globals ---------------------------------------------------------------------

bool g_self_test_passed = false;


// Forward declarations --------------------------------------------------------

bool test_key_and_led_toggle(void);
bool test_read_expansion(void);
bool test_read_adc(void);


// Hardware Testing functions --------------------------------------------------

// Run the hardware self tests on first time power up.
//
void self_test(void)
{
    bool passed = true;
    passed = passed && test_key_and_led_toggle();
    passed = passed && test_read_expansion();
    passed = passed && test_read_adc();

    // If any of the tests were failed, make it obvious by slowly flashing
    // the lights forever. Next boot, we will enter the tests all over
    // again.
    if(!passed) {
        // an infinite loop.
        for(;;) {
        led_set_state(0x0000);
        _delay_ms(200);
        led_set_state(0xffff);
        _delay_ms(200);
        }
    }

    // All the tests were passed, so set the EEPROM byte that says we never
    // have to do these again.
    eeprom_write(EE_FIRST_BOOT_CHECK, 0xff);
}

bool test_key_and_led_toggle(void)
{
    // Keep reading keys and, if we have a keydown, turn off the light on
    // that key. Once all keys have been pressed once the test is finished.
    uint16_t pressed = 0xffff;
    while(pressed != 0x0000) {
        led_set_state(pressed);
        key_read();
        key_calc();
        if (g_key_down) {
            pressed &= ~g_key_down;
        }
    }

    // Flash to signal success.
    led_set_state(0xffff);
    _delay_ms(100);
    led_set_state(0x0000);
    _delay_ms(100);
    led_set_state(0xffff);
    _delay_ms(100);

    return true;
}

bool test_read_expansion(void)
{
    // TODO(rgreen) Add this test once we have the hardware test rig.
    return true;
}

bool test_read_adc(void)
{
    // TODO(rgreen) Add this test once we have the hardware test rig.
    return true;
}

// -----------------------------------------------------------------------------
