/*
relays.c - Initializes the relay pins and allows their state to be
checked.
Copyright (C) 2013 Jeffrey Nelson <nelsonjm@macpod.net>

This file is part of the TwoStep firmware.

Lasershark is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Lasershark is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lasershark. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RELAYS_H_
#define RELAYS_H_


#include <avr/io.h>
#include <stdbool.h>


#define RELAYS_R1_A 1
#define RELAYS_R1_B 2
#define RELAYS_R2_A 4
#define RELAYS_R2_B 8
#define RELAYS_GC 0xf


inline bool relay_r1a_or_r1b_triggered();
inline bool relay_r2a_or_r2b_triggered();

uint8_t get_relay_status();

void relays_init();

#endif
