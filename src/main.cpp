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
#include <weather_time.h>
#include <pass.h>
#include <wristmateLib.h>

// Sleep mode setup
#define uS_TO_S_FACTOR 1000000

// Define button pin
#define BUTTON_PIN 20

// Define I2C pins
#define SDA 5
#define SCL 4

// Sharp Memory Display Initialization
#define SHARP_SCK  8
#define SHARP_MOSI 7
#define SHARP_SS   6
#define BLACK 0
#define WHITE 1
Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 240, 240);
int minorHalfSize;

// Initialize the BMP280 sensor
Adafruit_BMP280 bmp; 

// BMA400 Accelerometer setup
BMA400 accelerometer;
uint8_t i2cAddress = BMA400_I2C_ADDRESS_DEFAULT; 
int interruptPin = 2;
volatile bool interruptOccured = false; 

void thumbs_up_animation() {
  float AnimationSpeed = 0.3;                                                     // Animation speed. Lower = faster.
  int frameTimesMilliseconds[] = {1000, 10, 8, 10, 8, 1000, 8, 10, 8, 10, 1000};  // Time each frame is displayed in milliseconds
  int frameOrder[] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0};                           // Order of frames to be displayed
  for (int i = 0; i < 11; i++) {
    display.fillScreen(WHITE);
    display.drawBitmap(0, 0, vaultboy_thumbsupallArray[frameOrder[i]], 240, 240, BLACK);
    display.refresh();
    delay(frameTimesMilliseconds[i] * AnimationSpeed);
  }
}

// Init display on startup
void initDisplay() {
  display.begin();
  display.clearDisplay();
  minorHalfSize = min(display.width(), display.height()) / 2;
  display.setRotation(2);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
}

void getData(bool on_boot) {
  if (on_boot == false) {
    WiFi.begin(ssid, password);
    display.setTextSize(2);
    String dots = "  ";
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      if (dots == "  ") {
        dots = ". ";
      }
      else if (dots == ". ") {
        dots = "..";
      }
      else {
        dots = "  ";
      }
      display.setCursor(35,150);
      display.println("Connecting." + dots);
      display.refresh();
      Serial.println(dots);
    }
  }

  // Get weather data
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherApiKey + "&units=metric";
  jsonBuffer = httpGETRequest(serverPath.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);

  if (JSON.typeof(myObject) == "undefined") {
  Serial.println("Parsing input failed!");
  }

  weather_temp = JSON.stringify(myObject["main"]["temp"]);
  weather_icon = JSON.stringify(myObject["weather"][0]["icon"]);

  if (weather_temp == "null" || weather_icon == "null") {
    hasWeather = false;
  }
  else {
    hasWeather = true;
  }

  // Remove last digit from temperature if over two digits
  if (weather_temp.length() > 3) {
   weather_temp.remove(weather_temp.length() - 1, 1);
  }

  // remove quotation marks from weather_icon string
  weather_icon.remove(0, 1);
  weather_icon.remove(weather_icon.length() - 1, 1);

  // Get current time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
  printLocalTime();

  // Disconnect WiFi as no longer needed.
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void IRAM_ATTR isr() {
  Serial.println("Interrupt occured");
}

void setup() {
  // Initialization of display and WiFi
  WiFi.begin(ssid, password);
  Serial.begin(9600);
  initDisplay();

  // Loading screen while connecting on startup
  String dots = "  ";
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (dots == "  ") {
      dots = ". ";
    }
    else if (dots == ". ") {
      dots = "..";
    }
    else {
      dots = "  ";
    }
    display.setCursor(50,50);
    display.println("Connecting." + dots);
    display.refresh();
    Serial.println(dots);
  }

  // Get data from API
  getData(true);

  // I2C initialization
  Wire.begin(SDA, SCL); // SDA on GPIO5, SCL on GPIO4
  
  // BMP280 initalization
  if (!bmp.begin(0x76)) { // Initialize the BMP280 sensor at I2C address 0x76
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  // BMA400 initialization
  while(accelerometer.beginI2C(i2cAddress) != BMA400_OK) {
    Serial.println("BMA400 not connected!");
    delay(100);
  }

  bma400_step_int_conf config =
  {
      .int_chan = BMA400_INT_CHANNEL_1 // Which pin to use for interrupts
  };
  accelerometer.setStepCounterInterrupt(&config);

  accelerometer.setInterruptPinMode(BMA400_UNMAP_INT_PIN, BMA400_INT_PUSH_PULL_ACTIVE_1);

  accelerometer.enableInterrupt(BMA400_STEP_COUNTER_INT_EN, true);

  // IO Setup
  pinMode(BUTTON_PIN, INPUT);

  // Sleep wakeup setup
  esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
  gpio_wakeup_enable((gpio_num_t)BUTTON_PIN, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  thumbs_up_animation();
}

// Variable initialization
String temp = "";
String pressure = "";

uint32_t time_since_scan = 0;
bool on_startup = true;

uint32_t stepCount = 0;
uint8_t activityType = 0;
String activityTypeString = "Unknown";

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

  if (millis() - soc_temp_millis > 1000 && menu == 2) {
    soc_temp_millis = millis();
    internal_temp = (float)temperatureRead();
  }

  if (buttonState == HIGH && millis() - time_since_menu > 250) {
    time_since_menu = millis();
    if (menu < 2) {
      menu++;
    }
    else {
      menu = 0;
    }
    menu_switch = true;
  }

  // Count to shutdown while button is held
  if (buttonState == HIGH && millis() - button_millis > 1000) {
    button_count++;
    button_millis = millis();
  }
  else if (buttonState == LOW && millis() - button_millis > 200) {
    button_count = 0;
  }

  if (button_count > 10) {
    shutdown = true;
    button_count = 0;
  }

  // Initate shutdown 
  while (shutdown) {
    buttonState = digitalRead(BUTTON_PIN);
    display.fillScreen(WHITE);
    display.setTextSize(2);
    display.setCursor(50,50);
    display.println("Shutting down");
    if (buttonState == LOW) {
      button_count++;
    }
    else {
      button_count = 0;
      display.setCursor(50, 70);
      display.println("Release button");
    }
    if (button_count > 4) {
      esp_deep_sleep_start();
    }
    delay(1000);
    display.refresh();
  }

  // Clear screen so bitmaps aren't drawn on top of each other
  display.fillScreen(WHITE);

  if (button_count > 5) {
    get_wifi = true;
    button_count = 0;
  }

  // If time or weather data hasn't been collected, initiate wifi
  if (hasTime == false || hasWeather == false) {
    get_wifi = true;
  }

  if (get_wifi && buttonState == LOW) {
    display.drawBitmap(195, 145, bitmap_wifi, 30, 30, BLACK);
  }
  else if (get_wifi && button_count > 3) {
    getData(false);
    get_wifi = false;
    button_count = 0;
  }

  // Read BMA400 every 10s
  if (millis() - time_since_scan > 10000) {
    time_since_scan = millis();
    accelerometer.getStepCount(&stepCount, &activityType);

    switch(activityType) {
      case BMA400_RUN_ACT:
        activityTypeString = "Running";
        break;
      case BMA400_WALK_ACT:
        activityTypeString = "Walking";
        break;
      case BMA400_STILL_ACT:
        activityTypeString = "Still  ";
        break;
      default:
        activityTypeString = "Unknown";
        break;
    }
  }

  if (menu_switch) {
    display.clearDisplay();
    menu_switch = false;
  }

  if (menu == 1) {
    display.drawBitmap(0, 0, bitmap_analog_face, 240, 240, BLACK);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return;
    }

    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;

    int hourAngle = ((currentHour % 12) * 30) + (currentMinute / 2);
    int minuteAngle = currentMinute * 6;

    int hourEndX = 120 + (int)(sin((hourAngle) * PI / 180.0) * 60);
    int hourEndY = 120 - (int)(cos((hourAngle) * PI / 180.0) * 60);
    int minuteEndX = 120 + (int)(sin((minuteAngle) * PI / 180.0) * 80);
    int minuteEndY = 120 - (int)(cos((minuteAngle) * PI / 180.0) * 80);
    int hour[] = {hourEndX, hourEndY};


    int handWidth = 5;
    display.drawTriangle(hour[0] * 0.2, hour[1] * (-1) * 0.2, hour[0] * (-1) * 0.2, hour[1] * 0.2, hourEndX, hourEndY, BLACK);
    display.drawTriangle(120 - handWidth, 120 - handWidth, 120 + handWidth, 120 + handWidth, minuteEndX, minuteEndY, BLACK);
    display.fillTriangle(120 - handWidth, 120 - handWidth, 120 + handWidth, 120 + handWidth, minuteEndX, minuteEndY, BLACK);

    display.drawCircle(120, 120, 8, BLACK);
    display.fillCircle(120, 120, 8, BLACK);
  }

  else if (menu == 2) {
    display.setTextSize(3);
    display.setCursor(30, 100);
    display.println("SoC:");
    display.setCursor(100, 100);
    display.println(internal_temp);
    display.setCursor(195, 95);
    display.setTextSize(2);
    display.println("o");
  }

  else {
    // Print current time
    display.setCursor(15, 90);
    display.setTextSize(7);
    display.println(printLocalTime());
    
    // Print step count
    display.drawBitmap(50, 40, bitmap_footsteps, 25, 25, BLACK);
    display.setCursor(90, 45);
    display.setTextSize(2.5);
    display.println(String(stepCount));

    // Print activity type
    if (activityTypeString == "Running") {
      display.drawBitmap(160, 35, bitmap_run, 30, 30, BLACK);
    }
    else if (activityTypeString == "Walking") {
      display.drawBitmap(160, 35, bitmap_walking, 30, 30, BLACK);
    }
    else {
      display.drawBitmap(160, 35, bitmap_stand, 30, 30, BLACK);
    }
    
    // Print temperature from OpenWeather
    if (hasWeather) {
      display.setCursor(120, 175);
      display.setTextSize(3);
      display.println(weather_temp);
      display.setCursor(195, 170);
      display.setTextSize(2);
      display.println("o");
    }

    // Print weather icon from OpenWeather
    if (weather_icon == "01d" || weather_icon == "01n") {
      display.drawBitmap(50, 150, bitmap_01d, 60, 60, BLACK);
    }
    else if (weather_icon == "02d" || weather_icon == "02n") {
      display.drawBitmap(50, 150, bitmap_03d, 60, 60, BLACK);
    }
    else if (weather_icon == "03d" || weather_icon == "03n") {
      display.drawBitmap(50, 150, bitmap_03d, 60, 60, BLACK);
    }
    else if (weather_icon == "04d" || weather_icon == "04n") {
      display.drawBitmap(50, 150, bitmap_04d, 60, 60, BLACK);
    }
    else if (weather_icon == "09d" || weather_icon == "09n") {
      display.drawBitmap(50, 150, bitmap_09d, 60, 60, BLACK);
    }
    else if (weather_icon == "10d" || weather_icon == "10n") {
      display.drawBitmap(50, 150, bitmap_10d, 60, 60, BLACK);
    }
    else if (weather_icon == "11d" || weather_icon == "11n") {
      display.drawBitmap(50, 150, bitmap_11d, 60, 60, BLACK);
    }
    else if (weather_icon == "13d" || weather_icon == "13n") {
      display.drawBitmap(50, 150, bitmap_13d, 60, 60, BLACK);
    }
    else if (weather_icon == "50d" || weather_icon == "50n") {
      display.drawBitmap(50, 150, bitmap_50d, 60, 60, BLACK);
    }
  }

  display.refresh();

  if (buttonState == LOW && !init_sleep) {
    time_to_sleep = millis();
    init_sleep = true;
  }

  if (buttonState == LOW && millis() - time_to_sleep > 500) {
    init_sleep = false;
    esp_light_sleep_start();
  }

}
