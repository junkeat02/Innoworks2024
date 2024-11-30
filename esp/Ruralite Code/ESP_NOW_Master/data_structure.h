#ifndef T4G_DATA_STRUCTURE_H
#define T4G_DATA_STRUCTURE_H

/* Structs */

// The following struct is used to send data to the peer device.
// We use the attribute "packed" to ensure that the struct is not padded (all data
// is contiguous in the memory and without gaps).
// The maximum size of the complete message is 250 bytes (ESP_NOW_MAX_DATA_LEN).

typedef struct {
  uint8_t position;
  uint8_t device_type;
  uint8_t message_type;
  uint8_t command;
  bool motion_detected;
  bool rain_detected;
  bool day_time;
  float lamp_amperage;
  float lamp_power;
  float solar_voltage;
  bool faulty;
  bool ready;
} __attribute__((packed)) esp_now_data_t;


// Enum for Device Type
enum DEVICE_TYPE {
    MASTER_DEVICE,
    SLAVE_DEVICE
};

// Enum for Message Type
enum MESSAGE_TYPE {
    COMMAND_MESSAGE,
    SENSOR_MESSAGE,
    BROADCAST_MESSAGE,
};

// Enum for Commands
enum COMMAND_TYPE {
    DUMMY_COMMAND,          // 0
    FAULT_CHECK,            // 1
    LED_FULL_BEAM,          // 2
    LED_LOW_BEAM,           // 3
    LED_NO_BEAM,            // 4
    ENABLE_LIGHTING_ASSIST, // 5
    DISABLE_LIGHTING_ASSIST // 6
};

union i2c_data_t
{
    esp_now_data_t data;
    byte bytes[sizeof(esp_now_data_t)];
};

#endif