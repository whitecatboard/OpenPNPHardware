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

#include "gpio.h"
#include "can.h"
#include "task.h"
#include "motor.h"
#include "eeprom.h"

#include <xc.h>
#include <string.h>
#include <stdint.h>

// If no hole is detected within this time a feeder error will be triggered
#define FEED_TIMEOUT 1500

// EEPROM data
#define FEEDER_ID      0x0000 // Feeder id
#define FEEDER_PITCH   0x0001 // Feeder pitch
#define FEED_OK        0x0002 // Number of feed request OK
#define FEED_ERR       0x0006 // Number of feed request ERR

// Hardware mapping
#define STAT1_LED C0 // STAT1 led
#define STAT2_LED C1 // STAT2 led
#define MOTOR_CW  C5 // CW pin
#define MOTOR_CCW C4 // CCW pin
#define BUTTON1   C2 // Button 1
#define BUTTON2   C3 // Button 2
#define SWITCH1   B0 // Switch 1
#define SWITCH2   B1 // Switch 2

// Feeder status
typedef enum {
    StatusUnknown = 1,
    StatusPrevious = 2,
    StatusNotConfigured = 3,
    StatusWaitForConfig = 4,
    StatusReady = 5,
    StatusOnFeed = 6,
    StatusOnUnroll = 7,
    StatusDuplicatedId = 8,
} feeder_status_t;

// Feeder pitch types
typedef enum {
    Pitch_1_2 = 1, // Pitch 0.5
    Pitch_1,       // Pitch 1
    Pitch_2,       // Pitch 2
    Pitch_3,       // Pitch 3
    Pitch_4,       // Pitch 4
} feeder_pitch_t;

// Can message types
#define SERVICE_MSG 0x100
#define FEED_MSG    0X200
#define RESP_MSG    0x300
#define COUNT_MSG   0x301

// Service message
typedef enum {
    ServiceSetId = 1,
    ServiceSetPitch = 2,
    ServiceUpdate = 3,
    ServiceSaveCounters = 4,
    ServiceGetCounters = 5,
    ServiceResetCounters = 6,
} service_msg_type_t;

typedef struct {
    service_msg_type_t type; // Type of service message
    uint8_t data0;           // Message data 0
    uint8_t data1;           // Message data 1
} service_msg_t;

// Feed message
typedef struct {
    uint8_t feeder_id; // Feeder to perform feed operation
} feed_msg_t;

typedef struct {
    uint8_t feeder_id; // Feeder that has performed a feed operation
    uint8_t status;    // Feed operation status. 1 = ok, 0 = error
} feed_msg_response_t;

// Counter message
typedef struct {
    uint8_t feeder_id;
    uint8_t counter_id; // 1 = feed_ok, 2 = feed_err
    uint32_t value;
} counters_msg_t;

extern uint16_t _time_delta;
extern uint16_t _time_delta_led;
extern uint16_t _time_delta_button;

static uint8_t feeder_id = 0x00;
static feeder_pitch_t feeder_pitch = 0;

// Feeder status
static feeder_status_t status = StatusNotConfigured;
static feeder_status_t prev_status = StatusNotConfigured;

// Number of holes detected on current feed operation
static uint8_t detected_holes = 0;

// Number of holes required to get the next component for a feed operation
static uint8_t pitch_holes = 0;

// Counters
static uint32_t feed_ok = 0;
static uint32_t feed_err = 0;

static void update_status(feeder_status_t new_status) {
    if (new_status == StatusUnknown) {
        if (status != StatusDuplicatedId) {
            if ((feeder_id == 0) || (feeder_id == 0xff)) {
                if (status != new_status) {
                    prev_status = status;
                }
                
                status = StatusNotConfigured;
            } else {
                prev_status = StatusReady;
                status = StatusReady;
            }             
        }
        
        if ((feeder_pitch == 0) || (feeder_pitch == 0xff)) {
            // Default pitch
            feeder_pitch = Pitch_1;
            
            eeprom_write8(FEEDER_PITCH, feeder_pitch);
        }
        
        if (feed_ok == 0xffffffff) {
            feed_ok = 0;
        }
        
        if (feed_err == 0xffffffff) {
            feed_err = 0;
        }
    } else if (new_status == StatusPrevious) {
        status = prev_status;
    } else {
        if (status != new_status) {
            prev_status = status;
        }
        status = new_status;
    }
}

static uint8_t feed() {  
    uint8_t ok = 1;
    
    // Reset number of detected holes
    detected_holes = 0; 
    
    // Compute required number of holes to feed a component
    switch (feeder_pitch) {
        case Pitch_1_2: pitch_holes = 1; break;
        case Pitch_1:   pitch_holes = 1; break;
        case Pitch_2:   pitch_holes = 2; break;
        case Pitch_3:   pitch_holes = 3; break;
        case Pitch_4:   pitch_holes = 4; break;
    }    

    // Set timeout
    uint16_t timeout = FEED_TIMEOUT * pitch_holes;
    
    // Reset time
    _time_delta = 0;
    
    // Start move motor
    motor_cw();

    while ((detected_holes < pitch_holes) && (ok)) {
        ok = _time_delta < timeout;
        
        asm("nop");
    }
    
    if (!ok) {
        // If timeout stop motor
        motor_stop();
    }    
    
    update_status(StatusPrevious);

    return ok;
}

static void unroll(void) {
    update_status(StatusOnUnroll);

    motor_ccw();
    while (!gpio_pin_get(BUTTON2, 1));
    motor_stop();

    update_status(StatusPrevious);
}

static void process_feed_msg(can_frame_t *cf) {
    feed_msg_t msg;
    can_frame_t response;
    
    // Check that message size corresponds to a feed message
    if (cf->dlc == sizeof(feed_msg_t)) {
        // Get message
        memcpy(&msg, cf->data, cf->dlc);
        
        // Check that message is for us
        if (msg.feeder_id == feeder_id) {
            update_status(StatusOnFeed);    
            __delay_ms(5);
            task_disable();

            if (feed()) {
                feed_ok++;
                
                // Send response            
                response.sid = RESP_MSG;
                response.dlc = sizeof(feed_msg_response_t);
                response.data[0] = feeder_id;
                response.data[1] = 0x01;

                can_send(&response);                
            } else {
                feed_err++;
                
                // Send response            
                response.sid = RESP_MSG;
                response.dlc = sizeof(feed_msg_response_t);
                response.data[0] = feeder_id;
                response.data[1] = 0x00;

                can_send(&response);                                
            }
           
            task_enable();
        }
    }
}

static void process_service_msg(can_frame_t *cf) {
    service_msg_t msg;
    
    // Check that message size corresponds to a service message
    if (cf->dlc == sizeof(service_msg_t)) {
        // Get message
        memcpy(&msg, cf->data, cf->dlc);
        
        if ((msg.type == ServiceSetId) && (status == StatusWaitForConfig)) {
            feeder_id = msg.data0;
            
            eeprom_write8(FEEDER_ID, feeder_id);
            
            update_status(StatusUnknown);
        } else if ((msg.type == ServiceSetPitch) && (status == StatusReady)) {
            if (feeder_id == msg.data0) {
                feeder_pitch = msg.data1;
            
                eeprom_write8(FEEDER_PITCH, feeder_pitch);                
            }
        } else if ((msg.type == ServiceUpdate) && (status == StatusReady)) {
            if (msg.data0 == feeder_id) {
                asm("reset");
            }
        } else if ((msg.type == ServiceSaveCounters) && (status == StatusReady)) {
            if (msg.data0 == feeder_id) {
                eeprom_write32(FEED_OK, feed_ok);
                eeprom_write32(FEED_ERR, feed_err);
            }
        } else if ((msg.type == ServiceGetCounters) && (status == StatusReady)) {
           if (msg.data0 == feeder_id) {               
                // Send counters
                can_frame_t response;
                
                // feed_ok
                response.sid = COUNT_MSG;
                response.dlc = sizeof(counters_msg_t);
                response.data[0] = feeder_id;
                response.data[1] = 1;
                response.data[2] = (uint8_t)(feed_ok >> 24);
                response.data[3] = (uint8_t)(feed_ok >> 16);
                response.data[4] = (uint8_t)(feed_ok >> 8);
                response.data[5] = (uint8_t)(feed_ok);
                
                can_send(&response);                     

                // feed_err
                response.sid = COUNT_MSG;
                response.dlc = sizeof(counters_msg_t);
                response.data[0] = feeder_id;
                response.data[1] = 2;
                response.data[2] = (uint8_t)(feed_err >> 24);
                response.data[3] = (uint8_t)(feed_err >> 16);
                response.data[4] = (uint8_t)(feed_err >> 8);
                response.data[5] = (uint8_t)(feed_err);
                
                can_send(&response);   
                
                task_enable();
            }
        } else if ((msg.type == ServiceResetCounters) && (status == StatusReady)) {
           if (msg.data0 == feeder_id) {         
               feed_ok = 0;
               feed_err = 0;
               
               eeprom_write32(FEED_OK, feed_ok);
               eeprom_write32(FEED_ERR, feed_err);
           }      
        }
    }
}
                
static void leds(void) {
    if (status == StatusNotConfigured) {
        if (_time_delta_led > 125) {
            _time_delta_led = 0;
            gpio_pin_inv(STAT1_LED);
        }
        
        gpio_pin_clr(STAT2_LED);
    } else if (status == StatusWaitForConfig) {
        if (_time_delta_led > 250) {
            _time_delta_led = 0;
            gpio_pin_inv(STAT1_LED);
        }
        
        gpio_pin_clr(STAT2_LED);        
    } else if (status == StatusReady) {        
        _time_delta_led = 0;
        
        gpio_pin_set(STAT1_LED);        
        gpio_pin_clr(STAT2_LED);        
    } else if (status == StatusOnFeed) {
        _time_delta_led = 0;
        gpio_pin_set(STAT1_LED);        
        gpio_pin_set(STAT2_LED);        
    } else if (status == StatusOnUnroll) {
        _time_delta_led = 0;
        gpio_pin_clr(STAT1_LED);        
        gpio_pin_set(STAT2_LED);        
    } else if (status == StatusDuplicatedId) {
        if (_time_delta_led > 250) {
            _time_delta_led = 0;
            
            gpio_pin_inv(STAT1_LED);        
            gpio_pin_inv(STAT2_LED);                    
        }
    }
}

static void on_hole_1() {
    // Increment number of detectec holes
    detected_holes++;
    
    if (detected_holes >= pitch_holes) {
        // Stop motor
        motor_stop();        
    }        
    
    if (status == StatusReady) {
        gpio_pin_set(STAT2_LED);
        __delay_ms(50);
        gpio_pin_clr(STAT2_LED);
    }
}

static void on_hole_2() {
    if (feeder_pitch == Pitch_1_2) {
        // Increment number of detectec holes
        detected_holes++;

        if (detected_holes >= pitch_holes) {
            // Stop motor
            motor_stop();        
        }        

        if (status == StatusReady) {
            gpio_pin_set(STAT2_LED);
            __delay_ms(50);
            gpio_pin_clr(STAT2_LED);
        }        
    }
}

void schelude_tasks(void) {
    leds();
}

void main(void) {
    can_frame_t cf; // Current can frame

    // Init platform, default settings
    platform_init();
    
    // Init leds
    gpio_pin_output(STAT1_LED, 1);
    gpio_pin_output(STAT2_LED, 1);
    
    // Install an ISR on switches to detect when we are on a tape hole.
    // Tape holes are detected when a raising edge is detected.
    gpio_isr_attach(SWITCH1, on_hole_1, GPIOIntRaisingEdge);
    gpio_isr_attach(SWITCH2, on_hole_2, GPIOIntRaisingEdge);
    
    // Init motor
    motor_attach(MOTOR_CW, MOTOR_CCW);
    
    // Read configuration from EEPROM
    feeder_id = eeprom_read8(FEEDER_ID);
    feeder_pitch = eeprom_read8(FEEDER_PITCH);
    
    // Read counters
    feed_ok = eeprom_read32(FEED_OK);
    feed_err = eeprom_read32(FEED_ERR);
    
    // Init CAN
    can_setup_init();
    can_add_filter(SERVICE_MSG);
    can_add_filter(FEED_MSG);
    can_setup_end();

    // Init tasks
    task_init();
          
    update_status(StatusUnknown);
          
    while (1) {
        // Process CAN messages
        if (can_get(SERVICE_MSG, &cf)) {
            process_service_msg(&cf);
        }

        if (can_get(FEED_MSG, &cf) && (status == StatusReady)) {
            process_feed_msg(&cf);
        }
        
        // Process buttons
        if (((status == StatusNotConfigured) || (status == StatusDuplicatedId) || (status == StatusReady)) && (!gpio_pin_get(BUTTON1, 1))) {
            if ((status == StatusNotConfigured) || (status == StatusDuplicatedId)) {
                update_status(StatusWaitForConfig);
            } else if ((status == StatusReady)) {
                // How many time button is pressed
                uint16_t pressed_time;
                
                _time_delta_button = 0;

                while (!gpio_pin_get(BUTTON1, 1)) {
                    asm("nop");
                }
                
                pressed_time = _time_delta_button;
                
                if ((pressed_time >= 1000) && (pressed_time <= 2000)) {
                    if (status == StatusReady) {
                        update_status(StatusWaitForConfig);                    
                    }
                } else if (status == StatusReady) {
                    update_status(StatusOnFeed);    
                    __delay_ms(5);
                    task_disable();

                    feed();

                    task_enable();
                }                
            }
        }
        
        if ((status == StatusReady) && (!gpio_pin_get(BUTTON2, 1))) {
            unroll();
        }
    } 
}
