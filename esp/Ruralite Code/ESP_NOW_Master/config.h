#ifndef T4G_CONFIG_H
#define T4G_CONFIG_H

// this variable controls how many slaves can be connected to the Blynk Server.
#define MAX_LAMP_COUNT 2

// This address should be the same for the ESPNOW ESP32 Master
#define ESPNOW_MASTER_I2C_ADDRESS 9

/*
    ESP-NOW Network Example
    Lucas Saavedra Vaz - 2024

    This example is based on the ESP-NOW example from the ESP-IDF framework.

    The aim of this example is to demonstrate how to create a network of devices using the ESP-NOW protocol.
    The slave devices will broadcast random data to the master device every 5 seconds and from time to time
    they will ping the other slave devices with a "Hello!" message.

    The master device will receive the data from the slave devices and print it to the Serial Monitor. From time
    to time, the master device will calculate the average of the priorities of the slave devices and send it to
    all the slave devices.

    Each device will have a priority that will be used to decide which device will be the master.
    The device with the highest priority will be the master.

    Flow:
    1. Each device will generate a priority based on its MAC address.
    2. The devices will broadcast their priority on the network.
    3. The devices will listen to the broadcast messages and register the priorities of the other devices.
    4. After all devices have been registered, the device with the highest priority will be the master.
    5. The slave devices will send random data to the master every 5 seconds.
      - Every "REPORT_INTERVAL" messages, the slaves will send a message to the other slaves.
    6. The master device will calculate the average of the data and send it to the slave devices every "REPORT_INTERVAL" messages.

*/

/*
    ESP-NOW Network Example
    Lucas Saavedra Vaz - 2024

    This example is based on the ESP-NOW example from the ESP-IDF framework.

    The aim of this example is to demonstrate how to create a network of devices using the ESP-NOW protocol.
    The slave devices will broadcast random data to the master device every 5 seconds and from time to time
    they will ping the other slave devices with a "Hello!" message.

    The master device will receive the data from the slave devices and print it to the Serial Monitor. From time
    to time, the master device will calculate the average of the priorities of the slave devices and send it to
    all the slave devices.

    Each device will have a priority that will be used to decide which device will be the master.
    The device with the highest priority will be the master.

    Flow:
    1. Each device will generate a priority based on its MAC address.
    2. The devices will broadcast their priority on the network.
    3. The devices will listen to the broadcast messages and register the priorities of the other devices.
    4. After all devices have been registered, the device with the highest priority will be the master.
    5. The slave devices will send random data to the master every 5 seconds.
      - Every "REPORT_INTERVAL" messages, the slaves will send a message to the other slaves.
    6. The master device will calculate the average of the data and send it to the slave devices every "REPORT_INTERVAL" messages.

*/

#include "ESP32_NOW.h"
#include "WiFi.h"

#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

#include <vector>

/* Definitions */

// Wi-Fi interface to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_IFACE WIFI_IF_STA

// Channel to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_CHANNEL 0

// Delay between sending messages
#define ESPNOW_SEND_INTERVAL_MS 2000

// Number of peers to wait for (excluding this device)
#define ESPNOW_PEER_COUNT 2

// Report to other devices every 5 messages
#define REPORT_INTERVAL 5

/*
    ESP-NOW uses the CCMP method, which is described in IEEE Std. 802.11-2012, to protect the vendor-specific action frame.
    The Wi-Fi device maintains a Primary Master Key (PMK) and several Local Master Keys (LMK).
    The lengths of both PMK and LMK need to be 16 bytes.

    PMK is used to encrypt LMK with the AES-128 algorithm. If PMK is not set, a default PMK will be used.

    LMK of the paired device is used to encrypt the vendor-specific action frame with the CCMP method.
    The maximum number of different LMKs is six. If the LMK of the paired device is not set, the vendor-specific
    action frame will not be encrypted.

    Encrypting multicast (broadcast address) vendor-specific action frame is not supported.

    PMK needs to be the same for all devices in the network. LMK only needs to be the same between paired devices.
*/

// Primary Master Key (PMK) and Local Master Key (LMK)
#define ESPNOW_EXAMPLE_PMK "pmk1234567890123"
#define ESPNOW_EXAMPLE_LMK "lmk1234567890123"

#define LED_PIN 4

const uint16_t TRANSMIT_TIME = 250; 
unsigned long last_transmit_time[MAX_LAMP_COUNT];

#endif