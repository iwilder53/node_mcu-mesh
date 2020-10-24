#include <BH1750.h>
#include <ModbusMaster232.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
//#include <BH1750FVI.h>
#include <DHT.h>
#include <Adafruit_BMP183_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

unsigned long timer = 0;

BH1750 lightMeter;
ModbusMaster232 node(2);

#define address 100       //  RS485 starting address
#define bitQty 2          //  RS485 bit size
int a[70],b[70];

#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);    // Create the Lightsensor instance
WiFiServer server(80);

#define ONE_WIRE_BUS D5       //  Digital temperature sensor1 pin
#define ONE_WIRE_BUS2 D6      //  Digital temperature sensor1 pin
OneWire oneWire(ONE_WIRE_BUS);
OneWire oneWire2(ONE_WIRE_BUS2);
DallasTemperature sensors(&oneWire);
DallasTemperature sensors2(&oneWire2);

float Celsius = 0;
float Celsius2 = 0;

const char *ssid = "TP-LINK";            //ENTER YOUR WIFI SETTINGS
const char *password = "hardware";

//Web/Server address to read/write from
//const char *host = "192.168.4.1";   //https://circuits4you.com website or IP address of server
  int incre = 0;
  int Prev_Val0 = 0;
  int finalInt0 = 0;
  int Prev_Val1 = 0;
  int finalInt1 = 0;
  int Prev_Val2 = 0;
  int finalInt2 = 0;
  int Prev_Val3 = 0;
  int finalInt3 = 0;
  int Prev_Val4 = 0;
  int finalInt4 = 0;
  int Prev_Val5 = 0;
  int finalInt5 = 0;
  int Prev_Val6 = 0;
  int finalInt6 = 0;

  int incrDenoLux=0;
  int incrDenoUV=0;
  int incrDenoHum=0;
  int incrDenoTemp=0;
  int incrDenoHeat=0;
  int incrDenoTherm1=0;
  int incrDenoTherm2=0;

  int relayInput = D8;          // Relay pin
  int id=2;                     // If require Change here....>>> ID of the Device

  //String serverHost = "http://192.168.4.1/feed?";     // Local ESP web-server address
  String data1, serverHost;

  int sleepInterval = 5;          // DEEP_SLEEP Timeout interval
  int failConnectRetryInterval = 2;   // DEEP_SLEEP Timeout interval when connecting to AP fails
  int counter = 0;
  float h = 0.0, t = 0.0;

// Static network configuration
IPAddress ip(192, 168, 43, 33);// change ip address if second client
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiClient client;

void setup()
{
  Serial.begin(9600);
  server.begin();
  pinMode(relayInput, OUTPUT);
 
  dht.begin();
  Wire.begin(D1,D2);

  lightMeter.begin();
  //Serial.println(F("BH1750 Test"));

  digitalWrite(relayInput, HIGH);   // turn relay ON
  delay(5000);
  digitalWrite(relayInput, LOW);    // turn relay OFF
  delay(5000);

  sensors.begin();
  sensors2.begin();

  Serial.println("Setup over !");
  timer = millis();
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

delay(10);
node.clearResponseBuffer();

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
  //Serial.println("..............................");
  //String Data1, Data2, Data3, Data4, Data5 ;

                /*RS485*/
               
  int i=100,j=0;
  while(i<160)
  {
    int result =  node.readHoldingRegisters(i, bitQty);
    a[j] =node.getResponseBuffer(0);
    b[j] =node.getResponseBuffer(1);
    i=i+2;
    j++;
  }
  
    double Wattage = RSmeter(a[1], b[1]);
    double WATT = Wattage;
    //Data1 = String(WATT);
    if(WATT<0)
    {
      WATT = WATT*(-1);
      Serial.print("Total Watt = ");
      Serial.print(String (WATT) + " kW");
    }
    else
    {
      Serial.print("Total Watt = ");
      Serial.print(String (WATT) + " kW");
    }
    
    double PFactor = RSmeter(a[9], b[9]);
    //Data2 = String(PFactor);
    Serial.print("Power Factor = ");
    Serial.print(String (PFactor) + " PF");

    double VLL = RSmeter(a[21], b[21]);
    //Data3 = String(VLL);
    Serial.print("Line Voltage = ");
    Serial.print(String (VLL) + " VLn");

    double Current = RSmeter(a[25], b[25]);
    //Data4 = String(Current);
    Serial.print("Current = ");
    Serial.print(String (Current) + " A");
    
    double WHour = RSmeter(a[30], b[30]);
   //double kWatth = WHour/1000;
    double kWatth = WATT*0.024;
    //Data5 = String(kWatth);
    Serial.print("Watthour = ");
    Serial.print(String (kWatth) + " kWH");
    
  ////////////////  for light sensor /////////////////////////////

  //uint16_t lux = LightSensor.GetLightIntensity();  
  float lux = lightMeter.readLightLevel();
  Serial.print("\nAmbient Light: ");
  Serial.print(String(lux)+"\n");

///////////////////////////////PROGRAM OF UV INDEX/////////////////////////////////////

  float sensorVoltage;
  float sensorValue;
  float sensor_reading;
  float sensor_voltage;
  float uv_Index;
  sensorValue = analogRead(A0);
  sensorVoltage = (sensorValue/1024*5.0)*1000;

   if(sensorValue<10)
  {
    uv_Index = 0;
    Serial.println("uvindex:0");
    //sensorValue=0;
    //Serial.println(sensorValue);
  }
   else if(10<sensorValue and sensorValue<46)
  {
    uv_Index = 1;
    Serial.println("uvindex: 1");
    //sensorValue=1;
    //Serial.println(sensorValue);
   }
   else if(46<sensorValue and sensorValue<65)
   {
      uv_Index = 2;
      Serial.println("uvindex: 2");
    //sensorValue=2;
    //Serial.println(sensorValue);
   }
   else if(65<sensorValue and sensorValue<83)
   {
      uv_Index = 3;
      Serial.println("uvindex: 3");
      //sensorValue=3;
      //Serial.println(sensorValue);
   }
   else if(83<sensorValue and sensorValue<103)
   {
      uv_Index = 4;
      Serial.println("uvindex: 4");
      //sensorValue=4;
      //Serial.println(sensorValue);
   }
   else if(103<sensorValue and sensorValue<124)
   {
      uv_Index = 5;
      Serial.println("uvindex: 5");
      //sensorValue=5;
      //Serial.println(sensorValue);
   }
   else if(124<sensorValue and sensorValue<142)
   {
      uv_Index = 6;
      Serial.println("uvindex:6");
      //sensorValue=6;
      //Serial.println(sensorValue);
   }
   else if(142<sensorValue and sensorValue<162)
   {
      uv_Index = 7;
      Serial.println("uvindex: ");
      //sensorValue=7;
      //Serial.println(sensorValue);
   }
   else if(162<sensorValue and sensorValue<180)
   {
      uv_Index = 8;
      Serial.println("uvindex:8 ");
      //sensorValue=8;
      //Serial.println(sensorValue);
   }
   else if(180<sensorValue and sensorValue<200)
   {
      uv_Index = 9;
      Serial.println("uvindex:9 ");
      //sensorValue=9;
      //erial.println(sensorValue);
   }
   else if(200<sensorValue and sensorValue<221)
   {
      uv_Index = 10;
      Serial.println("uvindex: 10");
      //sensorValue=10;
      //Serial.println(sensorValue);
   }
   else if(221<sensorValue and sensorValue<240)
   {
      uv_Index = 11;
      Serial.println("uvindex: 11");
      //sensorValue=11;
      //Serial.println(sensorValue);
    }
    else
    { 
      uv_Index = 12;
      Serial.println("uvindex:high ");
      //Serial.println("high");
    }

/////////////////////Digital thermistor////////////

  sensors.requestTemperatures();
  sensors2.requestTemperatures();

  Celsius = sensors.getTempCByIndex(0);
  Celsius2 = sensors2.getTempCByIndex(0);

  Serial.print("\nTemperature 1 : " + String(Celsius));
  Serial.print(" C\n");
  Serial.print("Temperature 2 : " + String(Celsius2));
  Serial.print(" C\n");

/////////////////////TO CALCULATE OF DHT22////////////////////
  
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
     h = 0.00, t = 0.00;
  }
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\n");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C\n");

////////////////////////////////////////////////////////////////////
   
    data1 ="id=";
    data1 +=String('2');
    data1 +="&POWER=";
    data1 +=String(WATT);
    data1 +="&POWERFACTOR=";
    data1 +=String(PFactor);
    data1 +="&VOLTAGE=";
    data1 +=String(VLL);
    data1 +="&CURRENT=";
    data1 +=String(Current);
    data1 +="&ENERGY=";
    data1 +=String(kWatth);
    data1 +="&LIGHT=";
    data1 +=String(lux);
    data1 +="&UV=";
    data1 +=String(sensorValue);
    data1 +="&TEMP=";
    data1 +=String(t);
    data1 +="&HUMIDITY=";
    data1 +=String(h);
    data1 +="&TEMP1=";
    data1 +=String(Celsius);
    data1 +="&TEMP2=";
    data1 +=String(Celsius2);
    data1 +="&Speed1=";
    data1 +=String('0');
    data1 +="&Speed2=";
    data1 +=String('0');
    data1 +="&Gas=";
    data1 +=String('0');
  
    Serial.println("- data stream: "+data1);
    serverHost = "http://166.62.91.152/cummins_cooling_data/temp.php?id="+String(id)+"&POWER="+String(WATT)+"&POWERFACTOR="+String(PFactor)+"&VOLTAGE="+String(VLL)+"&CURRENT="+String(Current)+"&ENERGY="+String(kWatth)+"&LIGHT="+String(lux)+"&UV="+String(sensorValue)+"&TEMP="+String(t)+"&HUMIDITY="+String(h)+"&TEMP1="+String(Celsius)+"&TEMP2="+String(Celsius2)+"&Speed1="+String('0')+"&Speed2="+String('0')+"&Gas="+String('0');
  }
  
  void sendHttpRequest() 
  {
    HTTPClient http;
    http.begin(serverHost);
    int httpCode = http.GET();
    Serial.println("httpCode = "+ String(httpCode));
    if (httpCode < 0)
    {
      //int count = 0;
      while(1)
      {
        http.begin(serverHost);
        httpCode = http.GET();
        delay(50);
        //count++;
        if (httpCode > 0)
        {break;}
      } 
      Serial.println("httpCode after while loop = "+ String(httpCode));
    }
    String payload = http.getString();                //Get the request response payload
    payload.trim();
    Serial.println(payload);
  }

void loop()
{
  ESP.eraseConfig();
  WiFi.persistent(false);
  delay(500);
  //Serial.println("- set ESP STA mode");
  WiFi.mode(WIFI_STA);
  
  buildDataStream();
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    counter++;
  }
 
  Serial.println("- wifi connected");  
  sendHttpRequest();
  Serial.println("- got back to sleep");
 
  int a = 55000;
  int b = millis();

  delay(a+timer-b);    //delay for all programs
  Serial.println(a+timer-b);
  timer = millis(); 
}
