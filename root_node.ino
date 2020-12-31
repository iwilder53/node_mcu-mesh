//This Code will only be utilised for root node. 
// Cloud IP - 50.62.22.142 
//#include "RTCDS1307.h"
#include <TZ.h>
#include "RTClib.h"
#include <LittleFS.h>
#include "FS.h"
#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>

int prevPeriod = 0;                                         // Function "mllis()" gives time in milliseconds. Here "period" will store time in seconds

#include <coredecls.h>                  // settimeofday_cb()
#include <Schedule.h>
#include <PolledTimeout.h>

#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

#include <sntp.h>  
//#include "SD.h"
//#include "SPI.h"


bool isConnected;

#define MYTZ TZ_Asia_Kolkata
 
uint32_t ack_to_node;
boolean rtcSet ;
long time_now = 0;
static timeval tv;
static timespec tp;
static time_t now;
static uint32_t now_ms, now_us;

static esp8266::polledTimeout::periodicMs showTimeNow(60000);
static int time_machine_days = 0; // 0 = now
static bool time_machine_running = false;

extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

AsyncWebServer server(80);

#define   MESH_PORT       5555                               

#define HOSTNAME "MQTT_Bridge"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendLog();
void sendTime();
void showTime() ;
void time_is_set_scheduled();
void updateTime();

IPAddress getlocalIP();
IPAddress testIP(0,0,0,0);
IPAddress myIP(0,0,0,0);


IPAddress mqtt1 ;
IPAddress mqtt2 ;


uint8_t ip1_oct1,ip1_oct2,ip1_oct3,ip1_oct4;
uint8_t ip2_oct1,ip2_oct2,ip2_oct3,ip2_oct4;
 long pos;


String Secondary_SSID;
String Secondary_PASS;



painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);            
          
Scheduler userScheduler;
RTC_DS3231 rtc;

uint8_t year, month, weekday, day, hour, minute, second;

int period;

String m[12] = {"Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho", "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"};
String w[7] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
Task taskUpdateTime(TASK_SECOND, TASK_FOREVER, &updateTime );  
Task taskSetRtc(TASK_SECOND, TASK_ONCE, &RtcSetTime );  

void updateTime(){
  time_now++;
}
void RtcSetTime(){

  if(!rtcSet){

  Serial.print("getting timeStamp");
  // DateTime now = rtc.now();
// Serial.print(now.unixtime() / 86400L);
  gettimeofday(&tv, nullptr);
  clock_gettime(0, &tp);
  now = time(nullptr);
  //Serial.print("time:      ");
  int timeNow = ((uint32_t)now);
  //timeNow +=  timeNow + 19800;
  rtc.adjust((uint32_t)now);
  rtcSet = true;
  Serial.println("setting rtc to :" );
  Serial.print(timeNow);

  }
}
// to send time to mesh nodes 
Task taskSendTime(TASK_MINUTE * 1 , TASK_FOREVER, &sendTime );   
void sendTime()
{ 
  

  String timeFromRTC = String(time_now);
  mesh.sendBroadcast(timeFromRTC);
 Serial.println(timeFromRTC);
isConnected = false;
String ack_pulse_to_sub = "ready";
mqttClient.publish("hetadatainMesh/from/gateway", ack_pulse_to_sub.c_str());
 }
//for testing the topology 
String scanprocessor(const String& var)
{
  if(var == "SCAN")
  String connlist = mesh.subConnectionJson();
   // return connlist ;
  return String();
}
// sending the data stored 
Task taskSendLog( TASK_MILLISECOND * 500 , TASK_FOREVER, &sendLog );   // Set task second to send msg in a time interval

  void sendLog(){
         String logs;
         String topic = "hetadatainMesh/from/gateway";

    LittleFS.begin();
    File file = LittleFS.open("offlinelog.txt","r"); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (!file) {
    Serial.println("No File Found ");
    taskSendLog.disable();
          pos = 0;
          //timeIndex = 0;
             return;
  }
    // String logs;
    String buffer ;
    uint8_t i = 0;
 for (int i = 0; i < 1 ; i++) 
{ 
      file.seek(pos);
     buffer = file.readStringUntil('\n');
   // Serial.println(buffer); //Printing for debugging purpose         
       logs = buffer; 
  if(buffer != ""){
      mqttClient.publish(topic.c_str(), logs.c_str());
    // mesh.sendSingle(root, logs );                                       
      Serial.println(logs); 
      pos = file.position();
  }
 
  file.close();
  Serial.println(F("DONE Reading"));
 // if (pos == file.size()){
  }
    if (buffer == "") { 

      if(rtc.lostPower()){
        String msgRTCAlert =  "RTC lost time";
        mqttClient.publish(topic.c_str(),msgRTCAlert.c_str());
        Serial.print(msgRTCAlert);
      }
     Serial.print ("done dumping");
      LittleFS.remove("offlinelog.txt");
  }
      LittleFS.end();

    }

void setup() {
Serial.begin(115200);
  rtc.begin();



LittleFS.begin();
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<400> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {

    Serial.println("Failed to parse config file");

  }
  
  
  const char* MESH_PREFIX = doc["mssid"];
  const char* MESH_PASSWORD = doc["mpass"];
  const char* STATION_SSID = doc["rssid1"];
  const char* STATION_PASSWORD = doc["rpass1"];
  const char* STATION_SSID_1 = doc["rssid2"];
  const char* STATION_PASSWORD_1 = doc["rpass2"];

  Secondary_SSID = STATION_SSID_1;
  Secondary_PASS = STATION_PASSWORD_1;

  Serial.println(MESH_PREFIX);
  Serial.println(MESH_PASSWORD);
  Serial.println(STATION_SSID);
  Serial.println(STATION_PASSWORD);
  Serial.println(Secondary_SSID);
  Serial.println(Secondary_PASS);


  const char* ip1 = doc["ip11"]; 
  const char* ip2 = doc["ip12"]; 
  const char* ip3 = doc["ip13"]; 
  const char* ip4 = doc["ip14"];
  ip1_oct1 = atoi(ip1);
  ip1_oct2 = atoi(ip2);
  ip1_oct3 = atoi(ip3);
  ip1_oct4 = atoi(ip4);

  IPAddress temp1 (ip1_oct1, ip1_oct2, ip1_oct3, ip1_oct4);
  mqtt1 = temp1;
  Serial.println(mqtt1);

  const char* ip21 = doc["ip21"];
  const char* ip22 = doc["ip22"]; 
  const char* ip23 = doc["ip23"]; 
  const char* ip24 = doc["ip24"];
  

  ip2_oct1 = atoi(ip21);
  ip2_oct2 = atoi(ip22);
  ip2_oct3 = atoi(ip23);
  ip2_oct4 = atoi(ip24);

  IPAddress temp2 (ip2_oct1, ip2_oct2, ip2_oct3, ip2_oct4);
  mqtt2 = temp2;
  Serial.println(mqtt2);

  //for future use 
/*
uint8_t ip[4];
const char* ip_1 = doc["IP1"];
const char* ip_2 = doc["IP2"];
const char* ip_3 = doc["IP3"];
const char* ip_4 = doc["IP4"];
*/
//ip = atoi(doc["ip1"]);        //   test this later.

/*
  ip2_oct1 = atoi(ip_1);
  ip2_oct2 = atoi(ip_2);
  ip2_oct3 = atoi(ip_3);
  ip2_oct4 = atoi(ip_4);*/

//temp2 = (ip2_oct1, ip2_oct2, ip2_oct3, ip2_oct4);
//local_IP = temp2;





  LittleFS.end();
mqttClient.setServer(mqtt1, 1883);
mqttClient.setCallback(mqttCallback);

 // rtc.setDate(, 2, 28);
 // rtc.setTime(23, 59, 50);

// mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION  | MSG_TYPES | REMOTE | GENERAL);                  

 
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 1 );
  mesh.onReceive(&receivedCallback);
  Serial.print("Node id is: ");                                                 
  Serial.println(mesh.getNodeId());

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  mesh.setRoot(true);
  mesh.setContainsRoot(true);


  userScheduler.addTask( taskSendLog );
  userScheduler.addTask(taskSendTime);
  userScheduler.addTask(taskUpdateTime);
  userScheduler.addTask(taskSetRtc);
  taskUpdateTime.enable();
  taskSendTime.enable();
  
   configTime(MYTZ, "pool.ntp.org");
    sntp_servermode_dhcp(0);
    showTime();

    //RTC time 
  DateTime rtcnow = rtc.now();
  time_now = rtcnow.unixtime();

  //Async webserver for debugging 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
    if (request->hasArg("BROADCAST"))
      {
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
      }
    });
  server.on("/map", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send_P(200, "text/html", "<html><head><script type='text/javascript' src='https://cdnjs.cloudflare.com/ajax/libs/vis/4.21.0/vis.js'></script><link href='https://cdnjs.cloudflare.com/ajax/libs/vis/4.21.0/vis-network.min.css' rel='stylesheet' type='text/css' /><style type='text/css'>#mynetwork {width: 1024px;height: 768px;border: 1px solid lightgray;}</style></head><body><h1>PainlessMesh Network Map</h1><div id='mynetwork'></div><a href=https://visjs.org>Made with vis.js<img src='http://www.davidefabbri.net/files/visjs_logo.png' width=40 height=40></a><script>var txt = '%SCAN%';</script><script type='text/javascript' src='http://www.davidefabbri.net/files/painlessmeshmap.js'></script></body></html>", scanprocessor);
    });
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "application/json", mesh.subConnectionJson(false) );
    });
  server.on("/asnodetree", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "text/html", mesh.asNodeTree().toString() );
    });
  server.begin();



 

}

void loop() {
  mesh.update(); // mesh scheduler
  mqttClient.loop();


 if (time_now == 10800){ESP.reset();}
 //various checks to ensure redunduncy
  if(getlocalIP() == testIP){
    isConnected = false;

  } 
//for secondary network if it exists

if(millis() >= 60000 && testIP == getlocalIP()) 
  {
    Serial.print("trying secondary ssid");
    mesh.stationManual(Secondary_SSID, Secondary_PASS);
    Serial.print(mqtt1);
    mqttClient.setServer(mqtt2, 1883);
  }
//for recconecting 
  if (myIP == getlocalIP() &&(mqttClient.connected() == false)) {
            //taskSendLog.enable();

       if (mqttClient.connect("hetadatainMeshClient")) {
      mqttClient.publish("hetadatainMesh/from/gateway","Ready! Reconnected");
      mqttClient.subscribe("hetadatainMesh/to/gateway");
      //taskSendLog.enable();
       }
    } 
//  connecting for the first time
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("hetadatainMeshClient")) {
      mqttClient.publish("hetadatainMesh/from/gateway","ready");
      mqttClient.subscribe("hetadatainMesh/to/gateway");
      taskSetRtc.enableDelayed(1000);

    } 
  }
}

// Publish Received msg from nodes to mqtt broker if connected or save to internal storage
void receivedCallback( const uint32_t &from, const String &msg ) {
 Serial.println(msg);
  
  if(msg == "online?"){
  mesh.sendSingle(from, String("online"));

    }
  
  else{
    if (!mqttClient.connected() || isConnected == false)
    {
      taskSendLog.disable();
      LittleFS.begin();
      File dataFile = LittleFS.open("offlinelog.txt", "a");
       dataFile.println(msg);
       dataFile.close();
       LittleFS.end();
       Serial.print("to card");
    }
    else {
  String topic = "hetadatainMesh/from/" + String(from);
 
  mqttClient.publish(topic.c_str(), msg.c_str());
    }
  }
  
   if(period >= 600 && testIP == getlocalIP()){
  while(1);
  }
 
                                   
}
//for recieving data from broker
void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {

  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);
  String targetStr = String(topic).substring(16);

      isConnected = true;
      //taskSendLog.enable();

  if(targetStr == "gateway")
  {
      isConnected = true;
      taskSendLog.enable();
   
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("hetadatainMesh/from/gateway", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
    // mqttClient.publish("hetadatainMesh/from/gateway", "Client not connected!");
    }
 
  
}}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}

// time machine for maintaining ntp time
void time_is_set_scheduled() {
  if (time_machine_days == 0) {
    time_machine_running = !time_machine_running;
  }

  // time machine demo
  if (time_machine_running) {
    if (time_machine_days == 0)
      Serial.printf("---- settimeofday() has been called - possibly from SNTP\n"
                    "     (starting time machine demo to show libc's automatic DST handling)\n\n");
    now = time(nullptr);
    const tm* tm = localtime(&now);
    Serial.printf("future=%3ddays: DST=%s - ",
                  time_machine_days,
                  tm->tm_isdst ? "true " : "false");
    Serial.print(ctime(&now));
    gettimeofday(&tv, nullptr);
    constexpr int days = 30;
    time_machine_days += days;
    if (time_machine_days > 360) {
     // tv.tv_sec -= (time_machine_days - days) * 60 * 60 * 24;
      time_machine_days = 0;
    } else {
      //tv.tv_sec += days * 60 * 60 * 24;
    }
   // settimeofday(&tv, nullptr);
  } else {
    showTime();
  }
}
//check to see if ntp is working 
void showTime() {
  gettimeofday(&tv, nullptr);
  clock_gettime(0, &tp);
  now = time(nullptr);
  now_ms = millis();
  now_us = micros();



  // EPOCH+tz+dst
  Serial.print("gtod:      ");
  Serial.print((uint32_t)tv.tv_sec);
  Serial.print("s / ");
  Serial.print((uint32_t)tv.tv_usec);
  Serial.println("us");

  // EPOCH+tz+dst
  Serial.print("time:      ");
  Serial.println((uint32_t)now);

  // timezone and demo in the future
  Serial.printf("timezone:  %s\n", getenv("TZ") ? : "(none)");

  // human readable
  Serial.print("ctime:     ");
  Serial.print(ctime(&now));


  Serial.println();
}

 
