
#include <ModbusMaster.h>
#include <SD.h> 
#include "painlessMesh.h"
#include <SPI.h>
#include <MCP3008.h>
  
//define pin connections
#define CS_PIN D8
#define CLOCK_PIN D5
#define MOSI_PIN D7
#define MISO_PIN D6
#define sd_PIN D4

int fileCheck;
long pos;
String   msgSd;
String   mesh_ssid    ;                     // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
String   mesh_Pass        ;                   // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                                      // Mesh Port should be same for all  nodes in Mesh Network 
#define FILE_LINE_LENGTH        81  // a line has 80 chars 


int wdt = 0;
int sendDelay = 2;
// function declaration
int ts_epoch;
char txtLine[FILE_LINE_LENGTH];
char postdata [FILE_LINE_LENGTH];
bool readCondition = true;  // Has to be defined somewhere to trigger SD read
String buffer;
int a[70],b[70]; // We can change these to 16 bit to theorotically halve the memory space
uint32_t root = 314262534; 
unsigned long currentMillis = millis();
long interval = 1000;   
unsigned long period=0;  

String id ;
String msgMfd;
String msgMcp;


int LED = D0;
int ledState = LOW;

uint16_t val, val1, val2, val3, val4, val5, val6, val7 ;

String formattedDate;
String dayStamp;
String timeStamp;



bool ackStatus = false ;

uint8_t mfd;
uint8_t mcp ;

long timeIndex = 0;

ModbusMaster node;
Scheduler userScheduler;
painlessMesh  mesh;

// User stub
void sendMfd() ; 
void writeToCard();
void postFileContent(const char * path );
void loggedData();
void manageTasks();
void sendMfd();
void sendMcp();
double readWattage();
double readWattageY();
double readWattageB();
double readWattageR();
double readCurrent();
double readPf();
double readLineVoltage();
double readWH();
double readVa();
double readVah();
String readMfd();
String readMcp();
void updateTime();

Task taskSendMfd( TASK_MINUTE * sendDelay , TASK_FOREVER, &sendMfd );   // Set task second to send msg in a time interval

Task taskSendMsgSd( TASK_SECOND * 5 , TASK_FOREVER, &sendMsgSd );   // Set task second to send msg in a time interval


void sendMfd() {

  ledState = HIGH;

  //readMfd();

  mesh.sendSingle(root, String("online?"));     

  ackStatus = false;
  long previousMillis = 0;  
  unsigned long currentMillis = millis();

if(currentMillis - previousMillis > interval) {
   // previousMillis = currentMillis;   
 while (ackStatus = false)
 {
    writeToCard();
  }
  }
 node.clearResponseBuffer();
   } 


Task taskSendMcp( TASK_MINUTE * sendDelay , TASK_FOREVER, &sendMcp );


  void sendMcp(){
  ledState = HIGH;
  digitalWrite(CS_PIN,LOW);    
  ackStatus = false;
  mesh.sendSingle(root, String("online?"));                           
  
  long previousMillis = 0;  
  unsigned long currentMillis = millis();


 if(currentMillis - previousMillis > interval) {
   // previousMillis = currentMillis;   
 while (ackStatus = false)
 {
      
      Serial.print("root not found, writing to card");
    writeToCard();
  }
 }

  }

 Task taskUpdateTime( TASK_SECOND * 1 , TASK_FOREVER, &updateTime );
 void updateTime(){


   ts_epoch++;
   wdt++;
 }

 Task taskWriteToCard( TASK_MINUTE * sendDelay , TASK_FOREVER, &writeToCard );

  Task taskWriteToCardNoAck( TASK_IMMEDIATE , TASK_ONCE, &writeToCard );

 void writeToCard()
 { 
    ledState = HIGH;



   if(mcp == 1 && mfd ==1){ 
  String msg = String(ts_epoch)+ ",";
  msg += readMcp() + ",";
  msg += readMfd();
  msg += "from sd";

  msg += String(timeIndex);


   digitalWrite(sd_PIN,LOW);    
      SD.begin(sd_PIN);
    File dataFile = SD.open("/logs/offlinelog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(msg);
    dataFile.close();
     Serial.println("to SD Card"); 
    Serial.println(msg);
    Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");}
  else {
 Serial.println("error opening  offlinelog.txt"); 
    }
    timeIndex++;

    SD.end();
    digitalWrite(sd_PIN,HIGH);    
    }

       else if (mfd == 1 ){  
       String msg = String(ts_epoch)+ ",";
       msg += readMfd();
       msg += "from sd";
       msg += String(timeIndex);
    digitalWrite(sd_PIN,LOW);    
      SD.begin(sd_PIN);
    File dataFile = SD.open("/logs/offlinelog.txt", FILE_WRITE);
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

    SD.end();
       digitalWrite(sd_PIN,HIGH);  
        
        }

       else if (mcp == 1 ){  
       String msg = String(ts_epoch)+ ",";
       msg += readMcp();
       msg += "from sd";
       msg += String(timeIndex);
           Serial.println(msg);

    digitalWrite(sd_PIN,LOW);    
      SD.begin(sd_PIN);
    File dataFile = SD.open("/logs/offlinelog.txt", FILE_WRITE);
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
    SD.end();
       digitalWrite(sd_PIN,HIGH);  
        
        }




  } 

  
  Task taskLoggedData(TASK_SECOND * 5 , TASK_FOREVER , &loggedData );

 void loggedData(){
      
    } 
 


 Task taskManageTasks( TASK_MINUTE * sendDelay , TASK_FOREVER, &manageTasks );

void manageTasks(){
  
  
   if( (WiFi.RSSI()) == 31 ){
     taskWriteToCard.enable();
         digitalWrite(LED, LOW);   

     }  else {

        taskWriteToCard.disable();
        taskLoggedData.enable();
  //    digitalWrite(LED, HIGH);   

      }


 
  }



void receivedCallback( uint32_t from, String &msg ) {
  
 Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());

 if(msg == "online"){
  Serial.print("got ack, root online");
  if (mcp == 1 && mfd == 1 )
  {
    msgMfd = readMfd();
    msgMcp = readMcp();
    String xmsg = String(ts_epoch)+ "," ;
    xmsg += msgMfd + msgMcp;
  
    mesh.sendSingle(root, xmsg );
    Serial.println(msg);
    ackStatus = true;
    wdt = 0;
}

 else if(mfd == 1){
    String xmsgMfd =  String(ts_epoch) + ",";
    xmsgMfd+= readMfd();
   // msgMfd +=  "," + String(ts_epoch) ;
    mesh.sendSingle(root, xmsgMfd );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
    Serial.println(msgMfd);
    ackStatus = true;
        wdt = 0;

 }
 else if(mcp == 1){
    ledState = HIGH;
   String xmsgMcp = String(ts_epoch)+ ",";
    xmsgMcp += readMcp();
   // msgMcp +=  "," + String(ts_epoch) ;
    mesh.sendSingle(root, xmsgMcp );
    Serial.println(msgMcp);
    ackStatus = true;
        wdt = 0;

  
  }
   }
   String strMsg = String(msg);
   ts_epoch = strMsg.toInt();
    writeTimeToCard();}

void newConnectionCallback(uint32_t nodeId) {
   Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
        taskSendMsgSd.enable();
  
}

void changedConnectionCallback() {
 Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
 //digitalWrite(D2,LOW); 

  Serial.begin(9600, SERIAL_8E1);
  node.begin(1, Serial);

  SD.begin(sd_PIN);
  
  File configFile = SD.open("config.json", "r");
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

  const char* MESH_PREFIX = doc["MESH_PREFIX"];
  const char*  MESH_PASSWORD = doc["MESH_PASSWORD"];
  const char*  ROOT = doc["root"];
  const char*  ID = doc["id"];
  const char*  MFD = doc["mfd"];
  const char* MCP = doc["mcp"];

  // Real world application would store these values in some variables for
  // later use.

  Serial.print("Loaded : ");
  Serial.println(MESH_PREFIX);
  mesh_ssid = MESH_PREFIX;
  mesh_Pass = MESH_PASSWORD;
  //Serial.print("Loaded accessToken: ");
  Serial.println(MESH_PASSWORD);
  root = atoi(ROOT);
  Serial.println(root);
  id = atoi(ID);

  mfd = atoi(MFD);
  mcp = atoi(MCP);
  Serial.println(id);

  Serial.println(mfd);
  Serial.println(mcp);


  File ts_card = SD.open("time.txt"); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (ts_card) {

        // String logs;
    String buffer;
         buffer = ts_card.readStringUntil('\n');
          ts_epoch = buffer.toInt();
          ts_card.close();
          SD.remove("time.txt");
          SD.end();
          digitalWrite(sd_PIN, HIGH);
          
  }


 

   mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes( ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  //mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.setContainsRoot(true);
  
  
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask( taskSendMfd );
  userScheduler.addTask(taskWriteToCard);
  userScheduler.addTask(taskSendMcp);
  userScheduler.addTask(taskLoggedData);
  userScheduler.addTask(taskManageTasks);
//  userScheduler.addTask(dataStream);
  //userScheduler.addTask(taskGetTimeStamp);
  userScheduler.addTask(taskWriteToCardNoAck);
    userScheduler.addTask(taskUpdateTime);
        userScheduler.addTask(taskSendMsgSd);


taskUpdateTime.enable();

taskManageTasks.enable();

if(mfd == 1){
    taskSendMfd.enable();

  }else if(mcp == 1){
    taskSendMcp.enable();
    }

  //taskLoggedData.enable();
//taskGetTimeStamp.enable();

  pinMode(A0, INPUT);   // Define A0 pin as INPUT
 pinMode(LED, OUTPUT);

                           // LED will be ON when node is writibng to sd card                                
// dataStream.enable();
 
}

void loop() {
 
 
       digitalWrite(LED, ledState);

  // it will run the user scheduler as well


double seconds=millis()/1000; 
//Serial.print(seconds);

  mesh.update();
 //Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db

 if(wdt > 120)
 {
   while(1);
 }
  
 if (seconds > 120)                                                         // When period will be > 60 seconds, deep sleep mode will be active
  {
  //   mesh.stop();
 //ESP.deepSleep(30e6);                                                    // deepSleep mode will be active for 300*10^6 microseconds, i.e. for 300 seconds                                                   
//digitalWrite(D2,HIGH); 
 //seconds = 0;

    digitalWrite(sd_PIN,LOW);    
      SD.begin(sd_PIN);
    File dataFile = SD.open("time.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(ts_epoch);
    dataFile.close();
    return;
    }
  else {
    Serial.println("error opening  offlinelog.txt"); 
    SD.end();
      while(1); }
                 while(1);
  }

}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
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

double readRegister(int add){
  Serial.print("reading registers");
      int result =  node.readHoldingRegisters(add, 2);
     double Wattage;
      if (result == node.ku8MBSuccess){
          Serial.print("reading buffer");

   int a =node.getResponseBuffer(0);
    int b =node.getResponseBuffer(1);
      Wattage = RSmeter(a, b);
      }
     else if (result == node.ku8MBIllegalDataAddress)
     {

       Serial.print("wrong address");

     }
     else if (result == node.ku8MBInvalidCRC)
     {

       Serial.print("wrong crc");

     }
     else if (result == node.ku8MBSlaveDeviceFailure)
     {

       Serial.print("timed out");

     }



     //node.clearResponseBuffer();
    double WATT = Wattage;
    return WATT;
  }


   double readWattage(){
     double Wattage = RSmeter(a[0], b[0]);
    double WATT = Wattage;
    return WATT;
  }
    double readWattageR(){
     double Wattage = RSmeter(a[2], b[2]);
    double WATT = Wattage;
    return WATT;
  }
    double readWattageY(){
     double Wattage = RSmeter(a[3], b[3]);
    double WATT = Wattage;
    return WATT;
  }
    double readWattageB(){
     double Wattage = RSmeter(a[4], b[4]);
    double WATT = Wattage;
    return WATT;
  }
    double readCurrentR(){
     double Wattage = RSmeter(a[26], b[26]);
    double WATT = Wattage;
    return WATT;
  }
    double readCurrentY(){
     double Wattage = RSmeter(a[27], b[27]);
    double WATT = Wattage;
    return WATT;
  }
    double readCurrentB(){
     double Wattage = RSmeter(a[28], b[28]);
    double WATT = Wattage;
    return WATT;
  }
    double readPf(){
     double Wattage = RSmeter(a[3], b[3]);
    double WATT = Wattage;
    return WATT;
  }
    double readVll(){
     double Wattage = RSmeter(a[3], b[3]);
    double WATT = Wattage;
    return WATT;
  }
    double readR(){
     double Wattage = RSmeter(a[22], b[22]);
    double WATT = Wattage;
    return WATT;
  }
double readY(){
     double Wattage = RSmeter(a[23], b[23]);
    double WATT = Wattage;
    return WATT;
  }
  double readB(){
     double Wattage = RSmeter(a[24], b[24]);
    double WATT = Wattage;
    return WATT;
  }
String readMfd(){
  ledState = HIGH;

 int i=100,j=0;
  while(i<160)
  {
    int result =  node.readHoldingRegisters(i, 2);
    a[j] =node.getResponseBuffer(0);
    b[j] =node.getResponseBuffer(1);
    i=i+2;
    j++;
  }

    
   msgMfd = id   ;     
    // msgMfd +=  "," + String(readRegister(106));              //KW
    msgMfd += "," + String(readWattage());                //wy
    msgMfd += "," + String(readWattageR());//WR
    msgMfd += "," + String(readWattageY());//WB
    msgMfd +=  "," + String(readWattageB());//Iavg
    msgMfd +=  "," + String(readCurrentR());//Iavg
    msgMfd +=  "," + String(readCurrentY());//Iavg

    msgMfd += ","  + String(readCurrentB()) ; //Vll
    msgMfd += ","  + String(readR()) ;//pf
    msgMfd += ","  + String(readY()) ;//pf
    msgMfd += "," + String(readB());//KVA
   //msgMfd += "," + String(readRegister());//VAH
   //  node.clearResponseBuffer();

  return msgMfd;
}

String readMcp(){

ledState = HIGH;
   
 digitalWrite(CS_PIN,LOW);    

MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
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
   digitalWrite(CS_PIN,HIGH);
delay(2);
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

void sendMsgSd(){

SD.begin(sd_PIN);

  File file = SD.open("offlinelog.txt", FILE_READ); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
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
     // msg += " loggeddata ";
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
      SD.remove("offlinelog.txt");
  }

      SD.end();
      digitalWrite(sd_PIN, HIGH);


}
void writeTimeToCard()
{

 
    digitalWrite(sd_PIN,LOW);    
      SD.begin(sd_PIN);
      SD.remove("time.txt");
    File dataFile = SD.open("time.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(ts_epoch);
    dataFile.close();
    }
  else {
    Serial.println("error opening  time.txt"); 
    SD.end(); } 
}
