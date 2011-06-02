// Combo key functions for DJTechTools Midifighter3D
//
//   Copyright (C) 2011 Robin Green
//
// rgreen 2011-03-09

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#include "key.h"
#include "combo.h"

// COMBOS
//
//   A          B           C           D           E
// +--------+ +--------+  +--------+  +--------+  +--------+
// |        | |        |  |        |  |        |  |        |
// |        | |        |  |  3 4   |  |        |  |u       |
// |        | |n n n n |  |  1 2   |  |a b c d |  |l r A B |
// |1 2 3 4 | |        |  |        |  |        |  |d       |
// +--------+ +--------+  +--------+  +--------+  +--------+
//                                     a-b-c-c-d   uuddlrlrBA
//
// Combos retain a NoteOn while the final key is depressed and emit a NoteUp
// when it is released.


// Types ----------------------------------------------------------------------

typedef struct combo_state {
    uint8_t  state_num;        // these rules apply to state N
    uint8_t  key;              // index of the key + which keystate to test
    uint8_t  next_state;       // next state transition
    combo_action_t action;     // action on this transition
} combo_state_t;


// Globals ---------------------------------------------------------------------

// Top two bits of the key index tell us which keystate to test.
#define KEYMASK  0xF0
#define KEYDOWN  0x10
#define KEYUP    0x20

// count and list of active states.
static uint8_t combo_state;

// State Offset Table
// records for state N starts at offset M in the state table.
uint8_t state_offset[] PROGMEM = {
    0,
    4, 5, 6, 7,
    8, 9, 10, 11,
    12, 13, 14,
    16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30,
    32, 33, 34, 35, 36,
    38, 39,
    41, 42, 43
};

// State Transition Table
// Key numbers are:
//     +-----------+
//     | 0  1  2  3|
//     | 4  5  6  7|
//     | 8  9 10 11|
//     |12 13 14 15|
//     +-----------+
//
combo_state_t state_table[] PROGMEM = {
//          state  key           next action
/*  0 */   {  0,   12 | KEYDOWN,   1,  COMBO_NONE },  // combo A
/*  1 */   {  0,    9 | KEYDOWN,   5,  COMBO_NONE },  // combo B
/*  2 */   {  0,    8 | KEYDOWN,   9,  COMBO_NONE },  // combo D
/*  3 */   {  0,    4 | KEYDOWN,  18,  COMBO_NONE },  // combo E

/*  4 */   {  1,   13 | KEYDOWN,   2,  COMBO_NONE },  // combo A
/*  5 */   {  2,   14 | KEYDOWN,   3,  COMBO_NONE },
/*  6 */   {  3,   15 | KEYDOWN,   4,  COMBO_A_DOWN },
/*  7 */   {  4,   15 | KEYUP,     0,  COMBO_A_RELEASE },

/*  8 */   {  5,   10 | KEYDOWN,   6,  COMBO_NONE }, // combo B
/*  9 */   {  6,    5 | KEYDOWN,   7,  COMBO_NONE },
/* 10 */   {  7,    6 | KEYDOWN,   8,  COMBO_C_DOWN },
/* 11 */   {  8,    6 | KEYUP,     0,  COMBO_C_RELEASE },

/* 12 */   {  9,    8 | KEYUP,    10,  COMBO_NONE },  // combo D
/* 13 */   { 10,    9 | KEYDOWN,  11,  COMBO_NONE },
/* 14 */   { 11,    9 | KEYUP,    12,  COMBO_NONE },
/* 15 */   { 11,   10 | KEYDOWN,   6,  COMBO_NONE }, // --> combo C
/* 16 */   { 12,   10 | KEYDOWN,  13,  COMBO_NONE },
/* 17 */   { 13,   10 | KEYUP,    14,  COMBO_NONE },
/* 18 */   { 14,   10 | KEYDOWN,  15,  COMBO_NONE },
/* 19 */   { 15,   10 | KEYUP,    16,  COMBO_NONE },
/* 20 */   { 16,   11 | KEYDOWN,  17,  COMBO_D_DOWN },
/* 21 */   { 17,   11 | KEYUP,     0,  COMBO_D_RELEASE },

/* 22 */   { 18,    4 | KEYUP,    19, COMBO_NONE }, // combo E
/* 23 */   { 19,    4 | KEYDOWN,  20, COMBO_NONE },
/* 24 */   { 20,    4 | KEYUP,    21, COMBO_NONE },
/* 25 */   { 21,   12 | KEYDOWN,  22, COMBO_NONE },
/* 26 */   { 22,   12 | KEYUP,    23, COMBO_NONE },
/* 27 */   { 23,   12 | KEYDOWN,  24, COMBO_NONE },
/* 28 */   { 24,   12 | KEYUP,    25, COMBO_NONE },
/* 29 */   { 25,    8 | KEYDOWN,  26, COMBO_NONE },
/* 30 */   { 26,    8 | KEYUP,    27, COMBO_NONE },
/* 31 */   { 26,   10 | KEYDOWN,   6, COMBO_NONE }, // --> combo C
/* 32 */   { 27,    9 | KEYDOWN,  28, COMBO_NONE },
/* 33 */   { 28,    9 | KEYUP,    29, COMBO_NONE },
/* 34 */   { 29,    8 | KEYDOWN,  30, COMBO_NONE },
/* 35 */   { 30,    8 | KEYUP,    31, COMBO_NONE },
/* 36 */   { 31,    9 | KEYDOWN,  32, COMBO_NONE },
/* 37 */   { 31,   10 | KEYDOWN,  13, COMBO_NONE }, // --> combo D
/* 38 */   { 32,    9 | KEYUP,    33, COMBO_NONE },
/* 39 */   { 33,   11 | KEYDOWN,  34, COMBO_NONE },
/* 40 */   { 33,    9 | KEYDOWN,  11, COMBO_NONE }, // --> combo D
/* 41 */   { 34,   11 | KEYUP,    35, COMBO_NONE },
/* 42 */   { 35,   10 | KEYDOWN,  36, COMBO_E_DOWN },
/* 43 */   { 36,   10 | KEYUP,     0, COMBO_E_RELEASE },

};

// Functions -------------------------------------------------------------------

uint16_t rightmost_bit_16(uint16_t value)
{
    // return the rightmost set bit in a 16-bit value (the bit, not the bit
    // position), or zero of no bits are set.
    uint16_t bit = 0x0001;
    while (bit && (value & bit) == 0) {
        bit <<= 1;
    }
    return bit;
}

void combo_setup(void)
{
    // init the state machine at state 0.
    combo_state = 0;
}

combo_action_t combo_recognize(const uint16_t keydown,
                               const uint16_t keyup,
                               const uint16_t keystate)
{
    uint8_t combo_next_state = 0;
    static combo_action_t combo_action = COMBO_NONE;
    static uint16_t combo_release_key = 0;

    // quick out of there's no keys pressed.
    if (keydown == 0 && keyup == 0) {
        return COMBO_NONE;
    }

    // Check for ComboB, as the keys can be pressed in any order.
    if (keystate == 0x0f00 && combo_action != COMBO_B_DOWN) {
        combo_action = COMBO_B_DOWN;
        // Record the final key pressed to complete this combo, defined as
        // the right most bit of the instantaneous keydown bitmask.
        uint16_t keydown_bit = rightmost_bit_16(keydown);
        if (keydown_bit == 0) {
            // the keydown wasn't found inside this mask.
            // *** THIS SHOULD NEVER HAPPEN ***
            combo_action = COMBO_NONE;
        } else {
            combo_release_key = keydown_bit;
            combo_action = COMBO_B_DOWN;
        }
        return combo_action;
    }

    // If the previous keypress caused a Combo to fire, check for the combo
    // release keyup.
    if (combo_action != COMBO_NONE && combo_action <= COMBO_E_DOWN) {
        if (!(keystate & combo_release_key)) {
            // the release key is unset, reset the state machine to zero.
            combo_state = 0;
            // emit a keyup event.
            combo_action = combo_action + (COMBO_A_RELEASE - COMBO_A_DOWN);
            return combo_action;
        } else {
            // combo_action stays the same but we tell the combo scanner to
            // do nothing.
            return COMBO_NONE;
        }
    }

    // loop over the rules for this state attempting to transition.
    uint8_t offset = pgm_read_byte(&(state_offset[combo_state]));
    combo_state_t *rule = state_table + offset;
    while(pgm_read_byte(&(rule->state_num)) == combo_state) {
        uint8_t keyrule = pgm_read_byte(&(rule->key));

        // break the keyrule into type of test and bit to test.
        uint8_t keytest = keyrule & 0xf0;
        uint8_t keybit = keyrule & 0x0f;

        // setup the key test.
        uint16_t keymask = 0;
        if (keytest == KEYDOWN) {
            keymask = keydown;
        } else if (keytest == KEYUP) {
            keymask = keyup;
        } else {
            keymask = keystate;
        }

        // execute the keytest.
        if ((1 << keybit) & keymask) {
            // transition was successful.
            combo_next_state = pgm_read_byte(&(rule->next_state));
            combo_action = pgm_read_byte(&(rule->action));
            // no more rule following.
            break;
        } else {
            // rule failed, reset to zero.
            combo_next_state = 0;
            combo_action = COMBO_NONE;
        }
        rule++;
    }

    combo_state = combo_next_state;

    // If we've found a state with an action, record the combo release key.
    if (combo_action != COMBO_NONE) {
        uint16_t keydown_bit = rightmost_bit_16(keydown);
        if (keydown_bit == 0) {
            // the keydown wasn't found inside this mask.
            combo_release_key = 0;
        } else {
            combo_release_key = keydown_bit;
        }
    }

    return combo_action;
}

// -----------------------------------------------------------------------------
