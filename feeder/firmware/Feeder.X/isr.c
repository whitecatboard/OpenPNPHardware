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
#include "gpio.h"
#include <xc.h>

#if INCLUDE_GPIO_EXTERNAL_INTERRUPTS
extern gpio_isr_func_t gpio_isr[];
#endif

#if INCLUDE_TASK
extern void task_isr(void);
#endif

void __interrupt () my_isr_routine (void) {
#if INCLUDE_GPIO_EXTERNAL_INTERRUPTS
    if (INTCONbits.INT0IF) {
        gpio_isr[0]();
        INTCONbits.INT0IF = 0;
    }

    if (INTCON3bits.INT1IF) {
        gpio_isr[1]();
        INTCON3bits.INT1IF = 0;       
    }
    
    if (INTCON3bits.INT2IF) {
        gpio_isr[2]();
        INTCON3bits.INT2IF = 0;        
    }
    
    if (INTCON3bits.INT3IF) {
        gpio_isr[3]();
        INTCON3bits.INT3IF = 0;        
    }
#endif
#if INCLUDE_TASK
    if (PIR1bits.TMR2IF) {
        task_isr();
    }
#endif
}