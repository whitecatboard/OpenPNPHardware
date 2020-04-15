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

typedef struct {
    volatile unsigned char *tris;
    volatile unsigned char *lat;
    volatile unsigned char *port;
    volatile unsigned char *ansel;
    volatile unsigned char iomask;
    uint8_t achanmask;
    uint8_t anselmask;
} gpio_reg_t;
 
const gpio_reg_t gpio_reg[] = {
    {&TRISA,&LATA,&PORTA,&ANCON0, 1 << 0, 0 << 2, 1 << 0},
    {&TRISA,&LATA,&PORTA,&ANCON0, 1 << 1, 1 << 2, 1 << 1},
    {&TRISA,&LATA,&PORTA,&ANCON0, 1 << 2, 2 << 2, 1 << 2},
    {&TRISA,&LATA,&PORTA,&ANCON0, 1 << 3, 3 << 2, 1 << 3},
    {&TRISA,&LATA,&PORTA,0x0000,  1 << 4, 0xff, 0xff},
    {&TRISA,&LATA,&PORTA,0x0000,  1 << 5, 0xff, 0xff},
    {&TRISA,&LATA,&PORTA,0x0000,  1 << 6, 0xff, 0xff},
    {&TRISA,&LATA,&PORTA,0x0000,  1 << 7, 0xff, 0xff},

    {&TRISB,&LATB,&PORTB,&ANCON1, 1 << 0, 10 << 2, 1 << 2},
    {&TRISB,&LATB,&PORTB,&ANCON1, 1 << 1, 8 << 2, 1 << 0},
    {&TRISB,&LATB,&PORTB,0x0000, 1 << 2, 0xff, 0xff},
    {&TRISB,&LATB,&PORTB,0x0000, 1 << 3, 0xff, 0xff},
    {&TRISB,&LATB,&PORTB,&ANCON1, 1 << 4, 9 << 2, 1 << 1},
    {&TRISB,&LATB,&PORTB,0x0000, 1 << 5, 0xff, 0xff},
    {&TRISB,&LATB,&PORTB,0x0000, 1 << 6, 0xff, 0xff},
    {&TRISB,&LATB,&PORTB,0x0000, 1 << 7, 0xff, 0xff},

    {&TRISC,&LATC,&PORTC,0x0000, 1 << 0, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 1, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 2, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 3, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 4, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 5, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 6, 0xff, 0xff},
    {&TRISC,&LATC,&PORTC,0x0000, 1 << 7, 0xff, 0xff}
};

#if INCLUDE_GPIO_EXTERNAL_INTERRUPTS
static void dummy() { return ; }

gpio_isr_func_t gpio_isr[] = {
    dummy,
    dummy,
    dummy,
    dummy
};
#endif

void gpio_pin_input(uint8_t pin) {
    *gpio_reg[pin].tris |= gpio_reg[pin].iomask;
    
    if (gpio_reg[pin].anselmask != 0xff) {
        *gpio_reg[pin].ansel &= (uint8_t)~gpio_reg[pin].anselmask;
    }
}

void gpio_pin_output(uint8_t pin, uint8_t set) {
    if (set) {
        *gpio_reg[pin].lat |= (uint8_t)gpio_reg[pin].iomask;
    } else {
        *gpio_reg[pin].lat &= (uint8_t)~gpio_reg[pin].iomask;
    }
    
    *gpio_reg[pin].tris &= (uint8_t)~gpio_reg[pin].iomask;

   if (gpio_reg[pin].anselmask != 0xff) {
        *gpio_reg[pin].ansel &= (uint8_t)~gpio_reg[pin].anselmask;
    }
}

void gpio_pin_analog(uint8_t pin) {
    if (gpio_reg[pin].achanmask != 0xff) {        
        ADCON1bits.VCFG =  0X00;  // vref+ = AVDD
        ADCON1bits.VNCFG = 0X00;  // vref- = AVSS
        ADCON1bits.CHSN =  0x00;  // No negative channels
        
        ADCON2bits.ADFM =  1;     // Right justified
        ADCON2bits.ACQT =  0x02;  // 4 TAD
        ADCON2bits.ADCS =  0x02;  // FOSC/32
        
        // Configure as analog
        gpio_pin_input(pin);
        *gpio_reg[pin].ansel |= (uint8_t)gpio_reg[pin].anselmask;
      
        // Enable module
        ADCON0bits.ADON = 1;
   }
}

uint16_t gpio_pin_get_analog(uint8_t pin) {
    if (gpio_reg[pin].achanmask != 0xff) {
        // Enable channel
        ADCON0 |= gpio_reg[pin].achanmask;
        
        // Start conversion
        ADCON0bits.GO = 1;
        
        // Wait for converssion
        while (ADCON0bits.DONE) {
            asm("nop");
        }

        // Disable channel
        ADCON0 &= ~gpio_reg[pin].achanmask;
        
        // Read conversion
        return ((ADRESH<<8) | (ADRESL));
    }
    
    return 0;
}

void gpio_pin_set_val(uint8_t pin, uint8_t val) {
    if (val) {
        *gpio_reg[pin].lat |= (uint8_t)gpio_reg[pin].iomask;
    } else {
        *gpio_reg[pin].lat &= (uint8_t)~gpio_reg[pin].iomask;        
    }
}

void gpio_pin_set(uint8_t pin) {
    *gpio_reg[pin].lat |= (uint8_t)gpio_reg[pin].iomask;
}

void gpio_pin_clr(uint8_t pin) {
    *gpio_reg[pin].lat &= (uint8_t)~gpio_reg[pin].iomask;
}

void gpio_pin_inv(uint8_t pin) {
    *gpio_reg[pin].lat ^= (uint8_t)gpio_reg[pin].iomask;
}

uint8_t gpio_pin_get(uint8_t pin, uint8_t debounce) {
    if (!debounce) {
        return (uint8_t)((*gpio_reg[pin].port & gpio_reg[pin].iomask) != 0);
    } else {
        uint16_t count_l = 0;
        uint16_t count_h = 0;
        
        do {
            gpio_pin_output(pin, 1);
            __delay_us(10);
            gpio_pin_input(pin);
            
            if ((*gpio_reg[pin].port & gpio_reg[pin].iomask) != 0) {
                count_h++;
                count_l = 0;
            } else {
                count_h = 0;
                count_l++;
            }
        } while ((count_l < DEBOUNCE) && (count_h < DEBOUNCE));
        
        return (uint8_t)(count_h >= DEBOUNCE);
    }
}

#if INCLUDE_GPIO_EXTERNAL_INTERRUPTS
void gpio_isr_attach(uint8_t pin, gpio_isr_func_t func, gpio_int_type_t type) {
    gpio_pin_input(pin);
   
    if (pin == B0) {
        INTCONbits.INT0IF = 0;
        INTCONbits.INT0IE = 1;
        INTCON2bits.INTEDG0 = type;
        gpio_isr[0] = func;
    } else if (pin  == B1) {
        INTCON3bits.INT1IF = 0;
        INTCON3bits.INT1IE = 1;
        INTCON2bits.INTEDG1 = type;
        gpio_isr[1] = func;
    } else if (pin == B2) {
        INTCON3bits.INT2IF = 0;
        INTCON3bits.INT2IE = 1;
        INTCON2bits.INTEDG2 = type;
        gpio_isr[2] = func;
    } else if (pin == B2) {
        INTCON3bits.INT3IF = 0;
        INTCON3bits.INT3IE = 1;
        INTCON2bits.INTEDG3 = type;
        gpio_isr[3] = func;
    }
    
    INTCONbits.GIE=1;
}
#endif

