#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include "timeprinting.h"
#include "Location_detection.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "update.h"
#include "Weather_detection.h"
#include "Date_printing.h"
#include "Calender_events.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include "SparkFun_BMA400_Arduino_Library.h"
#include "bitmaps.h"


#define WEATHER_ICON_X 50
#define WEATHER_ICON_Y 100
// Define the weather icons


const uint8_t cloudyIcon[] = {
  B00111100,
  B01000010,
  B10000001,
  B10000001,
  B10011001,
  B01111110,
};

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

Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 240, 240);

#define BLACK 0
#define WHITE 1

int minorHalfSize;


// defining variable city name
char* cityName;
double temp = 0.0;

// timer variable for tracking elapsed time
unsigned long lastUpdate = 0;
unsigned long lastUpdate_2 = 0;
unsigned long lastUpdate_3 = 0;
const unsigned long displayDuration = 10000; // 10 seconds
unsigned long previousMillis = 0;    // variable to store the previous time
const long interval = 3000000; 
String Calender_inf= "";
String RealTime = "";
String title_real = "";

// WiFi and time initialization
const char* ssid = "";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;



// Init display on startup
void initDisplay() {
  display.begin();
  display.clearDisplay();
  minorHalfSize = min(display.width(), display.height()) / 2;
  display.setRotation(2);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
}

// Interrupt Occured
void IRAM_ATTR isr() {
  Serial.println("Interrupt occured");
}


void setup() {
  // Initialization of display and WiFi
  Serial.begin(9600);
  initDisplay();
  WiFi.begin(ssid, password);

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
      dots = "";
    }
    display.setCursor(50,50);
    display.println("Connecting" + dots);
    display.refresh();
    Serial.println(dots);
  }
  // Get current time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 

  // I2C initialization
  Wire.begin(SDA, SCL); // SDA on GPIO5, SCL on GPIO4


  // IO Setup
  pinMode(BUTTON_PIN, INPUT);

  // Sleep wakeup setup
  esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
  gpio_wakeup_enable((gpio_num_t)BUTTON_PIN, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();


  //make initial HTTP request
  char* newCityName = printCityName();
  free(cityName);
  cityName = newCityName;
  //get weather data at startu and store it

  temp = getTemperature(cityName, "DK");
  //serial print temperature
  Serial.println(temp);
  //clear display
  getFirstEvent(RealTime, title_real);

  // print the event details
  Serial.print("Event time: ");
  Serial.println(RealTime);
  Serial.print("Event title: ");
  Serial.println(title_real);
  display.clearDisplay();

}

uint32_t time_since_scan = 0;
bool on_startup = true;

bool get_wifi = false;
int buttonState = 0;
uint8_t count_to_shutdown = 0;
bool shutdown = false;
uint32_t shutdown_millis = 0;
int but_press = 0;

uint8_t menu_state = 0;

uint32_t time_since_button = 0;

void loop() {
  // read button
  buttonState = digitalRead(BUTTON_PIN);
  // Count to shutdown while button is held
  if (buttonState == HIGH && millis() - shutdown_millis > 1000) {
    count_to_shutdown++;
    shutdown_millis = millis();
  }
  else if (buttonState == LOW && millis() - shutdown_millis > 1000) {
    count_to_shutdown = 0;
  }

  if (count_to_shutdown > 4) {
    shutdown = true;
    count_to_shutdown = 0;
  }

  // Initate shutdown 
  while (shutdown) {
    buttonState = digitalRead(BUTTON_PIN);
    display.fillScreen(WHITE);
    display.setTextSize(2);
    display.setCursor(50,50);
    display.println("Shutting down");
    if (buttonState == LOW) {
      count_to_shutdown++;
    }
    else {
      count_to_shutdown = 0;
      display.setCursor(50, 70);
      display.println("Release button");
    }
    if (count_to_shutdown > 4) {
      esp_deep_sleep_start();
    }
    delay(1000);
    display.refresh();
  }

  char dayName[10];
  int dayNumber;
  char month[10];
  int weekNumber;
  static bool menu_2 = false;  // initialize flag to false

  if (buttonState == HIGH) {
    display.clearDisplay();
    if (menu_state < 3) {
      menu_state++;
      delay(100);
    }
    else {
      menu_state = 0;
    }
  }

  // get current time in milliseconds
  unsigned long currentTime = millis();
  // check if 1 hour has elapsed since the last update
  if (currentTime - lastUpdate >= 3600000) {
    //update city name
    free(cityName);
    cityName = printCityName();
    temp=getTemperature(cityName, "DK");
    //reset timer
    lastUpdate = currentTime;
  }

  // display menu_1 or menu_2
  if (menu_state == 1) {
    // show menu_2, show datainformation
    display.setCursor(120, 70);
    display.setTextSize(2);
    display.println(":"+ String(temp) + "C");
    display.drawBitmap(50, 40, bitmap_02d, 60, 60, BLACK);
    display.setCursor(120, 140);
    display.setTextSize(2);
    display.println(":"+ String(cityName));
    display.drawBitmap(40,100,epd_bitmap_pngtree_simple_city_landscape_background_png_image_2326328,80,80,BLACK);
  } 

  else if (menu_state == 2) {
    display.setTextSize(7);
    display.print("      "); // clear the old time text
    display.setCursor(20, 110);
    display.println(printLocalTime_2());
    display.drawBitmap(50,20, epd_bitmap_rolex_logo, 140, 76, BLACK);
  }

  else if (menu_state == 3){
    display.drawBitmap(0,0, epd_bitmap_Rolex_watch_face, 240,240 , BLACK);
  
  // Define the center point of the watch face
  int WATCH_CENTER_X = 120;
  int WATCH_CENTER_Y = 120;

  // Define the length of the hour and minute hands
  int HOUR_HAND_LENGTH = 40;
  int MINUTE_HAND_LENGTH = 60;

  // Get the current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Error occurred while retrieving time
    return;
  }
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  // Calculate the angle for the hour and minute hands based on the current time
  int hourAngle = ((currentHour % 12) * 30) + (currentMinute / 2);
  int minuteAngle = currentMinute * 6;

  // Calculate the endpoint positions for the hour and minute hands
  int hourEndX = WATCH_CENTER_X + (int)(sin((hourAngle) * PI / 180.0) * HOUR_HAND_LENGTH);
  int hourEndY = WATCH_CENTER_Y - (int)(cos((hourAngle) * PI / 180.0) * HOUR_HAND_LENGTH);
  int minuteEndX = WATCH_CENTER_X + (int)(sin((minuteAngle) * PI / 180.0) * MINUTE_HAND_LENGTH);
  int minuteEndY = WATCH_CENTER_Y - (int)(cos((minuteAngle) * PI / 180.0) * MINUTE_HAND_LENGTH);

  // Draw the hour and minute hands as a cursor
  display.drawLine(WATCH_CENTER_X, WATCH_CENTER_Y, hourEndX, hourEndY, WHITE);
  display.drawLine(WATCH_CENTER_X, WATCH_CENTER_Y, minuteEndX, minuteEndY, WHITE);
  }
    
    //////Menu_1
    else {
    String trimmedRealTime = RealTime.substring(0, RealTime.length() - 3);
    //display event information
    display.setCursor(120 - (strlen("Next event:") * 5), 20);
    display.setTextSize(2);
    display.println("NextEvent:");
    display.setCursor(120 - (strlen(title_real.c_str()) * 5), 45);
    display.setTextSize(2);
    display.println(title_real);
    display.setCursor(120 - (strlen(RealTime.c_str()) * 5), 70);
    display.setTextSize(2);
    display.println(trimmedRealTime);
    display.setCursor(20, 90);
    display.setTextSize(7);
    display.print("      "); // clear the old time text
    display.setCursor(20, 90);
    display.println(printLocalTime_2());
    // show date information
    printLocalDate(dayName, &dayNumber, month, &weekNumber);
    // Display day name, day number, month, and week number
    char date1Str[20];
    sprintf(date1Str, "%s d.%d", dayName, dayNumber);
    display.setCursor(120 - (strlen(date1Str) * 5), 170);
    display.setTextSize(2);
    display.print(date1Str);
    display.setCursor(120 - (strlen(month) * 5), 190);
    display.print(month);
    char date2Str[20];
    char weekName[] = "week";
    sprintf(date2Str, "%s %d",weekName, weekNumber);
    display.setCursor(120 - (strlen(date2Str) * 5), 210);
    display.print(date2Str);
  }
  // Refresh display
  display.refresh();
  // Delay to avoid flickering
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();   // get the current time
  if (currentMillis - previousMillis >= interval) {   // check if the interval has elapsed
    previousMillis = currentMillis;    // store the current time for the next iteration

    // send the request to get the first event
    getFirstEvent(RealTime, title_real);

    // print the event details
    Serial.print("Event time: ");
    Serial.println(RealTime);
    Serial.print("Event title: ");
    Serial.println(title_real);
  }
  delay(50);
}