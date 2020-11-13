    
  #include "painlessMesh.h"
  #include <MCP3008.h>
  #include <LittleFS.h>
  #include "FS.h"



  

#define CS_PIN D8
#define CLOCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6
#define   MESH_PREFIX     "HetaDatain"                         // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
#define   MESH_PASSWORD   "Test@Run_1"                         // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                                      // Mesh Port should be same for all  nodes in Mesh Network

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);

  uint8_t sendDelay = 2;
  unsigned long period=0; 
  uint16_t val, val1, val2, val3, val4, val5, val6, val7 ;
  String id; 
  int wdt = 0; 
  int ts_epoch;
  int timeIndex;
  int pos;
  uint32_t root;
  long previousMillis = 0;  

  String msgSd;
  uint8_t interval = 1000;
  bool ackStatus;
  // User stub
   void updateTime();
   void writeToCard();
   void sendMessage() ;// Prototype so PlatformIO doesn't complain
  void sendMsgSd();
  Task taskUpdateTime( TASK_SECOND * 1 , TASK_FOREVER, &updateTime );   // Set task second to send msg in a time interval (Here interval is 4 second)
  
  void updateTime(){
    wdt++;
    ts_epoch++;
    }


 
  Task taskSendMessage( TASK_MINUTE * 2 , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval (Here interval is 4 second)
  Task taskSendMsgSd( TASK_SECOND * 5 , TASK_FOREVER, &sendMsgSd );   // Set task second to send msg in a time interval
  Task taskWriteToCard( TASK_MINUTE * 2 , TASK_FOREVER, &writeToCard );

 
  // If you want to receive sensor readings from this node, write code in below function....

  void sendMessage() {
       digitalWrite(LED_BUILTIN, LOW);  

   String msg = "online?";                                       // You can write node name/no. here so that you may easily recognize it        
   uint32_t target = 314262534;                                         // Target is Node id of the node where you want to send sms (Here, write node id of mqtt bridge (Root Node))
   mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   Serial.println(msg);
  //Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db
    if (WiFi.RSSI() == 31){

      taskWriteToCard.enable();
    }
   } 

 
  // Msg recived by node. If you want to perform any task by receiving msg, write code in the below function....

  void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
 if (msg=="online")
  {
   digitalWrite(LED_BUILTIN, LOW);  
    // You can write node name/no. here so that you may easily recognize it        
   String msg = readMcp();
   mesh.sendSingle(root, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   Serial.println(msg);  // If msg published from mqtt broker is LightON, Turn ON the built in LED of Nodemcu
   wdt = 0;
   ackStatus = true;
   taskWriteToCard.disable();


   }


   String strMsg = String(msg);
   ts_epoch = strMsg.toInt();
 }

  void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
            taskSendMsgSd.enable();
            taskWriteToCard.disable();

 }

  void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
 }

  void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
 }

  void setup() {
  Serial.begin(115200);
  LittleFS.begin();
  File configFile = LittleFS.open("config.json", "r");
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

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
  }

  const char* ID = doc["id"];
  const char* ROOT = doc["root"];

  // Real world application would store these values in some variables for
  // later use.

  Serial.print("Loaded id: ");
    id = atoi(ID);
  Serial.println(ID);
  Serial.print("Loaded root id: ");
   root = atoi(ROOT);
  Serial.println(ROOT);

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  // mesh.setDebugMsgTypes( ERROR | DEBUG );  // set before init() so that you can see startup messages

    mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
   //mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
   mesh.setContainsRoot(true);

 
    mesh.onReceive(&receivedCallback);
   mesh.onNewConnection(&newConnectionCallback);
   mesh.onChangedConnections(&changedConnectionCallback);
   mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
   mesh.onNodeDelayReceived(&delayReceivedCallback);

    userScheduler.addTask( taskSendMessage );
    userScheduler.addTask( taskUpdateTime );
    userScheduler.addTask(taskSendMsgSd);
    userScheduler.addTask(taskWriteToCard);



   taskUpdateTime.enable();
   taskSendMessage.enable();
   pinMode(A0, INPUT);                                                    // Define A0 pin as INPUT
   pinMode(LED_BUILTIN, OUTPUT);                                          // Define LED_BUILTIN as OUTPUT
   digitalWrite(LED_BUILTIN, HIGH);                                       // Initially the LED will be off

 
  }

  void loop() {
   // it will run the user scheduler as well

  period=millis()/1000;                                                    // Function "mllis()" gives time in milliseconds. Here "period" will store time in seconds

    mesh.update();
    if(wdt == 180 ){
      writeTimeToCard();
      while(1);
      } 

             //digitalWrite(LED_BUILTIN,LOW);                              // LED will be ON when node in not deepSleep mode                                     

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

 String readMcp()
 {
   String msgMcp;
  val = adc.readADC(0);
  Serial.println(val);
  val1 = adc.readADC(1);
  Serial.println(val1);
  val2 = adc.readADC(2); 
  Serial.println(val2);
  val3 = adc.readADC(3);
  Serial.println(val3);
  val4 = adc.readADC(4);
  Serial.println(val4);
  val5 = adc.readADC(5);
  Serial.println(val5);
  val6 = adc.readADC(6); 
  Serial.println(val6);
  val7 = adc.readADC(7);
  Serial.println(val7);

msgMcp = id;
msgMcp += "," + String(ts_epoch);
msgMcp += "," + String (val); 
msgMcp += "," + String (val1); 
msgMcp += "," + String (val2);
msgMcp += "," + String (val3);
msgMcp += "," + String (val4); 
msgMcp += "," + String (val5); 
msgMcp += "," + String (val6);
msgMcp += "," + String (val7);
    return msgMcp;
}


void sendMsgSd(){

LittleFS.begin();

  File file = LittleFS.open("offlinelog.txt","r"); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (!file) {
    Serial.println("Failed to open file for reading");
    taskSendMsgSd.disable();
          pos = 0;
          timeIndex = 0;
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
       msgSd = buffer; 
  if(buffer != ""){

      mesh.sendSingle(root, msgSd );                                       
      Serial.println(msgSd); 
      pos = file.position();
  }
 
  file.close();
  Serial.println(F("DONE Reading"));
 // if (pos == file.size()){
  }
    if (buffer == "") { 
      String ackMsg = id ;
      ackMsg += "sent from sd card";
      mesh.sendSingle(root, ackMsg);
      LittleFS.remove("offlinelog.txt");
  }

      LittleFS.end();

}


void writeToCard(){
{  Serial.print("write to card works");
     if (ackStatus == false)
     {
     String  msg = readMcp();
       msg += "from sd";
       msg += String(timeIndex);
           Serial.println(msg);

      LittleFS.begin();
    File dataFile = LittleFS.open("offlinelog.txt", "w");
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
    Serial.println("error opening  offlinelog.txt"); 
    }
    timeIndex++;
    wdt = 0;
    ackStatus = false;

    LittleFS.end();
        
        }
    }

}
void writeTimeToCard()
{
      LittleFS.begin();
      SPIFFS.remove("time.txt");
    File dataFile = LittleFS.open("time.txt", "w");
  if (dataFile) {
    dataFile.println(ts_epoch);
    dataFile.close();
    }
  else {
    Serial.println("error opening  time.txt"); 
    }  LittleFS.end();
}
