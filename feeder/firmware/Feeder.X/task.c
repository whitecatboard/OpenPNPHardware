/*
 *          /\       /\
 *         /  \_____/  \
 *        /_____________\
 *        W H I T E C A T
 *
 * Copyright (C) 2020, IBEROXARXA SERVICIOS INTEGRALES, S.L.
 * Copyright (C) 2020, Jaume Olivé Petrus (jolive@whitecatboard.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "task.h"

#include <xc.h>

#if INCLUDE_TASK

extern void schelude_tasks(void);

uint16_t _ticks = 0;
uint16_t _time_delta = 0;
uint16_t _time_delta_led = 0;
uint16_t _time_delta_button = 0;

static uint8_t disabled = 1;

#define TICKS_PER_MS 4

void task_init(void) {
    // Configure timer to 250 usecs
    T2CON = 0x5C;     
       
    PR2 = 249;
    
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    
    INTCONbits.GIE  = 1;
    INTCONbits.PEIE = 1;
    
    _time_delta = 0;
    disabled = 0;
}

void task_isr(void) {  
    _ticks++;
    
    if (_ticks == TICKS_PER_MS) {
        _ticks = 0;

        _time_delta++;
        _time_delta_led++;
        _time_delta_button++;

        if (!disabled) {
            schelude_tasks();
        }
    }
    
    // Restart timer
    PIR1bits.TMR2IF = 0;
    PR2             = 249;
}

void task_disable() {
    disabled = 1;
}

void task_enable() {
    disabled = 0;
}

#endif