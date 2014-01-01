/*
uart.h - Contains uart initilization and rx/tx helper functions.
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

#ifndef UART_H_
#define UART_H_


#include <avr/io.h>


enum uart_baud_setting {
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    BAUD_115200
};


// nonzero if character has been received
#define uart_char_received() (USARTD0.STATUS & USART_RXCIF_bm)

// current character in UART receive buffer
#define uart_cur_char() USARTD0.DATA

// Receives a character. Blocks if none are present
inline uint8_t uart_char_receive_blocking();

// send character
#define uart_char_send(c) USARTD0.DATA = (c)

// send character, block until it is completely sent
inline void uart_char_send_blocking(uint8_t c);

void uart_init(enum uart_baud_setting baud_rate);

#endif
