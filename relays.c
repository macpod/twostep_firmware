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
#include "relays.h"


inline bool relay_r1a_or_r1b_triggered()
{
    return !((PORTA.IN & PIN4_bm) && (PORTA.IN & PIN5_bm));
}


inline bool relay_r2a_or_r2b_triggered()
{
    return !((PORTA.IN & PIN6_bm) && (PORTA.IN & PIN7_bm));
}


uint8_t get_relay_status()
{

    uint8_t res = 0;
    uint8_t port_in = PORTA.IN;

    res |= (port_in & PIN4_bm) ? 0 : RELAYS_R1_A; // R1_A
    res |= (port_in & PIN5_bm) ? 0 : RELAYS_R1_B; // R1_B
    res |= (port_in & PIN6_bm) ? 0 : RELAYS_R2_A; // R2_A
    res |= (port_in & PIN7_bm) ? 0 : RELAYS_R2_B; // R2_B

    return res;
}


void relays_init()
{
    // Set relay pins as inputs.
    PORTA.DIRCLR = PIN4_bm; // R1_A
    PORTA.DIRCLR = PIN5_bm; // R1_B
    PORTA.DIRCLR = PIN6_bm; // R2_A
    PORTA.DIRCLR = PIN7_bm; // R2_B

    // Pullup pins internally.
    PORTA.PIN4CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN5CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN6CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN7CTRL |= PORT_OPC_PULLUP_gc;
}

