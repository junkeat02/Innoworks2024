#ifndef T4G_ESP_NOW_UTILITIES_H
#define T4G_ESP_NOW_UTILITIES_H

#include "config.h"
#include "data_structure.h"

/* Global Variables */
uint8_t current_peer_count = 0;      // Number of peers that have been found
const bool device_is_master = false; // Flag to indicate if this device is the master
bool master_found = false;           // Flag to indicate if a master has been found
uint32_t sent_msg_count = 0;         // Counter for the messages sent. Only starts counting after all peers have been found
uint32_t recv_msg_count = 0;         // Counter for the messages received. Only starts counting after all peers have been found
esp_now_data_t broadcast_msg;        // Message that will be sent to the peers
esp_now_data_t sensor_msg;           // Message that will be sent to the master

bool fault_checking = false;
bool lamp_is_faulty = false;
bool run_command = false;

// lamp current sensor variables
float lamp_shuntvoltage = 0;
float lamp_busvoltage = 0;
float lamp_current_mA = 0;
float lamp_loadvoltage = 0;
float lamp_power_mW = 0;

// lamp current sensor variables
float solar_shuntvoltage = 0;
float solar_busvoltage = 0;
float solar_current_mA = 0;
float solar_loadvoltage = 0;
float solar_power_mW = 0;

void runCommand(uint8_t cmd)
{
  last_command_run_time = millis();
  run_command = true;

  switch (cmd)
  {
  case FAULT_CHECK:
    // do nothing
    break;

  case LED_FULL_BEAM:
    // toggle LED_FULL_BEAM
    target_lamp_pwm = FULL_BEAM_BRIGHTNESS;
    Serial.println("Toggled LED_FULL_BEAM");
    break;

  case LED_LOW_BEAM:
    // toggle LED_LOW_BEAM
    target_lamp_pwm = LOW_BEAM_BRIGHTNESS;
    Serial.println("Toggled LED_LOW_BEAM");
    break;

  case LED_NO_BEAM:
    // toggle LED_NO_BEAM
    target_lamp_pwm = 0;
    Serial.println("Toggled LED_NO_BEAM");
    break;

  case ENABLE_LIGHTING_ASSIST:
    // toggle ENABLE_LIGHTING_ASSIST
    target_lamp_pwm = LIGHTING_ASSIST_BRIGHTNESS;
    Serial.println("Toggled ENABLE_LIGHTING_ASSIST");
    break;

  case DISABLE_LIGHTING_ASSIST:
    // toggle DISABLE_LIGHTING_ASSIST
    Serial.println("Toggled DISABLE_LIGHTING_ASSIST");
    break;

  case DUMMY_COMMAND:
  default:
    break;
  }
}

/* Classes */

// We need to create a class that inherits from ESP_NOW_Peer and implement the _onReceive and _onSent methods.
// This class will be used to store the priority of the device and to send messages to the peers.
// For more information about the ESP_NOW_Peer class, see the ESP_NOW_Peer class in the ESP32_NOW.h file.

class ESP_NOW_Network_Peer : public ESP_NOW_Peer
{
public:
  uint8_t position;
  bool peer_is_master = false;
  bool peer_ready = false;

  ESP_NOW_Network_Peer(const uint8_t *mac_addr, uint8_t position = 0, const uint8_t *lmk = (const uint8_t *)ESPNOW_EXAMPLE_LMK)
      : ESP_NOW_Peer(mac_addr, ESPNOW_WIFI_CHANNEL, ESPNOW_WIFI_IFACE, lmk), position(position) {}

  ~ESP_NOW_Network_Peer() {}

  bool begin()
  {
    // In this example the ESP-NOW protocol will already be initialized as we require it to receive broadcast messages.
    if (!add())
    {
      log_e("Failed to initialize ESP-NOW or register the peer");
      return false;
    }
    return true;
  }

  bool send_message(const uint8_t *data, size_t len)
  {
    if (data == NULL || len == 0)
    {
      log_e("Data to be sent is NULL or has a length of 0");
      return false;
    }

    // Call the parent class method to send the data
    return send(data, len);
  }

  void onReceive(const uint8_t *data, size_t len, bool broadcast)
  {
    esp_now_data_t *msg = (esp_now_data_t *)data;

    if (peer_ready == false && msg->ready == true)
    {
      Serial.printf("Peer " MACSTR " reported ready\n", MAC2STR(addr()));
      peer_ready = true;
    }

    if (!broadcast)
    {
      recv_msg_count++;

      if (peer_is_master)
      {
        Serial.println("Received a message from the master");

        // check for command
        if (msg->device_type == MASTER_DEVICE)
        {
          if (msg->message_type == COMMAND_MESSAGE)
          {

            const uint8_t cmd = msg->command;
            switch (cmd)
            {
            case FAULT_CHECK:
              // toggle fault check
              fault_check_last_delay_time = millis();
              fault_checking = true;
              Serial.println("Toggled FAULT_CHECK");
              break;

            case LED_FULL_BEAM:
              runCommand(cmd);
              break;

            case LED_LOW_BEAM:
              // toggle LED_LOW_BEAM
              runCommand(cmd);
              break;

            case LED_NO_BEAM:
              // toggle LED_NO_BEAM
              runCommand(cmd);
              break;

            case ENABLE_LIGHTING_ASSIST:
              // toggle ENABLE_LIGHTING_ASSIST
              runCommand(cmd);
              break;

            case DISABLE_LIGHTING_ASSIST:
              // toggle DISABLE_LIGHTING_ASSIST
              runCommand(cmd);
              break;

            case DUMMY_COMMAND:
            default:
              run_command = false;
              break;
            }
          }
        }
      }
      else
      {
        Serial.printf("Peer " MACSTR "\n", MAC2STR(addr()));
      }

      last_transmit_time = millis();
    }
  }

  void onSent(bool success)
  {
    bool broadcast = memcmp(addr(), ESP_NOW.BROADCAST_ADDR, ESP_NOW_ETH_ALEN) == 0;
    if (broadcast)
    {
      log_i("Broadcast message reported as sent %s", success ? "successfully" : "unsuccessfully");
    }
    else
    {
      log_i("Unicast message reported as sent %s to peer " MACSTR, success ? "successfully" : "unsuccessfully", MAC2STR(addr()));
    }
  }
};

/* Peers */

std::vector<ESP_NOW_Network_Peer *> peers;                            // Create a vector to store the peer pointers
ESP_NOW_Network_Peer broadcast_peer(ESP_NOW.BROADCAST_ADDR, 0, NULL); // Register the broadcast peer (no encryption support for the broadcast address)
ESP_NOW_Network_Peer *master_peer = nullptr;                          // Pointer to peer that is the master
                                                                      // Pointer to peer that is the master

#endif