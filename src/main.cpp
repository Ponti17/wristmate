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
bool get_wifi = false;
int buttonState = 0;
uint8_t menu = 0; // 0 = main, 1 = analog, 2 = debug

uint32_t buttonStartTime = 0;
uint32_t buttonHoldDuration = 0;
void recordButtonHoldDuration(bool buttonState) {
  if (buttonState == HIGH) {
    buttonStartTime = millis();
  }
  else {
    buttonHoldDuration = millis() - buttonStartTime;
  }
}

int menuWrapAround(int menu) {
  if (menu < 2) {
    return menu + 1;
  }
  return 0;
}

uint32_t lastButtonTime = 0;
bool isButtonReady() {
  if (millis() - lastButtonTime > 300) {
    lastButtonTime = millis();
    return true;
  }
  return false;
}

void loop() 
{
  // read button
  buttonState = digitalRead(BUTTON_PIN);
  recordButtonHoldDuration(buttonState);

  if (buttonState == HIGH && isButtonReady()) {
    menu = menuWrapAround(menu);
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

  // esp_light_sleep_start();

}
