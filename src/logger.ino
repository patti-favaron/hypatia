/*
 * Position-aware data logger based on Arduino
 * Nano 33 BLE Sense, for testing the turbulence
 * vs pressure hypothesis.
 *
 * By: Patti Favaron
 */

////////////////////////////////////////////
/////////// Static Configuration ///////////
////////////////////////////////////////////

#include "Arduino.h"

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 1000;

// Number of "loop()" steps at which informational messages are logged
// (warning messages are emitted everywhen needed).
const int STEPS_TO_LOG = 10;
int iNumSteps = 1;

////////////////////////////////////////////////////////
////////// Initialization of critical devices //////////
////////////////////////////////////////////////////////

// MicroSD card reader
#include <SPI.h>
#include <SD.h>

// I2C
#include <Wire.h>

// GPS u-Blox NEO-M9N
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;
int yr;
byte mo, dy, hr, mi, se;
double lat, lon, alt, hAccuracy, vAccuracy;

//------------------------------------------------------------------------------
// MicroSD-related
const uint8_t chipSelect = 10;
/*
SdFat sd;
SdFile file;
SdFile logfile;
 */

char fileName[13] = "M0000000.csv";
char logName[13]  = "L0000000.log";
const int buttonPin = 3;
int buttonState = 0;

////////////////////////////////////////////////////////////
////////// Initialization of non-critical devices //////////
////////////////////////////////////////////////////////////

// Thermo-hygrometer + barometer/altimeter (BME-280)
#include "SparkFunBME280.h"

#define CELSIUS_SCALE 0 //Default
#define FAHRENHEIT_SCALE 1

BME280 mySensor;
double Ta, HRel, Pa;

// Actual time
uint32_t logTime;

////////////////////////////////////////
////////// Internal functions //////////
////////////////////////////////////////

//==============================================================================
// Communicate an error through blink
void error(uint8_t err_no) {
  while(1) {
    uint8_t i;
    for(i=0; i<err_no; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    delay(900);
  }
}


// Communicate a warning through blink
void warning(uint8_t err_no) {
  uint8_t i;
  for(i=0; i<err_no; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(75);
    digitalWrite(LED_BUILTIN, LOW);
    delay(225);
  }
  delay(775);
}


// Start GNSS
void startGNSS(uint16_t maxWait) {

  // Start GNSS
  while (myGNSS.begin() == false) //Try connecting the u-blox module using Wire port; call may fail if u-Blox boot seq is not complete
  {
    Serial.println(F("u-blox GNSS looking for some valid data."));
    warning((uint8_t)2);
  }
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

  // Wait until the GNSS has found a valid date-time
  digitalWrite(LED_BUILTIN, LOW);
  while (myGNSS.getTimeValid() == false) {
    Serial.println(F("Waiting valid GPS time before to start"));
    warning((uint8_t)3);
  }
  Serial.println("Valid GPS time received, go for startup sequence");

  // Get current date and time, for setting SD file up
  bool bRetCode = myGNSS.getPVT(maxWait);
  if(bRetCode) {
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.year = false;
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.month = false;
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.day = false;
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.hour = false;
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.min = false;
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.sec = false;
    myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.all = false;
    yr = myGNSS.packetUBXNAVPVT->data.year;
    mo = myGNSS.packetUBXNAVPVT->data.month;
    dy = myGNSS.packetUBXNAVPVT->data.day;
    hr = myGNSS.packetUBXNAVPVT->data.hour;
    mi = myGNSS.packetUBXNAVPVT->data.min;
    se = myGNSS.packetUBXNAVPVT->data.sec;
  }
  else {
    yr = 9999;
    mo = 99;
    dy = 99;
    hr = 99;
    mi = 99;
    se = 99;
  }

}


// Get GNSS time and positional information
bool getGNSSdata(uint16_t maxWait) {

  // Try getting last position info from GNSS
  bool bRetCode = myGNSS.getPVT(maxWait);
  if(!bRetCode) {
    yr = 9999;
    mo = 99;
    dy = 99;
    hr = 99;
    mi = 99;
    se = 99;
    lat = -9999.9;
    lon = -9999.9;
    alt = -9999.9;
    hAccuracy = -9999.9;
    vAccuracy = -9999.9;
    return false;
  }

  // Dispatch time information to relevant vars
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.year = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.month = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.day = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.hour = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.min = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.sec = false;
  yr = myGNSS.packetUBXNAVPVT->data.year;
  mo = myGNSS.packetUBXNAVPVT->data.month;
  dy = myGNSS.packetUBXNAVPVT->data.day;
  hr = myGNSS.packetUBXNAVPVT->data.hour;
  mi = myGNSS.packetUBXNAVPVT->data.min;
  se = myGNSS.packetUBXNAVPVT->data.sec;
  
  // Dispatch position information to relevant positional components
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.lat = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.lon = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.hMSL = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.hAcc = false;
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.vAcc = false;
  lat = (double)myGNSS.packetUBXNAVPVT->data.lat * 1.e-7;
  lon = (double)myGNSS.packetUBXNAVPVT->data.lon * 1.e-7;
  
  alt = (double)myGNSS.packetUBXNAVPVT->data.hMSL * 1.e-3;
  hAccuracy = (double)myGNSS.packetUBXNAVPVT->data.hAcc * 1.e-3;
  vAccuracy = (double)myGNSS.packetUBXNAVPVT->data.vAcc * 1.e-3;

  // Leave - Success
  myGNSS.packetUBXNAVPVT->moduleQueried.moduleQueried1.bits.all = false;
  return true;
  
}


// Internal RTC programming
void setDateTime(void) {
  
}


// Start SD card and set next data file name
void startSD(void) {
  
  // Initialize SPI at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    error((uint8_t)4);
  }

  newFile();

}


void newFile(void) {
  
  // Find first unused file name.
  int iNextFile = -1;
  for(int iFile = 0; iFile < 100000; iFile++) {
    sprintf(fileName, "M%7.7d.csv", iFile);
    sprintf(logName, "L%7.7d.log", iFile);
    if(!SD.exists(fileName) && !SD.exists(logName)) {
      iNextFile = iFile;
      break;
    }
  }
  if(iNextFile  < 0) {
    error((uint8_t)5);
  }

  // Access file
  File file = SD.open(fileName, FILE_WRITE);
  if(!file) {
    error((uint8_t)6);
  }
  file.close();
  delay(500);
  File logfile = SD.open(logName, FILE_WRITE);
  if(!logfile) {
    error((uint8_t)7);
  }
  logfile.close();
  Serial.println("SD files opened");
  Serial.print("Writing data to: ");
  Serial.println(fileName);
  Serial.print("Writing logs to: ");
  Serial.println(logName);
  digitalWrite(LED_BUILTIN, HIGH);

  // Write data header.
  file = SD.open(fileName, FILE_WRITE);
  file.println("Time.Stamp,Lat,Lon,Z,hAcc,vAcc,Pa,Ta,HRel");
  file.close();

}


void writeData(void) {

  if(yr < 9999) {
    File file = SD.open(fileName, FILE_WRITE);
    char line[256];
    sprintf(line, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", yr, mo, dy, hr, mi, se);
    file.print(line);
    file.print(","); file.print(lat, 7);
    file.print(","); file.print(lon, 7);
    file.print(","); file.print(alt, 3);
    file.print(","); file.print(hAccuracy, 3);
    file.print(","); file.print(vAccuracy, 3);
    file.print(","); file.print(Pa, 3);
    file.print(","); file.print(Ta, 3);
    file.print(","); file.print(HRel, 2);
    file.println();
    file.close();
  }
    
}


void logInfo(const char* sMessage) {
  File logfile = SD.open(logName, FILE_WRITE);
  char line[256];
  sprintf(line, "F(%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d)", yr, mo, dy, hr, mi, se);
  logfile.print(line);
  logfile.print(F(" - Info    - "));
  logfile.println(sMessage);
  logfile.close();
}


void logWarning(const char* sMessage) {
  File logfile = SD.open(logName, FILE_WRITE);
  char line[256];
  sprintf(line, "F(%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d)", yr, mo, dy, hr, mi, se);
  logfile.print(line);
  logfile.print(F(" - Warning - "));
  logfile.println(sMessage);
  logfile.close();
}


void get_pressure(void) {
  
  Ta = mySensor.readTempC();
  HRel = mySensor.readFloatHumidity();
  Pa = mySensor.readFloatPressure() / 100.0;
  
}


/////////////////////////////
////////// setup() //////////
/////////////////////////////

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(buttonPin, INPUT);
  
  Serial.begin(9600);
  // while(!Serial);

  Wire.begin();
  mySensor.setI2CAddress(0x76);

  // Start GNSS unit - waiting until it's fully ON
  startGNSS((uint16_t)250);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set internal processor RTC
  setDateTime();
  
  // Start SD
  startSD();
  digitalWrite(LED_BUILTIN, HIGH);
  logInfo("Acquisition started, with GNSS time set");
  
  // Start BME-280
  if (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("The sensor did not respond. Please check wiring.");
    while(1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }; //Freeze
  }
  digitalWrite(LED_BUILTIN, HIGH);

  // Start on a multiple of the sample interval.
  logTime = micros()/(1000UL*SAMPLE_INTERVAL_MS) + 1;
  logTime *= 1000UL*SAMPLE_INTERVAL_MS;
  digitalWrite(LED_BUILTIN, LOW);
    
}



////////////////////////////
////////// loop() //////////
////////////////////////////

void loop() {

  // Time for next record.
  logTime += 1000UL*SAMPLE_INTERVAL_MS;

  // Wait for log time.
  int32_t diff;
  do {
    diff = micros() - logTime;
  } while (diff < 0);

  // Update steps-to-log counter
  bool lLogInfo;
  iNumSteps++;
  lLogInfo = iNumSteps == STEPS_TO_LOG;
  if(lLogInfo) iNumSteps = 1;

  // Get button state and check for possible transition
  buttonState = digitalRead(buttonPin);
  if(buttonState == 0) {
    
    // No pushbutton pressed, and writing file: get measurements
    digitalWrite(LED_BUILTIN, LOW);
    
    // Query GPS for position and time reference data.
    getGNSSdata((uint16_t)250);
  
    // Get pressure (and its auxiliary quantities) from external BMP180
    get_pressure();

    // Log data to MicroSD
    digitalWrite(LED_BUILTIN, HIGH);
    writeData();
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
  
    // Dump data to serial line
    char line[256];
    sprintf(line, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", yr, mo, dy, hr, mi, se);
    Serial.print(line); Serial.print(",");
    Serial.print(hAccuracy); Serial.print(",");
    Serial.print(vAccuracy); Serial.print(",");
    Serial.print(Ta); Serial.print(",");
    Serial.print(HRel); Serial.print(",");
    Serial.print(Pa);
    Serial.print("\n");

  }
  else {
    
    // Button pressed: LED on, and stop processing: will restart
    // after power off->on or RESET.
    digitalWrite(LED_BUILTIN, HIGH);
    while(1);
    
  }

}
