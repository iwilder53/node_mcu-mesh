#include <SD.h>                                           // remove the comment to access SD card
#include "painlessMesh.h"                                   // Painless Mesh Lib
#include <SPI.h>                                            // Access to SPI Lib

#define   MESH_PREFIX     "HetaDatain"                         // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
#define   MESH_PASSWORD   "Test@Run_1"                         // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                                 // Mesh Port should be same for all  nodes in Mesh Network 




// constants won't change. Used here to set a pin number:
const int ledPin =  D3;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A0;  // Analog input pin that the sensor is attached to
const int analogOutPin = 9; // Analog output pin 

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

// function declaration
void postFileContent(const char * path );

#define FILE_LINE_LENGTH        81  // a line has 80 chars 
char txtLine[FILE_LINE_LENGTH];
char postdata [FILE_LINE_LENGTH];
bool readCondition = true;  // Has to be defined somewhere to trigger SD read
String buffer;

Scheduler userScheduler; // to control your personal task      // scheduler as delay can not be used in painlessmesh
painlessMesh  mesh;

unsigned long period=0;  

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void writeToCard() ;  //remove the comment for SD card 
void loggedData();
void manageTasks();

Task taskSendMessage( TASK_SECOND * 4 , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval (Here interval is 4 second)

// If you want to receive sensor readings from this node, write code in below function....

void sendMessage() {

  String msg = " NODE no.3  ";                            // You can write node name/no. here so that you may easily recognize it  
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 100, 190, 0, 110);
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);
  msg +=  " Sensor: " + String (sensorValue);
  msg +=  " PSI: " + String (outputValue);
  msg += mesh.getNodeId();                                              // Adding Node id in the msg
   
 // msg += " myFreeMemory: " + String(ESP.getFreeHeap());                 // Adding free memory of Nodemcu in the msg
// msg += "from end node1"; 
  uint32_t target = 314262122; 
   mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
  Serial.println(msg);
  Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");
  
  } 

 Task taskWriteToCard( TASK_SECOND * 4 , TASK_FOREVER, &writeToCard );

 void writeToCard(){ 

  String msg = " NODE no.5  "; 
 // read the analog in value:
    sensorValue = analogRead(analogInPin);
//  map it to the range of the analog out:
    outputValue = map(sensorValue, 100, 190, 0, 110);
//  change the analog out value:
  analogWrite(analogOutPin, outputValue);  
  msg +=  " Sensor: " + String (sensorValue);
  msg +=  " PSI: " + String (outputValue);
  File dataFile = SD.open("offlinelog.txt", FILE_WRITE);
 // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(msg);
      dataFile.close();
  //  print to the serial port too:
    Serial.println("to SD Card"); 
    Serial.println(msg);
    Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");}
 //  if the file isnt open, pop up an error:
    else {
   Serial.println("error opening datalog.txt"); 
    }
  } 

 Task taskLoggedData(TASK_SECOND * 30 , TASK_FOREVER, &loggedData );

void loggedData(){
  
  }

 Task taskManageTasks( TASK_SECOND * 4 , TASK_FOREVER, &manageTasks );

void manageTasks(){
  
   if( (WiFi.RSSI()) == 31 ){
     taskWriteToCard.enable();
      digitalWrite(D2,HIGH);                              // LED will be ON when node is writibng to sd card                                
     }  else {

           taskWriteToCard.disable();
            taskLoggedData.enable();
            digitalWrite(D2,LOW);
      }
  }
  
void receivedCallback( uint32_t from, String &msg ) {
 Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
 
}

void newConnectionCallback(uint32_t nodeId) {
   Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
   //File file = SD.open(path);
//#elif defined(ESP8266)
  File file = SD.open("offlinelog.txt", FILE_READ); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (!file) {
    Serial.println("Failed to open file for reading");
    
    return;
  }

    // String logs;
   String buffer;
  uint8_t i = 0;

 //  while (file.available())
for (int i = 0; i < 20 ; i++)   { 
    buffer = file.readStringUntil('\n');
   // Serial.println(buffer); //Printing for debugging purpose         
     
String msg  = " from sd";
      msg += buffer; 
     // msg += " loggenddata ";
      uint32_t target = 314262122; 
      mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
      
      Serial.println(msg);
   }   
  file.close();
  Serial.println(F("DONE Reading"));
//SD.remove(offlinelog.txt"); 
   //file = SD.open("offlinelog.txt", FILE_WRITE);                          //deleting file after data is sent
   //file.close();
}

void changedConnectionCallback() {
 Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  pinMode(D1, OUTPUT);
  
    //CHECK FOR SDCARD
    if (!SD.begin(D8)) {
    
   Serial.println("Card failed, or not present");
     // don't do anything more:
     //while (1);
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
  userScheduler.addTask(taskManageTasks);

  taskSendMessage.enable();
  taskLoggedData.enable();
  taskManageTasks.enable();
  
  pinMode(A0,INPUT);                                                    // Thermistor input
  pinMode(D1,HIGH);                                                      // LED ON for Power OutPut
  pinMode(D2,OUTPUT);
  pinMode(D3, OUTPUT);
}

void loop() {
  
  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

//   it will run the user scheduler as well

period=millis()/1000;                                                    // Function "mllis()" gives time in milliseconds. Here "period" will store time in seconds
 
  mesh.update();
// Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db
  
 if( (WiFi.RSSI()) == 31 ){
    taskWriteToCard.enable();
    digitalWrite(LED_BUILTIN,LOW);                              // LED will be ON when node is writibng to sd card                                
} 

  /*if (period>60)                                                         // When period will be > 60 seconds, deep sleep mode will be active
  {
     mesh.stop();
 ESP.deepSleep(300e6);                                                    // deepSleep mode will be active for 300*10^6 microseconds, i.e. for 300 seconds                                                   
 digitalWrite(LED_BUILTIN,HIGH); 
  }*/
if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(D3, ledState);
  }
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}
