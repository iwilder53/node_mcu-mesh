    
  #include "painlessMesh.h"
  #include <MCP3008.h>
  #include <LittleFS.h>
  #include "FS.h"
#include <Arduino.h>
  #include <ModbusMaster.h>


#define relayPin D3

#define MAX485_DE_RE      D4

#define CS_PIN D8
#define CLOCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6

#define sendLed D1    
#define connLed D0

#define CS_PIN D8
#define CLOCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6
//#define   MESH_PREFIX     "HetaDatain"                         // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
//#define   MESH_PASSWORD   "Test@Run_1"                         // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                                      // Mesh Port should be same for all  nodes in Mesh Network

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
ModbusMaster node;


int relay_pin_0_min, relay_pin_0_max,relay_pin_1_min, relay_pin_1_max,relay_pin_2_min, relay_pin_2_max,relay_pin_3_min, relay_pin_3_max,relay_pin_04_min, relay_pin_04_max,relay_pin_05_min, relay_pin_05_max,relay_pin_06_min, relay_pin_06_max,relay_pin_07_min, relay_pin_07_max;                               // Mesh Port should be same for all  nodes in Mesh Network
  int device_type;
  int mfd_dev_id;
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
  int mcp = 0, mfd = 0, pins = 0;
  int first_Reg, second_Reg;
String msgMfd_payload;
String msgMfd_payload1;

  String msgSd;
  uint8_t interval = 1000;
  bool ackStatus;
  // User stub
   void updateTime();
    String readMcp();
   void writeToCard();
      void writeTimeToCard();
  String readMfd();
void preTransmission();
void postTransmission();
bool dataStream();
void cpu_chill();
boolean read_Mfd_Task();

   void sendMessage() ;// Prototype so PlatformIO doesn't complain
  void sendMsgSd();
  void blink_con_led();
  Task taskUpdateTime( TASK_SECOND * 1 , TASK_FOREVER, &updateTime );   // Set task second to send msg in a time interval (Here interval is 4 second)
  
  void updateTime(){
    digitalWrite(connLed, LOW);
    wdt++;
    ts_epoch++;
    }

void preTransmission()
{
  //digitalWrite(MAX485_DE_RE, 1);
  digitalWrite(MAX485_DE_RE, 1);
  delay(40);
}

void postTransmission()
{
 // digitalWrite(MAX485_DE_RE, 0);
  digitalWrite(MAX485_DE_RE, 0);
}

   Task taskConnLed( TASK_SECOND * 1 , TASK_FOREVER, &blink_con_led );
  Task taskSendMessage( TASK_MINUTE * sendDelay , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval (Here interval is 4 second)
  Task taskSendMsgSd( TASK_SECOND * 3 , TASK_FOREVER, &sendMsgSd );   // Set task second to send msg in a time interval
  Task taskWriteToCard( TASK_MINUTE * sendDelay , TASK_FOREVER, &writeToCard );
  Task taskReadMfd( TASK_MINUTE * sendDelay, TASK_FOREVER, &read_Mfd_Task );   // Set task second to send msg in a time interval (Here interval is 4 second)

 
  // If you want to receive sensor readings from this node, write code in below function....

  void sendMessage() {
       digitalWrite(LED_BUILTIN, LOW);  

   if (mesh.isConnected(root)){
  Serial.print("found root, pinging . . . ");
  taskWriteToCard.disable();
   String msg = "online?";                                       // You can write node name/no. here so that you may easily recognize it        
   uint32_t target = root;                                         // Target is Node id of the node where you want to send sms (Here, write node id of mqtt bridge (Root Node))
   mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   Serial.println(msg);}
   else{
     taskWriteToCard.enable();
       Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db

   }
    digitalWrite(sendLed, LOW);  

   } 

 
  // Msg recived by node. If you want to perform any task by receiving msg, write code in the below function....
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
 if (msg=="online")
  {
   digitalWrite(sendLed, HIGH);  
    // You can write node name/no. here so that you may easily recognize it
    if (mcp == 1 && mfd == 1 ){
      String msg = msgMfd_payload;
      mesh.sendSingle(root,msg);
      msg = msgMfd_payload1;
      mesh.sendSingle(root, msg); 
      msg = readMcp();
      mesh.sendSingle(root, msg );                                             // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   wdt = 0;
   ackStatus = true;
   taskWriteToCard.disable();
  // taskIdle.enable();
      
      }        
      else if (mfd == 1 ){
       // taskDataStream.enable();
      
        String msg =  msgMfd_payload;
        mesh.sendSingle(root, msg);
        Serial.println(msg);  // If msg published from mqtt broker is LightON, Turn ON the built in LED of Nodemcu
         msg =  msgMfd_payload1;
        mesh.sendSingle(root, msg);
        Serial.println(msg);
        wdt = 0;
   ackStatus = true;
   taskWriteToCard.disable();

     // taskIdle.enable();

        }
        else if(mcp == 1 ){
  String msg = readMcp();
   mesh.sendSingle(root, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg))
   Serial.println(msg);  // If msg published from mqtt broker is LightON, Turn ON the built in LED of Nodemcu
   wdt = 0;
   ackStatus = true;
   taskWriteToCard.disable();
     // taskIdle.enable();

        }
           digitalWrite(sendLed, LOW);  

   }


   String strMsg = String(msg);
   ts_epoch = strMsg.toInt();
 }

  void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
           String nMap = mesh.asNodeTree().toString();
         mesh.sendSingle(root, nMap);
        
         if(mesh.startDelayMeas(root))
         {  
            taskSendMsgSd.enable();
            taskWriteToCard.disable();
           String configFile = String( id + "," + root + ","  + mcp + ","  + mfd + ","  + pins + ","  + sendDelay);
            mesh.sendSingle(root, configFile);
            }
            taskConnLed.enable();

 }

  void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  Serial.printf("Changed connections\n");
  String nMap = mesh.asNodeTree().toString();
  mesh.sendSingle(root, nMap);
 }

  void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
 }

 void delayReceivedCallback(uint32_t from, int32_t delay) {
   Serial.printf("Delay to node %u is %d us\n", from, delay);
 }

  void setup() {
  pinMode(MAX485_DE_RE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_DE_RE, 0);
  
  Serial.begin(9600, SERIAL_8E1);
  

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

  StaticJsonDocument<600> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
  }
  const char* MESH_PREFIX = doc["ssid"];
  const char* MESH_PASSWORD = doc["password"];
  const char* ID = doc["id"];
  const char* ROOT = doc["root"];
  const char* MCP = doc["mcp"];
  const char* MFD = doc["mfd"];
  const char* PINS = doc["pins"];
  const char* DELAY = doc["delay"];

   
  const char* pin1min = doc["pin1min"];
  const char* pin1max = doc["pin1max"];

relay_pin_0_min = atoi(pin1min);
relay_pin_0_max = atoi(pin1max);
Serial.println(relay_pin_0_min );
Serial.println(relay_pin_0_max );

 
 const char* pin2min = doc["pin2min"];
  const char* pin2max = doc["pin2max"];

relay_pin_1_min = atoi(pin2min);
relay_pin_1_max = atoi(pin2max);
Serial.println(relay_pin_1_min );
Serial.println(relay_pin_1_max );

 
   const char* pin3min = doc["pin3min"];
  const char* pin3max = doc["pin3max"];

relay_pin_2_min = atoi(pin3min);
relay_pin_2_max = atoi(pin3max);
Serial.println(relay_pin_2_min );
Serial.println(relay_pin_2_max );

 
 const char* pin4min = doc["pin4min"];
  const char* pin4max = doc["pin4max"];
  relay_pin_3_min = atoi(pin4min);
relay_pin_3_max = atoi(pin4max);
Serial.println(relay_pin_3_min );
Serial.println(relay_pin_3_max );
 
   const char* pin5min = doc["pin5min"];
  const char* pin5max = doc["pin5max"];
 
relay_pin_04_min = atoi(pin5min);
relay_pin_04_max = atoi(pin5max);
Serial.println(relay_pin_04_min );
Serial.println(relay_pin_04_max );

 const char* pin6min = doc["pin6min"];
  const char* pin6max = doc["pin6max"];

relay_pin_05_min = atoi(pin6min);
relay_pin_05_max = atoi(pin6max);
Serial.println(relay_pin_05_min );
Serial.println(relay_pin_05_max );
 
    const char* pin7max = doc["pin7max"];
   const char* pin7min = doc["pin7min"];


relay_pin_06_min = atoi(pin7min);
relay_pin_06_max = atoi(pin7max);
Serial.println(relay_pin_06_min );
Serial.println(relay_pin_06_max );

 
  const char* pin8max = doc["pin8max"];
 const char* pin8min = doc["pin8min"];

relay_pin_07_min = atoi(pin8min);
relay_pin_07_max = atoi(pin8max);
Serial.println(relay_pin_07_min );
Serial.println(relay_pin_07_max );
   

   
  sendDelay = atoi(DELAY);
  Serial.print("Loaded id: ");
    id = atoi(ID);
  Serial.println(ID);
  Serial.print("Loaded root id: ");
   root = atoi(ROOT);
  Serial.println(ROOT);
  mcp = atoi(MCP);
    Serial.println(mcp);

  mfd = atoi(MFD);
  Serial.println(mfd);
  const char* MFD_SERIAL_ID = doc["mfd_dev_id"];
  mfd_dev_id = atoi(MFD_SERIAL_ID);
  const char* DEV_TYPE = doc["device_type"];
  device_type = atoi(DEV_TYPE);

   pins = atoi(PINS);
  Serial.print("Loaded MCP pins: " );
Serial.print(pins);
  Serial.print(" loaded Send Delay: " );
Serial.print(sendDelay);

File timeFile = LittleFS.open("time.txt", "r");
  if (timeFile) {
     String timeTemp = timeFile.readStringUntil('\n');

    ts_epoch = timeTemp.toInt();
    
    timeFile.close();
    }
  else {
    Serial.println("error opening  time.txt"); 
    }  LittleFS.end();
  



  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
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
    userScheduler.addTask(taskConnLed);
        userScheduler.addTask(taskReadMfd);



 if(mfd == 1)
 {
   taskReadMfd.enable();
  }else{
   taskSendMessage.enable();

  }

   taskUpdateTime.enable();
   pinMode(A0, INPUT);                                                    // Define A0 pin as INPUT
   pinMode(LED_BUILTIN, OUTPUT);                                          // Define LED_BUILTIN as OUTPUT
   digitalWrite(LED_BUILTIN, HIGH);  
    pinMode(sendLed, OUTPUT);   
   pinMode(connLed, OUTPUT);    
     pinMode(relayPin, OUTPUT);  
   digitalWrite(relayPin, LOW);                                    // Initially the LED will be off

 
  node.preTransmission(preTransmission);//mfd callbacks 
  node.postTransmission(postTransmission);
  }

  void loop() {
   // it will run the  scheduler as well

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

  String readMcp()
 {{
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

msgMcp = String(ts_epoch);
if(mfd == 1 ){
msgMcp += "," + id +"C";}
else
{
  msgMcp += "," + id;
}


msgMcp += "," + String(device_type);



if(pins == 1 || pins > 1  ){

msgMcp += "," + String (val); 
trig_Relay(val,relay_pin_0_max, relay_pin_0_min);
}if(pins == 2 || pins > 2  ){

msgMcp += "," + String (val1); 
trig_Relay(val1, relay_pin_1_max, relay_pin_1_min );
}if(pins == 3 || pins > 3  ){
msgMcp += "," + String (val2); 
trig_Relay(val1, relay_pin_2_max, relay_pin_2_min );
}if(pins == 4 || pins > 4  ){

msgMcp += "," + String (val3); 
trig_Relay(val1, relay_pin_3_max, relay_pin_3_min );

}if(pins == 5 || pins > 5  ){

msgMcp += "," + String (val4); 
trig_Relay(val1, relay_pin_04_max, relay_pin_04_min );

}if(pins == 6 || pins > 6  ){

msgMcp += "," + String (val5); 
trig_Relay(val1, relay_pin_05_max, relay_pin_05_min );

}if(pins == 7|| pins > 7 ){

msgMcp += "," + String (val6); 
trig_Relay(val1, relay_pin_06_max, relay_pin_06_min );

}if(pins == 8 || pins > 8  ){

msgMcp += "," + String (val7); 
trig_Relay(val1, relay_pin_07_max, relay_pin_07_min );

    

}return msgMcp;
 }
 }

void trig_Relay(int thres ,int relay_max, int relay_min){
if (thres >= relay_max || thres <= relay_min){
  digitalWrite(relayPin, HIGH);
}

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
    // taskIdle.disable();

{  Serial.print("write to card works");
            taskConnLed.disable();
        digitalWrite(sendLed, HIGH);

     if (ackStatus == false)
     {

        String msg, msg_1, msg_2;
        if (mfd == 1  && mcp == 1){

          msg = msgMfd_payload;
          msg_1 = msgMfd_payload1;
          msg_2 = readMcp();
        }
       if (mfd == 1 ){
       
         msg =  msgMfd_payload;
         msg_1 = msgMfd_payload1;
       
      
       }
          if (mcp == 1 )
          {msg += readMcp();}
             Serial.println(msg);
             

      LittleFS.begin();
    File dataFile = LittleFS.open("offlinelog.txt", "a");
    if(dataFile.size() > 4e+6){
        Serial.print("memory full");
        digitalWrite(sendLed, LOW);

    }
// if the file is available, write to it:
  if (dataFile.size() < 4e+6) {
    if(mfd == 1 && mcp == 0 ){
      dataFile.println(msg);
      dataFile.println(msg_1);

    }
        else if(mfd == 1 && mcp == 1){
        dataFile.println(msg);
        dataFile.println(msg_1);
        dataFile.println(msg_2);
    }
else{
    dataFile.println(msg);
    }

    dataFile.close();
    // print to the serial port too:
     Serial.println("to SD Card"); 
    Serial.println(msg);
    Serial.println(msg_1);
    Serial.println(msg_2);
    Serial.println("WiFi signal: " + String(WiFi.RSSI()) + "db");}
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening  offlinelog.txt"); 
    }
    timeIndex++;
    wdt = 0;
    ackStatus = false;

    LittleFS.end();
        
        }
                digitalWrite(sendLed, LOW);
                  // taskIdle.enable();


    }

}

void writeTimeToCard()
{
      LittleFS.begin();
      LittleFS.remove("time.txt");
    File dataFile = LittleFS.open("time.txt", "w");
  if (dataFile) {
    dataFile.println(ts_epoch);
    dataFile.close();
    }
  else {
    Serial.println("error opening  time.txt"); 
    }  LittleFS.end();
}

void blink_con_led(){
  digitalWrite(connLed, HIGH);
}

int bin2dec(String sb)
{
 int rem, dec_val=0, base=1, expo=0, temp=0, i=0;
 for (i=sb.length()-1; i>=0; i--)
 {
  if(sb[i]=='0')
  temp=0;
  else
  {
   if(sb[i]=='1')
   temp=1;
  }
dec_val = dec_val + temp*base;
 base = base*2;
 }
return dec_val;
}

String dec2binary(int x)
{
 int num = x;
 uint8_t bitsCount = 16;
 char str[bitsCount + 1];
 uint8_t i = 0;
 while(bitsCount--)
   str[i++] = bitRead(num, bitsCount ) + '0';
   str[i] = '\0';
Serial.println(str[i]);
return str;
}

double RSmeter(int one, int two)
{
 int SIGN, EXPONENT;
 float division;
 char first[8], second1[8];
 char third[8], last[8];
 int i, j, d3, z;

 String hexa0 = String(two, HEX);
 String bin0 = dec2binary(int (two));

 String hexa1 = String(one, HEX);
 String bin1 = dec2binary(int (one));

 for (i=0; i<=7; i++)
 {
  first[i] = bin0[i];
 }

int aaaa = first[0];

if(aaaa != '1')
 {
  SIGN = 1;
 }
else
 {
  SIGN = -1;
 }

 for (j=8; j<=15; j++)
 {
  second1[j] = bin0[j];
 }
 for (int d3=0; d3<=7; d3++)
 {
  third[d3] = bin1[d3];
 }
 for (int z=8; z<=15; z++)
 {
  last[z] = bin1[z];
 }

String a = String (bin0[1]);
String b = String (bin0[2]);
String c = String (bin0[3]);
String d = String (bin0[4]);
String e = String (bin0[5]);
String f = String (bin0[6]);
String g = String (bin0[7]);
String h = String (bin0[8]);
String step1 = String((a)+(b)+(c)+(d)+(e)+(f)+(g)+(h));

String j2 = String (bin0[9]);
String k2 = String (bin0[10]);
String l2 = String (bin0[11]);
String m2 = String (bin0[12]);
String n2 = String (bin0[13]);
String o2 = String (bin0[14]);
String p2 = String (bin0[15]);
String step2 = String((j2)+(k2)+(l2)+(m2)+(n2)+(o2)+(p2));

  int decimal = bin2dec(String(step1));

  EXPONENT = decimal-127;

String a1 = String (bin1[0]);
String b1 = String (bin1[1]);
String c1 = String (bin1[2]);
String d1 = String (bin1[3]);
String e1 = String (bin1[4]);
String f1 = String (bin1[5]);
String g1 = String (bin1[6]);
String h1 = String (bin1[7]);
String step3= String((a1)+(b1)+(c1)+(d1)+(e1)+(f1)+(g1)+(h1));

String x2 = String (bin1[8]);
String q2 = String (bin1[9]);
String r2 = String (bin1[10]);
String s2 = String (bin1[11]);
String t2 = String (bin1[12]);
String u2 = String (bin1[13]);
String v2 = String (bin1[14]);
String w2 = String (bin1[15]);
String step4 = String((x2)+(q2)+(r2)+(s2)+(t2)+(u2)+(v2)+(w2));

String combined = String((step2)+(step3)+(step4));

float dec1 = bin2dec(String(step2));
float dec2 = bin2dec(String(step3));
float dec3 = bin2dec(String(step4));
float sum = ((dec1)*256*256)+((dec2)*256)+(dec3);

if(EXPONENT == 0)
{
 division = (sum/4194304);  
}
else
{
 division = (sum /8388608);
}

float MANTISSA = division+1;
double Power = pow(2, EXPONENT);
double FLOAT = SIGN*MANTISSA*Power;


 return (FLOAT);
}

int validDenominator(int number)
{
  if(number == 0 || isnan(number))
    return 1;
   else
   return number;
}

double readWattageR(int add){

    //dataStream(add );
    if(dataStream(add) == true){
      double Wattage = NAN;
      Wattage = RSmeter(first_Reg, second_Reg);
    double WATT = Wattage;
    return WATT;}
    
  }
bool dataStream(int one ){
  first_Reg = 0 ;
  second_Reg = 0 ;
      node.clearResponseBuffer();
        delay(60);
  int   result =  node.readHoldingRegisters(one, 2 );        //Don't change this EVER!!
    if (result == node.ku8MBSuccess){ 
    first_Reg =node.getResponseBuffer(0);
    second_Reg =node.getResponseBuffer(1);
      return true;
    }
    else{
      node.clearResponseBuffer();  
      delay(60);
      result =  node.readHoldingRegisters(one, 2 );//Don't change this EVER!!
    if (result == node.ku8MBSuccess){ 

      first_Reg =node.getResponseBuffer(0);
      second_Reg =node.getResponseBuffer(1);
    }else{
      node.clearResponseBuffer();  
      delay(60);
      result =  node.readHoldingRegisters(one, 2 );//Don't change this EVER!!
    if (result == node.ku8MBSuccess){ 

      first_Reg =node.getResponseBuffer(0);
      second_Reg =node.getResponseBuffer(1);
    }
      }
      return true;
   }

}

String readMfd(){
    node.begin(mfd_dev_id, Serial);
  String msgMfd; 


  msgMfd = String(ts_epoch);
  msgMfd += "," + id +"A";
  msgMfd += "," + String(device_type);

  if (dataStream() == true){
    msgMfd += "," + String(readWattageR(100)); 
    msgMfd += "," + String(readWattageR(102));//WR
    msgMfd += "," + String(readWattageR(104));//WB
    msgMfd +=  "," + String(readWattageR(106));//Iavg
    msgMfd += "," + String(readWattageR( 108)); 
    msgMfd += "," + String(readWattageR(110)); 
    msgMfd += "," + String(readWattageR(112)); 
    msgMfd += "," + String(readWattageR(114)); 
    msgMfd += "," + String(readWattageR(116));
    msgMfd += "," + String(readWattageR(118)); 
    msgMfd += "," + String(readWattageR(120)); 
    msgMfd += "," + String(readWattageR(122)); 
    msgMfd += "," + String(readWattageR(124)); 
    msgMfd += "," + String(readWattageR(126)); 
    msgMfd += "," + String(readWattageR(128)); 
    msgMfd += "," + String(readWattageR(130)); 
    msgMfd += "," + String(readWattageR(132)); 
    msgMfd += "," + String(readWattageR(134)); 
    msgMfd += "," + String(readWattageR(136)); 
    msgMfd += "," + String(readWattageR(138)); 
    msgMfd += "," + String(readWattageR(140)); 
    msgMfd += "," + String(readWattageR(142)); 
                     
   // msgMfd += readMfd2();
    
                   

   }
   //msgMfd += "," + String(readRegister());//VAH
   //  node.clearResponseBuffer();

  return msgMfd;
}
String readMfd2(){
String msgMfd2 = String(ts_epoch) + "," + id+"B" + "," + String(device_type);
             
    msgMfd2 += "," + String(readWattageR(144)); 
    msgMfd2 += "," + String(readWattageR(146)); 
    msgMfd2 += "," + String(readWattageR(148)); 
    msgMfd2 += "," + String(readWattageR(150)); 
    msgMfd2 += "," + String(readWattageR(152));
    msgMfd2 += "," + String(readWattageR(154));
    msgMfd2 += "," + String(readWattageR(156));
    msgMfd2 += "," + String(readWattageR(158));
    msgMfd2 += "," + String(readWattageR(160));
    msgMfd2 += "," + String(readWattageR(162));
    msgMfd2 += "," + String(readWattageR(164));
    msgMfd2 += "," + String(readWattageR(174));
    msgMfd2 += "," + String(readWattageR(176));
    msgMfd2 += "," + String(readWattageR(182));
    msgMfd2 += "," + String(readWattageR(184));
    msgMfd2 += "," + String(readWattageR(186));
    msgMfd2 += "," + String(readWattageR(188));
    msgMfd2 += "," + String(readWattageR(190));
    msgMfd2 += "," + String(readWattageR(192));
    msgMfd2 += "," + String(readWattageR(194));
    msgMfd2 += "," + String(readWattageR(196));
  
  return msgMfd2;

}

bool dataStream(){                                        // don't remove this  its needed to shut linker up
  int i =0 ;
   //while(i<160)
  int j = 0;
  int a[10],b[10];
    int result =  node.readHoldingRegisters(i, 2);
     a[j] =node.getResponseBuffer(0);
     b[j] =node.getResponseBuffer(1);
    i=i+2;
    j++;
    if (i = 160 ){
       i = 100;
        j = 0;
      //taskDataStream.disable();
      return true;
      }
}
boolean read_Mfd_Task()
{
  msgMfd_payload = readMfd();
  msgMfd_payload1 = readMfd2();
  Serial.println(msgMfd_payload);
  sendMessage();
  return true;
}
