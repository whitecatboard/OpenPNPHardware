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

#ifndef CAN_H
#define	CAN_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t sid;
    uint8_t  dlc;
    uint8_t  data[8];
} can_frame_t;

void can_setup_init(void);
void can_setup_end(void);
void can_send(can_frame_t *cf);
uint8_t can_get(uint16_t sid, can_frame_t *cf);
void can_add_filter(uint16_t sid);

#ifdef	__cplusplus
}
#endif

#endif	/* CAN_H */

