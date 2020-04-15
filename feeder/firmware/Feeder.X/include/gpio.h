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

#include "platform.h"

#include <stdint.h>

#ifndef GPIO_H
#define	GPIO_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*gpio_isr_func_t)(void);

typedef enum {
    GPIOIntRaisingEdge = 1,
    GPIOIntFailingEdge = 0,
} gpio_int_type_t;
    
void gpio_pin_input(uint8_t pin);
void gpio_pin_output(uint8_t pin, uint8_t set);
void gpio_pin_set(uint8_t pin);
void gpio_pin_clr(uint8_t pin);
void gpio_pin_inv(uint8_t pin);
uint8_t gpio_pin_get(uint8_t pin, uint8_t debounce);   
void gpio_isr_attach(uint8_t pin, gpio_isr_func_t func, gpio_int_type_t type);
void gpio_pin_analog(uint8_t pin);
uint16_t gpio_pin_get_analog(uint8_t pin);
void gpio_pin_set_val(uint8_t pin, uint8_t val);

#ifdef	__cplusplus
}
#endif

#endif	/* GPIO_H */

