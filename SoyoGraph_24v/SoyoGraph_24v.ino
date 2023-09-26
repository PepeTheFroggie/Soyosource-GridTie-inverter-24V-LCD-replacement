#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <time.h>                  
#include "soyo.h"

#include <SoftwareSerial.h>
#define txpin 12// gpio12
SoftwareSerial mySerial(-1, txpin); // RX, TX

// http://192.168.178.67/

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

const char *ssid = "FSM";
const char *password = "0101010101";

/* Configuration of NTP */
#define MY_NTP_SERVER "ch.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/03"
time_t now;  // this is the epoch
tm tm;       // the structure tm holds time information 

const int led = LED_BUILTIN;

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 ()
{
  Serial.println("sntp_update_delay_MS");
  return 12 * 60 * 60 * 1000UL; // 12 hours
}

void setup() 
{
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  pinMode(txpin, OUTPUT);
  mySerial.enableRx(false);
  mySerial.begin(4800); 
  mySerial.println("Hi ");   

  Serial.begin(9600);
  Serial.println();  
  Serial.println("Start");
  
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected to WiFi");
  Serial.println(WiFi.localIP());

  httpUpdater.setup(&server,"/up"); 
  server.on("/", getData);
  server.on("/settings", handleSettings);
  server.on("/pwr.svg", drawGraph);
  server.on("/limit", limiter);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  clearwp();
  
  Serial.println("Switching to alt. Port now");
  delay(500);
  Serial.swap(); //Tx = GPIO15, Rx = GPIO13
  
  configTime(MY_TZ, MY_NTP_SERVER); 
  digitalWrite(led, HIGH);
}

void flush_serinput()
{
  while(Serial.available()) Serial.read();
}

int msgct = 0;
int errmsgct = 0;
int cserr = 0;
byte inmsg[15];
bool IsRead;
bool IsBat;
bool IsPow;
bool IsLim;
bool DoLimit;

#define storeinterval 30 // 30 sec, 6h
time_t nextStoreTime = storeinterval;
#define msginterval 3
time_t nextmsg = msginterval;
#define limitinterval 1
time_t nextlimit = limitinterval;

void loop() 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    int avail = Serial.available();
    if (avail == 15) 
    {
      digitalWrite(led, LOW);
      Serial.read(se,15);
      if (se[0] == 0xA6) 
      {
        if (chksum_se(se) == se[14])
        {
          copy_msg_soyo_esp();
          msgct++;
          
          time(&now);
          if (now >= nextStoreTime)
          {
            nextStoreTime = storeinterval + now;
            localtime_r(&now, &tm);           
            storewp(sd.V_input,sd.A_input,tm.tm_hour);
          }
        }
        else cserr++;      
      }
      else flush_serinput();
    }
    else if (avail > 15)  flush_serinput();
    digitalWrite(led, HIGH);    
  }

  if (IsRead)
  {
    IsRead = false;
    msg_esp_soyo(0x02);
    Serial.write(es,6);
  }
  else if (IsLim)
  {
    IsLim = false;
    msg_esp_soyo(0x03);
    Serial.write(es,6);
  } 
  else if (IsBat)
  {
    IsBat = false;
    msg_esp_soyo(0x0A);
    Serial.write(es,6);
  } 
  else if (IsPow)
  {
    IsPow = false;
    msg_esp_soyo(0x12);
    Serial.write(es,6);
  } 
  else 
  {
    time(&now); // read the current time
    if (now >= nextmsg)
    {
      nextmsg = now + msginterval;    
      msg_esp_soyo(0x01);
      Serial.write(es,6);
    }
  } 
  if (DoLimit)
  {
    time(&now); // read the current time
    if (now >= nextlimit)
    {
      nextlimit = now + limitinterval; 
      sendlimit(sd.limit_power);   
      mySerial.write(lim,8);
    }
  }
          
  for (int i=0;i<25;i++)  // 500ms
  {
    delay(20); 
    server.handleClient();
  }
}
