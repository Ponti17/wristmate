#include <WiFi.h>
#include <HTTPClient.h>
#include "update.h"
#include <ArduinoJson.h>
#include "ESPmDNS.h"

const char* ipstack_api_key = "YOUR_API_KEY";


char* printCityName() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://api.ipstack.com/check?access_key=" + String(ipstack_api_key);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      const char* city = doc["city"];
      // return city as a char*
      return strdup(city);
    }
    http.end();
  }
  return NULL;
}

