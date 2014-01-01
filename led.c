/*
led.c - Blinks an LED. What more do you want from me?!
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

#include "led.h"
#include <avr/io.h>


inline void led_state(bool on)
{
    if (on) {
        PORTC.OUTCLR = PIN5_bm;
    } else {
        PORTC.OUTSET = PIN5_bm;
    }
}


inline void led_toggle()
{
    PORTC.OUTTGL  = PIN5_bm;
}


inline void led_init()
{
    // Set leds as outputs
    PORTC.DIRSET = PIN5_bm; // USR_LED
}

