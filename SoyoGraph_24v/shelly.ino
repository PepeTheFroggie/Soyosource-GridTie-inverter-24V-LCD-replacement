#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

String Shelly_IP = "192.168.178.47";

int http_getpwr1(String getstr)
{
  StaticJsonDocument<200> doc;
  WiFiClient client;
  HTTPClient http;
  int pwr = 12345;

  if (http.begin(client, getstr)) 
  {  
    int httpCode = http.GET();
    if (httpCode > 0) 
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
      {
        String payload = http.getString();
        //Serial.println(payload);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) 
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
        else pwr = doc["power"];
      }
    } 
    else Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
  } 
  else Serial.printf("[HTTP} Unable to connect\n");
  
  return pwr;
}

int http_getpwr3(String getstr)
{
  StaticJsonDocument<2000> doc;
  WiFiClient client;
  HTTPClient http;
  int pwr = 12345;

  if (http.begin(client, getstr)) 
  {  
    int httpCode = http.GET();
    if (httpCode > 0) 
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
      {
        String payload = http.getString();
        //Serial.println(payload);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) 
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
        else pwr = doc["total_power"];
      }
    } 
    else Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
  } 
  else Serial.printf("[HTTP} Unable to connect\n");
  
  return pwr;
}

#define power3
 
int shelly_getpwr()
{
  int pwrsum,pwr;
  
#if defined power3  
  pwrsum = http_getpwr3("http://" + Shelly_IP + "/status");
#else
  pwr = http_getpwr1("http://" + Shelly_IP + "/emeter/0");
  if (pwr == 12345) return pwr;
  pwrsum = pwr;
  pwr = http_getpwr1("http://" + Shelly_IP + "/emeter/1");
  if (pwr == 12345) return pwr;
  pwrsum += pwr;
  pwr = http_getpwr1("http://" + Shelly_IP + "/emeter/2");
  if (pwr == 12345) return pwr;
  pwrsum += pwr;
#endif 
  return pwrsum;
}
