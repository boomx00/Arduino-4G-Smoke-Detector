#include <SoftwareSerial.h>

//initialize GSM pins
#define GSM_RX  10  
#define GSM_TX  9
#define GSM_PWR 11
#define GSM_BAUD  9600



//initialize output pins for buzzer and LED
int LEDpinB = 8; //blue LED PIN
int LEDpinR = 7; //red LED PIN
int LEDpinG = 6; //green LED PIN

int BuzzerPin = 5; //buzzer PIN

//gas detect tracker

String calib = "waiting";
int passFlag = 0;

//initialize variable for it to communicate with serial monitor
SoftwareSerial SIM7600SS = SoftwareSerial(GSM_TX, GSM_RX);
SoftwareSerial *SIM7600Serial = &SIM7600SS;

 
char SIM7600Buffer[64]; //


void sendSms(String text){
    SIM7600Serial->println("AT+CMGS=\"+xxxxxx\"");

    delay(1000);

  SIM7600Serial->println(text);
  Serial.println("send SMS");
  SIM7600Serial->write(0x1A);
}
void Call(){
      SIM7600Serial->println("ATD+xxxxxxx;");

    Serial.println("make Call");
}
void readStatusSms(uint8_t slt, String sensorValue)
{
      SIM7600Serial->print("AT+CMGR=");
      SIM7600Serial->println(slt);
      if (SIM7600Serial->find("Status")){
        String warning = "Current environment ratio: ";
        warning += sensorValue;
        sendSms(warning);
      }
       
}
void readTestSms(uint8_t slt)
{
    bool test = true;
      SIM7600Serial->print("AT+CMGR=");
      SIM7600Serial->println(slt);
         if (SIM7600Serial->find("Test")){
        
        digitalWrite(LEDpinR, HIGH);
            tone(BuzzerPin, 100);
              delay(2000);

          digitalWrite(LEDpinR, LOW);
            noTone(BuzzerPin);

      }
}

void begin(){
   int16_t timeout = 10000;
  bool success = false;
  while (timeout > 0) {
     SIM7600Serial->println("AT");
if (SIM7600Serial->find("OK")){
  Serial.println("GSM SUCCESS");
  success=true;
  break;
}
delay(500);
    timeout -= 500;
  }
  if(success==false){
    Serial.println("ERROR");
    digitalWrite(LEDpinR, HIGH);
    while(1){
      
    }
  }
    
}

void setup()
{

  pinMode(GSM_PWR, OUTPUT);
  delay(1000);
  pinMode(GSM_PWR, INPUT_PULLUP);
  pinMode(LEDpinB,OUTPUT);
    pinMode(LEDpinR,OUTPUT);
  pinMode(LEDpinG,OUTPUT);

  pinMode(BuzzerPin, OUTPUT);
  Serial.begin(115200);
  
  Serial.println("Initializing... (May take a minute)");
  digitalWrite(LEDpinB, HIGH);
  digitalWrite(LEDpinR, HIGH);
  digitalWrite(LEDpinG, HIGH);
//  playStartMelody();
 
  SIM7600Serial->begin(GSM_BAUD);
    delay(15000);

  begin();
  Serial.println(F("SIM7600 is OK"));

    
  // Print SIM card IMEI number.
 


  SIM7600Serial->print("AT+CNMI=2,1\r\n");  // send a +CMTI notification when an SMS is received
  SIM7600Serial->print("AT+CMGF=1");  //set operating mode of gsm module to sms text mode instead of PDU 
 

  Serial.println("GSM is ready!");
  delay(1000);

  while(calib=="waiting"){
    int sensorValue = analogRead(A0); 
     if(sensorValue>300){
          Serial.println(sensorValue);
    }else{
      calib="done";
      digitalWrite(LEDpinG, HIGH);

  }
  }
   digitalWrite(LEDpinB, LOW);
   digitalWrite(LEDpinR, LOW);
    sendSms("ready");
    Call();
}

void loop()
{
//  Call();
//
  char* bufPtr = SIM7600Buffer; // Handy buffer pointer

 
 boolean smokeDetect = false;
 int sensorValue = analogRead(A0); 

 Serial.println(sensorValue);
// 
 String stringRatio = String(sensorValue);

  delay(1000); 

  if(sensorValue>=300){
    if (passFlag == 0) {
    String warning = "Warning, check location, ratio: ";
    warning += sensorValue; 
    sendSms(warning);

    passFlag++;                     // Increment passFlag
  }
       
       
           
          if(sensorValue > 650){
            digitalWrite(LEDpinR, HIGH);
            tone(BuzzerPin, 100);
          Call();
          }else{
            noTone(BuzzerPin);
            digitalWrite(LEDpinR, LOW);
          }
        
  }else{
    passFlag == 0;
        passFlag = 0;

  }

  if (SIM7600Serial->available()) {
 int slot = 0; // slot number of the SMS
    int charCount = 0;
    do {
      *bufPtr = SIM7600Serial->read();
      delay(1);
    } while ((*bufPtr++ != '\n') && (SIM7600Serial->available()));

    // Make pointer point to nothing
    *bufPtr = 0;
    

    if (1 == sscanf(SIM7600Buffer, "+CMTI: \"SM\",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);

    readStatusSms(slot,stringRatio);
    readTestSms(slot);

     
    }

  }

}
