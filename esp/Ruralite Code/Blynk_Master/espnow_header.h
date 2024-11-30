#ifndef T4G_ESP_NOW_UTILITIES_H
#define T4G_ESP_NOW_UTILITIES_H

#include "config.h"
#include "data_structure.h"

/* Global Variables */
uint8_t current_peer_count = 0;      // Number of peers that have been found
bool device_is_master = true;       // Flag to indicate if this device is the master
uint32_t sent_msg_count = 0;         // Counter for the messages sent. Only starts counting after all peers have been found
uint32_t recv_msg_count = 0;         // Counter for the messages received. Only starts counting after all peers have been found
esp_now_data_t broadcast_msg;              // Message that will be sent to the peers
esp_now_data_t command_msg;              // Message that will be sent to the peers
esp_now_data_t sensor_msgs[MAX_LAMP_COUNT];

/* Classes */

// We need to create a class that inherits from ESP_NOW_Peer and implement the _onReceive and _onSent methods.
// This class will be used to store the priority of the device and to send messages to the peers.
// For more information about the ESP_NOW_Peer class, see the ESP_NOW_Peer class in the ESP32_NOW.h file.

class ESP_NOW_Network_Peer : public ESP_NOW_Peer {
public:
  uint8_t position;
  bool peer_is_master = false;
  bool peer_ready = false;

  ESP_NOW_Network_Peer(const uint8_t *mac_addr, uint8_t position = 0, const uint8_t *lmk = (const uint8_t *)ESPNOW_EXAMPLE_LMK)
    : ESP_NOW_Peer(mac_addr, ESPNOW_WIFI_CHANNEL, ESPNOW_WIFI_IFACE, lmk), position(position) {}

  ~ESP_NOW_Network_Peer() {}

  bool begin() {
    // In this example the ESP-NOW protocol will already be initialized as we require it to receive broadcast messages.
    if (!add()) {
      log_e("Failed to initialize ESP-NOW or register the peer");
      return false;
    }
    return true;
  }

  bool send_message(const uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
      log_e("Data to be sent is NULL or has a length of 0");
      return false;
    }

    // Call the parent class method to send the data
    return send(data, len);
  }

  void onReceive(const uint8_t *data, size_t len, bool broadcast) {
    esp_now_data_t *msg = (esp_now_data_t *)data;

    if (peer_ready == false && msg->ready == true) {
      Serial.printf("Peer " MACSTR " reported ready\n", MAC2STR(addr()));
      peer_ready = true;
    }

    if (!broadcast) {
      sensor_msgs[position] = *msg;
      // Serial.printf(
          // "[Light #%d] Lamp Current: %0.2f Solar Voltage: %0.2f Daytime: %d Rain: %d\n", msg->position, msg->lamp_amperage, msg->solar_voltage, msg->day_time, msg->rain_detected);

      // Serial.printf(
          // "Peer: %d Msg: %d\n",sensor_msgs[0]->position, msg->position);
      recv_msg_count++;
        // Serial.printf("Received a message from peer " MACSTR "\n", MAC2STR(addr()));
        last_transmit_time[position] = millis();
    }
  }

  void onSent(bool success) {
    bool broadcast = memcmp(addr(), ESP_NOW.BROADCAST_ADDR, ESP_NOW_ETH_ALEN) == 0;
    if (broadcast) {
      log_i("Broadcast message reported as sent %s", success ? "successfully" : "unsuccessfully");
    } else {
      log_i("Unicast message reported as sent %s to peer " MACSTR, success ? "successfully" : "unsuccessfully", MAC2STR(addr()));
    }
  }
};

/* Peers */

std::vector<ESP_NOW_Network_Peer *> peers;                             // Create a vector to store the peer pointers
ESP_NOW_Network_Peer broadcast_peer(ESP_NOW.BROADCAST_ADDR, 0, NULL);  // Register the broadcast peer (no encryption support for the broadcast address)
ESP_NOW_Network_Peer *master_peer = nullptr;                           // Pointer to peer that is the master
                         // Pointer to peer that is the master

#endif