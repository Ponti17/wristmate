#include <Arduino.h>                            // Use Arduino framework
#include <Wire.h>                               // I2C library
#include <WiFi.h>                               // WiFi library
#include <time.h>                               // Time library
#include <Adafruit_Sensor.h>                    // Adafruit sensor library              
#include <Adafruit_BMP280.h>                    // Adafruit BMP280 Temperature sensor library
#include <Adafruit_GFX.h>                       // Adafruit GFX library            
#include <Adafruit_SharpMem.h>                  // Adafruit Sharp Memory Display library        
#include "SparkFun_BMA400_Arduino_Library.h"    // Sparkfun BMA400 Accelerometer library
#include <HTTPClient.h>                         // HTTP Client library     
#include <Arduino_JSON.h>                       // Arduino JSON library
#include <bitmaps.h>                            // Custom Bitmaps
#include <pass.h>
#include <wristmateLib.h>

Wristmate wristmate(ssid, password, openWeatherApiKey);

void IRAM_ATTR isr() {
  Serial.println("Interrupt occured");
}

const int uS_TO_S_FACTOR = 1000000;
const int BUTTON_PIN = 20;

void setup() {
  // Serial monitor setup
  Serial.begin(9600);

  // IO Setup
  pinMode(BUTTON_PIN, INPUT);
  wristmate.begin();
  wristmate.getData(true);
  wristmate.animationThumbsUp();

  // Sleep wakeup setup
  esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
  gpio_wakeup_enable((gpio_num_t)BUTTON_PIN, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();
}

// Variable initialization
String temp = "";
String pressure = "";

uint32_t time_since_scan = 0;
bool on_startup = true;

bool get_wifi = false;
int buttonState = 0;
uint8_t button_count = 0;
bool shutdown = false;
uint32_t button_millis = 0;
int but_press = 0;

uint8_t menu = 0; // 0 = main, 1 = analog
uint32_t time_since_menu = 0;
bool menu_switch = false;

uint32_t time_to_sleep = 0;
bool init_sleep = false;

uint32_t soc_temp_millis = 0;
float internal_temp = 0;

void loop() 
{
  // read button
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == HIGH && millis() - time_since_menu > 250) {
    time_since_menu = millis();
    if (menu >= 2) {
      menu++;
    }
    else {
      menu = 0;
    }
  }

  // If time or weather data hasn't been collected, initiate wifi
  if (wristmate.hasTime == false || wristmate.hasWeather == false) {
    get_wifi = true;
  }

  if (menu == 1) {
    wristmate.menu1();
  }

  else if (menu == 2) {
    wristmate.menu2();
  }

  else {
    wristmate.menu3();
  }

  if (buttonState == LOW && !init_sleep) {
    time_to_sleep = millis();
    init_sleep = true;
  }

  if (buttonState == LOW && millis() - time_to_sleep > 500) {
    init_sleep = false;
    // esp_light_sleep_start();
  }

}
