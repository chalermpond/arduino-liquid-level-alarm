#include <avr/wdt.h>
//-- Interrupt PIN
const int IPIN0 = 2;//Silent
const int IPIN1 = 3;//Lamp Test

//-- Digital I/O
const int S1 = 9;
const int S0 = 8;
const int IALARM = 12;
const int ALARM = 4;
const int WL = 5;
const int CL = 6;

void setup() {
  wdt_enable(WDTO_8S);
  pinMode(IPIN0, INPUT);
  pinMode(IPIN1, INPUT);
  pinMode(S1, INPUT);
  pinMode(S0, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ALARM, OUTPUT); 
  pinMode(IALARM,OUTPUT);
  pinMode(WL, OUTPUT);
  pinMode(CL, OUTPUT);
  deactivateAlarms();
  
  //attachInterrupt(digitalPinToInterrupt(IPIN0), buzzerISR, RISING);
  attachInterrupt(digitalPinToInterrupt(IPIN1), lampTestISR, FALLING);
  
  digitalWrite(WL, HIGH);
  digitalWrite(CL, HIGH);
  delay(2000);
  digitalWrite(WL, LOW);
  digitalWrite(CL, LOW);
  Serial.begin(9600);
}

volatile bool enableAlarm = false;
volatile byte prev = 0;
volatile byte stateCode = 0;
volatile bool lampTest = false;
volatile unsigned long lastValidChange = 0;
unsigned long lastDeactivateAlarm = 0;
unsigned long ignoreInputThreshold = 5000;
byte previousMaintainLamp = 0x0;
byte maintainLampDrive = 0x0;
bool alarmLocked = false;

void loop() {
  digitalWrite(LED_BUILTIN,HIGH);
  lampTestFunction();
  silentFunction();  
  unsigned long timeMillis = millis();
  stateCode =  (digitalRead(S1) << 1) | digitalRead(S0);
  stateCode ^= 0b11;
 

  if(stateCode==prev && timeMillis-lastValidChange > ignoreInputThreshold){
    maintainLampDrive=stateCode;
  } else if (stateCode!=prev){
    lastValidChange=timeMillis;
  }

  if(previousMaintainLamp!=maintainLampDrive){
    enableAlarm = true;
  }

  Serial.print(previousMaintainLamp);
  Serial.print(maintainLampDrive);
  Serial.println(enableAlarm);
  
  


  int h = maintainLampDrive >> 1;

  bool clResult = h&0x01;
  bool wlResult = (maintainLampDrive & 0x1) && !clResult;

  if((clResult  || wlResult) && enableAlarm ){
    activateAlarms();
  } else {
    deactivateAlarms();
  }
  
  lampDriver(maintainLampDrive);
  prev = stateCode;
  previousMaintainLamp = maintainLampDrive;
  digitalWrite(LED_BUILTIN,LOW);
  delay(250);
  wdt_reset();
}

void buzzerISR() {
  long initDebounce = millis();
  byte pin = digitalRead(IPIN0);
  long thresholdMs=100;
  while(pin && (millis()-initDebounce)<thresholdMs){
    byte pin = digitalRead(IPIN0);
  }
  if((millis()-initDebounce)>=thresholdMs){
    enableAlarm = false;  
    deactivateAlarms();
  }
}

void silentFunction(){
  long initDebounce = millis();
  byte pin = digitalRead(IPIN0);
  long thresholdMs=50;
  while(pin && (millis()-initDebounce)<thresholdMs){
    byte pin = digitalRead(IPIN0);
  }
  if((millis()-initDebounce)>=thresholdMs){
    enableAlarm = false;  
    deactivateAlarms();
  }
}

void lampTestISR(){
  lampTest = true;
}

void activateAlarms(){
  setAlarms(HIGH);
}

void deactivateAlarms(){
  setAlarms(LOW);
}

void setAlarms(byte b){
  digitalWrite(ALARM, b);
  digitalWrite(IALARM, b);
}

void lampTestFunction(){
  byte pin = digitalRead(IPIN1);
  long lastDebounce = millis();
  
  while(!pin&&lampTest){
    digitalWrite(WL,HIGH);
    digitalWrite(CL,HIGH);
    pin  = digitalRead(IPIN1);
    if(pin && (millis()-lastDebounce) > 50){
      lampTest = false;
    }
  }

}

void lampDriver(byte state){
  int h = state >> 1;

  bool clResult = h&0x01;
  bool wlResult = (state & 0x1) && !clResult;

  if(clResult){
    digitalWrite(CL, HIGH);  
  } else {
    digitalWrite(CL, LOW);  
  }
  if(wlResult){
    digitalWrite(WL,HIGH);  
  } else {
    digitalWrite(WL,LOW);  
  }
  
  delay(250);
  
  if(clResult && enableAlarm){
    digitalWrite(CL, LOW);  
  }
  if(wlResult &&  enableAlarm){
    digitalWrite(WL,LOW);  
  }
  
  
}


