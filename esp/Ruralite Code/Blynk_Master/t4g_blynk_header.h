#ifndef T4G_BLYNK_HEADER_H
#define T4G_BLYNK_HEADER_H

#include "config.h"
#include "data_structure.h"

// Blynk IOT Library and Variables
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL60wxFAi8G"
#define BLYNK_TEMPLATE_NAME "Ruralite"
#include <BlynkSimpleEsp32.h>
char auth[] = "kMo41KmT7z0kIvwhnsyybNVbg3SrYr73";
char ssid[] = "CHRIS-TUF 2263";
char pass[] = "M48789]h";

// number of lamps in Blynk
const uint8_t n = MAX_LAMP_COUNT;

esp_now_data_t sensor_msgs[MAX_LAMP_COUNT];

// Blynk virtual data pins
const int MOTION_VPINS[MAX_LAMP_COUNT] = {V2, V22};
const int LAMP_AMPERAGE_VPINS[MAX_LAMP_COUNT] = {V3, V23};
const int LDR_VPINS[MAX_LAMP_COUNT] = {V4, V24}; // String (day/night)
const int WEATHER_VPINS[MAX_LAMP_COUNT] = {V5, V25}; // String (clear/rain)
const int LAMP_POWER_VPINS[MAX_LAMP_COUNT] = {V6, V26};
const int SOLAR_VOLTAGE_VPINS[MAX_LAMP_COUNT] = {V7, V27};

// initiate timer BlynkTimer variable
BlynkTimer blynk_timer;

// Helper Functions

int updateFunction(int BlynkVpin, int dataVar)
{
  Blynk.virtualWrite(BlynkVpin, dataVar);
  return (dataVar = 0);
}

// detects for motion and update to Blynk
void updateBlynkDatastreams()
{
  
  for (uint8_t i = 0; i < n; i++)
  {
    // motion
    Blynk.virtualWrite(MOTION_VPINS[i], sensor_msgs[i].motion_detected);

    // lamp current
     Blynk.virtualWrite(LAMP_AMPERAGE_VPINS[i], sensor_msgs[i].lamp_amperage);

    // ldr
     Blynk.virtualWrite(LDR_VPINS[i], sensor_msgs[i].day_time ? "Day" : "Night");

    // weather
     Blynk.virtualWrite(WEATHER_VPINS[i], sensor_msgs[i].rain_detected ? "Rain" : "Clear");

    // lamp power
     Blynk.virtualWrite(LAMP_POWER_VPINS[i], sensor_msgs[i].lamp_power / 1000);

    // solar voltage
     Blynk.virtualWrite(SOLAR_VOLTAGE_VPINS[i], sensor_msgs[i].solar_voltage);
  }
}

#endif