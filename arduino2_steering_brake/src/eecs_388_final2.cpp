/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#include "eecs_388_lib.h"

/******************************************************************************
 *   Constant definitions
 *******************************************************************************/

/*ServoMotor constants*/
#define SERVO_PULSE_MAX (2400)    /* 2400 us */
#define SERVO_PULSE_MIN (544)     /* 544 us */
#define SERVO_PERIOD    (20000)   /* 20000 us (20ms) */
#define MIN_ANGLE       (0)       /* degrees */
#define MAX_ANGLE       (180)     /* degrees */

#define SET_GREEN()     setLED(OFF, ON, OFF)
#define SET_YELLOW()    setLED(OFF, OFF, ON) // FIXME: put this back
#define SET_RED()       setLED(ON, OFF, OFF);

#define MILLIS_SAFE (millis() % UINT_MAX)

#define TIMER(__T, __D) ((MILLIS_SAFE - (uint32_t)(__T)) >= (uint32_t)(__D))

int angleIndex;
uint32_t flashing;
uint8_t redFlash;
uint32_t start;


void setLED(uint8_t red, uint8_t green, uint8_t blue) {
    gpio_write(LED_RED, red);
    gpio_write(LED_GREEN, green);
    gpio_write(LED_BLUE, blue);
}

/******************************************************************************
 *   Function: auto_brake() - Auto Brake
 *      Pre condition:
 *          None
 *      Post condition:
 *          Checks the LiDAR distance and configures LEDs
 *******************************************************************************/
static void auto_brake(uint8_t* redFlash, uint32_t* flashing) {
    // Task-2: 
    // Your code goes here (Use Lab 2 & 4 for reference)
    // Check the project document to understand the task
    uint8_t dist = ser_read();
    // ser_printf("rx: %d", (int)dist);

    if (dist > 200) {
        SET_GREEN();
    } else if (100 < dist && dist <= 200) {
        SET_YELLOW();
    } else if (60 < dist && dist <= 100) {
        SET_RED();
    } else if (dist <= 60) {
        if (TIMER(*flashing, 100)) { // if it has been more than 100ms
            // flip flashing
            (*redFlash) ^= 1;
            setLED((*redFlash), OFF, OFF);
            (*flashing) = MILLIS_SAFE;
        }
    }
}

static int getPulseLength(int angle) {
    // formula to get the pulse length from the angle
    // formula is line between two points: (0, 544); (180, 2400)
    double p = (double)angle * 464.0 / 45.0 + 544.0;
    return floor(p);
}

/******************************************************************************
 *   Function: steering() - Steering
 *      Pre condition: 
 *          None
 *      Post condition: 
 *          Control the servomotor with GPIO_6
 *******************************************************************************/
static void steering(int pos) {
    int len = getPulseLength(pos);

    gpio_write(GPIO_6, ON);
    delay_us(len);
    // turn off for the remaining time
    gpio_write(GPIO_6, OFF);
    delay_us(10000); // can't call for longer than 16ms so we just take 10ms in one call
    delay_us(10000 - len);
}

/******************************************************************************
 *   Function: setup() - Initializes the Arduino System
 *      Pre condition: 
 *          Hardware must be properly connected (BMP180 sensors, etc.)
 *      Post condition: 
 *          Runs initialization calls one time on power up
 *          UART is initialized for ser_printf()
 *          I2C is initialized for communication with BMP180
 *          calib_data is filled with calibration data from BMP180
 *******************************************************************************/
void setup()  {
    uart_init();     // Initialize UART for serial output

    //Setup Auto-break LEDs for Distance
    gpio_mode(LED_RED, GPIO_OUTPUT); //RED
    gpio_mode(LED_GREEN, GPIO_OUTPUT); //GREEN
    gpio_mode(LED_BLUE, GPIO_OUTPUT); //BLUE

    //Setup GPIO_6 for PWM output
    gpio_mode(GPIO_6, GPIO_OUTPUT);

    ser_printf("System Initialized");

    angleIndex = 0;
    redFlash = 0;
    flashing = millis();
    start = millis();
}

/******************************************************************************
 *   Function: loop() - Main execution loop
 *      Pre condition: 
 *          setup() has been executed and system is initialized
 *      Post condition: 
 *          Performs a single iteration of the system's function
 *          Repeates indefinetely unless the board is reset or powered off
 *******************************************************************************/
void loop()  {
    // Task-4:
    // Setup simulated code for the angles from the lab sheet
    int angleValues[] = {10, 25, 75, 45, 100, 40, 125, 15, 150, 50, 170};

    // Task-1&2:
    // Receive data from arudino1 for the distance and setup the LEDs
    auto_brake(&redFlash, &flashing);

    //  Task-4:
    steering(angleValues[angleIndex]);

    if (TIMER(start, 1000)) {
        start = MILLIS_SAFE;

        angleIndex++;
        angleIndex %= 11;
    }
}
