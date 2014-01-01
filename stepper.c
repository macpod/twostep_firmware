/*
stepper.c - Initializes stepper motor pins and controls stepper
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

#include "stepper.h"
#include "relays.h"
#include <avr/interrupt.h>


static volatile bool steppers_running = false;
static volatile bool stepper_running[STEPPER_MAX_STEPPER_NUM];

static volatile uint16_t stepper_delay_increments[STEPPER_MAX_STEPPER_NUM];
static volatile uint16_t stepper_delay_count[STEPPER_MAX_STEPPER_NUM];

static volatile uint32_t stepper_step_count[STEPPER_MAX_STEPPER_NUM];
static volatile bool stepper_high[STEPPER_MAX_STEPPER_NUM];

static volatile bool stepper_step_until_relay[STEPPER_MAX_STEPPER_NUM];

static volatile bool stepper_step_safely[STEPPER_MAX_STEPPER_NUM];


ISR(TCC4_OVF_vect)
{
    uint8_t i;
    // Clear interrupt flag
    TCC4.INTFLAGS = TC4_OVFIF_bm;

    if (!steppers_running) {
        return;
    }

    for (i = 0; i < STEPPER_MAX_STEPPER_NUM; i++) {
        if (stepper_running[i] && (stepper_step_safely[i] || stepper_step_until_relay[i])) {
            stepper_running[i] = (i == 0) ? !relay_r1a_or_r1b_triggered() : !relay_r2a_or_r2b_triggered();
        }
        if (stepper_running[i] && !stepper_step_until_relay[i] && !stepper_high[i] && stepper_step_count[i] == 0) {
            stepper_running[i] = false;
        }
        if (stepper_running[i]) {
            if (stepper_delay_count[i] == 0) {
                if (stepper_high[i]) {
                    if (i == 0) {
                        PORTD.OUTSET = PIN0_bm; // STEP_1
                    } else {
                        PORTC.OUTSET = PIN0_bm; // Step 2
                    }
                    stepper_high[i] = false;
                } else {
                    if (i == 0) {
                        PORTD.OUTCLR = PIN0_bm; // STEP_1
                    } else {
                        PORTC.OUTCLR = PIN0_bm; // Step 2
                    }
                    stepper_high[i] = true;
                    if (!stepper_step_until_relay[i]) {
                        stepper_step_count[i]--;
                    }
                }
                stepper_delay_count[i] = stepper_delay_increments[i];
            } else {
                stepper_delay_count[i]--;
            }
        }
    }
}


// Helper method.
static bool stepper_num_valid(uint8_t stepper_num)
{
    bool res = true;
    if (stepper_num > STEPPER_MAX_STEPPER_NUM || stepper_num < STEPPER_MIN_STEPPER_NUM) {
        res = false;
    }
    return res;
}


// Helper method
static bool stepper_bitfield_valid(uint8_t stepper_bitfield)
{
    bool res = true;
    if (stepper_bitfield & ~(STEPPER_BITFIELD_STEPPER_1 | STEPPER_BITFIELD_STEPPER_2)) {
        res = false;
    }
    return res;
}


bool stepper_set_steps(uint8_t stepper_num, uint32_t steps)
{
    bool res = stepper_num_valid(stepper_num);

    if (res) {
        if (stepper_running[stepper_num-1]) {
            res = false;
        }
    }

    if (res) {
        stepper_step_count[stepper_num-1] = steps;
        stepper_high[stepper_num-1] = false;
        stepper_step_until_relay[stepper_num-1] = false;
        stepper_step_safely[stepper_num-1] = false;
    }

    return res;
}


bool stepper_set_safe_steps(uint8_t stepper_num, uint32_t steps)
{
    bool res = stepper_num_valid(stepper_num);

    if (res) {
        if (stepper_running[stepper_num-1]) {
            res = false;
        }
    }

    if (res) {
        stepper_step_count[stepper_num-1] = steps;
        stepper_high[stepper_num-1] = false;
        stepper_step_until_relay[stepper_num-1] = false;
        stepper_step_safely[stepper_num-1] = true;
    }

    return res;
}


bool stepper_set_step_until_relay(uint8_t stepper_num)
{
    bool res = stepper_num_valid(stepper_num);

    if (res) {
        if (stepper_running[stepper_num-1]) {
            res = false;
        }
    }

    if (res) {
        stepper_step_count[stepper_num-1] = 0;
        stepper_high[stepper_num-1] = false;
        stepper_step_until_relay[stepper_num-1] = true;
        stepper_step_safely[stepper_num-1] = true;
    }

    return res;
}


bool stepper_start(uint8_t stepper_bitfield)
{
    bool res = stepper_bitfield_valid(stepper_bitfield);

    if (res) {
        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_1 && stepper_running[0]) {
            res = false;
        }
    }

    if (res) {
        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_2 && stepper_running[1]) {
            res = false;
        }
    }

    if (res) {
        if (!stepper_running[0] && !stepper_running[1]) {
            // Enables starting both motors at exactly the same time.
            steppers_running = false;
        }

        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_1) {
            stepper_delay_count[0] = 0;
            stepper_running[0] = true;
        }
        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_2) {
            stepper_delay_count[1] = 0;
            stepper_running[1] = true;
        }

        steppers_running = true;
    }

    return res;
}


bool stepper_stop(uint8_t stepper_bitfield)
{
    bool res = stepper_bitfield_valid(stepper_bitfield);

    if (res) {
        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_1 && stepper_bitfield & STEPPER_BITFIELD_STEPPER_2) {
            // Stops steppers at exactly the same time.
            steppers_running = false;
        }
        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_1) {
            stepper_running[0] = false;
        }

        if (stepper_bitfield & STEPPER_BITFIELD_STEPPER_2) {
            stepper_running[1] = false;
        }
    }

    return res;
}


bool stepper_get_moving(uint8_t stepper_num, uint8_t *stepper_moving)
{
    bool res = stepper_num_valid(stepper_num);

    if (res) {
        *stepper_moving = stepper_running[stepper_num-1];
    }
    return res;

}


bool stepper_set_enable(uint8_t stepper_num, uint8_t enable)
{
    bool res = stepper_num_valid(stepper_num);

    if (enable != STEPPER_ENABLE && enable != STEPPER_DISABLE) {
        res = false;
    }

    if (stepper_running[stepper_num-1]) {
        res = false;
    }

    // Stepper enable pins are active low.
    switch (stepper_num) {
    case 1:
        if (enable) {
            PORTD.OUTCLR = PIN4_bm; // ENABLE_1
        } else {
            PORTD.OUTSET = PIN4_bm; // ENABLE_1
        }
        break;
    case 2:
        if (enable) {
            PORTC.OUTCLR = PIN4_bm; // ENABLE_2
        } else {
            PORTC.OUTSET = PIN4_bm; // ENABLE_2
        }
        break;
    }

    return res;
}


bool stepper_get_enable(uint8_t stepper_num, uint8_t *enable)
{
    bool res = true;
    // Stepper enable pins are active low.
    switch (stepper_num) {
    case 1:
        *enable = (PORTD.OUT & PIN4_bm) ? STEPPER_DISABLE: STEPPER_ENABLE;// ENABLE_1
        break;
    case 2:
        *enable = (PORTC.OUT & PIN4_bm) ? STEPPER_DISABLE: STEPPER_ENABLE;// ENABLE_2
        break;
    default:
        res = false;
        break;
    }

    return res;
}


bool stepper_set_microsteps(uint8_t stepper_num, uint8_t stepper_microstep_bitfield)
{
    bool res = stepper_num_valid(stepper_num);
    bool ms1 = false;
    bool ms2 = false;

    if (stepper_running[stepper_num-1]) {
        res = false;
    }

    switch(stepper_microstep_bitfield) {
    case STEPPER_MICROSTEP_BITFIELD_FULL_STEP:
        break;
    case STEPPER_MICROSTEP_BITFIELD_HALF_STEP:
        ms1 = true;
        break;
    case STEPPER_MICROSTEP_BITFIELD_QUARTER_STEP:
        ms2 = true;
        break;
    case STEPPER_MICROSTEP_BITFIELD_SIXTEENTH_STEP:
        ms1 = true;
        ms2 = true;
        break;
    default:
        res = false;
        break;
    }

    if (res) {
        switch (stepper_num) {
        case 1:
            if (ms1) {
                PORTD.OUTSET = PIN1_bm; // MS1_1
            } else {
                PORTD.OUTCLR = PIN1_bm; // MS1_1
            }
            if (ms2) {
                PORTD.OUTSET = PIN2_bm; // MS2_1
            } else {
                PORTD.OUTCLR = PIN2_bm; // MS2_1
            }
            break;
        case 2:
            if (ms1) {
                PORTC.OUTSET = PIN1_bm; // MS1_2
            } else {
                PORTC.OUTCLR = PIN1_bm; // MS1_2
            }
            if (ms2) {
                PORTC.OUTSET = PIN2_bm; // MS2_2
            } else {
                PORTC.OUTCLR = PIN2_bm; // MS2_2
            }
            break;
        }
    }

    return res;
}


bool stepper_get_microsteps(uint8_t stepper_num, uint8_t *stepper_microstep_bitfield)
{
    bool res = true;
    bool ms1 = false;
    bool ms2 = false;

    switch (stepper_num) {
    case 1:
        ms1 = (PORTD.OUT & PIN1_bm) ? true : false;
        ms2 = (PORTD.OUT & PIN2_bm) ? true : false;
        break;
    case 2:
        ms1 = (PORTC.OUT & PIN1_bm) ? true : false;
        ms2 = (PORTC.OUT & PIN2_bm) ? true : false;
        break;
    default:
        res = false;
        break;
    }

    if (res) {
        switch ((ms1 ? 1 : 0) + (ms2 ? 2 : 0)) {
        case STEPPER_MICROSTEP_BITFIELD_FULL_STEP:
            *stepper_microstep_bitfield = STEPPER_MICROSTEP_BITFIELD_FULL_STEP;
            break;
        case STEPPER_MICROSTEP_BITFIELD_HALF_STEP:
            *stepper_microstep_bitfield = STEPPER_MICROSTEP_BITFIELD_HALF_STEP;
            break;
        case STEPPER_MICROSTEP_BITFIELD_QUARTER_STEP:
            *stepper_microstep_bitfield = STEPPER_MICROSTEP_BITFIELD_QUARTER_STEP;
            break;
        case STEPPER_MICROSTEP_BITFIELD_SIXTEENTH_STEP:
            *stepper_microstep_bitfield = STEPPER_MICROSTEP_BITFIELD_SIXTEENTH_STEP;
            break;
        }
    }

    return res;
}


bool stepper_set_dir(uint8_t stepper_num, uint8_t dir)
{
    bool res = stepper_num_valid(stepper_num);

    if (dir != STEPPER_DIR_HIGH && dir != STEPPER_DIR_LOW) {
        res = false;
    }

    if (stepper_running[stepper_num-1]) {
        res = false;
    }

    switch (stepper_num) {
    case 1:
        if (dir) {
            PORTD.OUTSET = PIN3_bm; // DIR_1
        } else {
            PORTD.OUTCLR = PIN3_bm; // DIR_1
        }
        break;
    case 2:
        if (dir) {
            PORTC.OUTSET = PIN3_bm; // DIR_2
        } else {
            PORTC.OUTCLR = PIN3_bm; // DIR_2
        }
        break;
    }

    return res;
}


bool stepper_get_dir(uint8_t stepper_num, uint8_t *dir)
{
    bool res = true;
    // Stepper enable pins are active low.
    switch (stepper_num) {
    case 1:
        *dir = (PORTD.OUT & PIN3_bm) ? STEPPER_DIR_HIGH: STEPPER_DIR_LOW; // DIR_1
        break;
    case 2:
        *dir = (PORTC.OUT & PIN3_bm) ? STEPPER_DIR_HIGH: STEPPER_DIR_LOW;// DIR_2
        break;
    default:
        res = false;
        break;
    }

    return res;
}


bool stepper_set_current(uint8_t stepper_num, uint16_t val)
{
    bool res = stepper_num_valid(stepper_num);

    if (stepper_running[stepper_num-1]) {
        res = false;
    }

    if (val > STEPPER_MAX_CURRENT_VAL) {
        res = false;
    }

    if (res) {
        switch (stepper_num) {
        case 1:
            DACA.CH0DATA = val;
            break;
        case 2:
            DACA.CH1DATA = val;
            break;
        }
    }
    return res;
}


bool stepper_get_current(uint8_t stepper_num, uint16_t *val)
{
    bool res = true;

    switch (stepper_num) {
    case 1:
        *val = DACA.CH0DATA;
        break;
    case 2:
        *val = DACA.CH1DATA;
        break;
    default:
        res = false;
        break;
    }

    return res;
}


bool stepper_set_100uS_delay(uint8_t stepper_num, uint16_t val)
{
    bool res = stepper_num_valid(stepper_num);

    if (stepper_running[stepper_num-1]) {
        res = false;
    }

    if (res && (val >= USHRT_MAX)) {
        res = false;
    }

    if (res) {
        stepper_delay_increments[stepper_num-1] = val;
    }

    return res;
}


bool stepper_get_100uS_delay(uint8_t stepper_num, uint16_t *val)
{
    bool res = stepper_num_valid(stepper_num);

    if (res) {
        *val = stepper_delay_increments[stepper_num-1];
    }

    return res;
}


static void stepper_dacs_init()
{

    // DAC pins set as output for now.
    //PORTA.DIRSET = PIN2_bm; // DAC_0 = PORTA, Pin 2
    //PORTA.DIRSET = PIN3_bm; // DAC_1 = PORTA, PIN 3

    DACA.CTRLC = DAC_REFSEL_AVCC_gc;
    DACA.CTRLB = DAC_CHSEL_DUAL_gc;
    DACA.CTRLA = DAC_CH0EN_bm | DAC_CH1EN_bm | DAC_ENABLE_bm;
    while ( (DACA.STATUS & DAC_CH0DRE_bm) == false );   // Wait for Data register Empty
    while ( (DACA.STATUS & DAC_CH1DRE_bm) == false );   // Wait for Data register Empty
    DACA.CH0DATA = 0;
    DACA.CH1DATA = 0;
}


// Setup a timer that is called every 50uS
static void stepper_timer_init()
{
    cli();
    // We want a 50uS overflow.. and assume a 32Mhz clock
    // = 100uS
    // = (1/(32Mhz/64))*50
    // = (1/32Mhz/(DIV))*(PER+1)

    // Set per to 25-1
    TCC4.PER = 24;

    // Set CLK DIV to 1:64
    TCC4.CTRLA = TC45_CLKSEL_DIV64_gc;

    // Low level overflow interrupt
    TCC4.INTCTRLA = TC45_OVFINTLVL_LO_gc;

    // Enable low level interrupts
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei();
}


void stepper_init()
{
    int i;
    // Immediately set stepper motor enable pins to outputs and disable.
    PORTD.DIRSET = PIN4_bm; // ENABLE_1
    PORTC.DIRSET = PIN4_bm; // ENABLE_2
    stepper_set_enable(1, false);
    stepper_set_enable(2, false);

    // Set the rest of stepper 1 pins as outputs
    PORTD.DIRSET = PIN0_bm; // STEP_1
    PORTD.DIRSET = PIN1_bm; // MS2_1
    PORTD.DIRSET = PIN2_bm; // MS1_1
    PORTD.DIRSET = PIN3_bm; // DIR_1

    // Set the rest of stepper 2 pins as outputs
    PORTC.DIRSET = PIN0_bm; // STEP_2
    PORTC.DIRSET = PIN1_bm; // MS1_2
    PORTC.DIRSET = PIN2_bm; // MS2_2
    PORTC.DIRSET = PIN3_bm; // DIR_2

    // Set all stepper 1 ports low.
    PORTD.OUTCLR = PIN0_bm; // STEP_1
    PORTC.OUTCLR = PIN0_bm; // STEP_2

    // Set stepper motor current to minimum possible.
    stepper_dacs_init();

    steppers_running = false;
    for (i = 0; i < STEPPER_MAX_STEPPER_NUM; i++) {
        stepper_running[i] = false;

        stepper_step_count[i] = 0;
        stepper_high[i] = false;

        stepper_step_until_relay[i] = false;

        stepper_step_safely[i] = true;
        stepper_set_current(i+1, STEPPER_MIN_CURRENT_VAL);
        stepper_get_dir(i+1, false);
        stepper_set_microsteps(i+1, STEPPER_MICROSTEP_BITFIELD_FULL_STEP);
        stepper_set_100uS_delay(i+1, STEPPER_STEP_100US_DELAY_5MS);
    }

    stepper_timer_init();
}
