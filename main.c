/*
main.c
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

/*
config switches:
    ABC

    000 - Bootloader jumps directly into app code using default settings.
    001 - Bootloader programming mode entered.
    010 - (todo) Bootloader jumps directly into app code, using saved settings?
    011 - Reserved
    100 - (todo) Binary mode?
    101 - Reserved
    110 - (todo) Human mode?
    111 - Reserved


 */

//#include <avr/iox16e5.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
//#include <avr/eeprom.h>

#include "uart.h"
#include "stepper.h"
#include "switches.h"
#include "led.h"
#include "twostep_parser.h"


static void init_external_crystal()
{

    // We are using a 16Mhz oscillator
    OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
    // Enable external oscillator
    OSC.CTRL |= OSC_XOSCEN_bm;
    // wait for source to be stable
    while (!(OSC.STATUS & OSC_XOSCRDY_bm));

    // Make XOSC src for PLL and multiply by 2x
    OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | (OSC_PLLFAC_gm & 2 << OSC_PLLFAC_gp);
    // enable PLL
    OSC.CTRL |= OSC_PLLEN_bm;
    // wait for source to be stable
    while (!(OSC.STATUS & OSC_PLLRDY_bm));

    // unlock CLK.CTRL
    CCP = CCP_IOREG_gc;
    // Set PLL as system clock
    CLK.CTRL = CLK_SCLKSEL_gm & CLK_SCLKSEL_PLL_gc;

    // Disable other sources
    OSC.CTRL &= ~(OSC_RC2MEN_bm | OSC_RC32MEN_bm | OSC_RC32KEN_bm);
}


static void init_conf_switches()
{
    // Set config pins as inputs.
    PORTA.DIRCLR = PIN0_bm; // CFG_0
    PORTA.DIRCLR = PIN1_bm; // CFG_1
    PORTC.DIRCLR = PIN6_bm; // CFG_2

    // Pullup pins internally.
    PORTA.PIN0CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN1CTRL |= PORT_OPC_PULLUP_gc;
    PORTC.PIN6CTRL |= PORT_OPC_PULLUP_gc;
}


int main(void)
{
    init_external_crystal();
    stepper_init();
    switches_init();
    uart_init(BAUD_115200);
    init_conf_switches();
    led_init();

    _delay_ms(10);

    while(1) {
        twostep_parser_parse();
    }

    return 0;
}


