/*
 *          /\       /\
 *         /  \_____/  \
 *        /_____________\
 *        W H I T E C A T
 *
 * Copyright (C) 2020, IBEROXARXA SERVICIOS INTEGRALES, S.L.
 * Copyright (C) 2020, Jaume Oliv� Petrus (jolive@whitecatboard.org)
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

#ifndef EEPROM_H
#define	EEPROM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

void eeprom_write8(uint16_t address, uint8_t data);
void eeprom_write16(uint16_t address, uint16_t data);
void eeprom_write32(uint16_t address, uint32_t data);

uint8_t eeprom_read8(uint16_t address);
uint16_t eeprom_read16(uint16_t address);
uint32_t eeprom_read32(uint16_t address);

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

