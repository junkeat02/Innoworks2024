#include "espnow_header.h"
#include "Wire.h"
#include <ArduinoJson.h>

const uint8_t I2C_DATA_SIZE = sizeof(esp_now_data_t) * MAX_LAMP_COUNT;
byte I2C_DATA[I2C_DATA_SIZE];
bool i2c_transmissionAvailable = false;

/* Helper functions */

// Function to reboot the device
void fail_reboot()
{
  Serial.println("Rebooting in 5 seconds...");
  delay(5000);
  ESP.restart();
}

// Function to check if all peers are ready
bool check_all_peers_ready()
{
  for (auto &peer : peers)
  {
    if (!peer->peer_ready)
    {
      return false;
    }
  }
  return true;
}

// Function to Ping Master
void ping_slaves()
{

  bool enable_led = false; 

  for (uint8_t i = 0; i < MAX_LAMP_COUNT; i++)
  {
    if (millis() - last_transmit_time[i] > TRANSMIT_TIME)
    {
      sensor_msgs[i] = {0}; // clear stored sensor readings

      // broadcast
      if (!broadcast_peer.send_message((const uint8_t *)&broadcast_msg, sizeof(broadcast_msg)))
      {
        Serial.println("Failed to broadcast message");
      }

      last_transmit_time[i] = millis();
    }

    if(sensor_msgs[i].ready) enable_led = true;
  }

  // turn on LED if any device is connected
  if(enable_led) analogWrite(LED_PIN, 150); 
  else analogWrite(LED_PIN, 0); 
}

// Function to send command
bool send_command(uint8_t _position, uint8_t _command)
{

  if (current_peer_count == 0)
    return false;

  if (_position >= MAX_LAMP_COUNT)
  {
    Serial.println("[Error: 43] Light position invalid");

    return false;
  }

  for (auto &peer : peers)
  {

    if (peer->position == _position)
    {

      command_msg.command = _command;
      if (!peer->send_message((const uint8_t *)&command_msg, sizeof(command_msg)))
      {
        Serial.printf("Failed to send message to peer\n");
      }
      else
      {
        Serial.printf(
            "Sent message to peer\n");
        sent_msg_count++;
      }

      return true;
    }
  }

  return false;
}

/* Callbacks */
void onRequest() {
  
  i2c_data_t _msg;
  uint8_t data_index;

  for(uint8_t i = 0; i < MAX_LAMP_COUNT; i++){
  _msg.data = sensor_msgs[i];

  for(uint8_t byteIndex = 0; byteIndex < sizeof(esp_now_data_t); byteIndex++){
    I2C_DATA[data_index] = _msg.bytes[byteIndex];
    data_index++;
  }
  }

  Wire.write(I2C_DATA, I2C_DATA_SIZE);
}

void onReceive(int len) {
  
}

// Callback called when a new peer is found
void register_new_peer(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg)
{
  esp_now_data_t *msg = (esp_now_data_t *)data;
  uint8_t position = msg->position;

  if (current_peer_count < ESPNOW_PEER_COUNT)
  {
    Serial.printf("New peer found: " MACSTR " with position %d\n", MAC2STR(info->src_addr), position);

    ESP_NOW_Network_Peer *new_peer = new ESP_NOW_Network_Peer(info->src_addr, position);

    if (new_peer == nullptr || !new_peer->begin())
    {
      Serial.println("Failed to create or register the new peer");
      delete new_peer;
      return;
    }

    peers.push_back(new_peer);
    current_peer_count++;

    if (current_peer_count == ESPNOW_PEER_COUNT)
    {
      Serial.println("All peers have been found");
      broadcast_msg.ready = true;
    }
  }
}

/* Main */

void setup()
{
  uint8_t self_mac[6];

  Serial.begin(115200);
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin(ESPNOW_MASTER_I2C_ADDRESS);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  while (!WiFi.STA.started())
  {
    delay(100);
  }

  // Generate this device's priority based on the 3 last bytes of the MAC address
  WiFi.macAddress(self_mac);

  Serial.println("ESP-NOW Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // initialize LED
  pinMode(LED_PIN, OUTPUT);

  // Initialize the ESP-NOW protocol
  if (!ESP_NOW.begin((const uint8_t *)ESPNOW_EXAMPLE_PMK))
  {
    Serial.println("Failed to initialize ESP-NOW");
    fail_reboot();
  }

  if (!broadcast_peer.begin())
  {
    Serial.println("Failed to initialize broadcast peer");
    fail_reboot();
  }

  // Register the callback to be called when a new peer is found
  ESP_NOW.onNewPeer(register_new_peer, NULL);

  // Initialise Broadcast Message
  Serial.println("Setup complete. Broadcasting dummy message to find the slaves...");
  memset(&broadcast_msg, 0, sizeof(broadcast_msg));
  broadcast_msg.device_type = MASTER_DEVICE;
  broadcast_msg.message_type = BROADCAST_MESSAGE;
  broadcast_msg.command = DUMMY_COMMAND;

  // Initialise Command Message
  memset(&command_msg, 0, sizeof(command_msg));
  command_msg.device_type = MASTER_DEVICE;
  command_msg.message_type = COMMAND_MESSAGE;
}
                                              
void loop()
{

   // Check for disconnected devices
  ping_slaves();

  // return;
  // print received messages
  String position_list = "";
  String current_list = "";
  String power_list = "";
  String day_time_list = "";
  String rain_detection_list = "";
  String motion_detection_list = "";
  for (uint8_t i = 0; i < MAX_LAMP_COUNT; i++)
  {
    
    esp_now_data_t _msg = sensor_msgs[i];

    if(!_msg.ready) continue;

    // Serial.printf("[Light #%d] Lamp Current: %0.2f Solar Voltage: %0.2f Daytime: %d Rain: %d\n", _msg.position, _msg.lamp_amperage, _msg.solar_voltage, _msg.day_time, _msg.rain_detected);

    position_list = position_list + _msg.position + ", ";
    current_list = current_list + _msg.lamp_amperage + ", ";
    power_list = power_list + _msg.lamp_power + ", ";
    day_time_list = day_time_list + _msg.day_time + ", ";
    rain_detection_list = rain_detection_list + _msg.rain_detected + ", ";  
    motion_detection_list = motion_detection_list + _msg.motion_detected + ", ";

      
  }
  StaticJsonDocument<1052> jsonData;
  jsonData["position"] = position_list;
  jsonData["current"] = current_list;
  jsonData["power"] = power_list;
  jsonData["day_time"] = day_time_list;
  jsonData["rain_detection"] = rain_detection_list;
  jsonData["motion_detection"] = motion_detection_list;
  String jsonObj;
  serializeJson(jsonData, jsonObj);
  Serial.println(jsonObj);
  if(Serial.available() > 0){
    int cmd = Serial.parseInt();
    
    send_command(0, cmd);
  }

  delay(200);
}