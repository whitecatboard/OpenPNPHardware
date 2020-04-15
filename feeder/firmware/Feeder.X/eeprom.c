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
#include "eeprom.h"

#include <xc.h>

void eeprom_write8(uint16_t address, uint8_t data) {
    EEADR   = (uint8_t)(address & 0x00ff);        // Lower address bits
    EEADRH  = (uint8_t)((address & 0xff00) >> 8); // Upper address bits
    EEDATA  = data;

    EECON1bits.EEPGD = 0; // Point to DATA memory
    EECON1bits.CFGS  = 0; // Access EEPROM
    EECON1bits.WREN  = 1; // Enable writes
    
    // Required sequence
    EECON2 = 0x55;
    EECON2 = 0xaa;
    
    EECON1bits.WR    = 1; // Begin write
    INTCONbits.GIE   = 1; // Enable interrupts
    
    // Wait for write completion
    while(EECON1bits.WR==1);
}

void eeprom_write16(uint16_t address, uint16_t data) {
    eeprom_write8(address, (uint8_t)(data >> 8));
    eeprom_write8(address + 1, (uint8_t)(data));
}

void eeprom_write32(uint16_t address, uint32_t data) {
    eeprom_write8(address,     (uint8_t)(data >> 24));
    eeprom_write8(address + 1, (uint8_t)(data >> 16));
    eeprom_write8(address + 2, (uint8_t)(data >> 8));
    eeprom_write8(address + 3, (uint8_t)(data));
}

uint8_t eeprom_read8(uint16_t address) {
    EEADR   = (uint8_t)(address & 0x00ff);        // Lower address bits
    EEADRH  = (uint8_t)((address & 0xff00) >> 8); // Upper address bits
    
    EECON1bits.EEPGD = 0; // Point to DATA memory
    EECON1bits.CFGS  = 0; // Access EEPROM
    EECON1bits.RD    = 1; // Read
    
    asm("nop");
    
    return EEDATA;
}

uint16_t eeprom_read16(uint16_t address) {
    uint16_t tmp = 0;
    
    tmp  = ((uint16_t)eeprom_read8(address)) << 8;
    tmp |= ((uint16_t)eeprom_read8(address + 1));
    
    return tmp;
}

uint32_t eeprom_read32(uint16_t address) {
    uint32_t tmp = 0;
    
    tmp  = ((uint32_t)eeprom_read8(address)) << 24;
    tmp |= ((uint32_t)eeprom_read8(address + 1)) << 16;
    tmp |= ((uint32_t)eeprom_read8(address + 2)) << 8;
    tmp |= ((uint32_t)eeprom_read8(address + 3));
    
    return tmp;
}