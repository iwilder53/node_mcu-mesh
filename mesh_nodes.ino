//Libraries

#include "painlessMesh.h"
#include <MCP3008.h>
#include "FS.h"
#include <Arduino.h>
#include <ModbusMaster.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "DHT.h"
#include <Arduino.h>
#include "esp_wifi.h"
#include <SPIFFS.h>

#include <EasyButton.h>
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#define DHTPIN 4
#define relayPin 13
#define MAX485_DE_RE 5
#define DHTTYPE DHT22
#define sendLed 32
#define connLed 2
#define BUTTON_PIN 0

#define CS_PIN 5
#define CLOCK_PIN 18
#define MOSI_PIN 23
#define MISO_PIN 19
#include <AsyncTCP.h>
#define MESH_PORT 5555 // Mesh Port should be same for all  nodes in Mesh Network

//objects declaraation
Adafruit_BME280 bme;
DHT dht(DHTPIN, 22);
LiquidCrystal_I2C lcd(0x27, 20, 4);
EasyButton button(BUTTON_PIN);

Scheduler userScheduler; // to control your personal task
hetadatainMesh mesh;
ModbusMaster node;

//variables
uint8_t mfd_read_pos = 0;
int relay_pin_0_min, relay_pin_0_max, relay_pin_1_min, relay_pin_1_max, relay_pin_2_min, relay_pin_2_max, relay_pin_3_min, relay_pin_3_max, relay_pin_04_min, relay_pin_04_max, relay_pin_05_min, relay_pin_05_max, relay_pin_06_min, relay_pin_06_max, relay_pin_07_min, relay_pin_07_max; // Mesh Port should be same for all  nodes in Mesh Network
int bmeRelay_t_Min, bmeRelay_t_Max, bmeRelay_p_Max, bmeRelay_h_Max, bmeRelay_h_Min, bmeRelay_p_Min;
int device_count;
int mfd_dev_id[5];
uint8_t sendDelay = 2;
unsigned long period = 0;
String id;
int wdt = 0;
int ts_epoch;
int timeIndex;
int rebootTime;
int pos;
uint32_t root;
long previousMillis = 0;
int mcp = 0, mfd = 0, pins = 0, smb = 0;
int first_Reg, second_Reg;
int time_to_print;
bool MCP_Sent = false;
boolean led_active;
int16_t rssi;
uint16_t led_refresh;
uint32_t connectedNode;
String msgMfd_payload;
String msgMfd_payload1;
boolean meshAlive;
String msgSd;
uint8_t interval = 1000;
bool ackStatus;
// User stub
void updateTime();
void mbe();
void lcdShiet();
void meshUpdate(void *random);
void readMcp();
void writeToCard();
void writeTimeToCard();
String readMfd();
void preTransmission();
void postTransmission();
bool dataStream(int one);
void cpu_chill();
boolean read_Mfd_Task();
void updateTime();
void sendMessage();
void sendMsgSd();
void blink_con_led();
void sendPayload(String &payload);
void saveToCard();
void sendMFD();
void updateRssi();
void lcdInfo();
void multi_mfd_read();
boolean trig_Relay(int thres, int relay_max, int relay_min);
boolean trig_Relay_bme(int thres, int relay_max, int relay_min);

void updateTime()
{ //will update time from root && also watchdog
    //digitalWrite(sendLed, LOW);
    wdt++;
    ts_epoch++;
    rebootTime++;
    //  lcd.blink_on();
    //Serial.println(WiFi.RSSI());
}

//Declarations for tasks scheduling
Task taskUpdateTime(TASK_SECOND * 1, TASK_FOREVER, &updateTime); // Set task second to send msg in a time interval (Here interval is 4 second)
Task taskConnLed(TASK_MILLISECOND, TASK_FOREVER, &blink_con_led);
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);        // Set task second to send msg in a time interval (Here interval is 4 second)
Task taskSendMsgSd(TASK_MILLISECOND * 500, TASK_FOREVER, &sendMsgSd); // Set task second to send msg in a time interval
Task task_Multi_Mfd_Read(TASK_SECOND * 10, TASK_FOREVER, &multi_mfd_read);
Task taskReadMBE(TASK_SECOND * 10, TASK_FOREVER, &mbe);
Task taskReadMcp(TASK_MINUTE *sendDelay, TASK_FOREVER, &readMcp);
Task taskReadMfd(TASK_MINUTE *sendDelay, TASK_FOREVER, &read_Mfd_Task); // Set task second to send msg in a time interval (Here interval is 4 second)
Task taskUpdateRssi(TASK_SECOND, TASK_FOREVER, &updateRssi);
Task taskUpdateLcd(TASK_SECOND * 2, TASK_FOREVER, &lcdShiet);

void sendMessage()
{
    // digitalWrite(sendLed, HIGH);  // for testing  && debugging
}

//runs when node recieves something
void receivedCallback(uint32_t from, String &msg)
{
    Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
    String strMsg = String(msg);
    ts_epoch = strMsg.toInt();
}
// runs when a new connection is established
void newConnectionCallback(uint32_t nodeId)
{
    connectedNode = nodeId;
    meshAlive = true;
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
    String nMap = mesh.asNodeTree().toString();
    mesh.sendSingle(root, nMap);
    if (mesh.startDelayMeas(root))
    {
        taskSendMsgSd.enable();
        String configFile = String(id + "," + String(root) + "," + String(mcp) + "," + String(mfd) + "," + String(pins) + "," + String(sendDelay));
        mesh.sendSingle(root, configFile);
    }
    taskConnLed.enable();
}
//runs when the topology changes
void changedConnectionCallback()
{
    meshAlive = true;
    Serial.printf("Changed connections\n");
    String nMap = mesh.subConnectionJson(true);
    mesh.sendSingle(root, nMap);
}
// for internal timekeeping of the mesh

void droppedConnection(uint32_t node_id)
{
    meshAlive = false;
    lcd.setCursor(0, 1);
    lcd.clear();
    lcd.println("Lost connection To : " + node_id);
    taskConnLed.disable();
    digitalWrite(connLed, LOW);
}
void setup()
{

    lcd.init(); // initialize the lcd
    lcd.backlight();
    int8_t *pawaaa;
    lcd.blink_off();
    Serial.begin(115200);
    esp_wifi_set_max_tx_power(50);
    esp_wifi_get_max_tx_power(pawaaa);

    // parsing the config
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B);

    SPIFFS.begin();

    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file");
        lcd.println("Failed to open config file");
    }
    size_t size = configFile.size();
    if (size > 1024)
    {
        Serial.println("Config file size is too large");
    }
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);

    StaticJsonDocument<1024> doc;
    auto error = deserializeJson(doc, buf.get());
    if (error)
    {
        Serial.println("Failed to parse config file");
    }
    const char *MESH_PREFIX = doc["ssid"];
    const char *MESH_PASSWORD = doc["password"];
    const char *ID = doc["id"];
    const char *ROOT = doc["root"];
    const char *MCP = doc["mcp"];
    const char *MFD = doc["mfd"];
    const char *PINS = doc["pins"];
    const char *DELAY = doc["delay"];

    const char *pin1min = doc["pin1min"];
    const char *pin1max = doc["pin1max"];

    relay_pin_0_min = atoi(pin1min);
    relay_pin_0_max = atoi(pin1max);
    Serial.println(relay_pin_0_min);
    Serial.println(relay_pin_0_max);

    const char *pin2min = doc["pin2min"];
    const char *pin2max = doc["pin2max"];

    relay_pin_1_min = atoi(pin2min);
    relay_pin_1_max = atoi(pin2max);
    Serial.println(relay_pin_1_min);
    Serial.println(relay_pin_1_max);

    const char *pin3min = doc["pin3min"];
    const char *pin3max = doc["pin3max"];

    relay_pin_2_min = atoi(pin3min);
    relay_pin_2_max = atoi(pin3max);
    Serial.println(relay_pin_2_min);
    Serial.println(relay_pin_2_max);

    const char *pin4min = doc["pin4min"];
    const char *pin4max = doc["pin4max"];
    relay_pin_3_min = atoi(pin4min);
    relay_pin_3_max = atoi(pin4max);
    Serial.println(relay_pin_3_min);
    Serial.println(relay_pin_3_max);

    const char *pin5min = doc["pin5min"];
    const char *pin5max = doc["pin5max"];

    relay_pin_04_min = atoi(pin5min);
    relay_pin_04_max = atoi(pin5max);
    Serial.println(relay_pin_04_min);
    Serial.println(relay_pin_04_max);

    const char *pin6min = doc["pin6min"];
    const char *pin6max = doc["pin6max"];

    relay_pin_05_min = atoi(pin6min);
    relay_pin_05_max = atoi(pin6max);
    Serial.println(relay_pin_05_min);
    Serial.println(relay_pin_05_max);

    const char *pin7max = doc["pin7max"];
    const char *pin7min = doc["pin7min"];

    relay_pin_06_min = atoi(pin7min);
    relay_pin_06_max = atoi(pin7max);
    Serial.println(relay_pin_06_min);
    Serial.println(relay_pin_06_max);

    const char *pin8max = doc["pin8max"];
    const char *pin8min = doc["pin8min"];

    relay_pin_07_min = atoi(pin8min);
    relay_pin_07_max = atoi(pin8max);
    Serial.println(relay_pin_07_min);
    Serial.println(relay_pin_07_max);

    sendDelay = atoi(DELAY);
    Serial.print("Loaded id: ");

    id = atoi(ID);
    lcd.print("Loaded id: " + id);

    Serial.println(ID);
    // Serial.print("Loaded root id: ");
    root = 3222342507; // atoi(ROOT);
    Serial.println(ROOT);
    mcp = atoi(MCP);
    Serial.println(mcp);

    mfd = atoi(MFD);
    Serial.println(mfd);

    const char *DEV_COUNT = doc["device_count"];
    device_count = atoi(DEV_COUNT);
    Serial.println(device_count);

    const char *MFD_SERIAL_ID_1 = doc["mfd_dev_id_1"];
    mfd_dev_id[0] = atoi(MFD_SERIAL_ID_1);
    Serial.println(mfd_dev_id[0]);

    const char *MFD_SERIAL_ID_2 = doc["mfd_dev_id_2"];
    mfd_dev_id[1] = atoi(MFD_SERIAL_ID_2);
    Serial.println(mfd_dev_id[1]);

    const char *MFD_SERIAL_ID_3 = doc["mfd_dev_id_3"];
    mfd_dev_id[2] = atoi(MFD_SERIAL_ID_3);
    Serial.println(mfd_dev_id[2]);

    const char *MFD_SERIAL_ID_4 = doc["mfd_dev_id_4"];
    mfd_dev_id[3] = atoi(MFD_SERIAL_ID_4);
    Serial.println(mfd_dev_id[3]);

    const char *MFD_SERIAL_ID_5 = doc["mfd_dev_id_5"];
    mfd_dev_id[4] = atoi(MFD_SERIAL_ID_5);
    Serial.println(mfd_dev_id[4]);
    const char *bme_temp_min = doc["bme_min_temp"];
    const char *bme_temp_max = doc["bme_max_temp"];
    const char *bme_hum_min = doc["bme_min_hum"];
    const char *bme_hum_max = doc["bme_max_hum"];
    const char *bme_prs_min = doc["bme_prs_min"];
    const char *bme_prs_max = doc["bme_prs_max"];
    bmeRelay_t_Min = atoi(bme_temp_min);
    bmeRelay_t_Max = atoi(bme_temp_max);
    bmeRelay_p_Min = atoi(bme_prs_min);
    bmeRelay_p_Max = atoi(bme_prs_max);
    bmeRelay_h_Min = atoi(bme_hum_min);
    bmeRelay_h_Max = atoi(bme_hum_max);

    const char *SMB = doc["sensor"];
    smb = atoi(SMB);

    pins = atoi(PINS);
    Serial.print("Loaded MCP pins: ");
    Serial.print(pins);
    Serial.print(" loaded Send Delay: ");
    Serial.print(sendDelay);
    // maintain time in case of wdt reset
    File timeFile = SPIFFS.open("/time.txt", "r");
    if (timeFile)
    {
        String timeTemp = timeFile.readStringUntil('\n');

        ts_epoch = timeTemp.toInt();

        timeFile.close();
    }
    SPIFFS.end();

    //start the mesh
    mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE); // all types on
    uint8_t channel = 13;
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, channel);
    mesh.setContainsRoot(true);

    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onDroppedConnection(&droppedConnection);
    //declarations for scheduler                                                                                                                                                                                                                                                                                          UwU
    userScheduler.addTask(taskSendMessage);
    userScheduler.addTask(taskUpdateTime);
    userScheduler.addTask(taskSendMsgSd);
    userScheduler.addTask(taskConnLed);
    userScheduler.addTask(taskReadMfd);
    userScheduler.addTask(task_Multi_Mfd_Read);
    userScheduler.addTask(taskReadMBE);
    userScheduler.addTask(taskReadMcp);
    userScheduler.addTask(taskUpdateRssi);
    userScheduler.addTask(taskUpdateLcd);
    //tasks to enable
    taskConnLed.enable();
    taskUpdateRssi.enable();

    if (mcp == 1)
    {
        Serial.print("mcp activated");
        taskReadMcp.enable();
    }
    if (smb == 1)
    {
        taskReadMBE.enable();
        if (!bme.begin(0x76))
        {
            Serial.println("Could not find a valid BME280 sensor, check wiring!");
        }
        bme.MODE_FORCED;
    }
    if (mfd == 1)
    {
        taskReadMfd.enable();
    }
    taskUpdateTime.enable();

    //set IO pins
    pinMode(A0, INPUT);           // Define A0 pin as INPUT
    pinMode(LED_BUILTIN, OUTPUT); // Define LED_BUILTIN as OUTPUT
    digitalWrite(LED_BUILTIN, HIGH);
    //pinMode(sendLed, OUTPUT);
    pinMode(connLed, OUTPUT);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW); // Initially the LED will be off
    xTaskCreatePinnedToCore(meshUpdate, "meshTask", 40000, NULL, 3, NULL, 0);
    lcd.clear();    lcd.print("Hetadatain");

    delay(100);
    
    lcd.setCursor(3, 1);
    lcd.print("Spyder Eye");
    lcd.setCursor(10, 1);
    delay(300);

    lcd.print(".");
    delay(300);

    lcd.setCursor(11, 1);
    lcd.print(".");
    delay(300);

    lcd.setCursor(12, 1);
    lcd.print(".");
    delay(300);
    lcd.setCursor(10, 1);
    lcd.print("EYE");
    lcd.blink_on();
    delay(300);
    button.begin();
    // Add the callback function to be called when the button is pressed.
    button.onPressed(lcdInfo);
    // xTaskCreate(lcdInfo, "show lcd shiet", 16000, NULL, 2, NULL);
    taskUpdateLcd.enable();
}
void meshUpdate(void *random)
{

    for (;;)
    {
        mesh.update();
        button.read();
        delay(10);
    }
}
void loop()
{
    // it will run the  scheduler as well

    //updateRssi(); //maintains the led flash frequency
    userScheduler.execute();
    //watchdog
    if (wdt == 180)
    {
        writeTimeToCard();
        while (1)
            ;
    }

    //periodic restart to avoid memory fragmentation
    if (rebootTime == 86400)
    {
        writeTimeToCard();
        ESP.restart();
    }
}
//for getting the adc data
void readMcp()
{
    digitalWrite(connLed, HIGH);
    MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
    int mcpVals[pins];
    String msgMcp;

    for (int i = 0; i < 8; i++)
    {
        mcpVals[i] = 0;
    }
    for (int i = 0; i < 10; i++)
    {

        for (int j = 0; j < 8; j++)
        {

            mcpVals[j] += adc.readADC(j);
        }
        delay(200);
    }

    for (int j = 0; j < 8; j++)
    {
        int k = mcpVals[j];
        mcpVals[j] = k / 10;
    }

    msgMcp += String(ts_epoch) + String(",") + String(id.c_str());

    msgMcp.concat(",");
    msgMcp.concat("6");

    if (pins == 1 || pins > 1)
    {

        msgMcp.concat(",");
        msgMcp.concat(mcpVals[0]);
        trig_Relay(mcpVals[0], relay_pin_0_max, relay_pin_0_min);
    }
    if (pins == 2 || pins > 2)
    {

        msgMcp += String(",") + String(mcpVals[1]);
        trig_Relay(mcpVals[1], relay_pin_1_max, relay_pin_1_min);
    }
    if (pins == 3 || pins > 3)
    {
        msgMcp += String(",") + String(mcpVals[2]);
        trig_Relay(mcpVals[2], relay_pin_2_max, relay_pin_2_min);
    }
    if (pins == 4 || pins > 4)
    {

        msgMcp += String(",") + String(mcpVals[3]);
        trig_Relay(mcpVals[3], relay_pin_3_max, relay_pin_3_min);
    }
    if (pins == 5 || pins > 5)
    {

        msgMcp += String(",") + String(mcpVals[4]);
        trig_Relay(mcpVals[4], relay_pin_04_max, relay_pin_04_min);
    }
    if (pins == 6 || pins > 6)
    {

        msgMcp += String(",") + String(mcpVals[5]);
        trig_Relay(mcpVals[5], relay_pin_05_max, relay_pin_05_min);
    }
    if (pins == 7 || pins > 7)
    {

        msgMcp += String(",") + String(mcpVals[6]);
        trig_Relay(mcpVals[6], relay_pin_06_max, relay_pin_06_min);
    }
    if (pins == 8 || pins > 8)
    {

        msgMcp += String(",") + String(mcpVals[7]);
        trig_Relay(mcpVals[7], relay_pin_07_max, relay_pin_07_min);
    }
    sendPayload(msgMcp);
}
//for triggering the relay based on adc parameters
boolean trig_Relay(int thres, int relay_max, int relay_min)
{
    if (thres >= relay_max || thres <= relay_min)
    {
        digitalWrite(relayPin, 0x00);
        return true;
    }
    else
    {
        digitalWrite(relayPin, 0x01);
    }
}

//to dump data from internal storage
void sendMsgSd()
{
    if (mesh.isConnected(root))
    {
        SPIFFS.begin();

        File file = SPIFFS.open("/offlinelog.txt", "r"); // FILE_READ is default so not realy needed but if you like to use this technique for e.g. write you need FILE_WRITE
                                                         //#endif
        if (!file)
        {
            Serial.println("Failed to open file for reading");
            taskSendMsgSd.disable();
            pos = 0;
            timeIndex = 0;
            return;
        }
        String buffer;
        for (int i = 0; i < 1; i++)
        {
            file.seek(pos);
            buffer = file.readStringUntil('\n');
            msgSd = buffer;
            if (buffer != "")
            {

                mesh.sendSingle(root, msgSd);
                Serial.println(msgSd);
                pos = file.position();
            }
            file.close();
            Serial.println(F("DONE Reading"));
        }
        if (buffer == "")
        {
            String ackMsg = id;
            ackMsg += "sent from sd card";
            mesh.sendSingle(root, ackMsg);
            SPIFFS.remove("/offlinelog.txt");
            taskSendMsgSd.disable();
        }
        SPIFFS.end();
    }
}

void writeTimeToCard()
{
    SPIFFS.begin();
    SPIFFS.remove("/time.txt");
    File dataFile = SPIFFS.open("/time.txt", "w");
    if (dataFile)
    {
        dataFile.println(ts_epoch);
        dataFile.close();
    }
    else
    {
        Serial.println("error opening  time.txt");
    }
    SPIFFS.end();
}
//to blink the LED
void blink_con_led()
{

    if (led_active == true)
    {
        digitalWrite(connLed, HIGH);
        delay(led_refresh);
        digitalWrite(connLed, LOW);

        led_active = false;
    }

    else
    {
        digitalWrite(connLed, LOW);
        delay(led_refresh);
        digitalWrite(connLed, HIGH);
        led_active = true;
    }
}
//bit-banging for mfd data
int bin2dec(String sb)
{
    int rem, dec_val = 0, base = 1, expo = 0, temp = 0, i = 0;
    for (i = sb.length() - 1; i >= 0; i--)
    {
        if (sb[i] == '0')
            temp = 0;
        else
        {
            if (sb[i] == '1')
                temp = 1;
        }
        dec_val = dec_val + temp * base;
        base = base * 2;
    }
    return dec_val;
}

String dec2binary(int x)
{
    int num = x;
    uint8_t bitsCount = 16;
    char str[bitsCount + 1];
    uint8_t i = 0;
    while (bitsCount--)
        str[i++] = bitRead(num, bitsCount) + '0';
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
    String bin0 = dec2binary(int(two));

    String hexa1 = String(one, HEX);
    String bin1 = dec2binary(int(one));

    for (i = 0; i <= 7; i++)
    {
        first[i] = bin0[i];
    }

    int aaaa = first[0];

    if (aaaa != '1')
    {
        SIGN = 1;
    }
    else
    {
        SIGN = -1;
    }

    for (j = 8; j <= 15; j++)
    {
        second1[j] = bin0[j];
    }
    for (int d3 = 0; d3 <= 7; d3++)
    {
        third[d3] = bin1[d3];
    }
    for (int z = 8; z <= 15; z++)
    {
        last[z] = bin1[z];
    }

    String a = String(bin0[1]);
    String b = String(bin0[2]);
    String c = String(bin0[3]);
    String d = String(bin0[4]);
    String e = String(bin0[5]);
    String f = String(bin0[6]);
    String g = String(bin0[7]);
    String h = String(bin0[8]);
    String step1 = String((a) + (b) + (c) + (d) + (e) + (f) + (g) + (h));

    String j2 = String(bin0[9]);
    String k2 = String(bin0[10]);
    String l2 = String(bin0[11]);
    String m2 = String(bin0[12]);
    String n2 = String(bin0[13]);
    String o2 = String(bin0[14]);
    String p2 = String(bin0[15]);
    String step2 = String((j2) + (k2) + (l2) + (m2) + (n2) + (o2) + (p2));

    int decimal = bin2dec(String(step1));

    EXPONENT = decimal - 127;

    String a1 = String(bin1[0]);
    String b1 = String(bin1[1]);
    String c1 = String(bin1[2]);
    String d1 = String(bin1[3]);
    String e1 = String(bin1[4]);
    String f1 = String(bin1[5]);
    String g1 = String(bin1[6]);
    String h1 = String(bin1[7]);
    String step3 = String((a1) + (b1) + (c1) + (d1) + (e1) + (f1) + (g1) + (h1));

    String x2 = String(bin1[8]);
    String q2 = String(bin1[9]);
    String r2 = String(bin1[10]);
    String s2 = String(bin1[11]);
    String t2 = String(bin1[12]);
    String u2 = String(bin1[13]);
    String v2 = String(bin1[14]);
    String w2 = String(bin1[15]);
    String step4 = String((x2) + (q2) + (r2) + (s2) + (t2) + (u2) + (v2) + (w2));

    String combined = String((step2) + (step3) + (step4));

    float dec1 = bin2dec(String(step2));
    float dec2 = bin2dec(String(step3));
    float dec3 = bin2dec(String(step4));
    float sum = ((dec1)*256 * 256) + ((dec2)*256) + (dec3);

    if (EXPONENT == 0)
    {
        division = (sum / 4194304);
    }
    else
    {
        division = (sum / 8388608);
    }

    float MANTISSA = division + 1;
    double Power = pow(2, EXPONENT);
    double FLOAT = SIGN * MANTISSA * Power;

    return (FLOAT);
}

int validDenominator(int number)
{
    if (number == 0 || isnan(number))
        return 1;
    else
        return number;
}

double readWattageR(int add)
{

    if (dataStream(add) == true)
    {
        double Wattage = NAN;
        Wattage = RSmeter(first_Reg, second_Reg);
        double WATT = Wattage;
        return WATT;
    }
}

//getting data from mfd
bool dataStream(int one)
{
    first_Reg = 0;
    second_Reg = 0;
    node.clearResponseBuffer();
    delay(40);
    int result = node.readHoldingRegisters(one, 2);
    delay(40);
    if (result == node.ku8MBSuccess)
    {
        first_Reg = node.getResponseBuffer(0);
        second_Reg = node.getResponseBuffer(1);
        return true;
    }
    else
    {
        node.clearResponseBuffer();
        delay(60);
        result = node.readHoldingRegisters(one, 2);
        if (result == node.ku8MBSuccess)
        {

            first_Reg = node.getResponseBuffer(0);
            second_Reg = node.getResponseBuffer(1);
        }
        else
        {
            node.clearResponseBuffer();
            delay(60);
            result = node.readHoldingRegisters(one, 2);
            if (result == node.ku8MBSuccess)
            {

                first_Reg = node.getResponseBuffer(0);
                second_Reg = node.getResponseBuffer(1);
            }
            else
            {
                //   mfd_read_pos++;
            }
        }
        return true;
    }
}
// MFD data to send
String readMfd(int &mfd_dev_id)
{
    digitalWrite(connLed, HIGH);

    node.begin(mfd_dev_id, Serial2);
    String msgMfd;

    msgMfd = String(time_to_print);
    msgMfd += "," + id;
    msgMfd += "," + String(7);
    msgMfd += "," + String(mfd_dev_id);
    msgMfd += "," + String(1);
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

//second part of the MFD data
String readMfd2(int &mfd_dev_id)
{
    digitalWrite(connLed, HIGH);

    String msgMfd2 = String(time_to_print) + "," + id + "," + String(7);
    msgMfd2 += "," + String(mfd_dev_id);
    msgMfd2 += "," + String(2);
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
}

boolean read_Mfd_Task()
{
    MCP_Sent = false;

    task_Multi_Mfd_Read.enable();
    time_to_print = ts_epoch;

    return true;
}

void multi_mfd_read()
{
    time_to_print++; //set the time for mfd data to be in sync
    Serial2.begin(9600, SERIAL_8E1);
    msgMfd_payload = readMfd(mfd_dev_id[mfd_read_pos]);
    msgMfd_payload1 = readMfd2(mfd_dev_id[mfd_read_pos]);
    sendMFD();

    Serial2.end();

    mfd_read_pos++;
    if (mfd_read_pos >= device_count)
    {
        task_Multi_Mfd_Read.disable();
        mfd_read_pos = 0;
    }
}

//For sending bmp280 data
void mbe()
{
    dht.begin();
    bme.takeForcedMeasurement();
    int prs = (bme.readPressure() * 0.01) * 10.197162129779;
    String readMbe;
    if (ts_epoch < 1611040428)
    {
        readMbe.concat("notime");
        readMbe.concat(",");
    }
    readMbe.concat(String(ts_epoch));
    readMbe.concat(",");
    readMbe.concat(id);
    readMbe.concat(",");
    readMbe.concat("8");
    /*readMbe.concat(",");
 readMbe.concat(String(bme.readTemperature()));
 readMbe.concat(",");
 readMbe.concat(String(bme.readHumidity()));
 readMbe.concat(",");
 readMbe.concat(String((bme.readPressure()*0.01 )*10.197162129779)); */
    readMbe.concat(",");
    readMbe.concat(String(bme.readTemperature()));
    readMbe.concat(",");
    readMbe.concat(String(bme.readHumidity()));
    readMbe.concat(",");
    readMbe.concat(String((bme.readPressure() * 0.01) * 10.197162129779));
    readMbe.concat(",");
    if (trig_Relay(bme.readTemperature(), bmeRelay_t_Max, bmeRelay_t_Min))
    {
        readMbe.concat("1");
    }
    else
    {
        readMbe.concat("0");
    }
    readMbe.concat(",");
    if (trig_Relay(bme.readHumidity(), bmeRelay_h_Max, bmeRelay_h_Min))
    {
        readMbe.concat("1");
    }
    else
    {
        readMbe.concat("0");
    }
    readMbe.concat(",");
    if (trig_Relay(prs, bmeRelay_p_Max, bmeRelay_p_Min))
    {
        readMbe.concat("1");
    }
    else
    {
        readMbe.concat("0");
    }
    readMbe.concat(",");
    if (trig_Relay(bme.readTemperature(), bmeRelay_t_Max, bmeRelay_t_Min) || trig_Relay(bme.readHumidity(), bmeRelay_h_Max, bmeRelay_h_Min) || trig_Relay(prs, bmeRelay_p_Max, bmeRelay_p_Min))
    {
        digitalWrite(relayPin, HIGH);
    }
    else
    {
        digitalWrite(relayPin, LOW);
    }
    sendPayload(readMbe);
}
void sendMFD()
{
    //digitalWrite(sendLed, HIGH);
    sendPayload(msgMfd_payload);
    sendPayload(msgMfd_payload1);
}
//writing data to card
void saveToCard(String &payload)
{
    SPIFFS.begin();
    File dataFile = SPIFFS.open("/offlinelog.txt", "a");
    dataFile.println(payload);
    dataFile.close();
    SPIFFS.end();
}
//sending data to root
void sendPayload(String &payload)
{
    Serial.println(payload);
    if (mesh.isConnected(root))
    {
        // digitalWrite(sendLed, HIGH);
        taskSendMsgSd.enable();
        mesh.sendSingle(root, String(payload));
        if (WiFi.RSSI() <= (-90))
        {
            mesh.sendSingle(root, (id + ("signal Low ")));
            esp_wifi_set_max_tx_power(80);

            delay(10);
        }
    }
    else
    {
        saveToCard(payload);
    }
    wdt = 0;
}

//to set blink frequency based on signal strength
void updateRssi()
{

    rssi = WiFi.RSSI();
    rssi = rssi * (-1);

    if (rssi >= 40 && rssi < 45)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 300);
    }
    else if (rssi >= 46 && rssi < 50)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 500);
    }
    else if (rssi >= 51 && rssi < 55)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 500);
    }
    else if (rssi >= 56 && rssi < 60)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 700);
    }
    else if (rssi >= 61 && rssi < 65)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 700);
    }
    else if (rssi >= 66 && rssi < 70)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 700);
    }
    else if (rssi >= 71 && rssi < 76)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 1000);
    }
    else if (rssi >= 81 && rssi < 86)
    {
        taskConnLed.setInterval(TASK_MILLISECOND * 1300);
    }
    else if (rssi > 91)
    {
        digitalWrite(connLed, LOW);
    }
}
//for triggering the relay based on adc parameters
boolean trig_Relay_bme(int thres, int relay_max, int relay_min)
{
    if (thres >= relay_max || thres <= relay_min)
    {
        digitalWrite(relayPin, 0x00);
    }
    else
    {
        digitalWrite(relayPin, 0x01);

        return true;
    }
}
boolean infoNow = true;
void lcdInfo()
{
}
void lcdShiet()
{
   

    if (infoNow == true)
    {
        infoNow = false;
        if (meshAlive || WiFi.RSSI() !=0)
        {

            lcd.blink_off();
            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("Connected To : ");
            lcd.setCursor(3, 1);
            if (connectedNode > 20 || connectedNode == 0)
            {
                lcd.print("ROOT NODE");
            }
            else
            {
                lcd.print(String(connectedNode));
            }
        }
        else
        {
            lcd.clear();
            lcd.print("Searching");
        }
    } else
        {
            infoNow = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Spyder EYE");
            lcd.setCursor(0, 1);
            lcd.print("Signal :");
            lcd.setCursor(9, 1);
            lcd.print(String(WiFi.RSSI()) + "db");
          /*  lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Free Ram :");
            lcd.setCursor(3, 1);
            lcd.print(String(ESP.getFreeHeap()));*/
        }
}
