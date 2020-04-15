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

#include "can.h"

#include <xc.h>
#include <string.h>
 
void can_setup_init(void) {
    // Enter module in config mode
    CANCON = 0x80;
	while(CANSTATbits.OPMODE != 0x04);

	ECANCON     = 0x50; 		// Select ECAN Mode 1
	BSEL0 	    = 0x00; 		// Buffers B5 to B0 Input Buffers
    BRGCON1     = 0x02; 		// 500 Kbps recommended config T1(12) T2(4) BTQ(16) SP%(75) SJW(1)
	BRGCON2     = 0xAC; 		// 500 Kbps recommended config T1(12) T2(4) BTQ(16) SP%(75) SJW(1)
	BRGCON3     = 0x03; 		// 500 Kbps recommended config T1(12) T2(4) BTQ(16) SP%(75) SJW(1)
    RXM0EIDH    = 0x00; 		// Mask0 EFF Disabled
	RXM0EIDL    = 0x00; 		// Mask0 EFF Disabled
	RXM0SIDH    = 0xFF; 		// Mask0 SFF Enabled
	RXM0SIDL    = 0xE0; 		// Mask0 SFF Enabled
    MSEL0 	    = 0x00; 		// Select Mask0 For All Filters
	MSEL1 	    = 0x00; 		// Select Mask0 For All Filters
	MSEL2 	    = 0x00; 		// Select Mask0 For All Filters
	MSEL3 	    = 0x00; 		// Select Mask0 For All Filters
    RXFCON0     = 0x00; 		// Disable All Filters
	RXFCON1     = 0x00; 		// Disable All Filters
    RXFBCON0    = 0xFF; 		// Disable Buffer Filters
    RXFBCON1    = 0xFF; 		// Disable Buffer Filters
    RXFBCON2    = 0xFF; 		// Disable Buffer Filters
    RXFBCON3    = 0xFF; 		// Disable Buffer Filters
    RXFBCON4    = 0xFF; 		// Disable Buffer Filters
    RXFBCON5    = 0xFF; 		// Disable Buffer Filters
    RXFBCON6    = 0xFF; 		// Disable Buffer Filters
    RXFBCON7    = 0xFF; 		// Disable Buffer Filters
}

void can_setup_end(void) {
    RXB0CON     = 0x00; 		// Configure RXB0 Receive Valid Std Messages
    RXB1CON     = 0x00; 		// Configure RXB1 Receive Valid Std Messages
    B0CON       = 0x00; 		// Configure B0 Receive Valid Std Messages
    B1CON       = 0x00; 		// Configure B1 Receive Valid Std Messages
    B2CON       = 0x00; 		// Configure B2 Receive Valid Std Messages
    B3CON       = 0x00; 		// Configure B3 Receive Valid Std Messages
    B4CON       = 0x00; 		// Configure B4 Receive Valid Std Messages
    B5CON       = 0x00; 		// Configure B5 Receive Valid Std Messages
    CIOCON      = 0x20; 		// CAN Recessive High, Always set this bit when using a differential bus to avoid signal crosstalk in CANTX from other nearby pins
	
    // Enter module in normal mode
    CANCON = 0x00;
	while(CANSTATbits.OPMODE != 0x00);
}

void can_send(can_frame_t *cf) {
    TXB0SIDH        = (cf->sid >> 3);
    TXB0SIDL        = (cf->sid << 5);
    TXB0DLC         = cf->dlc;
    TXB0D0          = cf->data[0];
    TXB0D1          = cf->data[1];
    TXB0D2          = cf->data[2];
    TXB0D3          = cf->data[3];
    TXB0D4          = cf->data[4];
    TXB0D5          = cf->data[5];
    TXB0D6          = cf->data[6];
    TXB0D7          = cf->data[7];
    
    TXB0CONbits.TXREQ = 1;
    while(TXB0CONbits.TXREQ == 1);
}

void can_add_filter(uint16_t sid) {
   if (RXFCON0 == 0x00) {
        RXF0EIDH = 0x00;      		// Filter 0 (EFF Disabled)
        RXF0EIDL = 0x00; 	   		// Filter 0 (EFF Disabled)
        RXF0SIDH = (sid >> 3); 		// Filter 0 (High Byte)
        RXF0SIDL = (sid << 5); 		// Filter 0 (Low Byte)
        RXFBCON0 = 0xF0;      		// Assign Buffers (0->RXB0)
        RXFCON0  = 0x01;       		// Enable Filters (0)
    } else if (RXFCON0 == 0x01) {
        RXF1EIDH = 0x00;      		// Filter 1 (EFF Disabled)
        RXF1EIDL = 0x00;      		// Filter 1 (EFF Disabled)
        RXF1SIDH = (sid >> 3); 		// Filter 1 (High Byte)
        RXF1SIDL = (sid << 5); 		// Filter 1 (Low Byte)
        RXFBCON0 = 0x10;      		// Assign Buffers (1->RXB1,0->RXB0)
        RXFCON0  = 0x03;       		// Enable Filters (0,1)
    } else if (RXFCON0 == 0x03) {
        RXF2EIDH = 0x00;      		// Filter 2 (EFF Disabled)
        RXF2EIDL = 0x00;      		// Filter 2 (EFF Disabled)
        RXF2SIDH = (sid >> 3); 		// Filter 2 (High Byte)
        RXF2SIDL = (sid << 5); 		// Filter 2 (Low Byte)
        RXFBCON1 = 0xF2;      		// Assign Buffers (2->B0)
        RXFCON0  = 0x07;      		// Enable Filters (0,1,2)
    } else if (RXFCON0 == 0x07) {
        RXF3EIDH = 0x00;      		// Filter 3 (EFF Disabled)
        RXF3EIDL = 0x00;      		// Filter 3 (EFF Disabled)
        RXF3SIDH = (sid >> 3); 		// Filter 3 (High Byte)
        RXF3SIDL = (sid << 5); 		// Filter 3 (Low Byte)
        RXFBCON1 = 0x32;      		// Assign Buffers (3->B1,2->B0)
        RXFCON0  = 0x0F;    		// Enable Filters (0,1,2,3)
    } else if (RXFCON0 == 0x0F) {
        RXF4EIDH = 0x00;      		// Filter 4 (EFF Disabled)
        RXF4EIDL = 0x00;      		// Filter 4 (EFF Disabled)
        RXF4SIDH = (sid >> 3); 		// Filter 4 (High Byte)
        RXF4SIDL = (sid << 5); 		// Filter 4 (Low Byte)
        RXFBCON2 = 0xF4;      		// Assign Buffers (4->B2)
        RXFCON0  = 0x1F;      		// Enable Filters (0,1,2,3,4)
    } else if (RXFCON0 == 0x1F) {
        RXF5EIDH = 0x00;      		// Filter 5 (EFF Disabled)
        RXF5EIDL = 0x00;      		// Filter 5 (EFF Disabled)
        RXF5SIDH = (sid >> 3); 		// Filter 5 (High Byte)
        RXF5SIDL = (sid << 5); 		// Filter 5 (Low Byte)
        RXFBCON2 = 0x54;          	// Assign Buffers (5->B3,4->B2)
        RXFCON0  = 0x3F;          	// Enable Filters (0,1,2,3,4,5)
    } else if (RXFCON0 == 0x3F) {
        RXF6EIDH = 0x00;      		// Filter 6 (EFF Disabled)
        RXF6EIDL = 0x00;      		// Filter 6 (EFF Disabled)
        RXF6SIDH = (sid >> 3); 		// Filter 6 (High Byte)
        RXF6SIDL = (sid << 5); 		// Filter 6 (Low Byte)
        RXFBCON3 = 0xF6;      		// Assign Buffers (6->B4)
        RXFCON0  = 0x7F;       		// Enable Filters (0,1,2,3,4,5,6)
    } else if (RXFCON0 == 0x7F) {
        RXF7EIDH = 0x00;      		// Filter 7 (EFF Disabled)
        RXF7EIDL = 0x00;      		// Filter 7 (EFF Disabled)
        RXF7SIDH = (sid >> 3); 		// Filter 7 (High Byte)
        RXF7SIDL = (sid << 5); 		// Filter 7 (Low Byte)
        RXFBCON3 = 0x76;      		// Assign Buffers (7->B5,6->B4)
        RXFCON0  = 0xFF;       		// Enable Filters (0,1,2,3,4,5,6,7)
    } else if (RXFCON1 == 0x00) {  		
        RXF8EIDH = 0x00;      		// Filter 8 (EFF Disabled)
        RXF8EIDL = 0x00; 	   		// Filter 8 (EFF Disabled)
        RXF8SIDH = (sid >> 3); 		// Filter 8 (High Byte)
        RXF8SIDL = (sid << 5); 		// Filter 8 (Low Byte)
        RXFBCON4 = 0xF0;     		// Assign Buffers (8->RXB0)
        RXFCON1  = 0x01;       		// Enable Filters (8)
    } else if (RXFCON1 == 0x01) {
        RXF9EIDH = 0x00;      		// Filter 9 (EFF Disabled)
        RXF9EIDL = 0x00;      		// Filter 9 (EFF Disabled)
        RXF9SIDH = (sid >> 3); 		// Filter 9 (High Byte)
        RXF9SIDL = (sid << 5); 		// Filter 9 (Low Byte)
        RXFBCON4 = 0x10;      		// Assign Buffers (9->RXB1,8->RXB0)
        RXFCON1  = 0x03;       		// Enable Filters (8,9)
    } else if (RXFCON1 == 0x03) {
        RXF10EIDH = 0x00;         	// Filter 10 (EFF Disabled)
        RXF10EIDL = 0x00;          	// Filter 10 (EFF Disabled)
        RXF10SIDH = (sid >> 3); 	// Filter 10 (High Byte)
        RXF10SIDL = (sid << 5); 	// Filter 10 (Low Byte)
        RXFBCON5  = 0xF2;           // Assign Buffers (10->B0)
        RXFCON1   = 0x07;           // Enable Filters (8,9,10)
    } else if (RXFCON1 == 0x07) {
        RXF11EIDH = 0x00;           // Filter 11 (EFF Disabled)
        RXF11EIDL = 0x00;          	// Filter 11 (EFF Disabled)
        RXF11SIDH = (sid >> 3); 	// Filter 11 (High Byte)
        RXF11SIDL = (sid << 5); 	// Filter 11 (Low Byte)
        RXFBCON5  = 0x32;        	// Assign Buffers (11->B1,10->B0)
        RXFCON1   = 0x0F;          	// Enable Filters (8,9,10,11)
    } else if (RXFCON1 == 0x0F) {
        RXF12EIDH = 0x00;         	// Filter 12 (EFF Disabled)
        RXF12EIDL = 0x00;          	// Filter 12 (EFF Disabled)
        RXF12SIDH = (sid >> 3);     // Filter 12 (High Byte)
        RXF12SIDL = (sid << 5);     // Filter 12 (Low Byte)
        RXFBCON6  = 0xF4;           // Assign Buffers (12->B2)
        RXFCON1   = 0x1F;           // Enable Filters (8,9,10,11,12)
    } else if (RXFCON1 == 0x1F) {
        RXF13EIDH = 0x00;         	// Filter 13 (EFF Disabled)
        RXF13EIDL = 0x00;          	// Filter 13 (EFF Disabled)
        RXF13SIDH = (sid >> 3); 	// Filter 13 (High Byte)
        RXF13SIDL = (sid << 5); 	// Filter 13 (Low Byte)
        RXFBCON6  = 0x54;           // Assign Buffers (13->B3,12->B2)
        RXFCON1   = 0x3F;           // Enable Filters (8,9,10,11,12,13)
    } else if (RXFCON1 == 0x3F) {
        RXF14EIDH = 0x00;           // Filter 14 (EFF Disabled)
        RXF14EIDL = 0x00;           // Filter 14 (EFF Disabled)
        RXF14SIDH = (sid >> 3); 	// Filter 14 (High Byte)
        RXF14SIDL = (sid << 5); 	// Filter 14 (Low Byte)
        RXFBCON7  = 0xF6;       	// Assign Buffers (14->B4)
        RXFCON1   = 0x7F;           // Enable Filters (8,9,10,11,12,13,14)
    } else if (RXFCON1 == 0x7F) {
        RXF15EIDH = 0x00;           // Filter 15 (EFF Disabled)
        RXF15EIDL = 0x00;           // Filter 15 (EFF Disabled)
        RXF15SIDH = (sid >> 3); 	// Filter 15 (High Byte)
        RXF15SIDL = (sid << 5); 	// Filter 15 (Low Byte)
        RXFBCON7  = 0x76;           // Assign Buffers (15->B5,14->B4)
        RXFCON1   = 0xFF;           // Enable Filters (8,9,10,11,12,13,14,15)
    }
}

uint8_t can_get(uint16_t sid, can_frame_t *cf) {
    memset(cf, 0, sizeof(can_frame_t));

    if (RXB0CONbits.RXFUL & (sid == (uint16_t)((RXB0SIDH << 3) | (RXB0SIDL >> 5)))) {
        cf->sid      = (uint16_t)(RXB0SIDH << 3) | (uint16_t)(RXB0SIDL  >> 5);
        cf->dlc      = RXB0DLC;
        cf->data[0]  = RXB0D0;
        cf->data[1]  = RXB0D1;
        cf->data[2]  = RXB0D2;
        cf->data[3]  = RXB0D3;
        cf->data[4]  = RXB0D4;
        cf->data[5]  = RXB0D5;
        cf->data[6]  = RXB0D6;
        cf->data[7]  = RXB0D7;
        
        RXB0CONbits.RXFUL = 0;
        
        return 1;
    } else if (RXB1CONbits.RXFUL & (sid == (uint16_t)((RXB1SIDH << 3) | (RXB1SIDL >> 5)))) {
        cf->sid      = (uint16_t)(RXB1SIDH << 3) | (uint16_t)(RXB1SIDL  >> 5);
        cf->dlc      = RXB1DLC;
        cf->data[0]  = RXB1D0;
        cf->data[1]  = RXB1D1;
        cf->data[2]  = RXB1D2;
        cf->data[3]  = RXB1D3;
        cf->data[4]  = RXB1D4;
        cf->data[5]  = RXB1D5;
        cf->data[6]  = RXB1D6;
        cf->data[7]  = RXB1D7;
        
        RXB1CONbits.RXFUL = 0;
        
        return 1;
    } else if (B0CONbits.RXFUL & (sid == (uint16_t)((B0SIDH << 3) | (B0SIDL >> 5)))) {
        cf->sid      = (uint16_t)(B0SIDH << 3) | (uint16_t)(B0SIDL  >> 5);
        cf->dlc      = B0DLC;
        cf->data[0]  = B0D0;
        cf->data[1]  = B0D1;
        cf->data[2]  = B0D2;
        cf->data[3]  = B0D3;
        cf->data[4]  = B0D4;
        cf->data[5]  = B0D5;
        cf->data[6]  = B0D6;
        cf->data[7]  = B0D7;
        
        B0CONbits.RXFUL = 0;
        
        return 1;
    } else if (B1CONbits.RXFUL & (sid == (uint16_t)((B1SIDH << 3) | (B1SIDL >> 5)))) {
        cf->sid      = (uint16_t)(B1SIDH << 3) | (uint16_t)(B1SIDL  >> 5);
        cf->dlc      = B1DLC;
        cf->data[0]  = B1D0;
        cf->data[1]  = B1D1;
        cf->data[2]  = B1D2;
        cf->data[3]  = B1D3;
        cf->data[4]  = B1D4;
        cf->data[5]  = B1D5;
        cf->data[6]  = B1D6;
        cf->data[7]  = B1D7;
        
        B1CONbits.RXFUL = 0;
        
        return 1;
    } else if (B2CONbits.RXFUL & (sid == (uint16_t)((B2SIDH << 3) | (B2SIDL >> 5)))) {
        cf->sid      = (uint16_t)(B2SIDH << 3) | (uint16_t)(B2SIDL  >> 5);
        cf->dlc      = B2DLC;
        cf->data[0]  = B2D0;
        cf->data[1]  = B2D1;
        cf->data[2]  = B2D2;
        cf->data[3]  = B2D3;
        cf->data[4]  = B2D4;
        cf->data[5]  = B2D5;
        cf->data[6]  = B2D6;
        cf->data[7]  = B2D7;
        
        B2CONbits.RXFUL = 0;
        
        return 1;
    } else if (B3CONbits.RXFUL & (sid == (uint16_t)((B3SIDH << 3) | (B3SIDL >> 5)))) {
        cf->sid      = (uint16_t)(B3SIDH << 3) | (uint16_t)(B3SIDL  >> 5);
        cf->dlc      = B3DLC;
        cf->data[0]  = B3D0;
        cf->data[1]  = B3D1;
        cf->data[2]  = B3D2;
        cf->data[3]  = B3D3;
        cf->data[4]  = B3D4;
        cf->data[5]  = B3D5;
        cf->data[6]  = B3D6;
        cf->data[7]  = B3D7;
        
        B3CONbits.RXFUL = 0;
        
        return 1;
    } else if (B4CONbits.RXFUL & (sid == (uint16_t)((B4SIDH << 3) | (B4SIDL >> 5)))) {
        cf->sid      = (uint16_t)(B4SIDH << 3) | (uint16_t)(B4SIDL  >> 5);
        cf->dlc      = B4DLC;
        cf->data[0]  = B4D0;
        cf->data[1]  = B4D1;
        cf->data[2]  = B4D2;
        cf->data[3]  = B4D3;
        cf->data[4]  = B4D4;
        cf->data[5]  = B4D5;
        cf->data[6]  = B4D6;
        cf->data[7]  = B4D7;
        
        B4CONbits.RXFUL = 0;
        
        return 1;
    } else if (B5CONbits.RXFUL & (sid == (uint16_t)((B5SIDH << 3) | (B5SIDL >> 5)))) {
        cf->sid      = (uint16_t)(B5SIDH << 3) | (uint16_t)(B5SIDL  >> 5);
        cf->dlc      = B5DLC;
        cf->data[0]  = B5D0;
        cf->data[1]  = B5D1;
        cf->data[2]  = B5D2;
        cf->data[3]  = B5D3;
        cf->data[4]  = B5D4;
        cf->data[5]  = B5D5;
        cf->data[6]  = B5D6;
        cf->data[7]  = B5D7;
        
        B5CONbits.RXFUL = 0;
        
        return 1;
    }
    
    return 0;
}