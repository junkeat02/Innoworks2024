#include "t4g_blynk_header.h"
#include "Wire.h"

const uint8_t I2C_DATA_SIZE = sizeof(esp_now_data_t) * MAX_LAMP_COUNT;
byte I2C_DATA[I2C_DATA_SIZE];
bool i2c_transmissionAvailable = false;

#ifdef ENABLE_LED_STRIP
#include <FastLED.h>
const bool ENABLE_LED = true;
#endif

#ifndef ENABLE_LED_STRIP
const bool ENABLE_LED = false;
#endif

bool animation_forward_direction = true;
unsigned long prev_run_animation_time;
int ledIndex;
uint8_t hue = 0;

/* Helper functions */
void initLEDStrip(){
  if(!ENABLE_LED) return;

FastLED.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	FastLED.setBrightness(84);
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void runLEDAnimation(){
  if(!ENABLE_LED) return;
  if(animation_forward_direction){
    
    if((millis() - prev_run_animation_time) < 1) return;

    if(ledIndex == NUM_LEDS){
      animation_forward_direction = !animation_forward_direction;
      ledIndex = (NUM_LEDS) - 1;
      return;
    }

    // Set the i'th led to red 
    hue += 50;
		leds[ledIndex] = CHSV(hue, 255, 255);
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();

		// Wait a little bit before we loop around and do it again
  prev_run_animation_time = millis();
  // Serial.println(ledIndex);
  ledIndex++;
  }
  else if(!animation_forward_direction){

    if((millis() - prev_run_animation_time) < 1) return;

    if(ledIndex < 0){
      animation_forward_direction = !animation_forward_direction;
      ledIndex = 0;

      if(hue >= 255) hue = 0;
      return;
    }

    // Set the i'th led to red 
    hue += 50;
		leds[ledIndex] = CHSV(hue, 255, 255);
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();

		// Wait a little bit before we loop around and do it again
  prev_run_animation_time = millis();
  ledIndex--;
  }

}

// Function to reboot the device
void fail_reboot()
{
  Serial.println("Rebooting in 5 seconds...");
  delay(5000);
  ESP.restart();
}

// Function to request data
void requestData(){

  uint8_t bytesReceived = Wire.requestFrom(ESPNOW_MASTER_I2C_ADDRESS, I2C_DATA_SIZE);

  if (bytesReceived != I2C_DATA_SIZE) {  //If received less than required bytes
    return;
  }

  Wire.readBytes(I2C_DATA, bytesReceived);

  // Destructure Bytes
  i2c_data_t _msg;
  uint8_t data_index;

  for(uint8_t i = 0; i < MAX_LAMP_COUNT; i++){

    // get bytes for each message
    for(uint8_t byteIndex = 0; byteIndex < sizeof(esp_now_data_t); byteIndex++){
    _msg.bytes[byteIndex] = I2C_DATA[data_index];
    data_index++;
  }

  // update message
  sensor_msgs[i] = _msg.data;
  }

}

void sendData(uint8_t cmd){
  Wire.beginTransmission(ESPNOW_MASTER_I2C_ADDRESS);
  Wire.write(cmd);
  uint8_t error = Wire.endTransmission(true);
  Serial.printf("endTransmission: %u\n", error);
}

/* Main */

void setup()
{
  uint8_t self_mac[6];

  Serial.begin(115200);
  Wire.begin();

  // Blynk Initialization
  Blynk.begin(auth, ssid, pass);
  
  blynk_timer.setInterval(5, runLEDAnimation);
  blynk_timer.setInterval(200L, requestData);
  blynk_timer.setInterval(500L, updateBlynkDatastreams);

  initLEDStrip();

  Serial.println("\nESP32 Blynk ready!");
}

void loop()
{

  Blynk.run();
  blynk_timer.run();

  // if(Serial.available() > 0){
  //   int cmd = Serial.parseInt();
  //   sendData(cmd);
  // }

  // delay(200);
}