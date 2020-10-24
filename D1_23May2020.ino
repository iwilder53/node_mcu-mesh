  #include <ModbusMaster232.h>
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266HTTPClient.h>
  //#include <ModbusMaster232.h>
  #include <Wire.h>
  #include <RtcDS3231.h> 
  RtcDS3231<TwoWire> Rtc(Wire);
  #include <OneWire.h>
  #include <DallasTemperature.h>
  #include<SPI.h>
  #include<SD.h>
  #include "DHT.h"
  
  unsigned long timer = 0;
  
  ModbusMaster232 node(1);
  
  #define address 100
  #define bitQty 2
  int a[70],b[70];                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
  //int a5, b5, c5, d5, e5, f5, g5, h5, i5, j5 , k5, l5;
  
  #define DHTPIN D6     // what digital pin the DHT22 is conected to
  #define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
  
  DHT dht(DHTPIN, DHTTYPE);
  
  #define ONE_WIRE_BUS D4
  #define ONE_WIRE_BUS2 D5
  #define anInput A0  
  #define co2Zero 55
  
  WiFiServer server(80);
  
  OneWire oneWire(ONE_WIRE_BUS);
  OneWire oneWire2(ONE_WIRE_BUS2);
  DallasTemperature sensors(&oneWire);
  DallasTemperature sensors2(&oneWire2);
  
  int id=1, co2ppm;                             // If require Change here....>>> ID of the Device 
  float h = 0.00, t = 0.00, Celsius = 0, Celsius2 = 0;
  double WATT, VLL, Current, kWatth, PFactor;
  
  uint8_t GPIO_Pin = 4;
  char newPulse;
  int revMicros, latestPulseMicros, prevPulseMicros;
  int count = 0;
  float rpm = 0.0, divisor;
  
  uint8_t GPIO_Pin1 = 0;
  char newPulse1;
  int revMicros1, latestPulseMicros1, prevPulseMicros1;
  int count1 = 0;
  float rpm1 =0.0, divisor1;
  
  int relayInput = 5;                           // the input to the relay pin
  int timeSinceLastRead = 0;
  
  // AP Wi-Fi credentials
  const char* ssid =  "TP-LINK";          //Hetadatain_FF,Mother_Network,"sushil",,TP-LINK,AndroidAP4,GIONEEph;
  const char* password = "hardware";       //hetadatain@123,hetadatain123,"sushil@123",,hardware,evst4317,1234purva;
  
  // Local ESP web-server address
  //String serverHost = "http://166.62.91.152/cummins_cooling_data/temp.php?id="+String(id)+"&POWER="+String(WATT)+"&POWERFACTOR="+String(PFactor)+"&VOLTAGE="+String(VLL)+"&CURRENT="+String(Current)+"&ENERGY="+String(kWatth)+"&LIGHT="+String('0')+"&UV="+String('0')+"&TEMP="+String(t)+"&HUMIDITY="+String(h)+"&TEMP1="+String(Celsius)+"&TEMP2="+String(Celsius2)+"&Speed1="+String(rpm)+"&Speed2="+String(rpm1)+"&Gas="+String(co2ppm);
  String data1, serverHost;
  
  // DEEP_SLEEP Timeout interval
  int sleepInterval = 5;
  // DEEP_SLEEP Timeout interval when connecting to AP fails
  int failConnectRetryInterval = 2;
  int counter = 0;
  
  // Static network configuration
  IPAddress ip(192, 168, 43, 25);// change ip address if second client
  IPAddress gateway(192, 168, 43, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFiClient client;
  
  void setup()
  {
    Serial.begin(9600);
    //Serial.println("Modbus communication over RS-232");
    
    server.begin();
    pinMode(anInput,INPUT);                     //MQ135 analog feed set for input
    //node.begin(9600);
   
    Serial.setTimeout(2000);
    dht.begin();
  
    // Wait for serial to initialize.
    while(!Serial) { }
  
    Serial.println("Device Started");
    Serial.println("-------------------------------------");
    Serial.println("Running DHT!");
    Serial.println("-------------------------------------");
   
    pinMode(relayInput, OUTPUT); // initialize pin as OUTPUT
    digitalWrite(relayInput, HIGH); // turn relay on
    delay(5000);
    digitalWrite(relayInput, LOW); // turn relay off
    delay(5000);
  
    sensors.begin();
    sensors2.begin();
    
    attachInterrupt(digitalPinToInterrupt(GPIO_Pin), IntCallback, RISING);
    attachInterrupt(digitalPinToInterrupt(GPIO_Pin1), IntCallback1, RISING);
    //ESP.reset();
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
  
  //getting data from sensor and creating the list
  void buildDataStream() 
  {
    //Serial.println("..............................");
    String Data1, Data2, Data3, Data4, Data5 ;
  
          /*RS485 METER*/
  
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
      WATT = Wattage;//*(-1);
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
  
      PFactor = RSmeter(a[9], b[9]);
      //Data2 = String(PFactor);
      Serial.print("Power Factor = ");
      Serial.print(String (PFactor) + " PF");
  
      VLL = RSmeter(a[21], b[21]);
      //Data3 = String(VLL);
      Serial.print("Line Voltage = ");
      Serial.print(String (VLL) + " VLn");
  
      Current = RSmeter(a[25], b[25]);
      //Data4 = String(Current);
      Serial.print("Current = ");
      Serial.print(String (Current) + " A");
      
      double WHour = RSmeter(a[30], b[30]);
      //double kWatth = WHour/1000;
      kWatth = WATT*0.024;
      //Data5 = String(kWatth);
      Serial.print("Watthour = ");
      Serial.print(String (kWatth) + " kWH"); 
  
              /*GAS SENSOR*/
  
    int co2now[10];                               //int array for co2 readings
    int co2raw = 0;                               //int for raw value of co2
    int co2comp = 0;                              //int for compensated co2
    co2ppm = 0;                               //int for calculated ppm
    int zzz = 0;                                  //int for averaging
    int grafX = 0;                                //int for x value of graph
  
    for (int x = 0;x<10;x++)
    {                                                 //samplpe co2 10x over 2 seconds
      co2now[x]=analogRead(A0);
      delay(200);
    }
    for (int x = 0;x<10;x++)
    {                    
      zzz=zzz + co2now[x];                        //add samples together
    }
    co2raw = zzz/10;                            //divide samples by 10
    co2comp = co2raw - co2Zero;                 //get compensated value
    co2ppm = map(co2comp,0,1023,400,5000);      //map value for atmospheric levels
   
    Serial.print("\nGas sensor reading : ");
    Serial.print(String (co2ppm) + "  PPM");
  
          /*TEMPERATURE SENSOR*/
         
    sensors.requestTemperatures();
    sensors2.requestTemperatures();
  
    Celsius = sensors.getTempCByIndex(0);
    Celsius2 = sensors2.getTempCByIndex(0);
  
    Serial.print("\n");
    Serial.print("Temperature 1 : " + String(Celsius));
    Serial.print(" C    \t\n");
    Serial.print("Temperature 2 : " + String(Celsius2));
    Serial.print(" C    \n");
  
    if (newPulse == true) 
    {
        revMicros = latestPulseMicros - prevPulseMicros;
        prevPulseMicros = latestPulseMicros;
        newPulse = false;
    }
    if (newPulse1 == true) 
    {
        revMicros1 = latestPulseMicros1 - prevPulseMicros1;
        prevPulseMicros1 = latestPulseMicros1;
        newPulse1 = false;
    }
  
    /*DHT SENSOR*/
     
      h = dht.readHumidity();                       // Read temperature as Celsius (the default)
      t = dht.readTemperature();                    // Read temperature as Fahrenheit (isFahrenheit = true)
     
      if (isnan(h) || isnan(t))
    {
      //Serial.println("Failed to read from DHT sensor!");
      h = 0.00, t = 0.00;
      //return;
    }
    Serial.print("\nHumidity: ");
    Serial.print(h);
    Serial.print(" %\n");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C\n ");
    
  ////////////////////////////////////////////////////////////////////
  
    data1 ="id=";
    data1 +=String('1');
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
    data1 +=String('0');
    data1 +="&UV=";
    data1 +=String('0');
    data1 +="&TEMP=";
    data1 +=String(t);
    data1 +="&HUMIDITY=";
    data1 +=String(h);
    data1 +="&TEMP1=";
    data1 +=String(Celsius);
    data1 +="&TEMP2=";
    data1 +=String(Celsius2);
    data1 +="&Speed1=";
    data1 +=String(rpm);
    data1 +="&Speed2=";
    data1 +=String(rpm1);
    data1 +="&Gas=";
    data1 +=String(co2ppm);
   
    Serial.println("- data stream: "+data1);
    serverHost = "http://166.62.91.152/cummins_cooling_data/temp.php?id="+String(id)+"&POWER="+String(WATT)+"&POWERFACTOR="+String(PFactor)+"&VOLTAGE="+String(VLL)+"&CURRENT="+String(Current)+"&ENERGY="+String(kWatth)+"&LIGHT="+String('0')+"&UV="+String('0')+"&TEMP="+String(t)+"&HUMIDITY="+String(h)+"&TEMP1="+String(Celsius)+"&TEMP2="+String(Celsius2)+"&Speed1="+String(rpm)+"&Speed2="+String(rpm1)+"&Gas="+String(co2ppm);

  }
 
  void sendHttpRequest() 
  {
    HTTPClient http;
    http.begin(serverHost);
    int httpCode = http.GET();
    Serial.println("httpCode = "+ String(httpCode));
    if (httpCode < 0)
    {
      while(1)
      {
        http.begin(serverHost);
        httpCode = http.GET();
        delay(50);
        if (httpCode > 0)
        {break;}
      } 
      /*int count = 0;
      while(count <= 5)
      {
        http.begin(serverHost);
        httpCode = http.GET();
        count++;
        if (httpCode > 0)
        {break;}
      }*/
      Serial.println("httpCode after while loop = "+ String(httpCode));
    }
    String payload = http.getString();                //Get the request response payload
    payload.trim();
    Serial.println(payload);
    //http.end();
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
  
  void IntCallback()
  {
    latestPulseMicros = micros();
    newPulse = true;
    count++;
    divisor = millis()/60000.00;
    rpm = (count/divisor);
    Serial.print("Speed of proximity 1 : ");
    Serial.println(String(rpm) + "  rpm");
  }
  
  void IntCallback1()
  {
    latestPulseMicros1 = micros();
    newPulse1 = true;
    count1++;
    divisor1 = millis()/60000.00;
    rpm1 = (count1/divisor1);
    Serial.print("Speed of proximity 2 : ");
    Serial.println(String(rpm1) + "  rpm");
  }
