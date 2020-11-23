//This Code will only be utilised for root node.
//#include "RTCDS1307.h"
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
//#include "SD.h"
//#include "SPI.h"

#define OTA_PART_SIZE 1024 //How many bytes to send per OTA data packet

bool isConnected;


AsyncWebServer server(80);


//#define   MESH_PREFIX     "HetaDatain"                  
//#define   MESH_PASSWORD   "Test@Run_1"              
#define   MESH_PORT       5555                               
// Add wi-fi credentials to connect with mqtt  broker
//#define   STATION_SSID     "Hetadatain_GF"                     
//#define   STATION_PASSWORD "hetadatain@123"               
//#define   STATION_SSID1     "Hetadatain_FF"                     
//#define   STATION_PASSWORD1 "hetadatain@123"
#define HOSTNAME "MQTT_Bridge"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendNmap();
void sendTime();

IPAddress getlocalIP();
IPAddress testIP(0,0,0,0);
IPAddress myIP(0,0,0,0);
//IPAddress mqttBroker(192, 168, 31, 35);     
//IPAddress mqttBroker_secondary(192, 168, 31, 35);


IPAddress mqtt1 (0,0,0,0);
IPAddress mqtt2 (0,0,0,0);


uint8_t ip1_oct1,ip1_oct2,ip1_oct3,ip1_oct4;
uint8_t ip2_oct1,ip2_oct2,ip2_oct3,ip2_oct4;

String Secondary_SSID;
String Secondary_PASS;



painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqtt1, 1883, mqttCallback, wifiClient);    
//PubSubClient nMapClient(mqttBroker, 1884, mqttCallback, wifiClient);              
          
Scheduler userScheduler;
RTC_DS1307 rtc;

uint8_t year, month, weekday, day, hour, minute, second;

int period;

String m[12] = {"Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho", "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"};
String w[7] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

Task taskSendTime( TASK_SECOND * 5 , TASK_FOREVER, &sendTime );   
void sendTime()
{ Serial.print("getting timeStamp");
   
    DateTime now = rtc.now();
    Serial.print(now.unixtime() / 86400L);

    String msg = String(now.unixtime() );
  mesh.sendBroadcast(msg);
  Serial.print(msg);

 //String topic = "Nmap/from/gateway";
 
 //mqttClient.publish(topic.c_str(), msg.c_str());
  
  }
String scanprocessor(const String& var)
{
  if(var == "SCAN")
    return mesh.subConnectionJson(false) ;
  return String();
}

Task taskSendNmap( TASK_IMMEDIATE , TASK_ONCE, &sendNmap );   // Set task second to send msg in a time interval

  void sendNmap(){
    Serial.print("sending tree");
      String topic = "hetadatainMesh/from/gateway";
     String nMap = mesh.asNodeTree().toString();
 
 mqttClient.publish(topic.c_str(), nMap.c_str());
    
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

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<300> doc;
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


  LittleFS.end();

  
  //rtc.setDate(19, 2, 28);
  //rtc.setTime(23, 59, 50);

 mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION  | MSG_TYPES | REMOTE | GENERAL);                  

 
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);
  Serial.print("Node id is: ");                                                 
  Serial.println(mesh.getNodeId());
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

   mesh.setRoot(true);
 mesh.setContainsRoot(true);


  userScheduler.addTask( taskSendNmap );
  userScheduler.addTask(taskSendTime);
  taskSendNmap.enable();
  taskSendTime.enable();
  
 
  
  //Async webserver
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


  pinMode(LED_BUILTIN, OUTPUT);

 /*

if (!SD.begin(D8)) {
    ("Could not mount SD card");
  }

  File dir = SD.open("/");
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) { //End of files
      ("Could not find valid firmware, please validate and restart");
    }

    //This block of code parses the file name to make sure it is valid.
    //It will also get the role and hardware the firmware is targeted at.
    if (!entry.isDirectory()) {
      TSTRING name = entry.name();
      if (name.length() > 1 && name.indexOf('_') != -1 &&
          name.indexOf('_') != name.lastIndexOf('_') &&
          name.indexOf('.') != -1) {
        TSTRING firmware = name.substring(1, name.indexOf('_'));
        TSTRING hardware =
            name.substring(name.indexOf('_') + 1, name.lastIndexOf('_'));
        TSTRING role =
            name.substring(name.lastIndexOf('_') + 1, name.indexOf('.'));
        TSTRING extension =
            name.substring(name.indexOf('.') + 1, name.length());
        if (firmware.equals("firmware") &&
            (hardware.equals("ESP8266") || hardware.equals("ESP32")) &&
            extension.equals("bin")) {

          Serial.println("OTA FIRMWARE FOUND, NOW BROADCASTING");

          //This is the important bit for OTA, up to now was just getting the file. 
          //If you are using some other way to upload firmware, possibly from 
          //mqtt or something, this is what needs to be changed.
          //This function could also be changed to support OTA of multiple files
          //at the same time, potentially through using the pkg.md5 as a key in
          //a map to determine which to send
          mesh.initOTASend(
              [&entry](painlessmesh::plugin::ota::DataRequest pkg,
                       char* buffer) {
                
                //fill the buffer with the requested data packet from the node.
                entry.seek(OTA_PART_SIZE * pkg.partNo);
                entry.readBytes(buffer, OTA_PART_SIZE);
                
                //The buffer can hold OTA_PART_SIZE bytes, but the last packet may
                //not be that long. Return the actual size of the packet.
                return min((unsigned)OTA_PART_SIZE,
                           entry.size() - (OTA_PART_SIZE * pkg.partNo));
              },
              OTA_PART_SIZE);

          //Calculate the MD5 hash of the firmware we are trying to send. This will be used
          //to validate the firmware as well as tell if a node needs this firmware.
          MD5Builder md5;
          md5.begin();
          md5.addStream(entry, entry.size());
          md5.calculate(); 

          //Make it known to the network that there is OTA firmware available.
          //This will send a message every minute for an hour letting nodes know
          //that firmware is available.
          //This returns a task that allows you to do things on disable or more,
          //like closing your files or whatever.
          mesh.offerOTA(role, hardware, md5.toString(),
                        ceil(((float)entry.size()) / OTA_PART_SIZE), false);

        }
      }
    }
  }


*/

}

void loop() {
  mesh.update();
  period=millis()/1000;                                                    // Function "mllis()" gives time in milliseconds. Here "period" will store time in seconds

  mqttClient.loop();
  //nMapClient.loop();

if(period == 60 && testIP == getlocalIP()) 

  {
    Serial.print("trying secondary ssid");
    mesh.stationManual(Secondary_SSID, Secondary_PASS);
    mqtt1 = mqtt2;
    Serial.print(mqtt1);
  }

 

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("hetadatainMeshClient")) {
      //mqttClient.publish("hetadatainMesh/from/gateway","Ready!");
      mqttClient.publish("Nmap/from/gateway","Ready!");

      mqttClient.subscribe("hetadatainMesh/to/+");
    } 





  }





   
}

// Publish Received msg from nodes to mqtt broker
void receivedCallback( const uint32_t &from, const String &msg ) {
 //Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
 
  
  if(msg == "online?"){
  // if(isConnected == true){
    mesh.sendSingle(from, String("online"));

    //  }
    }
  
  else{
    
  String topic = "hetadatainMesh/from/" + String(from);
 
  mqttClient.publish(topic.c_str(), msg.c_str());


String nMap = mesh.asNodeTree().toString();
 Serial.print(mesh.asNodeTree().toString());

 String meshTopology = mesh.subConnectionJson();
     if (meshTopology != NULL)
          Serial.printf("MeshTopology: %s\n", meshTopology.c_str());
    
 
 mqttClient.publish("hetadatainMesh/from/gateway", nMap.c_str());
    isConnected = false;
    sendTime();
    }
  
  
   if(period >= 600 && testIP == getlocalIP()){
  while(1);
  }
 
                                   
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
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
      mqttClient.publish("hetadatainMesh/from/gateway", "Client not connected!");
    }
 
  
}}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
