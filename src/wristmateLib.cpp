/*
    * wristmateLib.cpp
    * Created by Andreas Pedersen, 19/05/23.
*/

#include <Arduino.h>
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
#include <wristmateLib.h>

// Time keeping function. Don't touch.
String printLocalTime()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return "00:00";
    }
    char timeHour[6];
    strftime(timeHour,6,"%H:%M",&timeinfo);
    return timeHour;
}

// httpGETRequest function. Don't touch.
String httpGETRequest(const char* serverName) {
    WiFiClient client;
    HTTPClient http;
        
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    
    // Send HTTP POST request
    int httpResponseCode = http.GET();
    
    String payload = "{}"; 
    
    if (httpResponseCode>0) {
        payload = http.getString();
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

Wristmate::Wristmate(const char* ssid, const char* password, const char* openWeatherApiKey) 
    : display(8, 7, 6, 240, 240)
{
    _ssid = ssid;
    _password = password;
    _openWeatherApiKey = openWeatherApiKey;
}

void Wristmate::begin() {
    // Define I2C pins
    const int SDA = 5;
    const int SCL = 4;

    // Sharp Memory Display Initialization
    const int SHARP_SCK = 8;
    const int SHARP_MOSI = 7;
    const int SHARP_SS = 6;

    // Initialize the display using constructor initialization list
    display = Adafruit_SharpMem(SHARP_SCK, SHARP_MOSI, SHARP_SS, 240, 240);

    // Initialize the BMP280 sensor
    bmp = Adafruit_BMP280(); 

    // BMA400 Accelerometer setup
    accelerometer = BMA400();
    uint8_t i2cAddress = BMA400_I2C_ADDRESS_DEFAULT; 
    int interruptPin = 2;
    volatile bool interruptOccured = false; 

    // Init display
    display.begin();
    display.clearDisplay();
    display.setRotation(2);
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(2);

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
}

void Wristmate::getData(bool on_boot) {
    WiFi.begin(_ssid, _password);
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

    // WiFi and time initialization
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 3600;
    const int   daylightOffset_sec = 3600;

  // Get weather data
  // http://api.openweathermap.org/data/2.5/weather?q=aarhus,DK&APPID=bd491523d4252d78faacefbce30470d3
    String city = "Aarhus";
    String countryCode = "DK";
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + _openWeatherApiKey + "&units=metric";
    String jsonBuffer = httpGETRequest(serverPath.c_str());
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

void Wristmate::menu1() {
    display.fillScreen(WHITE);
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
    display.refresh();
}

void Wristmate::menu2() {
    display.fillScreen(WHITE);
    float internal_temp = (float)temperatureRead();
    display.setTextSize(3);
    display.setCursor(30, 100);
    display.println("SoC:");
    display.setCursor(100, 100);
    display.println(internal_temp);
    display.setCursor(195, 95);
    display.setTextSize(2);
    display.println("o");
    display.refresh();
}

void Wristmate::menu3() {
    display.fillScreen(WHITE);
    // Print current time
    display.setCursor(15, 90);
    display.setTextSize(7);
    display.println(printLocalTime());

    uint32_t stepCount = 0;
    uint8_t activityType = 0;
    String activityTypeString = "Unknown";
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
    display.refresh();
}

void Wristmate::animationThumbsUp() {
    int AnimationSpeed = 5;                                                     // Animation speed.
    int frameTimesMilliseconds[] = {1000, 10, 8, 10, 8, 1000, 8, 10, 8, 10, 1000};  // Time each frame is displayed in milliseconds
    int frameOrder[] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0};                           // Order of frames to be displayed
    for (int i = 0; i < 11; i++) {
        display.fillScreen(WHITE);
        display.drawBitmap(0, 0, vaultboy_thumbsupallArray[frameOrder[i]], 240, 240, BLACK);
        display.refresh();
        delay(frameTimesMilliseconds[i] / AnimationSpeed);
  }
}