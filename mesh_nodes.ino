    
  #include "painlessMesh.h"
  #include <MCP3008.h>'
  

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

  unsigned long period=0; 
  uint16_t val, val1, val2, val3, val4, val5, val6, val7 ;
  String id = "1"; 
  int wdt = 0; 

  // User stub
   void updateTime();
 void sendMessage() ; // Prototype so PlatformIO doesn't complain
  Task taskUpdateTime( TASK_SECOND * 1 , TASK_FOREVER, &updateTime );   // Set task second to send msg in a time interval (Here interval is 4 second)
  
  void updateTime(){
    wdt++;
    }
 
  Task taskSendMessage( TASK_MINUTE * 2 , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval (Here interval is 4 second)

 
  // If you want to receive sensor readings from this node, write code in below function....

  void sendMessage() {
   String msg = "online?";                                       // You can write node name/no. here so that you may easily recognize it        
   uint32_t target = 314262534;                                         // Target is Node id of the node where you want to send sms (Here, write node id of mqtt bridge (Root Node))
   mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   Serial.println(msg);
  //Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db
 }

 
  // Msg recived by node. If you want to perform any task by receiving msg, write code in the below function....

  void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
 if (msg=="online")
  {
   digitalWrite(LED_BUILTIN, LOW);  
    // You can write node name/no. here so that you may easily recognize it        
   String msg = readMcp();
   uint32_t target = 314262534;                                         // Target is Node id of the node where you want to send sms (Here, write node id of mqtt bridge (Root Node))
   mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   Serial.println(msg);  // If msg published from mqtt broker is LightON, Turn ON the built in LED of Nodemcu
   wdt = 0;
   }
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

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
   mesh.setDebugMsgTypes( ERROR | DEBUG );  // set before init() so that you can see startup messages

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
    if(wdt == 120 ){
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

 String readMcp(){
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

