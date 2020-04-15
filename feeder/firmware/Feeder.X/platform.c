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

#include <xc.h>

void platform_init(void) {
    // Hardware init
    LATC    = 0xff; // Initialize PORTC Clearing Output Latches
    TRISC   = 0x00; // PORTC All Outputs
    ANCON0  = 0x00; // Configure A/D For Digital
    ANCON1  = 0x00; // Configure A/D For Digital
    ADCON0  = 0x00; // Configure A/D For Digital
    ADCON1  = 0x00; // Configure A/D For Digital
    ADCON2  = 0x00; // Configure A/D For Digital
    CM1CON  = 0x00; // Disable Comparator
    CM2CON  = 0x00; // Digital I/O
    ODCON   = 0x00; // Disable Open Drain Capability
    INTCON  = 0x00; // Disable Interrupts
    INTCON2 = 0x00; // Pull-UP Are Enabled By Individual TRISB
    INTCON3 = 0x00; // Disable Interrupts
    LATA    = 0x00; // Initialize PORTA Clearing Output Latches
    TRISA   = 0x00; // PORTA All Outputs
    LATB    = 0x00; // Initialize PORTB Clearing Output Latches
    TRISB   = 0x08; // PORTB All Outputs Except RB3 CANRX
    WPUB    = 0xf3; // PORTB Pull-UP Enabled On All Pins Except RB2 and RB3
    PADCFG1 = 0xc0; // PORTD and PORTE Pull-UP Enabled On All Pins
    OSCTUNE = 0x40; // PLL Bit Enable
}
