/*
twostep_parser.c - Parses, executes, and responds to twostep binary
commands.
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

#include "twostep_parser.h"
#include "uart.h"
#include "led.h"
#include "stepper.h"
#include "relays.h"
#include "twostep_common_lib.h"
#include <string.h>


void twostep_parser_send_resp(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++) {
        uart_char_send_blocking(buf[i]);
    }
}


void twostep_parser_get_param(uint8_t **src, void *dest, uint8_t len)
{
    memcpy(dest, *src, len);
    *src+= len;
}


void twostep_parser_set_param(uint8_t **dest, const void *src, uint8_t len)
{
    memcpy(*dest, src, len);
    *dest+= len;
}


// It is assumed that the format of the cmd is at least right at this point.
static bool twostep_parser_handle_cmd(uint8_t *cmd_buf, uint8_t len)
{
    bool res = true;

    uint8_t *cmd_pos = cmd_buf+2; // Skip start token and command.
    uint8_t stepper_num = 0;
    uint8_t stepper_bitfield = 0;
    uint8_t uint8_param1 = 0;
    uint16_t uint16_param1 = 0;
    uint32_t uint32_param1 = 0;

    uint8_t resp_buf[TWOSTEP_BUF_SIZE];
    uint8_t *resp_pos = resp_buf + 3;
    uint8_t i;

    for (i = 0; i < TWOSTEP_BUF_SIZE; i++) {
        resp_buf[i] = 0xff;
    }

    resp_buf[0] = TWOSTEP_START_TOKEN;
    resp_buf[1] = cmd_buf[1]; // Copy command in so client knows what we are responding to.
    resp_buf[2] = TWOSTEP_CMD_SUCCESS; // Be optimistic and mark that we passed.


    switch(cmd_buf[1]) {
    case TWOSTEP_SET_STEPS:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint32_param1, sizeof(uint32_t)); // Step count
        res = stepper_set_steps(stepper_num, uint32_param1);
        break;
    case TWOSTEP_SET_SAFE_STEPS:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint32_param1, sizeof(uint32_t)); // Step count
        res = stepper_set_safe_steps(stepper_num, uint32_param1);
        break;
    case TWOSTEP_SET_STEP_UNTIL_RELAY:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        res = stepper_set_step_until_relay(stepper_num);
        break;
    case TWOSTEP_START:
        twostep_parser_get_param(&cmd_pos, &stepper_bitfield, sizeof(uint8_t)); // Stepper bitfield
        res = stepper_start(stepper_bitfield);
        break;
    case TWOSTEP_STOP:
        twostep_parser_get_param(&cmd_pos, &stepper_bitfield, sizeof(uint8_t)); // Stepper bitfield
        res = stepper_stop(stepper_bitfield);
        break;
    case TWOSTEP_GET_IS_MOVING:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        res = stepper_get_moving(stepper_num, &uint8_param1); // Stepper moving status
        if (res) {
            twostep_parser_set_param(&resp_pos, &uint8_param1, sizeof(uint8_t)); // Stepper moving status
        }
        break;
    case TWOSTEP_SET_ENABLE:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint8_param1, sizeof(uint8_t)); // Enable status
        res = stepper_set_enable(stepper_num, uint8_param1);
        break;
    case TWOSTEP_GET_ENABLE:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        res = stepper_get_enable(stepper_num, &uint8_param1);
        if (res) {
            twostep_parser_set_param(&resp_pos, &uint8_param1, sizeof(uint8_t)); // Enable status
        }
        break;
    case TWOSTEP_SET_MICROSTEPS:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint8_param1, sizeof(uint8_t)); // Microstep bitfield
        res = stepper_set_microsteps(stepper_num, uint8_param1);
        break;
    case TWOSTEP_GET_MICROSTEPS:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        res = stepper_get_microsteps(stepper_num, &uint8_param1);
        if(res) {
            twostep_parser_set_param(&resp_pos, &uint8_param1, sizeof(uint8_t)); // Microstep bitfield
        }
        break;
    case TWOSTEP_SET_DIR:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint8_param1, sizeof(uint8_t)); // Dir status
        res = stepper_set_dir(stepper_num, uint8_param1);
        break;
    case TWOSTEP_GET_DIR:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        res = stepper_get_dir(stepper_num, &uint8_param1);
        if(res) {
            twostep_parser_set_param(&resp_pos, &uint8_param1, sizeof(uint8_t)); // Dir status
        }
        break;
    case TWOSTEP_SET_CURRENT:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint16_param1, sizeof(uint16_t)); // Curren val
        res = stepper_set_current(stepper_num, uint16_param1);
        break;
    case TWOSTEP_GET_CURRENT:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint16_t)); // Stepper num
        res = stepper_get_current(stepper_num, &uint16_param1);
        if(res) {
            twostep_parser_set_param(&resp_pos, &uint16_param1, sizeof(uint16_t)); // Current val
        }
        break;
    case TWOSTEP_SET_100US_DELAY:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        twostep_parser_get_param(&cmd_pos, &uint16_param1, sizeof(uint16_t)); // Delay val
        res = stepper_set_100uS_delay(stepper_num, uint16_param1);
        break;
    case TWOSTEP_GET_100US_DELAY:
        twostep_parser_get_param(&cmd_pos, &stepper_num, sizeof(uint8_t)); // Stepper num
        res = stepper_get_100uS_delay(stepper_num, &uint16_param1);
        if(res) {
            twostep_parser_set_param(&resp_pos, &uint16_param1, sizeof(uint16_t)); // Delay val
        }
        break;
    case TWOSTEP_GET_RELAY_STATUS:
        uint8_param1 = get_relay_status();
        twostep_parser_set_param(&resp_pos, &uint8_param1, sizeof(uint8_t)); // Relay status
        break;
    case TWOSTEP_GET_VERSION:
        uint8_param1 = TWOSTEP_VERSION;
        twostep_parser_set_param(&resp_pos, &uint8_param1, sizeof(uint8_t)); // Version
        break;
    default:
        res = false;
        break;
    }

    if (res == false) {
        resp_buf[2] = TWOSTEP_CMD_FAIL;
    }

    i = twostep_resp_len(resp_buf[1]);
    twostep_insert_resp_end_tokens(resp_buf);
    twostep_parser_send_resp(resp_buf, i);
    return res;
}


bool twostep_parser_parse()
{
    bool res = true;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t i = 0;
    uint8_t len;

    buf[i++] = uart_char_receive_blocking();

    if (!twostep_verify_start_token(buf)) {
        res = false;
    }

    if (res) {
        buf[i++] = uart_char_receive_blocking();

        len = twostep_cmd_len(buf[i-1]);
        if (len == 0) {
            res = false;
        }
    }

    if (res) {
        while (i < len-2) {
            buf[i++] = uart_char_receive_blocking();
        }
    }

    if (res) {
        buf[i++] = uart_char_receive_blocking();

        if (buf[i-1] != TWOSTEP_END1_TOKEN) {
            res = false;
        }
    }

    if (res) {
        buf[i++] = uart_char_receive_blocking();

        if (buf[i-1] != TWOSTEP_END2_TOKEN) {
            res = false;
        }
    }

    if (res) {
        led_toggle();
        twostep_parser_handle_cmd(buf, len);

    }

    return res;
}
