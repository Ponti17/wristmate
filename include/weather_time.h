#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi and time initialization
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

// Weather Initialization
// http://api.openweathermap.org/data/2.5/weather?q=aarhus,DK&APPID=bd491523d4252d78faacefbce30470d3
String city = "Aarhus";
String countryCode = "DK";
String jsonBuffer;
String weather_temp;
String weather_icon;

// Bools for checking if data has been collected
bool hasTime = false;
bool hasWeather = false;

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
  hasTime = true;
  return timeHour;
}
