/*
uart.c - Contains uart initilization and rx/tx helper functions.
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

#include "uart.h"


#if (F_CPU != 32000000L)
#error Clock expected to be 32000000L for uart initialization to work properly!
#endif


inline uint8_t uart_char_receive_blocking()
{
    while (!uart_char_received());
    return uart_cur_char();
}


inline void uart_char_send_blocking(uint8_t c)
{
    uart_char_send(c);
    while (!(USARTD0.STATUS & USART_TXCIF_bm));
    USARTD0.STATUS |= USART_TXCIF_bm;
}


void uart_init(enum uart_baud_setting baud_rate)
{
    uint8_t  bsel_val = 0, bscale_val = 0;

    switch(baud_rate) {
    case BAUD_19200:
        bsel_val = 103;
        bscale_val = 0;
        break;
    case BAUD_38400:
        bsel_val = 51;
        bscale_val = 0;
        break;
    case BAUD_57600:
        bsel_val = 34;
        bscale_val = 0;
        break;
    case BAUD_115200:
        bsel_val = 16;
        bscale_val = 0;
        break;
    default:
        return;
    }

    PORTD.DIRCLR = PIN6_bm; // RX
    PORTD.DIRSET = PIN7_bm; // TX

    PORTD.REMAP |= PORT_USART0_bm;

    USARTD0.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;

    USARTD0.BAUDCTRLA = (bsel_val & USART_BSEL_gm);
    USARTD0.BAUDCTRLB = ((bscale_val << USART_BSCALE_gp) & USART_BSCALE_gm) | ((bsel_val >> 8) & ~USART_BSCALE_gm);

	USARTD0.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;

}
