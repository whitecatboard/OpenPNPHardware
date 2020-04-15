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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>
#include <pic18f26k80.h>

#include "can.h"
#include "eeprom.h"

#define BOOTLOADER_MAJOR_VERSION 0x01
#define BOOTLOADER_MINOR_VERSION 0x00

#define FLASH_ERASE_BLOCK 64

/* guest program start/end addresses */
#define PROG_START 0x2000
#define PROG_END   0xffff

// EEPROM data
#define MY_ID_ADDR 0x0000 // Node id

/* point to guest program interrupt vectors */
asm("PSECT intcode");
asm("GOTO " ___mkstr(PROG_START) " + 0x8");
asm("PSECT intcodelo");
asm("GOTO " ___mkstr(PROG_START) " + 0x18");

// CPU frequency
#define _XTAL_FREQ 48000000

// Configuration bits
    
// CONFIG1L
#pragma config RETEN = OFF      // VREG Sleep Enable bit (Ultra low-power regulator is Disabled (Controlled by REGSLP bit))
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = DIG    // SOSC Power Selection and mode Configuration bits (Digital (SCLKI) mode)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config FOSC = HS1       // Oscillator (HS oscillator (Medium power, 4 MHz - 16 MHz))
#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN = OFF     // Power Up Timer (Disabled)
#pragma config BOREN = SBORDIS  // Brown Out Detect (Enabled in hardware, SBOREN disabled)
#pragma config BORV = 0         // Brown-out Reset Voltage bits (3.0V)
#pragma config BORPWR = HIGH    // BORMV Power level (BORMV set to high power level)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config WDTPS = 512      // Watchdog Postscaler (1:512)

// CONFIG3H
#pragma config CANMX = PORTB    // ECAN Mux bit (ECAN TX and RX pins are located on RB2 and RB3, respectively)
#pragma config MSSPMSK = MSK5   // MSSP address masking (5 bit address masking mode)
#pragma config MCLRE = OFF      // Master Clear Enable (MCLR Disabled, RE3 Enabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset (Enabled)
#pragma config BBSIZ = BB1K     // Boot Block Size (1K word Boot Block size)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protect 00800-03FFF (Disabled)
#pragma config CP1 = OFF        // Code Protect 04000-07FFF (Disabled)
#pragma config CP2 = OFF        // Code Protect 08000-0BFFF (Disabled)
#pragma config CP3 = OFF        // Code Protect 0C000-0FFFF (Disabled)

// CONFIG5H
#pragma config CPB = ON         // Code Protect Boot (Enabled)
#pragma config CPD = OFF        // Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protect 00800-03FFF (Disabled)
#pragma config WRT1 = OFF       // Table Write Protect 04000-07FFF (Disabled)
#pragma config WRT2 = OFF       // Table Write Protect 08000-0BFFF (Disabled)
#pragma config WRT3 = OFF       // Table Write Protect 0C000-0FFFF (Disabled)

// CONFIG6H
#pragma config WRTC = OFF       // Config. Write Protect (Disabled)
#pragma config WRTB = OFF       // Table Write Protect Boot (Disabled)
#pragma config WRTD = OFF       // Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protect 00800-03FFF (Disabled)
#pragma config EBTR1 = OFF      // Table Read Protect 04000-07FFF (Disabled)
#pragma config EBTR2 = OFF      // Table Read Protect 08000-0BFFF (Disabled)
#pragma config EBTR3 = OFF      // Table Read Protect 0C000-0FFFF (Disabled)

// CONFIG7H
#pragma config EBTRB = OFF      // Table Read Protect Boot (Disabled)

/* erase flash memory */
static void erase_flash() {
    LATCbits.LC1 = 1;

    /* erase */
    for (uint16_t address = PROG_START;
            address < (uint16_t)((uint16_t)PROG_END - (uint16_t)FLASH_ERASE_BLOCK); /* beware of overflow */
            address += (uint16_t)FLASH_ERASE_BLOCK) {
               
        // Set block address to erase
        TBLPTRU = 0x00;
        TBLPTRH = (uint8_t) (address >> 8);
        TBLPTRL = (uint8_t) (address);

        EECON1bits.EEPGD = 1; // Select flash memory
        EECON1bits.CFGS  = 0; // Select program memory
        EECON1bits.WREN  = 1; // Enable write cycles
        EECON1bits.FREE  = 1; // Erase operation 
        INTCONbits.GIE   = 0; // Disable interrupts
        
        // Required sequence for flash update
        EECON2 = 0x55;
        EECON2 = 0xAA;
       
        EECON1bits.WR   = 1;  // Initiates erase cycle   
        INTCONbits.GIE  = 1;  // Reenable interrupts
        EECON1bits.WREN = 0;  // Disable write cycles
    }
    
    LATCbits.LC1 = 0;
}

/* write data on flash memory */
static void write_data(uint16_t address, uint8_t *data) {
    LATCbits.LC1 = 1;

    // Set block address to write
    TBLPTRU = 0x00;
    TBLPTRH = (uint8_t)(address >> 8);
    TBLPTRL = (uint8_t)(address);

    // Write into holding registers
    for (uint8_t i = 0; i < FLASH_ERASE_BLOCK; i++) {
        TABLAT = data[i];
        asm("TBLWT*+");
    }
    
    asm("TBLRD*-");

    EECON1bits.EEPGD = 1; // Select flash memory
    EECON1bits.CFGS  = 0; // Select program memory
    EECON1bits.WREN  = 1; // Enable write cycles
    EECON1bits.FREE  = 0; // Disable erase operation 
    INTCONbits.GIE   = 0; // Disable interrupts

    // Required sequence for flash update
    EECON2 = 0x55;
    EECON2 = 0xAA;

    EECON1bits.WR   = 1;  // Initiates write   
    INTCONbits.GIE  = 1;  // Reenable interrupts
    EECON1bits.WREN = 0;  // Disable write cycles

    LATCbits.LC1 = 0;
}

void main(void) {   
    uint16_t address, new_address;
    uint8_t buff[FLASH_ERASE_BLOCK];
    uint8_t *cbuff;
    int i, bytes;
   
    can_frame_t cf; // Current can frame

    // Configure leds
    TRISCbits.TRISC0 = 0;
    LATCbits.LC0 = 0;
    
    TRISCbits.TRISC1 = 0;
    LATCbits.LC1 = 0;
        
    if (RCONbits.RI == 0) {
        // Software reset, enable bootloader
        
        // Read configuration from EEPROM
        uint8_t my_id = eeprom_read8(MY_ID_ADDR);
               
        // Init CAN
        can_setup_init();
        can_add_filter(0x102); // Request for erase flash
        can_add_filter(0x103); // Fragment address
        can_add_filter(0x104); // Flash fragment
        can_add_filter(0x105); // End
        can_setup_end();

        // Send bootloader response message: bootloader enabled
        cf.sid = 0x101;
        cf.dlc = 4;
        cf.data[0] = 0x01;
        cf.data[1] = BOOTLOADER_MAJOR_VERSION;
        cf.data[2] = BOOTLOADER_MINOR_VERSION;
        cf.data[3] = my_id;
        
        can_send(&cf);

        while (1) {
            if (can_get(0x102, &cf)) {
                // Erase flash
                erase_flash();
                
                cbuff = buff;
                bytes = 0;
                memset(buff, 0, FLASH_ERASE_BLOCK);

                // Send bootloader response message: flash erased
                cf.sid = 0x101;
                cf.dlc = 2;
                cf.data[0] = 0x02;
                cf.data[1] = my_id;
                
                can_send(&cf);
            } else if (can_get(0x103, &cf)) {
                // Fragment address
                new_address = ((((uint16_t)cf.data[0]) << 8) | ((uint16_t)cf.data[1]));
                if (bytes == 0) {
                    address = new_address;                    
                } else if (new_address - address > FLASH_ERASE_BLOCK) {
                    write_data(address, buff);
                    cbuff = buff;
                    memset(buff, 0, FLASH_ERASE_BLOCK);
                    bytes = 0;
                    
                    address = new_address;
                }                
            } else if (can_get(0x104, &cf)) {
                // Flash fragment
                for(i = 0;i < cf.dlc;i++) {
                    if (bytes < FLASH_ERASE_BLOCK) {
                        // Put data on buffer
                        *cbuff++ = cf.data[i];
                        bytes++;
                    } else {
                        // Buffer full, flush
                        write_data(address, buff);
                        address += FLASH_ERASE_BLOCK;
                        cbuff = buff;
                        bytes = 1;
                        memset(buff, 0, FLASH_ERASE_BLOCK);
                        *cbuff++ = cf.data[i];
                    }
                }
            } else if (can_get(0x105, &cf)) {
                // End
                if (bytes > 0) {
                    write_data(address, buff);
                }
                
                // Run app
                asm("goto " ___mkstr(PROG_START));
            }
        }
    } else {       
        // Run app
        asm("goto " ___mkstr(PROG_START));
    }    
}