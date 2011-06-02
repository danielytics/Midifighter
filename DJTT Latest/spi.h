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

#endif // _SPI_H_INCLUDED
