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
    DUMMY_COMMAND,
    FAULT_CHECK,
    LED_FULL_BEAM,
    LED_LOW_BEAM,
    LED_NO_BEAM,
    ENABLE_LIGHTING_ASSIST,
    DISABLE_LIGHTING_ASSIST
};

#endif