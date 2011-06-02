// Key reading functions for DJTechTools Midifighter
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "key.h"
#include "constants.h"
#include "expansion.h"

// Globals ---------------------------------------------------------------------

uint8_t g_key_fourbanks_mode;  // Which four banks mode are we using?
uint8_t g_key_bank_selected;   // Which bank is currently active?

uint16_t g_key_debounce_buffer[DEBOUNCE_BUFFER_SIZE]; // The debounce buffer
uint16_t g_key_state = 0;      // Current state of the keys after debounce.
uint16_t g_key_prev_state = 0; // State of the keys when last polled.
uint16_t g_key_up = 0;         // Key was released since last poll.
uint16_t g_key_down = 0;       // Key was pressed since last poll.


// Key Functions --------------------------------------------------

// Setup the keys IO ports for reading and set up the timer interrupt
// for servicing the debounce algorithm.
//
void key_setup(void)
{
    // Data Direction Register C
    //   (PC4) = input for switches 9-16
    //    PC5  = clock, shared by both chips.
    //   (PC6) = input for switches 1-8
    //    PC7  = read latch, shared by both chips.
    DDRC = KEY_CLOCK + KEY_LATCH;

    // Parallal Load (PL) on the chips is triggered low, so start it off
    // high and make sure we return it there after use. The clock pulse (CP)
    // should be high before latching (according to the datasheet), so we
    // also start it off high.
    PORTC |= KEY_LATCH & KEY_CLOCK;

    // Start the debounce buffer in an empty state.
    memset(g_key_debounce_buffer, 0, DEBOUNCE_BUFFER_SIZE*sizeof(uint16_t));

    // Setup TIMER0 to trigger an overflow interrupt 1000 times a second.
    // Our counter is incremented every 256 / 16000000 = 0.000016 seconds.
    // Our key debounce buffer is 10 samples, meaning we need the counter
    // to count to x where (256*10*x)/16000000 = 0.001 seconds. This gives us
    // A counter value of 62.5 (rounded to 62), but the counter increments
    // from a base number to the overflow point at 255 so we need to set
    // the counter to (255 - 62) = 193 = 0xC1.
    //
    // Strictly, we're polling the buttons at 1008Hz, but who's counting...

    // Set the Timer0 prescaler to clock/256.
    TCCR0B |= _BV(CS02);
    // Setup Timer0 to count up from 192 (see calculations above).
    TCNT0 = 0xC1;
    // Set the Timer0 Overflow Interrupt Enable bit.
    TIMSK0 |= _BV(TOIE0);
    // Enable all interrupts.
    sei();

    // setup the global key state variables to empty values.
    g_key_state = 0;
    g_key_prev_state = 0;
    g_key_up = 0;
    g_key_down = 0;

    // If we're in fourbanks mode, start up with bank 0.
    g_key_bank_selected = 0;
}

// Disable the timer interrupt. This is needed during teardown before
// entering the bootloader.
//
void key_disable(void)
{
    // Zero out the Timer0 Overflow Interrupt Enable bit so interrupts will
    // no longer be generated.
    TIMSK0 &= ~(_BV(TOIE0));
}


// The key read Interrupt Service Routine (ISR). This is called at 1008Hz
// by Timer0 overflow interrupt and used to poll the key states and drop
// the results into a ring buffer for later analysis by the key debouncer.
//
// The two 74HC165 chips share use the same clock (PC5) and latch lines (PC7),
// so each clock we have to pick up two bits of value, on pins PC4 (SW9 to
// SW16) and PC6 (SW1 to SW8)
//
ISR(TIMER0_OVF_vect)
{
    // Where to write the next value in the ring buffer.
    static uint8_t buffer_pos = 0;

    // The counter just overflowed, so reset the counter to the magic number
    // 193 (see above).
    TCNT0 = 0xC1;
    // Latch the key read (active LOW, reset to HI).
    PORTC &= ~KEY_LATCH;
    PORTC |= KEY_LATCH;
    // Latching the inputs also presented the first
    // bit to the output pin. Shift the captured bits back to the CPU.
    uint16_t value = 0;
    for (uint8_t i=0; i<8; ++i) {
        PORTC &= ~KEY_CLOCK;
        // Read Port C Input, remembering that open keys read as a "1" so we
        // will have to invert the read values. Each read of PINC will give
        // us a single bit of input from each key buffer.
        value |= (PINC & KEY_LOBIT) ? 0 : (1 << (7-i));
        value |= (PINC & KEY_HIBIT) ? 0 : (1 << (15-i));
        PORTC |= KEY_CLOCK; // clock works on a rising edge
    }
    // Store the new value in our ring buffer and increment the ring buffer
    // offset, wrapping the write position to a point inside the buffer.
    g_key_debounce_buffer[buffer_pos] = value;
    buffer_pos = (buffer_pos + 1) % DEBOUNCE_BUFFER_SIZE;

    // If we have enabled the digital inputs, read them into their debounce
    // buffer.
    if (g_exp_digital_read) {
       exp_buffer_digital_inputs();
    }
}

// Read the current keystate by reconstructing the key samples from the
// debounce buffer by ANDing together all the samples. Each bit represents a
// single sample of one key, so the columns line up to represent that state
// of a single key over time. The result of this read is stored in the
// global variable "g_key_state".
//
// NOTE: If any bit is set in the column, the key is considered "activated",
// so this debouncer is good at recognizing bounces on press/release but
// doesn't filter or surpress transitory EMF bits. If you've got EMF bits
// being triggered on a Midifighter, you have bigger problems than
// debouncing. For more, read Jack Ganssle's short guide to debounce
// algorithms:
//
//     http://www.ganssle.com/debouncing.pdf
//
uint16_t key_read(void)
{
    // Debounce the keys by ANDing the columns of key samples together.
    g_key_state = 0xffff;
    for(uint8_t i=0; i<DEBOUNCE_BUFFER_SIZE; ++i) {
        g_key_state &= g_key_debounce_buffer[i];
    }
    return g_key_state;
}

// Update the key up and key down global variables. We need to separate this
// from reading the key state as we may read the state many times to update
// the LEDs while waiting for a USB endpoint to become available.
//
// We only regenerate these key diffs just before we are about to send MIDI
// events, as these transitions are the events we are looking for.
//
void key_calc(void)
{
    // If a bit has changed, and it is 1 in the current state, it's a KeyDown.
    g_key_down = (g_key_prev_state ^ g_key_state) & g_key_state;
    // If a bit has changed and it was 1 in the previous state, it's a KeyUp.
    g_key_up = (g_key_prev_state ^ g_key_state) & g_key_prev_state;
    // Demote the current state to history.
    g_key_prev_state = g_key_state;
}
