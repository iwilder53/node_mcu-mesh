
    
#include "painlessMesh.h"
#include <MCP3008.h>
#include <LittleFS.h>
#include "FS.h"
#include <Arduino.h>
#include <ModbusMaster.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "DHT.h"
#include<Arduino.h>

#define DHTPIN D3
#define DHTTYPE DHT22 

#define relayPin D3

#define MAX485_DE_RE      D4

#define sendLed D8
#define connLed D0

#define CS_PIN D8
#define CLOCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6

#define   MESH_PORT       5555                                      // Mesh Port should be same for all  nodes in Mesh Network

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
ModbusMaster node;


int rebootTime;
//int mcp_relay_trigs[8][8];
uint8_t mfd_read_pos = 0;
int relay_pin_0_min, relay_pin_0_max,relay_pin_1_min, relay_pin_1_max,relay_pin_2_min, relay_pin_2_max,relay_pin_3_min, relay_pin_3_max,relay_pin_04_min, relay_pin_04_max,relay_pin_05_min, relay_pin_05_max,relay_pin_06_min, relay_pin_06_max,relay_pin_07_min, relay_pin_07_max;                               // Mesh Port should be same for all  nodes in Mesh Network
  int device_count;
  int mfd_dev_id[5];
  uint8_t sendDelay = 2;
  unsigned long period=0; 
  //uint16_t val=0, val1=0, val2=0, val3=0, val4=0, val5=0, val6=0, val7=0 ;
  String id; 
  int wdt = 0; 
  int ts_epoch;
  int timeIndex;
  int pos;
  uint32_t root;
  long previousMillis = 0;  
  int mcp = 0, mfd = 0, pins = 0, smb =0;
  int first_Reg, second_Reg;
  int time_to_print;
  bool MCP_Sent = false;

String msgMfd_payload;
String msgMfd_payload1;

  String msgSd;
  uint8_t interval = 1000;
  bool ackStatus;
  // User stub
void updateTime();
void mbe();

void readMcp();
void writeToCard();
void writeTimeToCard();
String readMfd();
void preTransmission();
void postTransmission();
bool dataStream(int one );
void cpu_chill();
boolean read_Mfd_Task();
void sendMessage() ;// Prototype so PlatformIO doesn't complain
void sendMsgSd();
void blink_con_led();
void sendPayload( String payload);
void saveToCard();
void sendMFD();
void multi_mfd_read();
void trig_Relay(int thres ,int relay_max, int relay_min)

Task taskUpdateTime( TASK_SECOND * 1 , TASK_FOREVER, &updateTime );   // Set task second to send msg in a time interval (Here interval is 4 second)
  
void updateTime(){
    digitalWrite(connLed, LOW);
    wdt++;
    ts_epoch++;
    rebootTime++;
  }

void preTransmission(){
   digitalWrite(MAX485_DE_RE, 1);
  // delay(40);
  }

void postTransmission(){
   digitalWrite(MAX485_DE_RE, 0);
  }

  Task taskConnLed( TASK_SECOND * 1 , TASK_FOREVER, &blink_con_led );
  Task taskSendMessage( TASK_MINUTE * sendDelay , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval (Here interval is 4 second)
  Task taskSendMsgSd( TASK_SECOND * 3 , TASK_FOREVER, &sendMsgSd );   // Set task second to send msg in a time interval
  Task task_Multi_Mfd_Read(TASK_SECOND * 10, TASK_FOREVER, &multi_mfd_read );
  Task taskReadMBE( TASK_MINUTE * sendDelay , TASK_FOREVER, &mbe );
  Task taskReadMcp( TASK_MINUTE * sendDelay , TASK_FOREVER, &readMcp );
  Task taskReadMfd( TASK_MINUTE * sendDelay, TASK_FOREVER, &read_Mfd_Task );   // Set task second to send msg in a time interval (Here interval is 4 second)


  void sendMessage() {
   digitalWrite(sendLed, HIGH);  
  }
 
  // Msg recived by node. If you want to perform any task by receiving msg, write code in the below function....
void receivedCallback( uint32_t from, String &msg ) {
  Serial1.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  
  
   String strMsg = String(msg);
   ts_epoch = strMsg.toInt();
 
  }

  void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
           String nMap = mesh.asNodeTree().toString();
         mesh.sendSingle(root, nMap);
      if(mesh.startDelayMeas(root))
         { taskSendMsgSd.enable();
          String configFile = String( id + "," + String(root) + ","  + String(mcp) + ","  + String(mfd) + ","  + String(pins) + ","  + String(sendDelay));
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
    Serial1.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
 }

 void delayReceivedCallback(uint32_t from, int32_t delay) {
   Serial1.printf("Delay to node %u is %d us\n", from, delay);
 }

  void setup() {

    Serial.begin(115200);
  pinMode(MAX485_DE_RE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_DE_RE, 0);
  
  

  LittleFS.begin();

  File configFile = LittleFS.open("config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial1.println("Config file size is too large");
  }
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<1024> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial1.println("Failed to parse config file");
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
Serial1.println(relay_pin_0_min );
Serial1.println(relay_pin_0_max );

 
const char* pin2min = doc["pin2min"];
const char* pin2max = doc["pin2max"];

relay_pin_1_min = atoi(pin2min);
relay_pin_1_max = atoi(pin2max);
Serial1.println(relay_pin_1_min );
Serial1.println(relay_pin_1_max );

 
const char* pin3min = doc["pin3min"];
const char* pin3max = doc["pin3max"];

relay_pin_2_min = atoi(pin3min);
relay_pin_2_max = atoi(pin3max);
Serial1.println(relay_pin_2_min );
Serial1.println(relay_pin_2_max );

 
const char* pin4min = doc["pin4min"];
const char* pin4max = doc["pin4max"];
relay_pin_3_min = atoi(pin4min);
relay_pin_3_max = atoi(pin4max);
Serial1.println(relay_pin_3_min );
Serial1.println(relay_pin_3_max );
 
const char* pin5min = doc["pin5min"];
const char* pin5max = doc["pin5max"];
 
relay_pin_04_min = atoi(pin5min);
relay_pin_04_max = atoi(pin5max);
Serial1.println(relay_pin_04_min );
Serial1.println(relay_pin_04_max );

const char* pin6min = doc["pin6min"];
const char* pin6max = doc["pin6max"];

relay_pin_05_min = atoi(pin6min);
relay_pin_05_max = atoi(pin6max);
Serial1.println(relay_pin_05_min );
Serial1.println(relay_pin_05_max );
 
const char* pin7max = doc["pin7max"];
const char* pin7min = doc["pin7min"];


relay_pin_06_min = atoi(pin7min);
relay_pin_06_max = atoi(pin7max);
Serial1.println(relay_pin_06_min );
Serial1.println(relay_pin_06_max );

 
const char* pin8max = doc["pin8max"];
const char* pin8min = doc["pin8min"];

relay_pin_07_min = atoi(pin8min);
relay_pin_07_max = atoi(pin8max);
Serial1.println(relay_pin_07_min );
Serial1.println(relay_pin_07_max );
   

   
  sendDelay = atoi(DELAY);
  Serial1.print("Loaded id: ");
    id = atoi(ID);
  Serial1.println(ID);
  Serial1.print("Loaded root id: ");
   root = atoi(ROOT);
  Serial1.println(ROOT);
  mcp = atoi(MCP);
    Serial1.println(mcp);

  mfd = atoi(MFD);
  Serial1.println(mfd);

  const char* DEV_COUNT = doc["device_count"];
  device_count = atoi(DEV_COUNT);
  Serial.println(device_count);


  const char* MFD_SERIAL_ID_1 = doc["mfd_dev_id_1"];
  mfd_dev_id[0] = atoi(MFD_SERIAL_ID_1);
  Serial.println(mfd_dev_id[0]);


  const char* MFD_SERIAL_ID_2 = doc["mfd_dev_id_2"];
  mfd_dev_id[1] = atoi(MFD_SERIAL_ID_2); 
  Serial.println(mfd_dev_id[1]);

  const char* MFD_SERIAL_ID_3 = doc["mfd_dev_id_3"];
  mfd_dev_id[2] = atoi(MFD_SERIAL_ID_3);
  Serial.println(mfd_dev_id[2]);

  const char* MFD_SERIAL_ID_4 = doc["mfd_dev_id_4"];
  mfd_dev_id[3] = atoi(MFD_SERIAL_ID_4);
  Serial.println(mfd_dev_id[3]);

  const char* MFD_SERIAL_ID_5 = doc["mfd_dev_id_5"];
  mfd_dev_id[4] = atoi(MFD_SERIAL_ID_5);
  Serial.println(mfd_dev_id[4]);
  
  const char*  SMB = doc["sensor"];
  smb= atoi(SMB);


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

  userScheduler.addTask(taskSendMessage);
  userScheduler.addTask(taskUpdateTime);
  userScheduler.addTask(taskSendMsgSd);
  userScheduler.addTask(taskConnLed);
  userScheduler.addTask(taskReadMfd);
  userScheduler.addTask(task_Multi_Mfd_Read);
  userScheduler.addTask(taskReadMBE);
  userScheduler.addTask(taskReadMcp);

if (mcp == 1){
  Serial.print("mcp activated");
   taskReadMcp.enable();
}
if(smb == 1)
 {
   taskReadMBE.enable();    
   if (!bme.begin(0x76)) {
		Serial.println("Could not find a valid BME280 sensor, check wiring!");
     }
     bme.MODE_FORCED;
 }
  if(mfd == 1)
 {
   taskReadMfd.enable();
  }


   taskUpdateTime.enable();
   pinMode(A0, INPUT);                                                                   // Define A0 pin as INPUT
   pinMode(LED_BUILTIN, OUTPUT);                                                         // Define LED_BUILTIN as OUTPUT
   digitalWrite(LED_BUILTIN, HIGH);         
   pinMode(sendLed, OUTPUT);   
   pinMode(connLed, OUTPUT);    
   pinMode(relayPin, OUTPUT);  
   digitalWrite(relayPin, LOW);                                                         // Initially the LED will be off

 
  node.preTransmission(preTransmission);                                                //mfd callbacks 
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
      if(rebootTime == 86400){
        ESP.reset();
      }

   /* if (period>60)                                                         // When period will be > 60 seconds, deep sleep mode will be active
   {
      mesh.stop();
  ESP.deepSleep(300e6);                                                    // deepSleep mode will be active for 300*10^6 microseconds, i.e. for 300 seconds                                                         
  digitalWrite(LED_BUILTIN,HIGH); 
   }*/
 }

  void readMcp()
 {

   MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  int mcpVals[pins];
   String msgMcp;

  for (int i = 0; i < 8; i++){
    mcpVals[i] = 0;
  }
   for(int i = 0; i < 10; i++){

     for(int j = 0; j < 8; j++){

         mcpVals[j] += adc.readADC(j);

    }   delay(400);
}

for(int j = 0; j < 8; j++){

        int k = mcpVals[j];
        mcpVals[j] = k / 10 ;

}
 
  msgMcp +=  String(ts_epoch) + String(",") +String(id.c_str());

msgMcp.concat(",");
msgMcp.concat("7");

if(pins == 1 || pins > 1  ){

msgMcp.concat(",");
msgMcp.concat(mcpVals[0]); 
trig_Relay(mcpVals[0],relay_pin_0_max, relay_pin_0_min);
}
if(pins == 2 || pins > 2  ){

msgMcp += String(",") + String (mcpVals[1]); 
trig_Relay(mcpVals[1], relay_pin_1_max, relay_pin_1_min );

}if(pins == 3 || pins > 3  ){
msgMcp += String(",") + String (mcpVals[2]); 
trig_Relay(mcpVals[2], relay_pin_2_max, relay_pin_2_min );
}if(pins == 4 || pins > 4  ){

msgMcp += String(",") + String (mcpVals[3]); 
trig_Relay(mcpVals[3], relay_pin_3_max, relay_pin_3_min );

}if(pins == 5 || pins > 5  ){

msgMcp += String(",") + String (mcpVals[4]); 
trig_Relay(mcpVals[4], relay_pin_04_max, relay_pin_04_min );

}if(pins == 6 || pins > 6  ){

msgMcp += String(",") + String (mcpVals[5]); 
trig_Relay(mcpVals[5], relay_pin_05_max, relay_pin_05_min );

}if(pins == 7|| pins > 7 ){

msgMcp += String(",") + String (mcpVals[6]); 
trig_Relay(mcpVals[6], relay_pin_06_max, relay_pin_06_min );

}if(pins == 8 || pins > 8  ){

msgMcp += String(",") + String(mcpVals[7]); 
trig_Relay(mcpVals[7], relay_pin_07_max, relay_pin_07_min );
}
sendPayload(msgMcp);
 }

 

void trig_Relay(int thres ,int relay_max, int relay_min)
{
 if (thres >= relay_max || thres <= relay_min)
  {
    digitalWrite(relayPin, HIGH);
  } 
 else
  {
    digitalWrite(relayPin, HIGH);
  }

}



void sendMsgSd(){

LittleFS.begin();

  File file = LittleFS.open("offlinelog.txt","r"); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (!file) {
    Serial1.println("Failed to open file for reading");
    taskSendMsgSd.disable();
          pos = 0;
          timeIndex = 0;
             return;
  }
    // String logs;
    String buffer ;
 for (int i = 0; i < 1 ; i++) 
{ 
      file.seek(pos);
     buffer = file.readStringUntil('\n');
   // Serial1.println(buffer); //Printing for debugging purpose         
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
    Serial1.println("error opening  time.txt"); 
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
Serial1.println(str[i]);
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
      delay(40);
  int   result =  node.readHoldingRegisters(one, 2 ); 
          delay(40);
       //Don't change this EVER!!
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

String readMfd( int mfd_dev_id){

    node.begin(mfd_dev_id, Serial);
  String msgMfd; 


  msgMfd = String(time_to_print);
  msgMfd += "," + id ;
  msgMfd += "," + String(2);
  msgMfd += "," + String(mfd_dev_id);
  msgMfd += "," + String(readWattageR(100)); 
  msgMfd += "," + String(readWattageR(102));
  msgMfd += "," + String(readWattageR(104));
  msgMfd += "," + String(readWattageR(106));
  msgMfd += "," + String(readWattageR(108)); 
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
  msgMfd += "," + String(readWattageR(144));  
  msgMfd += "," + String(readWattageR(146));  

  return msgMfd;
  }
String readMfd2(int mfd_dev_id){

String msgMfd2 = String(time_to_print) + "," + id + "," + String(2);
       msgMfd2 += "," + String(mfd_dev_id);
 
    msgMfd2 += "," + String(readWattageR(148)); 
    msgMfd2 += "," + String(readWattageR(150)); 
    msgMfd2 += "," + String(readWattageR(152));
    msgMfd2 += "," + String(readWattageR(154));
    msgMfd2 += "," + String(readWattageR(156));
    msgMfd2 += "," + String(readWattageR(158));
    msgMfd2 += "," + String(readWattageR(160));
    msgMfd2 += "," + String(readWattageR(162));
    msgMfd2 += "," + String(readWattageR(164));
    msgMfd2 += "," + String(readWattageR(166));
    msgMfd2 += "," + String(readWattageR(168));
    msgMfd2 += "," + String(readWattageR(170));
    msgMfd2 += "," + String(readWattageR(172));
    msgMfd2 += "," + String(readWattageR(174));
    msgMfd2 += "," + String(readWattageR(176));
    msgMfd2 += "," + String(readWattageR(178));
    msgMfd2 += "," + String(readWattageR(180));
    msgMfd2 += "," + String(readWattageR(182));
    msgMfd2 += "," + String(readWattageR(184));
    msgMfd2 += "," + String(readWattageR(186));
    msgMfd2 += "," + String(readWattageR(188));
    msgMfd2 += "," + String(readWattageR(190));
    msgMfd2 += "," + String(readWattageR(192));
    msgMfd2 += "," + String(readWattageR(194));
    msgMfd2 += "," + String(readWattageR(196));
  
  return msgMfd2;
 // sendPayload(msgMfd2);


}

boolean read_Mfd_Task()
{
  MCP_Sent = false;

task_Multi_Mfd_Read.enable();
  time_to_print = ts_epoch;

  return true;
}

void multi_mfd_read(){
  time_to_print++;
  Serial.end();
  Serial.begin(9600, SERIAL_8E1);
  msgMfd_payload = readMfd(mfd_dev_id[mfd_read_pos]);
  msgMfd_payload1 = readMfd2(mfd_dev_id[mfd_read_pos]);
  sendMFD();

  Serial.end();

  mfd_read_pos++;
  if (mfd_read_pos == device_count){
    task_Multi_Mfd_Read.disable();
    mfd_read_pos = 0;
    Serial.begin(115200);
  }
}

 void mbe ()
 { 
   DHT dht(DHTPIN, DHTTYPE);
   dht.begin();
   bme.takeForcedMeasurement();
   String readMbe;
       //String(ts_epoch) + "," + String(id.c_str() )+ "," + String(6)+ "," + String(bme.readTemperature())+String(",")+String(bme.readHumidity())+String(",")+String(bme.readPressure()) + "," + String(dht.readTemperature())+ "," +String(dht.readHumidity());
 readMbe.concat(String(ts_epoch));
 readMbe.concat(",");
 readMbe.concat(id);
 readMbe.concat(",");
 readMbe.concat("6");
 readMbe.concat(",");
 readMbe.concat(String(bme.readTemperature()));
 readMbe.concat(",");
 readMbe.concat(String(bme.readHumidity()));
 readMbe.concat(",");
 readMbe.concat(String(bme.readPressure()));
 readMbe.concat(",");
 readMbe.concat(String(dht.readTemperature()));
 readMbe.concat(",");
 readMbe.concat(String(dht.readHumidity()));


  
  sendPayload(readMbe);
 }


 void sendMFD(){

digitalWrite(sendLed, HIGH);  
sendPayload(msgMfd_payload);
sendPayload(msgMfd_payload1);
}

void saveToCard( String payload){

    LittleFS.begin();
    File dataFile = LittleFS.open("offlinelog.txt","a");
    dataFile.println(payload);
    dataFile.close();
    LittleFS.end();

}
void sendPayload( String payload){
  Serial.print(payload);
if (mesh.isConnected(root)){
    digitalWrite(sendLed, HIGH);  

   mesh.sendSingle(root,String(payload));
 }else{
saveToCard(payload);

 }
   wdt = 0;

}




String mcp_payload(int pin_num){
if(pins == (pin_num+1) || pins > (pin_num+1)  ){
//msgMcp += "," + String (mcpVals[pin_num]); 
} }
