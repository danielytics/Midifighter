
#include <util/delay.h>

#include <avr/pgmspace.h>

#include "mod.h"
#include "expansion.h"
#include "midi.h"
#include "adc.h"
#include "key.h"
#include "spi.h"
#include "led.h"

// Globals

uint8_t global_bank = 0;
uint8_t shift_bank = 0;
uint8_t midifighter_bank = 0;

uint16_t adc_value[NUM_ANALOG];
uint8_t midifighter_bank_state = 0;

uint8_t static_button_state = 0;
uint8_t shift_button = 0;

// Is demo mode enabled?
uint8_t demo_mode = 0;

// Previous state (pic or mf) used to decide if leds need to be blanked or not - should blank on every transition
uint8_t prev = 0;

// Num leds to light * 3
#define max  60

// Define demo mode LED sequence, each led in sequence is represented by 3 bytes {{pic or mf, shift button}, byte 1 or mf msb, byte 2 or mf lsb}
char demo[max] PROGMEM = {
	0x00, 0x00, 0x01,	0x00, 0x00, 0x02,
	0x00, 0x00, 0x04,	0x00, 0x00, 0x08,
	0x10, 0x00, 0x08,	0x10, 0x00, 0x80,
	0x10, 0x08, 0x00,	0x10, 0x80, 0x00,
	0x10, 0x40, 0x00,	0x10, 0x20, 0x00,
	0x10, 0x10, 0x00,	0x00, 0x08, 0x00,
	0x00, 0x04, 0x00,	0x00, 0x02, 0x00,
	0x00, 0x01, 0x00,	0x01, 0x00, 0x00,
	0x00, 0x10, 0x00,	0x00, 0x20, 0x00,
	0x00, 0x40, 0x00,	0x00, 0x80, 0x00};

char* cycle = demo;
	
void set_external_leds ()
{	
	if (demo_mode) {
		// Run demo mode
		uint8_t mf_or_pic = pgm_read_byte(cycle++);
		uint8_t byte_1 = pgm_read_byte(cycle++); // also MSB of mf leds
		uint8_t byte_2 = pgm_read_byte(cycle++); // also LSB of mf leds
		
		if (prev != (mf_or_pic & 0xf0)) {
			// Blank LEDs
			spi_select(SPI_SLAVE_PIC);
			spi_transmit(0);
			spi_transmit(0);
			spi_transmit(0);
			spi_select_none();
			led_set_state((uint16_t)0);
		}
		
		if (mf_or_pic) {
			// Light midifighter LEDs
			led_set_state((byte_1 << 8) | byte_2);
		} else {
			// Light PIC LEDs
			spi_select(SPI_SLAVE_PIC);
			spi_transmit(byte_1); // global bank, buttons
			spi_transmit(byte_2); // shift bank, bank
			spi_transmit(mf_or_pic & 0x0f); // N/A , shift_button
			spi_select_none();
		}
		_delay_ms(100);
		prev = mf_or_pic & 0xf0;
		
		if (cycle == demo + max) cycle = demo;
		
	} else {	
		// Send LED states to PIC
		spi_select(SPI_SLAVE_PIC);
		spi_transmit(((1 << global_bank) << 4) | static_button_state); // global bank, buttons
		spi_transmit(((1 << shift_bank) << 4) | (1 << midifighter_bank)); // shift bank, bank
		spi_transmit(shift_button); // N/A , shift_button
		spi_select_none();
	}
}
