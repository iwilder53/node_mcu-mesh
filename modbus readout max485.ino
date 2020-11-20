  #include <ModbusMaster.h>

#define MAX485_DE      D2
#define MAX485_RE_NEG  D3

// instantiate ModbusMaster object
ModbusMaster node;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

int i, j;
  uint16_t a,b;  

void setup() {
    pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  // put your setup code here, to run once:
  
  Serial.begin(9600, SERIAL_8E1);
  node.begin(1, Serial);

  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}
bool state = true;



void loop() {
  // put your main code here, to run repeatedly:
    Serial.print("STArt "); 
    delay(80);
      Serial.print(String(readWattageR(100)) + "watt total");                
      delay(80);
 

    Serial.print(String(readWattageR(102)) + "wattR ");  
      delay(80);
    
    Serial.print(String(readWattageR(104)) + "wattY");             
  delay(80);

    Serial.print(String(readWattageR(106)) + "wattB");        
  delay(80);

    Serial.print(String(readWattageR(142)) + "voltR");                
  delay(80);

    Serial.print(String(readWattageR(144)) + "voltY");                
  delay(80);

    Serial.print(String(readWattageR(146)) + "voltB");        
      delay(80);
        

    Serial.print(String(readWattageR(150)) + "CurrentR");                
      delay(80);

    Serial.print(String(readWattageR(152)) + "CurrentY");                
      delay(80);

    Serial.print(String(readWattageR(154)) + "CurrentB");                
    
  delay(80);
    Serial.print(String(readWattageR(116)) + "pf avg");                
      delay(80);

  
    Serial.print(String(readWattageR(140)) + "vln avg");                
      delay(80);

    Serial.print(String(readWattageR(118)) + "pf r");                
              delay(80);

    Serial.print(String(readWattageR(120)) + " pf y");                
              delay(80);

    Serial.print(String(readWattageR(122)) + "pf b");                
              delay(80);

    Serial.print(String(readWattageR(124)) + " VA Total");                
              delay(80);

    Serial.print(String(readWattageR(126)) + "VA R");                
              delay(80);

    Serial.print(String(readWattageR(128)) + " VA Y");                
              delay(80);

    Serial.print(String(readWattageR(130)) + " VA B");                
              delay(80);

    Serial.print(String(readWattageR(132)) + " VLL ");                
              delay(80);

    Serial.print(String(readWattageR(134)) + " vry phase");                
              delay(80);

    Serial.print(String(readWattageR(136)) + " vyb phase");                
              delay(80);

    Serial.print(String(readWattageR(138)) + "vbr phase");                
              delay(80);

    Serial.print(String(readWattageR(156)) + "frquency");                
              delay(80);

    Serial.print(String(readWattageR(158)) + " WH Recieved");                
              delay(80);

    Serial.print(String(readWattageR(160)) + " VAH Recieved");                
              delay(80);

    Serial.print(String(readWattageR(148)) + "current avg");                
              delay(80);
                  Serial.print("END ");  


}
  double readWattageR(int add){

    dataStream(add );
    if(dataStream){
      
     double Wattage = RSmeter(a, b);
    double WATT = Wattage;
    return WATT;}
    
  }
  

bool dataStream(int one ){
  a = 0 ;
  b = 0 ;

  int   result =  node.readHoldingRegisters(one, 2 );        //Don't change this EVER!!
  delay(60);

  //  delay(65);                                            // we dont whats going on here, but its the only to make it work 

    if (result == node.ku8MBSuccess){ 
               Serial.print("it works : "); 

    a =node.getResponseBuffer(0);
    b =node.getResponseBuffer(1);
      return true;
    }
    else{
      
      Serial.print(result, HEX);
      }
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
