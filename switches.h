/*
switches.h - Initializes the switch pins and allows their state to be
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

#ifndef SWITCHES_H_
#define SWITCHES_H_


#include <avr/io.h>
#include <stdbool.h>


#define SWITCHES_R1_A 1
#define SWITCHES_R1_B 2
#define SWITCHES_R2_A 4
#define SWITCHES_R2_B 8
#define SWITCHES_GC 0xf


inline bool switch_r1a_or_r1b_triggered();
inline bool switch_r2a_or_r2b_triggered();

uint8_t get_switch_status();

void switches_init();

#endif
