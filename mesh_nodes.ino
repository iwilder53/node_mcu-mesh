#include <NTPClient.h>

#include <ModbusMaster.h>

#include <SD.h> 

//#include <easyMesh.h>
 #include "painlessMesh.h"
#include <SPI.h>
#include <WiFiUdp.h>

 


#define   MESH_PREFIX     "HetaDatain"                         // Mesh Prefix (SSID) should be same for all  nodes in Mesh Network
#define   MESH_PASSWORD   "Test@Run_1"                         // Mesh Password should be same for all  nodes in Mesh Network
#define   MESH_PORT       5555                                      // Mesh Port should be same for all  nodes in Mesh Network 
  
//String mesh_prefix;
//String mesh_pasword;

// function declaration
void postFileContent(const char * path );

#define FILE_LINE_LENGTH        81  // a line has 80 chars 
char txtLine[FILE_LINE_LENGTH];
char postdata [FILE_LINE_LENGTH];
bool readCondition = true;  // Has to be defined somewhere to trigger SD read
String buffer;
int a[70],b[70]; // We can change these to 16 bit to theorotically halve the memory space
uint32_t root = 314262534; 
unsigned long currentMillis = millis();
long interval = 1000;   
unsigned long period=0;  
        

String formattedDate;
String dayStamp;
String timeStamp;

bool ackStatus = false ;

int LED = 5;

ModbusMaster node;
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
WiFiUDP ntpUDP;// Define NTP Client to get time
NTPClient timeClient(ntpUDP);

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void writeToCard() ;
void loggedData();
void manageTasks();
void buildDataStream() ;
void getTimeStamp();

Task taskGetTimeStamp( TASK_MINUTE * 2 , TASK_FOREVER, &getTimeStamp );   // Set task second to send msg in a time interval


Task taskSendMessage( TASK_SECOND * 10 , TASK_FOREVER, &sendMessage );   // Set task second to send msg in a time interval



void sendMessage() {


 /*                
  int i=100,j=0;
  while(i<160)
  {
    int result =  node.readHoldingRegisters(i, 2);
    a[j] =node.getResponseBuffer(0);
    b[j] =node.getResponseBuffer(1);
    i=i+2;
    j++;
  }
  */

mesh.sendSingle(root, String("online?"));                             //ping to root node
ackStatus = false;
long previousMillis = 0;  
unsigned long currentMillis = millis();


 if(currentMillis - previousMillis > interval) {
   // previousMillis = currentMillis;   
 while (ackStatus = false)
 {

  Serial.print("root not found, writing to card");
  
 // taskWriteToCard.enable();
 String msg = "NODE no.x"   ;     
       msg += " Analog: " + String (analogRead(A0)); 
       msg += "  offlinelog"; 

   
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
    Serial.println("error opening  offlinelog.txt"); 
    }
  }
  }
/*

  String msg = "x4"   ;                                       // You can write node name/no. here so that you may easily recognize it        
 // msg += mesh.getNodeId);                                              // Adding Node id in the msg
   msg += "," + String (analogRead(A0));                          // Adding  analog reading in the msg. You can also add other pin readings 
 //msg += " myFreeMemory: " + String(ESP.getFreeHeap());                 // Adding free memory of Nodemcu in the msg
  msg += ","  + String(readVyb()) ; 
 // msg +=  "," + String(readWattage());
  msg += ","  + String(readLineVoltage()) ; 
  //msg += ","  + String(readPf()) ; 
  //uint32_t target = 314262534; 
  mesh.sendSingle(root, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
 Serial.println(msg);
  Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");*/

 node.clearResponseBuffer();


  } 
 Task dataStream( TASK_SECOND * 90 , TASK_FOREVER, &buildDataStream);


 Task taskWriteToCard( TASK_MINUTE * 2 , TASK_FOREVER, &writeToCard );

 
 void writeToCard()
 { 


String msg = "NODE no.x"   ;     
       msg += " Analog: " + String (analogRead(A0)); 
       msg += "  offlinelog"; 

   
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
    Serial.println("error opening  offlinelog.txt"); 
    }
  } 

  
  Task taskLoggedData(TASK_SECOND * 3 , TASK_FOREVER , &loggedData );

 void loggedData(){ 


/*
  File file = SD.open("offlinelog.txt", FILE_READ); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (!file) {
    Serial.println("Failed to open file for reading");
    
    return;
  }

    // String logs;
   String buffer;
  uint8_t i = 0;

  while (buffer != NULL())
//for (int i = 0; i < 20 ; i++)   { 
    buffer = file.readStringUntil('\n');
   // Serial.println(buffer); //Printing for debugging purpose         
     
 
 
  String   msg = buffer; 
     // msg += " loggeddata ";
      uint32_t target = 2137585097; 
      mesh.sendSingle(target, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
      
      Serial.println(msg); 
 
   }   
  file.close();
  Serial.println(F("DONE Reading"));
  SD.remove("offlinelog.txt");
//SD.remove(offlinelog.txt"); 
   //file = SD.open("offlinelog.txt", FILE_WRITE);                          //deleting file after data is sent
   //file.close();
  
 
 */

      
    } 
 


 Task taskManageTasks( TASK_MINUTE * 3 , TASK_FOREVER, &manageTasks );

void manageTasks(){
  
  
   if( (WiFi.RSSI()) == 31 ){
     taskWriteToCard.enable();
         digitalWrite(LED, LOW);   

     }  else {

           taskWriteToCard.disable();
            taskLoggedData.enable();
              //   digitalWrite(LED, HIGH);   

      }


 
  }



void receivedCallback( uint32_t from, String &msg ) {
 Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());

 if(msg == "online"){
  Serial.print("got ack, root online");
  
  String msg = dayStamp + timeStamp + "x4"   ;                                       // You can write node name/no. here so that you may easily recognize it        
 // msg += mesh.getNodeId);                                              // Adding Node id in the msg
   msg += "," + String (analogRead(A0));                          // Adding  analog reading in the msg. You can also add other pin readings 
 //msg += " myFreeMemory: " + String(ESP.getFreeHeap());                 // Adding free memory of Nodemcu in the msg
  msg += ","  + String(readVyb()) ; 
 // msg +=  "," + String(readWattage());
  msg += ","  + String(readLineVoltage()) ; 
  //msg += ","  + String(readPf()) ; 
  //uint32_t target = 314262534; 
  mesh.sendSingle(root, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
 Serial.println(msg);
 ackStatus = true;
 }
 
}

void newConnectionCallback(uint32_t nodeId) {
   Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);

  File file = SD.open("offlinelog.txt", FILE_READ); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
//#endif
  if (!file) {
    Serial.println("Failed to open file for reading");
    
    return;
  }

    // String logs;
   String buffer;
  uint8_t i = 0;

   

while (file.available())
//  while (buffer != NULL())
// for (int i = 0; i < 20 ; i++) 
{ 


  
     buffer = file.readStringUntil('\n');
   // Serial.println(buffer); //Printing for debugging purpose         
     
 
 
  String   msg = buffer; 
     // msg += " loggeddata ";
      mesh.sendSingle(root, msg );                                        // Send msg to single node. To broadcast msg (mesh.sendBroadcast(msg)) 
      
      Serial.println(msg); 
     
 
}   
 
 
  file.close();
  Serial.println(F("DONE Reading"));
  SD.remove("offlinelog.txt");
 
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
  Serial.begin(9600);
  node.begin(1, Serial);
    
    //CHECK FOR SDCARD
    if (!SD.begin(D8)) {
    
   Serial.println("Card failed, or not present");
     // don't do anything more:
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
  userScheduler.addTask(dataStream);
  userScheduler.addTask(taskGetTimeStamp);

  taskSendMessage.enable();
  //taskLoggedData.enable();
taskManageTasks.enable();
 // dataStream.enable();
taskGetTimeStamp.enable();
  pinMode(A0, INPUT);   // Define A0 pin as INPUT
 pinMode(LED, OUTPUT);

                           // LED will be ON when node is writibng to sd card                                


     
 


  
}

void loop() {
 
  
  // it will run the user scheduler as well

period=millis()/1000;                                                    // Function "mllis()" gives time in milliseconds. Here "period" will store time in seconds
 
  mesh.update();
 //Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");       // Prints wi-fi signal strength in db

 
 
  
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




void buildDataStream() 
{
  
                /*MODBUS*/
               
  int i=100,j=0;
  while(i<160)
  {
    int result =  node.readHoldingRegisters(i, 2);
    a[j] =node.getResponseBuffer(0);
    b[j] =node.getResponseBuffer(1);
    i=i+2;
    j++;
  }
  
   }


   double readWattage(){
     double Wattage = RSmeter(a[0], b[0]);
    double WATT = Wattage;
    return WATT;
  }


  double readPf(){
     double PFactor = RSmeter(a[9], b[9]);
     return PFactor;
    }
double readLineVoltage(){
  
   double VLL = RSmeter(a[21], b[21]);
  return VLL;
  }
double readCurrent(){
      double Current = RSmeter(a[25], b[25]);
return Current;
  }
  double readWattHour(){
        double WHour = RSmeter(a[30], b[30]);
 return WHour;
    }
    double readVyb(){
        double Vyb = RSmeter(a[24], b[24]);
 return Vyb;
    }

void getTimeStamp() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.println(timeStamp);
}
