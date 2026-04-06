/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "eecs_388_lib.h"

/******************************************************************************
 *   Function: setup() - Initializes the Arduino System
 *      Pre condition: 
 *          Hardware must be properly connected (sensors, LEDS, etc.)
 *      Post condition: 
 *          Runs initialization calls one time on power up
 *          Input/Output (IO) pins are configured
 *******************************************************************************/
void setup() {
    gpio_mode(LED_RED, GPIO_OUTPUT);
    gpio_mode(LED_GREEN, GPIO_OUTPUT);
    gpio_mode(LED_BLUE, GPIO_OUTPUT);

    uart_init();
}

void setLED(uint8_t red, uint8_t green, uint8_t blue) {
    gpio_write(LED_RED, red);
    gpio_write(LED_GREEN, green);
    gpio_write(LED_BLUE, blue);
}

/******************************************************************************
 *   Function: loop() - Main execution loop
 *      Pre condition: 
 *          setup() has been executed and system is initialized
 *      Post condition: 
 *          Performs a single iteration of the system's function
 *          Repeates indefinetely unless the board is reset or powered off
 *******************************************************************************/
void loop() {
    uint16_t dist = 0;              /* LIDAR distance data is 16 bits. */
    int flashing = millis();
    uint8_t redFlash = 0;

    ser_printline("Setup completed.");
    ser_write('\n');

    while(1) {
        char dist_l, dist_h;
        if ('Y' == ser_read() && 'Y' == ser_read()) {
            dist_l = ser_read();
            dist_h = ser_read();
            // calculate checksum
            char calculated_checksum = ('Y' + 'Y' + dist_l + dist_h);
            for (int i = 0; i < 4; i++)
                calculated_checksum += ser_read();

            if (calculated_checksum != ser_read()) {
                ser_printf("Checksum Failed");
                continue;
            }
        }
        dist = (dist_h << 8) | dist_l;
        ser_printf("%d", dist);

        if (dist > 200) {
            setLED(OFF, ON, OFF);
        } else if (100 < dist && dist <= 200) {
            setLED(ON, ON, OFF);
        } else if (60 < dist && dist <= 100) {
            setLED(ON, OFF, OFF);
        } else if (dist <= 60) {
            if (millis() - flashing >= 100) { // if it has been more than 100ms
                // flip flashing
                redFlash ^= 1;
                setLED(redFlash, OFF, OFF);
                flashing = millis();
            }
        }
    }
}