#ifndef T4G_CONFIG_H
#define T4G_CONFIG_H

#include <vector>

// this variable controls how many slaves can be connected to the Blynk Server.
#define MAX_LAMP_COUNT 2

// I2C Address for Slave ESP
// This address should be the same for the ESPNOW ESP32 Master
#define ESPNOW_MASTER_I2C_ADDRESS 9

// comment out the below line to disable LED Strip functions 
#define ENABLE_LED_STRIP

/* Definitions */
#define NUM_LEDS 3 
#define DATA_PIN 4

// Define the array of leds
CRGB leds[NUM_LEDS];

// Report to other devices every 5 messages
#define REPORT_INTERVAL 5

const uint16_t TRANSMIT_TIMEOUT = 250; 
unsigned long last_transmit_time[MAX_LAMP_COUNT];

#endif