//This Code will only be utilised for root node.
//#include "RTCDS1307.h"
#include "RTClib.h"

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

bool isConnected;


AsyncWebServer server(80);


#define   MESH_PREFIX     "HetaDatain"                  
#define   MESH_PASSWORD   "Test@Run_1"              
#define   MESH_PORT       5555                               
// Add wi-fi credentials to connect with mqtt  broker
#define   STATION_SSID     "Hetadatain_GF"                     
#define   STATION_PASSWORD "hetadatain@123"               

#define HOSTNAME "MQTT_Bridge"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendNmap();
void sendTime();

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 31, 35);                    

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);    
//PubSubClient nMapClient(mqttBroker, 1884, mqttCallback, wifiClient);              
          
Scheduler userScheduler;
RTC_DS1307 rtc;

uint8_t year, month, weekday, day, hour, minute, second;

bool period = 0;

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

 String topic = "Nmap/from/gateway";
 
 mqttClient.publish(topic.c_str(), msg.c_str());
  
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
      String topic = "Nmap/from/gateway";
     String nMap = mesh.asNodeTree().toString();
 
 mqttClient.publish(topic.c_str(), nMap.c_str());
    
    }

void setup() {
  Serial.begin(115200);
  rtc.begin();
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

 
  mesh.initOTAReceive("bridge");

}

void loop() {
  mesh.update();

  mqttClient.loop();
  //nMapClient.loop();


if (myIP == getlocalIP())
{isConnected = true;}

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    isConnected = true;

    if (mqttClient.connect("hetadatainMeshClient")) {
      mqttClient.publish("hetadatainMesh/from/gateway","Ready!");
      mqttClient.publish("Nmap/from/gateway","Ready!");

      mqttClient.subscribe("hetadatainMesh/to/+");
    } 
 /*  if (nMapClient.connect("Nmap/")) {
      nMapClient.publish("Nmap/","Ready!");
      nMapClient.subscribe("Nmap/+");
    }
*/
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
 
 mqttClient.publish("Nmap/from/gateway", nMap.c_str());
    isConnected = false;
    sendTime();
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
