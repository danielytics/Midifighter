// SPI (Serial Peripheral Interface) functions for DJTechTools Midifighter
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

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

// SPI functions ---------------------------------------------------------------

void spi_setup(void);
uint8_t spi_transmit(uint8_t byte);
void spi_install_slave (uint8_t id, uint8_t port, uint8_t pin, uint8_t select_with);
uint8_t spi_is_selected (uint8_t id);
void spi_select (uint8_t slave);
void spi_select_none (void);

#define SPI_PORT_A 	0
#define SPI_PORT_B 	1
#define SPI_PORT_C 	2
#define SPI_PORT_D 	3

#define SPI_LOW_TO_SELECT 0
#define SPI_HIGH_TO_SELECT 1

#endif // _SPI_H_INCLUDED
