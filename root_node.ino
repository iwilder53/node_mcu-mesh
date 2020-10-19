//************************************************************

// this is a simple example that uses the painlessMesh library to connect to a another network and relay messages from a
// MQTT broker to the nodes of the mesh network. It acts as bridge between mesh network and mqtt broker.
// To send a message to a mesh node, you can publish it to "painlessMesh/to/12345678" where 12345678 equals the nodeId.
// To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
// Every message from the mesh which is send to the gateway node will be published to "painlessMesh/from/12345678" where 12345678 
// is the nodeId from which the packet was send.


// To know Node id, Upload code to Nodemcu, Open Serial Monitor and Press RST button on Nodemcu.
// Enter wi-fi ssid and wi-fi password in line 24 and 25 respectively.
// Enter local ip address/mqtt broker server ip address in line 40.

//************************************************************

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




AsyncWebServer server(80);


#define   MESH_PREFIX     "HetaDatain"                  // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
#define   MESH_PASSWORD   "Test@Run_1"                  // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                               // Mesh Port should be same for all  nodes in Mesh Network

// Add wi-fi credentials to connect with mqtt  broker
#define   STATION_SSID     "Hetadatain_GF"                      // Enter Wi-Fi SSID (Your PC/Laptop should be connected to same network)
#define   STATION_PASSWORD "hetadatain@123"                        // Enter Wi-Fi Password

#define HOSTNAME "MQTT_Bridge"

// Prototypes
void receivedCallback( const uint32_t &from, const String &msg );
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 31, 232);                     // Enter Local ip address of your PC/Laptop

painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);               // 1883 is default port for mqtt broker.



String scanprocessor(const String& var)
{
  if(var == "SCAN")
    return mesh.subConnectionJson(false) ;
  return String();
}



void setup() {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION  | MSG_TYPES | REMOTE );                           // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 11 );
  mesh.onReceive(&receivedCallback);
  Serial.print("Node id is: ");                                                   //To know Node id, Upload code to Nodemcu, Open Serial Monitor and Press RST button on Nodemcu
  Serial.println(mesh.getNodeId());
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  
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
  /*server.on("/asnodetree", HTTP_GET, [](AsyncWebServerRequest *request)
    {
    request->send(200, "text/html", mesh.asNodeTree().c_str() );
    });*/
  server.begin();

 
  mesh.initOTAReceive("bridge");

}

void loop() {
  mesh.update();

  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("hetadatainMeshClient")) {
      mqttClient.publish("hetadatainMesh/from/gateway","Ready!");
      mqttClient.subscribe("hetadatainMesh/to/#");
    } 
  }
}

// Publish Received msg from nodes to mqtt broker
void receivedCallback( const uint32_t &from, const String &msg ) {
 Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "hetadatainMesh/from/" + String(from);
 
  mqttClient.publish(topic.c_str(), msg.c_str());
  Serial.println(mesh.asNodeTree().toString());                                   // Gives the mesh network tree structre

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
  }
  
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
