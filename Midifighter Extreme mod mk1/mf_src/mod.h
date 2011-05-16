
#ifndef _MOD_H_INCLUDED
#define _MOD_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

// Configuration

// Number of multiplexer pins to use for analog inputs. Valid values: 1, 2 and 3
// Four inputs per pin, so max of 12 inputs
#define NUM_ANALOG_PINS 2

// analog buttons (12 general + 4 midifighter external banks + 4 global banks)
#define NUM_ANALOG_BUTTONS 20

// enabled general purpose analog buttons
#define NUM_GENERAL_ANALOG_BUTTONS 6
//12

// Shift button (pin 5, multiplexer 0)
#define SHIFT_BUTTON 4

// Number of external (ie, external to the midifighter pcb) leds
#define NUM_EXTERNAL_LEDS 17

// Globals

extern uint8_t global_bank;
extern uint8_t shift_bank;
extern uint8_t midifighter_bank;

extern uint16_t adc_value[];
extern uint8_t midifighter_bank_state;

extern uint8_t static_button_state;

extern uint8_t shift_button;

extern uint8_t demo_mode;

// Functions

uint16_t read_external_inputs (void/*uint8_t* midifighter_bank_up, uint8_t* midifighter_bank_down*/);
void set_external_leds (void);

//uint8_t switch_bank (uint8_t current_bank, uint8_t new_bank, uint8_t base_note);

#define switch_bank(current_bank,new_bank,base_note)       \
    if (new_bank != current_bank) {                        \
        midi_set_bank(current_bank);                       \
        midi_stream_note(base_note + current_bank, false); \
    }                                                      \
    midi_set_bank(new_bank);                               \
    midi_stream_note(base_note + new_bank, true);          \
    current_bank = new_bank


#endif
