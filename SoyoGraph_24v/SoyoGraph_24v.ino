#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <time.h>                  
#include "soyo.h"

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

  Serial.begin(9600);
  Serial.println();  
  Serial.println("Start");
  
  limiterInit();
  
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
  Serial.swap(); //Tx = GPIO15 D8, Rx = GPIO13 D7
  
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
bool DoLimit = true;

int pwrsum;

#define msginterval 3
time_t nextmsg = msginterval;
#define limitinterval 1
time_t nextlimit = limitinterval;
#define getpwrinterval 10 //getpwr and store interval
time_t nextgetpwr = getpwrinterval;

void loop() 
{
  int avail;
  
  time(&now); // read the current time

  avail = Serial.available();
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
      }
      else cserr++;      
    }
    else flush_serinput();
  }
  else if (avail > 15)  flush_serinput();
  digitalWrite(led, HIGH);    

  if (IsRead)
  {
    IsRead = false;
    msg_esp_soyo(0x02);
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
    if (now >= nextmsg)
    {
      nextmsg = now + msginterval;    
      msg_esp_soyo(0x01);
      Serial.write(es,6);
    }
  } 
  
  if (now >= nextgetpwr)
  {
    nextgetpwr = now + getpwrinterval; 
    int testpwr = shelly_getpwr();
    if (testpwr != 12345) 
    {
      pwrsum = constrain(testpwr,-1000,1000);

      // calc new limiter value
      if (sd.V_input >= 25.0)
      { 
        int locpwr = constrain(sd.req_power + pwrsum - sd.target_power,0,600);
        if (locpwr <= sd.min_power) locpwr = 0;
        if (pwrsum > 0) sd.req_power = (3*sd.req_power + locpwr)/4;   
        else            sd.req_power = locpwr;
      }
      else sd.req_power = 0;  
          
    }
    else pwrsum = 0; // getpwr failed

    //Store values for graph
    localtime_r(&now, &tm);           
    storewp(sd.V_input,sd.A_input,pwrsum,tm.tm_hour);
  }
          
  if (DoLimit)
  {
    if (now >= nextlimit)
    {
      nextlimit = now + limitinterval; 
      if (sd.manual_power > 0) SendLimit(sd.manual_power);
      else                     SendLimit(sd.req_power);
    }
  }
  else sd.req_power = 0;
  
  for (int i=0;i<25;i++)  // 500ms
  {
    delay(20); 
    server.handleClient();
  }
}
