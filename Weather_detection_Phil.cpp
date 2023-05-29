#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
String weather_icon;

const char* openWeatherMapApiKey = "YOUR_API_KEY";

double getTemperature(String city, String countryCode) {
  WiFiClient client;
  HTTPClient http;
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
  http.begin(client, serverPath);


  int httpResponseCode = http.GET();
  double temperature = 0.0;

  if (httpResponseCode > 0) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.println("Failed to parse JSON");
    } else {
      double tempKelvin = doc["main"]["temp"];
      temperature = tempKelvin - 273.15;
    }
  } else {
    Serial.println("Error code: " + String(httpResponseCode));
  }

  http.end();
  return temperature;
}
