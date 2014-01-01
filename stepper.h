/*
stepper.h - Initializes stepper motor pins and controls stepper
motor movement.
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

#ifndef STEPPER_H_
#define STEPPER_H_


#include <avr/io.h>
#include <stdbool.h>
#include <limits.h>


#define STEPPER_STEP_100US_DELAY_5MS 50
#define STEPPER_STEP_100US_DELAY_MIN 1
#define STEPPER_STEP_100US_DELAY_MAX (USHRT_MAX)

#define STEPPER_MAX_CURRENT_VAL 4095
#define STEPPER_MIN_CURRENT_VAL 0

#define STEPPER_MIN_STEPPER_NUM 1
#define STEPPER_MAX_STEPPER_NUM 2

#define STEPPER_ENABLE 0x01
#define STEPPER_DISABLE 0x00

#define STEPPER_MOVING 0x01
#define STEPPER_STOPPED 0x00

#define STEPPER_DIR_HIGH 0x01
#define STEPPER_DIR_LOW 0x00

#define STEPPER_BITFIELD_STEPPER_1 1
#define STEPPER_BITFIELD_STEPPER_2 2
#define STEPPER_BITFIELD_STEPPER_GM 3

#define STEPPER_MICROSTEP_BITFIELD_FULL_STEP 0
#define STEPPER_MICROSTEP_BITFIELD_HALF_STEP 1
#define STEPPER_MICROSTEP_BITFIELD_QUARTER_STEP 2
#define STEPPER_MICROSTEP_BITFIELD_SIXTEENTH_STEP 3


bool stepper_set_steps(uint8_t stepper_num, uint32_t steps);
bool stepper_set_safe_steps(uint8_t stepper_number, uint32_t steps);
bool stepper_set_step_until_relay(uint8_t stepper_num);
bool stepper_start(uint8_t stepper_bitfield);
bool stepper_stop(uint8_t stepper_bitfield);

bool stepper_get_moving(uint8_t stepper_num, uint8_t *stepper_moving);


bool stepper_set_enable(uint8_t stepper_num, uint8_t enable);
bool stepper_get_enable(uint8_t stepper_num, uint8_t *enable);

bool stepper_set_microsteps(uint8_t stepper_num, uint8_t stepper_microstep_bitfield);
bool stepper_get_microsteps(uint8_t stepper_num, uint8_t *stepper_microstep_bitfield);

bool stepper_set_dir(uint8_t stepper_num, uint8_t dir);
bool stepper_get_dir(uint8_t stepper_num, uint8_t *dir);

bool stepper_set_current(uint8_t stepper_num, uint16_t val);
bool stepper_get_current(uint8_t stepper_num, uint16_t *val);

bool stepper_set_100uS_delay(uint8_t stepper_num, uint16_t val);
bool stepper_get_100uS_delay(uint8_t stepper_num, uint16_t *val);

void stepper_init();


#endif
