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
  
  attachInterrupt(digitalPinToInterrupt(IPIN0), buzzerISR, RISING);
  attachInterrupt(digitalPinToInterrupt(IPIN1), lampTestISR, FALLING);
  
  digitalWrite(WL, HIGH);
  digitalWrite(CL, HIGH);
  delay(2000);
  digitalWrite(WL, LOW);
  digitalWrite(CL, LOW);
}

bool enableAlarm = true;
byte maintainLampDrive = 0x0;
volatile byte prev = 0;
volatile byte stateCode = 0;
volatile bool lampTest = false;
volatile bool enableBuzzerCheck = false;
volatile unsigned long lastValidChange = 0;
unsigned long lastDeactivateAlarm = 0;
static int ignoreInputThreshold = 10000;


void loop() {
  wdt_reset();
  digitalWrite(LED_BUILTIN,HIGH);
  lampTestFunction(); 
  buzzerCheck(); 
  stateCode =  (digitalRead(S1) << 1) | digitalRead(S0);
  stateCode ^= 0b11;
  delay(50);
  unsigned long time = millis();
  
  if(stateCode==prev && time-lastValidChange > ignoreInputThreshold){
    enableAlarm=true;
    maintainLampDrive=stateCode;
  } else if (stateCode!=prev){
    lastValidChange=time;
  }
  

  Serial.println(lampDriver(maintainLampDrive));
  Serial.println(time-lastValidChange);

  prev = stateCode;
  digitalWrite(LED_BUILTIN,LOW);
  delay(500);
}

void buzzerISR() {
  enableBuzzerCheck = true;  
}

void buzzerCheck(){
  unsigned long initDebounce = millis();
  byte pin = digitalRead(IPIN0);
  unsigned long thresholdMs=100;
  while(pin && (millis()-initDebounce)<thresholdMs){
    delay(25);
    byte pin = digitalRead(IPIN0);
  }
  if((millis()-initDebounce)>=thresholdMs){
    enableAlarm = false;  
    deactivateAlarms();
    lastDeactivateAlarm = millis(); 
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

bool lampDriver(bool maintainLampDrive){
  byte localStateCode = maintainLampDrive;
  
  int h = localStateCode >> 1;

  bool clResult = h&0x01;
  bool wlResult = (localStateCode & 0x1) && !clResult;
  
  if((clResult  || wlResult) && enableAlarm){
    activateAlarms(); 
  } else {
    deactivateAlarms();
  }

  

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

  return clResult || wlResult;
  
}

