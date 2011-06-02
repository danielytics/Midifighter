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
// rgreen 2009-05-22

#ifndef _EEPROM_H_INCLUDED
#define _EEPROM_H_INCLUDED

// EEPROM functions -----------------------------------------------

void eeprom_write(uint16_t address, uint8_t data);
uint8_t eeprom_read(uint16_t address);
void eeprom_factory_reset(void);
void eeprom_setup(void);
void eeprom_save_edits(void);

#endif // _EEPROM_H_INCLUDED
