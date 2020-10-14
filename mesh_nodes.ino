#include <SD.h> 

#include "painlessMesh.h"
#include <SPI.h> 


#define   MESH_PREFIX     "HetaDatain"                         // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
#define   MESH_PASSWORD   "Test@Run_1"                         // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                                      // Mesh Port should be same for all  nodes in Mesh Network 

 

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

unsigned long period=0;  

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void writeToCard() ;
void loggedData();


Task taskSendMessage( TASK_SECOND * 4 , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval (Here interval is 4 second)


// If you want to receive sensor readings from this node, write code in below function....

void sendMessage() {
 
  String msg = " NODE no.1  ";                                       // You can write node name/no. here so that you may easily recognize it        
 // msg += mesh.getNodeId();                                              // Adding Node id in the msg
   msg += " Analog: " + String (analogRead(A0));                          // Adding  analog reading in the msg. You can also add other pin readings 
 // msg += " myFreeMemory: " + String(ESP.getFreeHeap());                 // Adding free memory of Nodemcu in the msg
// msg += "from end node1"; 
  uint32_t target = 314262534; 
   mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
  Serial.println(msg);
  Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");


        

  } 


 Task taskWriteToCard( TASK_SECOND * 4 , TASK_FOREVER, &writeToCard );

 
 void writeToCard()
 { 

  String msg = " NODE no.1  "; 
   msg += " Analog: " + String (analogRead(A0)); 
   
    File dataFile = SD.open("offlinelog.txt", FILE_WRITE);
// if the file is available, write to it:
  if (dataFile) {
    dataFile.println(msg);
    dataFile.close();
    // print to the serial port too:
    Serial.println("to SD Card"); 
    Serial.println(msg);
    Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");}
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt"); 
    }
  } 

  
   Task taskLoggedData(TASK_SECOND * 4 , TASK_FOREVER, &loggedData );

void loggedData(){
  
  }


void receivedCallback( uint32_t from, String &msg ) {
 Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
 
}

void newConnectionCallback(uint32_t nodeId) {
   Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
 Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
    
    //CHECK FOR SDCARD
    if (!SD.begin(D8)) {
    
   Serial.println("Card failed, or not present");
     // don't do anything more:
     while (1);
  }
  Serial.println("card initialized."); 

// mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  //mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.setContainsRoot(true);
  
  
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask( taskSendMessage );
  userScheduler.addTask(taskWriteToCard);
  userScheduler.addTask(taskLoggedData);
  taskSendMessage.enable();
  
  pinMode(A0, INPUT);                                                    // Define A0 pin as INPUT
 
  
}

void loop() {
  // it will run the user scheduler as well

period=millis()/1000;                                                    // Function "mllis()" gives time in milliseconds. Here "period" will store time in seconds
 
  mesh.update();
 //Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db


  
  if( (WiFi.RSSI()) == 31 ){
    taskWriteToCard.enable();
     digitalWrite(LED_BUILTIN,LOW);                              // LED will be ON when node is writibng to sd card                                
    } 

  
 /* if (period>60)                                                         // When period will be > 60 seconds, deep sleep mode will be active
  {
     mesh.stop();
 ESP.deepSleep(300e6);                                                    // deepSleep mode will be active for 300*10^6 microseconds, i.e. for 300 seconds                                                   
 digitalWrite(LED_BUILTIN,HIGH); 
  }*/
}
void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}

