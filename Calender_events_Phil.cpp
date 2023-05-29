#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* serverName = "YOUR_ServerName";
const char * headerKeys[] = {"date", "server"} ;
const size_t numberOfHeaders = 2;

String unixTimeToDateTime(unsigned long unixTime) {
  char buffer[32];
  time_t time = unixTime;
  struct tm* tmInfo = localtime(&time);
  strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", tmInfo);
  return String(buffer);
}

void getFirstEvent(String& RealTime, String& title_real) {
  HTTPClient http;

  // IP address with path or name with URL path 
  http.begin(serverName);
  http.collectHeaders(headerKeys, numberOfHeaders);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);


  // Send HTTP POST request
  int httpResponseCode = http.GET();

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    //Serial.println(payload);
    // Parse the JSON payload
    JSONVar data = JSON.parse(payload);

    // Extract the first event title and time
    JSONVar titles = data["title"];
    JSONVar times = data["event"];
    if (titles.length() > 0 && times.length() > 0) {
      title_real = (const char*)titles[0];
      int time = times[0];
      RealTime = unixTimeToDateTime(time);
    }
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
