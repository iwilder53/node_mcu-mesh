

#include <SPI.h>
#include <SD.h>


const int chipSelect = D8;

File testfile;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("\r\nWaiting for SD card to initialise...");
  if (!SD.begin(chipSelect)) { // CS is D8 in this example
    Serial.println("Initialising failed!");
    return;
  }
  Serial.println("Initialisation completed");
}


void loop() {
  // put your main code here, to run repeatedly:
Serial.println("Open a file called 'testfile.txt' and read it");
  testfile = SD.open("offlinelog.txt", FILE_READ); // FILE_WRITE opens file for writing and moves to the end of the file
  while (testfile.available()) {
    Serial.write(testfile.read());
  }
  Serial.println("\r\nCompleted reading from the file\r\n");
  // close the file:
}
