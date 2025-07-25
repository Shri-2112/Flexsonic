##ESP32

The ESP32 is a series of chip microcontrollers developed by Espressif. 

// Peripherals

Capacitive touch, ADC (analog to digital converter), DAC (digital to analog converter), 
I2C (Inter-Integrated Circuit), UART (universal asynchronous receiver/transmitter), 
CAN 2.0 (Controller Area Netwokr), SPI (Serial Peripheral Interface), I2S (Integrated Inter-IC Sound), 
RMII (Reduced Media-Independent Interface), PWM (pulse width modulation), and more.

## Analog to Digital Converter (ADC) Oneshot Mode Driver

ADC (Analog-to-Digital Converter) is a hardware feature that converts analog voltages 
(like the one from a flex sensor) into a digital number (usually from 0 to 4095 for 12-bit ADC).

## UART & DFPlayer

What is DFPlayer Mini?

DFPlayer Mini is a low-cost MP3 player module that:

    Plays .mp3 files from a microSD card
    Can be controlled using UART serial commands
    Has its own audio amplifier and works with a small speaker

What is UART?

    TX (Transmit): sends data
    RX (Receive): receives data
    Serial communication protocol
    Baud rate = 9600 bps for DFPlayer

Why Voltage Divider is Needed?

DFPlayer runs at 5V logic, but ESP32’s RX pin accepts only 3.3V max.

So:
SP32 TX → DFPlayer RX = Needs voltage divider (5V → ~3.3V)
DFPlayer TX → ESP32 RX = Usually safe, but best to double-check

How to Send Commands to DFPlayer
[0] 0x7E   // Start byte
[1] 0xFF   // Version
[2] 0x06   // Length
[3] 0xXX   // Command byte (e.g., 0x03 = play track)
[4] 0x00   // No feedback
[5] 0xYY   // High byte of parameter
[6] 0xZZ   // Low byte of parameter
[7] CRC High
[8] CRC Low
[9] 0xEF   // End byte

Checksum = 0xFFFF - (sum of bytes 1 to 6) + 1

## Embedded C
In Embedded C, you're talking directly to the hardware.
Embedded C is a variant of the C programming language specifically optimized to 
program embedded systems — small computers like:

    Microcontrollers (e.g., ESP32, Arduino, STM32, PIC)

    Devices inside appliances, wearables, robots, etc.

It lets you directly control hardware: pins, sensors, timers, memory, and peripherals 
like UART, I²C, PWM, ADC, etc.

#Toggles an LED

'''
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_2

void app_main() {
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

  while(1) {
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    gpio_set_level(LED_PIN, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

'''

## What is FreeRTOS?

    A Real-Time Operating System (RTOS) allows you to run multiple tasks in parallel on microcontrollers.


