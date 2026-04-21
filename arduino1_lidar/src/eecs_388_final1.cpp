/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

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
    uart_init();
}

void upload(uint16_t* uploadDist, uint16_t dist, uint32_t* uploadTimer) {
    // only want to get a new value every 200ms
    if ((millis() % UINT_MAX) - (*uploadTimer) >= 200) {
        *uploadDist = dist;
        uint8_t up = uint8_t(dist);
        if (up < *uploadDist) up = UCHAR_MAX;
        ser_write(up);
    }
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
    // want a rolling average
    uint16_t dist = 0;              /* LIDAR distance data is 16 bits. */

    uint32_t uploadTimer = millis();
    uint16_t uploadDist = dist;

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

        upload(&uploadDist, dist, &uploadTimer);
    }
}