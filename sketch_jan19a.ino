/*
  Project: Interfacing SIM7600 GSM Module with Maker UNO

  Item used:
  - Maker UNO https://www.cytron.io/p-maker-uno
  - SIM7600 https://www.cytron.io/p-4g-3g-2g-gsm-gprs-gnss-hat-for-raspberry-pi
*/

#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4); 



#define GSM_RX  10  
#define GSM_TX  9
#define GSM_PWR 11
#define GSM_RST 20 // Dummy
#define GSM_BAUD  9600
int LEDpin = 7;
int BuzzerPin = 6;
int gasDetect = false;
char replybuffer[255];

SoftwareSerial SIM7600SS = SoftwareSerial(GSM_TX, GSM_RX);
SoftwareSerial *SIM7600Serial = &SIM7600SS;

Adafruit_FONA SIM7600 = Adafruit_FONA(GSM_RST);




uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
char SIM7600InBuffer[64]; // For notifications from the FONA
char callerIDbuffer[32]; // We'll store the SMS sender number in here
char SMSbuffer[32]; // We'll store the SMS content in here
uint16_t SMSLength;
String SMSString = "";
boolean buttonEnable = false;

void setup()
{

  pinMode(GSM_PWR, OUTPUT);
  delay(1000);
  pinMode(GSM_PWR, INPUT_PULLUP);
  pinMode(LEDpin,OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("Interfacing SIM7600 GSM Module with Maker UNO");
  Serial.println("Initializing... (May take a minute)");
  lcd.begin();                      // initialize the lcd 
  //  lcd.init();
    // Print a message to the LCD.
//  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Initializing..");
  lcd.setCursor(1,1);
  lcd.print("Please Wait");
  
//  playStartMelody();
  delay(15000);
  lcd.clear();
  // Make it slow so its easy to read!
  SIM7600Serial->begin(GSM_BAUD);
  if (!SIM7600.begin(*SIM7600Serial)) {
    Serial.println("Couldn't find SIM7600");
    lcd.setCursor(1,0);
    lcd.print("ERROR");
    lcd.setCursor(1,1);
    lcd.print("Please Reset");
   
    while (1);
  }
  Serial.println(F("SIM7600 is OK"));
    

  // Print SIM card IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = SIM7600.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  }

//  SIM7600Serial->print("AT+CMGD=1,4");

  SIM7600Serial->print("AT+CNMI=2,1\r\n");  // Set up the SIM800L to send a +CMTI notification when an SMS is received
  SIM7600Serial->print("AT+CMGF=1");
  Serial.println("GSM is ready!");
  SIM7600.sendSMS("+6281328775858", "ready");
  delay(1000);
}

void loop()
{
  char* bufPtr = SIM7600InBuffer;
  float sensor_volt; 
 float RS_gas; 
 float ratio; 
 float R0 = 0.19; 
 boolean smokeDetect = false;
 int sensorValue = analogRead(A0); 
 sensor_volt = ((float)sensorValue / 1024) * 5.0; 
 RS_gas = (5.0 - sensor_volt) / sensor_volt; 
 ratio = RS_gas / R0; // ratio = RS/R0 
////------------------------------------------------------------/ 
 Serial.print("sensor_volt = "); 
 Serial.println(sensor_volt); 
 Serial.print("RS_ratio = "); 
 Serial.println(RS_gas); 
 Serial.print("Rs/R0 = "); 
 Serial.println(ratio); 
 Serial.print("\n\n"); 
 
 String stringRatio = String(ratio);
 lcd.setCursor(1,0);
 lcd.print("arara");
 lcd.setCursor(1,1);
 lcd.print("Ratio: "+stringRatio);
  delay(3000); 
  if(ratio < 3.5){
  smokeDetect = true;
  
 }
if(ratio > 3.5){
  smokeDetect = false;
 }
 if(smokeDetect==true){
  gasDetect = true;
  
 }else{
   gasDetect = false;
//  tone(BuzzerPin, 100);
 }

 if(gasDetect==true){
  digitalWrite(LEDpin, HIGH);
  tone(BuzzerPin, 100);
  SIM7600.callPhone("+6281328775858");


 }else{
  digitalWrite(LEDpin, LOW);
  noTone(BuzzerPin);
 
   Serial.print("No gas detected"); 
 }
  if (SIM7600.available()) {
 int slot = 0; // This will be the slot number of the SMS
    int charCount = 0;

    // Read the notification into fonaInBuffer
    do {
      *bufPtr = SIM7600.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (SIM7600.available()) && (++charCount < (sizeof(SIM7600InBuffer) - 1)));

    // Add a terminal NULL to the notification string
    *bufPtr = 0;
    if (1 == sscanf(SIM7600InBuffer, "+CMTI: \"SM\",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);
      if(SIM7600.readSMS(slot, SMSbuffer, 250, &SMSLength)){
      SMSString = String(SMSbuffer);
      Serial.print("SMS: "); Serial.println(SMSString);
    }
    if (SMSString == "Status") {
        Serial.print("Setting LED 13 to on.");
        digitalWrite(13, HIGH);
        digitalWrite(13, HIGH);
        tone(12, 100);
        delay(100);
      }
      if (SMSString == "Off") {
        Serial.print("Setting LED 13 to on.");
        digitalWrite(13, LOW);
        digitalWrite(13, LOW);
        noTone(12);
        delay(100);
      }
    }

  }
}
