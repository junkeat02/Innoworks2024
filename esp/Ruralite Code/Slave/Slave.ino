#include "espnow_header.h"

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
void ping_master()
{
  if (millis() - last_transmit_time > TRANSMIT_TIMEOUT)
  {
    // broadcast
    if (!broadcast_peer.send_message((const uint8_t *)&broadcast_msg, sizeof(broadcast_msg)))
    {
      Serial.println("Failed to broadcast message");
    }

    last_transmit_time = millis();
  }
}

void send_sensor_readings()
{

  if (!master_found)
    return;

  // Send a message to the master
  sensor_msg.position = POSITION;
  sensor_msg.ready = true;
  sensor_msg.motion_detected = digitalRead(MOTION_SENSOR_PIN) == MOTION_DETECTED;
  sensor_msg.rain_detected = digitalRead(RAIN_SENSOR_PIN) == RAIN_DETECTED;
  sensor_msg.day_time = digitalRead(LDR_PIN) == DAYLIGHT_DETECTED;
  sensor_msg.lamp_power = lamp_power_mW;
  sensor_msg.lamp_amperage = lamp_current_mA;
  sensor_msg.solar_voltage = solar_busvoltage;
  sensor_msg.faulty = lamp_is_faulty;
  
  if(!fault_checking){
  Serial.print("Motion: ");
  Serial.print(sensor_msg.motion_detected);
  Serial.print("\tRain: ");
  Serial.print(sensor_msg.rain_detected);
  Serial.print("\tLDR: ");
  Serial.print(sensor_msg.day_time);
  Serial.print("\tCurrent: ");
  Serial.print(sensor_msg.lamp_amperage);
  Serial.print("\tSolar Voltage: ");
  Serial.println(sensor_msg.solar_voltage);
  }

  if (!master_peer->send_message((const uint8_t *)&sensor_msg, sizeof(sensor_msg)))
  {
    Serial.println("Failed to send message to the master");
  }
  else
  {
    // Serial.printf("Sent message to the master.\n");
    sent_msg_count++;
  }
}

void measurePower()
{
  if (millis() - last_power_measurement_time < POWER_MEASUREMENT_TIMEOUT)
    return;

  // Lamp current sensor
  lamp_shuntvoltage = lamp_ina219.getShuntVoltage_mV();
  lamp_busvoltage = lamp_ina219.getBusVoltage_V();
  lamp_current_mA = lamp_ina219.getCurrent_mA();
  lamp_power_mW = lamp_ina219.getPower_mW();

  // solar current sensor
  solar_shuntvoltage = solar_ina219.getShuntVoltage_mV();
  solar_busvoltage = solar_ina219.getBusVoltage_V();
  solar_current_mA = solar_ina219.getCurrent_mA();
  solar_power_mW = solar_ina219.getPower_mW();
  
  last_power_measurement_time = millis();
}

void checkMotion()
{
  if (fault_checking || run_command)
    return;

  bool motion_detected = digitalRead(MOTION_SENSOR_PIN) == MOTION_DETECTED;
  bool rain_detected = digitalRead(RAIN_SENSOR_PIN) == RAIN_DETECTED;
  bool ldr_light_detected = digitalRead(LDR_PIN) == DAYLIGHT_DETECTED;

  // Serial.print("Motion: ");
  // Serial.print(motion_detected);
  // Serial.print("\tRain: ");
  // Serial.print(rain_detected);
  // Serial.print("\tLDR: ");
  // Serial.println(ldr_light_detected);

  // if daytime turn off lamp
  if(ldr_light_detected){
    target_lamp_pwm = 0;
    last_motion_detected_time = millis();
  }
  else if (rain_detected)
  {
    target_lamp_pwm = FULL_BEAM_BRIGHTNESS;
  }
  else if (motion_detected)
  {
    target_lamp_pwm = FULL_BEAM_BRIGHTNESS;
  }
  else
  {
    // If no motion is still detected after 5s, dim Lamp
    if (millis() - last_motion_detected_time > LED_MOTION_DETECTED_TIMEOUT)
    {
      // dim Lamp
    target_lamp_pwm = LOW_BEAM_BRIGHTNESS;
    last_motion_detected_time = millis();
    }
  }

  // Measure Power Consumption
  measurePower();
}

void fault_check()
{
  if (!fault_checking)
    return;

  // Switch OFF Lamp
  target_lamp_pwm = 0;

  if (millis() - fault_check_last_delay_time < 2000)
    return;

  target_lamp_pwm = FULL_BEAM_BRIGHTNESS;

  if (millis() - fault_check_last_delay_time < 5000)
    return;

  // Get Current readings
  lamp_shuntvoltage = lamp_ina219.getShuntVoltage_mV();
  lamp_busvoltage = lamp_ina219.getBusVoltage_V();
  lamp_current_mA = lamp_ina219.getCurrent_mA();

  if (lamp_current_mA < properCurrent)
  {
    Serial.println("[WARN] Lamp is FAULTY!");
    lamp_is_faulty = true;
  }
  else{
    Serial.println("[INFO] Lamp is FUNCTIONAL!");
    lamp_is_faulty = false;
  }

  // stop fault checking
  fault_checking = false;
}

/* Callbacks */

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

    // check if new peer is master
    char peer_mac[18];
    sprintf(peer_mac, "%X:%X:%X:%X:%X:%X", info->src_addr[0], info->src_addr[1], info->src_addr[2], info->src_addr[3], info->src_addr[4], info->src_addr[5]);

    if (String(peer_mac) == ESPNOW_MASTER_MACSTR)
    {
      master_found = true;
      new_peer->peer_is_master = true;
      master_peer = new_peer;
      Serial.println("Master has been found");
    }

    if (current_peer_count == ESPNOW_PEER_COUNT)
    {
      // Serial.println("All peers have been found");
      broadcast_msg.ready = true;
    }
  }
}

/* Main */

void setup()
{
  uint8_t self_mac[6];

  Serial.begin(115200);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started())
  {
    delay(100);
  }

  Serial.println("ESP-NOW Slave #" + String(POSITION));
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Generate yhis device's priority based on the 3 last bytes of the MAC address
  WiFi.macAddress(self_mac);

  // Lamp Current Sensor
   if (!lamp_ina219.begin())
  { 

    Serial.println("LAMP Current Sensor not found!");
    while (1) { delay(10);}
    }

  lamp_ina219.setCalibration_16V_400mA();

  // Lamp Current Sensor
   if (!solar_ina219.begin())
  { 

    Serial.println("Solar Current Sensor not found!");
    fail_reboot(); // Reboot the ESP32
    while (1) {
       delay(10);
    }
    }

  solar_ina219.setCalibration_16V_400mA();

  // Initialise Pins
   pinMode(MOTION_SENSOR_PIN, INPUT_PULLDOWN);
   pinMode(LDR_PIN, INPUT_PULLDOWN);
   pinMode(RAIN_SENSOR_PIN, INPUT_PULLDOWN);
   pinMode(RAIN_SENSOR_ANALOG_PIN, INPUT);
   pinMode(LDR_ANALOG_PIN, INPUT);
   pinMode(LAMP_PIN, OUTPUT);

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

  // Initialise Broadcasting Message
  Serial.println("Setup complete. Broadcasting dummy data to find the master...");
  memset(&broadcast_msg, 0, sizeof(broadcast_msg));
  broadcast_msg.position = POSITION;
  broadcast_msg.device_type = SLAVE_DEVICE;
  broadcast_msg.message_type = BROADCAST_MESSAGE;
  broadcast_msg.command = DUMMY_COMMAND;

  // Initialise Sensor Message
  memset(&sensor_msg, 0, sizeof(sensor_msg));
  sensor_msg.device_type = SLAVE_DEVICE;
  sensor_msg.message_type = SENSOR_MESSAGE;
  sensor_msg.position = POSITION;

  // Turn on Lamp
  target_lamp_pwm = LOW_BEAM_BRIGHTNESS;
}

void loop()
{

  // Try to reconnect to Master if connection lost
  ping_master();

  // Transit Sensor readings to master
  send_sensor_readings();

  measurePower();

  checkMotion();

  fault_check();

  lamp_pwm = (update_threshold * target_lamp_pwm + (1.0 - update_threshold) * prev_lamp_pwm);
  prev_lamp_pwm = lamp_pwm;

  // Serial.print("Target: ");
  // Serial.print(target_lamp_pwm);
  // Serial.print("\tOutput: ");
  // Serial.println(lamp_pwm);
  
  // Control Lamp PWM
  analogWrite(LAMP_PIN, lamp_pwm);
  
  // Run Commands sent from Master until Timeout
  if(millis() - last_command_run_time > RUN_COMMAND_TIMEOUT){
    run_command = false;
    last_command_run_time = millis();
  }
  
  delay(ESPNOW_SEND_INTERVAL_MS);
}
