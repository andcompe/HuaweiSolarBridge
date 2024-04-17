/*
  Huawei Solar Bridge
  Created by Anders Lilja, 2023.
  Please use as you wish, but don't blame me. :)
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define DATABUFSIZE 4096
#define DATACOLLECTIONDELAY 100

// Please see https://github.com/wlcrs/huawei_solar/wiki/Connecting-to-the-inverter for infomation about how to ADD the item to your HA. 
// This must be running with correct settings before you configure it in HA. I recommend to set a fixed IP for your Server side device in your DHCP.

// SET THIS PARAMETER TO true to generate the LAN side ESP8266 code. It will use the "LAN SERVER SIDE" WiFi settings below.
// SET THIS PARAMETER TO false to generate the LAN side ESP8266 code. It will use the "Huawei Solar Side" WiFi settings below.

#define IS_LAN_SIDE false
//-------------------------------------------
// LAN SERVER SIDE 
#if IS_LAN_SIDE

  #ifndef STASSID
    #define STASSID "MY_HOME_WIFI_SSID"
    #define STAPSK  "MY_HOME_WIFI_PASSWORD"
  #endif
  #define HSB_HOSTNAME "HuaweiSolarBridge"
//-------------------------------------------
// Huawei Solar Side
#else
  #ifndef STASSID
    #define STASSID "SUN2000-A_BUCH_OF_CHARS"
    #define STAPSK  "Changeme"  // Changeme is actually the default password for the inverter. It might be changed...
  #endif
  #define HSB_HOSTNAME "HuaweiSolarBridgeClient"
const char* inverter_host = "192.168.200.1";
  const int inverter_port = 6607;
#endif
//-------------------------------------------

//********** Nothing NEED to be changed below this line. Don't blame me if you do. The code works for me. :) *****************



const char* hostname = HSB_HOSTNAME;
const char* ssid = STASSID;
const char* password = STAPSK;
ESP8266WebServer server(80);  // WEBSERVER PORT
#if IS_LAN_SIDE
  WiFiServer hsserver(6607);      // HUAWEISOLAR PORT
#endif
WiFiClient hsclient;


uint8_t hsbuf[DATABUFSIZE];
uint8_t serialbuf[DATABUFSIZE];
int hsbufpos=0;
int serialbufpos=0;
long lastserialdata=0;
long lasthsdata=0;
size_t cnt_read;

#define MAXLOG 64
char logbuf[MAXLOG][128];
uint8_t logpos=0;
char log_tmpbuf[128];

void log(IPAddress ip)
{
  sprintf(logbuf[logpos],"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  logpos = (logpos+1)%MAXLOG;
}

void log(const char *msg)
{
  strncpy(logbuf[logpos],msg,127);
  logbuf[logpos][127]=0;
  logpos = (logpos+1)%MAXLOG;
}


void handleRoot() {
  StreamString temp;
  temp.reserve(MAXLOG*130);
  for(int i=0;i<MAXLOG;i++)
    if (logbuf[(i+logpos)%MAXLOG][0]!=0)
      temp.println(logbuf[(i+logpos)%MAXLOG]);
  server.send(200, "text/plain", temp.c_str());
}

void handleNotFound() {
  server.send(404, "text/plain", "not found");
}

void setup(void) {
  Serial.setRxBufferSize(DATABUFSIZE);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  
  log("Searching...");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("");
  log("Connected to ");
  log(ssid);
  log("IP address: ");
  log(WiFi.localIP());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  log("HTTP server started");
#if IS_LAN_SIDE  
  hsserver.begin();
#endif
// CLEAR SERIAL BUFFER!
  while (Serial.available())
    Serial.read(serialbuf,DATABUFSIZE);
}

void loop(void) {
  server.handleClient();

#if !IS_LAN_SIDE
  if (Serial.available() && !hsclient.connected())
  {
    hsclient.connect(inverter_host, inverter_port);
    if (!hsclient.connected())
    {
      log("Cannot connect to server");
      log(inverter_host);
      log(inverter_port);
      delay(1000);
    }
  }
#endif

  if (hsclient.connected())
  {
    if (hsclient.available() && hsbufpos<DATABUFSIZE-2)
    {
      cnt_read = hsclient.read(hsbuf+hsbufpos,DATABUFSIZE-hsbufpos);
      hsbufpos += cnt_read;
      lasthsdata=millis();
    }
    if(Serial.available() && serialbufpos<DATABUFSIZE-2)
    {
      cnt_read = Serial.read(serialbuf+serialbufpos,DATABUFSIZE-serialbufpos);
      serialbufpos += cnt_read;
      lastserialdata=millis();
    }
    if (hsbufpos>0 && millis()-lasthsdata>DATACOLLECTIONDELAY)
    {
      sprintf(log_tmpbuf,"Got tcp data (%d)",hsbufpos);
      log (log_tmpbuf);
      Serial.write(hsbuf,hsbufpos);
      hsbufpos=0;
    }
    if (serialbufpos>0 && millis()-lastserialdata>DATACOLLECTIONDELAY)
    {
      sprintf(log_tmpbuf,"Got serial data (%d)",serialbufpos);
      log (log_tmpbuf);
      hsclient.write(serialbuf,serialbufpos);
      serialbufpos=0;
    }
  }
  else 
  {
#if IS_LAN_SIDE
    if (hsserver.hasClient()) 
    {
      hsclient = hsserver.available();
      log("New client ");
      log(hsclient.remoteIP());
    }
#endif
  }
}
