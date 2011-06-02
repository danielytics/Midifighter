// Interactive menu routines for DJTechTools Midifighter
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
// rgreen 2009-07-07

#include <stdint.h>
#include "key.h"
#include "led.h"
#include "midi.h"
#include "menu.h"
#include "eeprom.h"
#include "expansion.h"

// The menu system.
//
// Activate the menu system by holding down the top-left key (key 0) while
// booting. You will be presented with seven lights on the top row which are
// he seven options available. More details in the user manual documentation.
//
// Programming the LEDs and keys for a UI is a little tricky as the key
// input bits do not match the keyboard layout in the way you might think
// they do. The bit pattern that you will get from reading the key inputs is
// reversed to the physical layout. Bit 1, the least significant bit (lsb),
// is the top-left key, and subsequent keys read left-to-right while their
// bit patters read right-to left in bitwise significance.
//
//    0  1  2  3
//    4  5  6  7   =>  bit |15|14|13|12| |11|10|9|8| |7|6|5|4| |3|2|1|0|
//    8  9 10 11   =>  hex 0xABCD
//   12 13 14 15
//
// So, when you want to light an LED on the top row it's found in the least
// significant nibble, the "D" of the hex number. Bottom row lights are
// located in the most significant nibble, the "A" in the hex value. If you
// want to write a bit pattern from a value (like the global MIDI channel
// "g_midi_channel") you'll need to reverse the bits before you position it
// into the display to keep the usual right-to-left reading of bit patterns.
//
// To add a bit of visual interest I added a simulated Pulse Wave Modulation
// (PWM) on some of the LED values. Turning the LEDs on and off quickly
// makes them appear dimly lit, so we get the effect of varying the LED
// intensity. The LED is on for two beats and off for the rest of the period.
//
// This is pretty much the least well documented code in the entire system,
// but it's a pretty basic Finite State Machine with a dispatch routine at
// the top that gets called in an endless loop until the "finished" flag is
// set by the top level menu. Because if this repeated polling at high
// frequency the menu items should only react to keydown messages.
//
// NOTE: persistent values are only written if you exit through the
// top-level "exit" button. If you reset before using that key your changes
// will not be stored. This is to allow you to back out of a bad editing
// session.
//
// - Fatlimey


// Constant Tables and Static Globals -----------------------------------------

typedef enum menu_state {
    TOP_LEVEL,
    CHANNEL,
    VELOCITY,
    BASENOTE,
    KEYPRESS_LED,
    FOUR_BANKS,
    READ_DIGITAL,
    READ_ANALOG,
} menu_state;

// Which menu page is currently active.
static menu_state g_menu_state = TOP_LEVEL;

// The global flashing light timer and mask. The counter is incremented
// every time through the menu loop and when it flips over to zero the mask
// is inverted.  Lights that should be flashing can be ORed into the light
// state to get the correct flashing effect, e.g.
//
//     uint16_t fixed = 0x0300;
//     uint16_t flashing = 0x000C0;
//     uint16_t half = 0x6000;
//     uint16_t lights = fixed | (flashing & flash_mask) | (half & half_mask);
//
// gives us:
//
//   . . . .
//   # # . .
//   . . * *
//   . o o .
//
//  where "*" are fixed, "#" are flashing and "o" are half intensity lights.
//
static uint16_t dim_mask = 0xffff;
static uint16_t half_mask = 0xffff;
static uint16_t flash_mask = 0xffff;
static uint16_t flash_counter = 4096; // the flash period

// Prototypes ------------------------------------------------------------------

void run_empty(const uint8_t menu_item);
void run_bool_toggle(bool *value, const uint16_t menu_item);
void run_4bit_toggle(uint8_t *value, const uint16_t menu_item);
void run_8bit_toggle(uint8_t *value, const uint16_t menu_item);
void run_4bit_value(uint8_t *value, const uint16_t menu_item);
void run_7bit_value(uint8_t *value, const uint16_t menu_item);

void menu(void);
bool menu_top_level(void);
void menu_channel(void);
void menu_velocity(void);
void menu_basenote(void);
void menu_keypress_led(void);
void menu_fourbanks_mode(void);
void menu_read_digital(void);
void menu_read_analog(void);

// Functions -------------------------------------------------------------------

inline uint8_t MIN(const uint8_t a, const uint8_t b) { return a<b ? a : b ; }
inline uint8_t MAX(const uint8_t a, const uint8_t b) { return a>b ? a : b ; }

uint8_t REVERSE_BYTE(uint8_t x)
{
    // This looks like ugly C, but it compiles into not bad code on the AVR
    // that uses skip-branches and immediate operands. About the same number
    // of instructions as the classic ROR/ROL bit shuffling technique.
    uint8_t result = 0;
    if (x & (1 << 0)) result |= (0x80 >> 0);
    if (x & (1 << 1)) result |= (0x80 >> 1);
    if (x & (1 << 2)) result |= (0x80 >> 2);
    if (x & (1 << 3)) result |= (0x80 >> 3);
    if (x & (1 << 4)) result |= (0x80 >> 4);
    if (x & (1 << 5)) result |= (0x80 >> 5);
    if (x & (1 << 6)) result |= (0x80 >> 6);
    if (x & (1 << 7)) result |= (0x80 >> 7);
    return result;
}

void run_empty(const uint8_t menu_item)
{
    // Add the flashing exit lights
    uint16_t leds = menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    // Process key presses.
    if (g_key_down & menu_item) {
        // exit key is pressed
       g_menu_state = TOP_LEVEL;
    }
}

void run_bool_toggle(bool *value, const uint16_t menu_item)
{
    // Run a menu page for toggling a single vaue bar.
    //
    //   .  .  *  .    <- menu item
    //   .  .  .  .
    //   #  #  #  #    <- bits to be toggled.
    //   .  .  .  .    <- toggle

    // Add the bar for the value.
    int16_t leds = (*value) ? 0b0000111100000000 : 0x0000;
    // Add in the dim background for the toggle bits
    leds |= 0b0000111100000000 & dim_mask;
    // Add the flashing exit light
    leds |= menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    // Process key presses.
    if (g_key_down & menu_item) {
        // exit key is pressed.
       g_menu_state = TOP_LEVEL;
    } else if (g_key_down & 0b0000111100000000) {
        // if any of the keys inside the bar are pressed, toggle the bar.
        *value = !(*value);
    }
}

void run_4bit_toggle(uint8_t *value, const uint16_t menu_item)
{
    // Run a menu page where 4 bits can be independently toggled.
    //
    //   .  .  *  .    <- menu item
    //   .  .  .  .
    //   #  #  #  #    <- bits to be toggled.
    //   .  .  .  .

    int16_t leds = REVERSE_BYTE(*value) << 4;
    // Add in the dim background for the toggle bits
    leds |= 0b0000111100000000 & dim_mask;
    // Add the flashing exit lights
    leds |= menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    // Process key presses.
    // Multiple key presses are sorted out by importance: Exit buttons
    // first, then increment/decrement, then toggle bits.
    if (g_key_down & menu_item) {
        // exit key is pressed
       g_menu_state = TOP_LEVEL;
    } else if (g_key_down & 0x0f00) {
        // toggle bits using an exclusive-or of the keydown bits.
        uint16_t bits = g_key_down & 0x0f00;
        uint8_t toggle = bits >> 4;
        // Then we reverse the LSB, moving the top 4 bits to the bottom 4.
        *value = *value ^ REVERSE_BYTE(toggle);
    }

}

void run_8bit_toggle(uint8_t *value, const uint16_t menu_item)
{
    // Run a menu page where 4 bits can be independently toggled.
    //
    //   .  .  *  .    <- menu item
    //   .  .  .  .
    //   #  #  #  #    <- bits to be toggled.
    //   #  #  #  #    <- bits to be toggled.

    int16_t leds = REVERSE_BYTE(*value) << 8;
    // Add in the dim background for the toggle bits
    leds |= 0b1111111100000000 & dim_mask;
    // Add the flashing exit lights
    leds |= menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    // Process key presses.
    // Multiple key presses are sorted out by importance: Exit buttons
    // first, then increment/decrement, then toggle bits.
    if (g_key_down & menu_item) {
        // exit key is pressed
       g_menu_state = TOP_LEVEL;
    } else if (g_key_down & 0xffff) {
        // toggle bits using an exclusive-or of the keydown bits.
        uint16_t bits = g_key_down & 0xff00;
        uint8_t toggle = bits >> 8;
        // Then we reverse the LSB, moving the top 4 bits to the bottom 4.
        *value = *value ^ REVERSE_BYTE(toggle);
    }
}

void run_4bit_value(uint8_t *value, const uint16_t menu_item)
{
    // Run a menu page where 4 bits can be incremented/decremented and toggled.
    //
    //   # . . .   <- flashing menu item
    //   . . . .
    //   * * * *   <- channel in binary
    //   o . . o   <- increment/decrement

    int16_t leds = REVERSE_BYTE(*value & 0x0f) << 4;
    // Add in the dim background for the toggle bits
    leds |= 0b0000111100000000 & dim_mask;
    // Add the half bright incr/decr lights
    leds |= 0x9000 & half_mask;
    // Add the flashing exit lights
    leds |= menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    // Process key presses.
    // Multiple key presses are sorted out by importance: Exit buttons
    // first, then increment/decrement, then toggle bits.
    if (g_key_down & menu_item) {
        // exit key is pressed
       g_menu_state = TOP_LEVEL;
    } else if (g_key_down & 0x1000) {
        // increment
        if (*value < 15) *value = *value + 1;
    } else if (g_key_down & 0x8000) {
        // decrement
        if (*value > 0) *value = *value - 1;
    } else if (g_key_down & 0x0f00) {
        // toggle bits using an exclusive-or of the keydown bits.
        uint16_t bits = g_key_down & 0x0f00;
        // shift the 4 bits down into the top 4 bits of the Lower Byte.
        uint8_t toggle = bits >> 4;
        // Then we reverse the LSB, moving the top 4 bits to the bottom 4.
        *value = *value ^ REVERSE_BYTE(toggle);
    }
}

void run_7bit_value(uint8_t *value, const uint16_t menu_item)
{
    // Run a menu page where 4 bits can be independently toggled.
    //
    //   .  .  *  .    <- menu item
    //   .  #  #  #
    //   #  #  #  #    <- bits to be toggled.
    //   o  .  .  o    <- increment/decrement

    // Add the 7-bit basenote value
    int16_t leds = REVERSE_BYTE(*value & 0x7f) << 4;
    // Add in the dim background for the toggle bits
    leds |= 0b0000111111100000 & dim_mask;
    // Add the half bright incr/decr lights
    leds |= 0x9000 & half_mask;
    // Add the flashing exit light
    leds |= menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    if (g_key_down & menu_item) {
        // exit key is pressed.
       g_menu_state = TOP_LEVEL;
    } else if (g_key_down & 0x1000) {
        // increment the basenote.
        if (*value < 127) *value = *value + 1;
    } else if (g_key_down & 0x8000) {
        // decrement the basenote value, stopping at zero.
        if (*value > 0) *value = *value - 1;
    } else if (g_key_down & 0b0000111111100000) {
        // toggle bits using an exclusive-or of the keydown bits.
        uint16_t bits = g_key_down & 0b0000111111100000;
        // shift the bits down into the top 7 bits of the variable.
        uint8_t toggle = bits >> 4;
        // reverse the bits of the byte to end up with the bottom-most 0
        // positioned at the most significant bit.
        *value = *value ^ REVERSE_BYTE(toggle);
    }
}

void run_3value_option(uint8_t *value, uint8_t menu_item)
{
    // menu of the form:
    //
    //   * . . .
    //   . . . .
    //   A A B B
    //   # . . #
    //
    // where A and B are independently toggles, giving four possible values.

    // Calculate and display the current light state.
    //
    // Add the bars for the three options.
    // of the form:
    //  0 =  . . . .
    //  1 =  . . * *
    //  2 =  * * * *
    //
    uint16_t pattern = 0;
    switch (*value) {
    case 0:
        pattern = 0;
        break;
    case 1:
        pattern = 0b0000110000000000;
        break;
    default:
        pattern = 0b0000111100000000;
    }
    int16_t leds = pattern;
    // Add in the dim background for the toggle bits
    leds |= 0b0000111100000000 & dim_mask;
    // Add the halfbright incr/decr lights
    leds |= 0b1001000000000000 & half_mask;
    // Add the flashing exit lights
    leds |= menu_item & flash_mask;
    // Update LEDs.
    led_set_state(leds);

    // Process key presses.
    // Multiple key presses are sorted out by importance: Exit buttons
    // first, then increment/decrement, then toggle bits.
    if (g_key_down & menu_item) {
        // exit key is pressed
       g_menu_state = TOP_LEVEL;
    } else if (g_key_down & 0x1000) {
        // increment
        if (*value < 3) (*value) = (*value) + 1;
    } else if (g_key_down & 0x8000) {
        // decrement
        if(*value > 0) (*value) = (*value) - 1;
    } else if (g_key_down & 0x0f00) {
        // keypress inside the value.
    }

}


// The main menu dispatcher, sending control to the correct routine
// depending on which is the currently active menu page (called the
// "state"). Once it returns the Midifighter can continue boot up.
//
// NOTE: values are not written to the EEPROM until we exit the menu through
// the "exit button". This allows us to panic reset if we screw up.
//
void menu(void)
{
    bool finished = false;

    // Loop until the "menu exit" button has been selected.
    while (!finished) {
        // Read the key state once before dispatching to the current menu
        // handler. No other function updates the key state from now on.
        key_read();
        key_calc();

        // update the flashing light counter and masks.
        --flash_counter;
        if (flash_counter==0) {
            flash_counter = 4096;
            flash_mask = ~flash_mask;
        }

        // Simulate PWM for the dim and half masks.
        if ((flash_counter & 0x3c) == 0) {
            half_mask = 0xffff;
        } else {
            half_mask = 0x0000;
        }

        if ((flash_counter & 0x3fc) == 0) {
            dim_mask = 0xffff;
        } else {
            dim_mask = 0x0000;
        }


        // Dispatch control to the current menu page.
        switch (g_menu_state) {
        case TOP_LEVEL:
            finished = menu_top_level();
            break;
        case CHANNEL:
            menu_channel();
            break;
        case VELOCITY:
            menu_velocity();
            break;
        case BASENOTE:
            menu_basenote();
            break;
        case KEYPRESS_LED:
            menu_keypress_led();
            break;
        case FOUR_BANKS:
            menu_fourbanks_mode();
            break;
        case READ_DIGITAL:
            menu_read_digital();
            break;
        case READ_ANALOG:
            menu_read_analog();
            break;
        }
    }

    // We have exited the menu correctly, write the edited values back to
    // the EEPROM.
    eeprom_save_edits();

    // Everything done, return to the main loop to finish
    // bootup.
    return;
}

// The first menu page, where each flashing light is one of the menu pages.
// Press a lit key to enter that page, press it again to return to the root.
//
bool menu_top_level()
{
    // initial menu lights:
    //
    //   * * * *  <- Menu items
    //   * * * .
    //   . . . .
    //   . . . #  <- Flashing exit menus

    // Update the LED display.
    uint16_t lights = (0x007F & half_mask) | (0x8000 & flash_mask);
    led_set_state(lights);

    // If one of the menu items has been selected, switch the menu state.
    switch (g_key_down) {
    case 0x0001:
        g_menu_state = CHANNEL;
        break;
    case 0x0002:
        g_menu_state = VELOCITY;
        break;
    case 0x0004:
        g_menu_state = BASENOTE;
        break;
    case 0x0008:
        g_menu_state = KEYPRESS_LED;
        break;
    case 0x0010:
        g_menu_state = FOUR_BANKS;
        break;
    case 0x0020:
        g_menu_state = READ_DIGITAL;
        break;
    case 0x0040:
        g_menu_state = READ_ANALOG;
        break;
    case 0x8000:
        // exit button has been pressed.
        return true;
    }
    return false;
}

void menu_channel()
{
    // lights for MIDI channel
    //
    //   # . . .   <- flashing menu item
    //   . . . .
    //   * * * *   <- channel in binary
    //   o . . o   <- increment/decrement

    run_4bit_value(&g_midi_channel, 0x0001);
}

void menu_velocity()
{
    // lights for MIDI velocity
    //
    //   . # . .   <- flashing menu item
    //   . * * *   <- channel in binary 0..127
    //   * * * *
    //   o . . o   <- increment/decrement

    run_7bit_value(&g_midi_velocity, 0x0002);
}

void menu_basenote(void)
{
    // Currently displays nothing.
    //
    //   . . # .   <- flashing menu item
    //   . . . .
    //   . . . .
    //   . . . .

    run_empty(0x0004);
}

void menu_keypress_led()
{
    // Enable or disable led when key is depressed.
    // Defaults to ON.
    //
    //   . . . #   <- flashing menu item
    //   . . . .
    //   * * * *   <- all on or all off
    //   . . . .

    run_bool_toggle(&g_led_keypress_enable, 0x0008);
}

void menu_fourbanks_mode()
{
    // Select Fourbanks Internal, Fourbanks External or Fourbanks off.
    //
    //   . . . .
    //   # . . .  <- flashing menu item
    //   . . * *  <- bar graph
    //   o . . o  <- increment/decrement

    run_3value_option(&g_key_fourbanks_mode, 0x0010);
}

void menu_read_digital()
{
    // Enable reading from the digital pins of the expansion port and
    // generating MIDI from the results.
    //
    //   . . . .
    //   . # . .   <- flashing menu item
    //   * * * *   <- 4 toggle bits
    //   . . . .   <- toggle

    if (g_key_fourbanks_mode == FOURBANKS_EXTERNAL) {
        run_empty(0x0020);
    } else {
        run_4bit_toggle(&g_exp_digital_read, 0x0020);
    }
}

void menu_read_analog()
{
    // Enable reading from the analog pins expansion port through the ADC
    // and generating MIDI from the results.
    //
    //   . . . .
    //   . . # .   <- flashing menu item
    //   * * * *   <- 4 toggle bits
    //   . . . .   <- toggle

    run_4bit_toggle(&g_exp_analog_read, 0x0040);
}
