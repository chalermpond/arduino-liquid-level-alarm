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
  wdt_enable(WDTO_4S);
  pinMode(IPIN0, INPUT);
  pinMode(IPIN1, INPUT);
  pinMode(S1, INPUT);
  pinMode(S0, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ALARM, OUTPUT);
  pinMode(IALARM,OUTPUT);
  pinMode(WL, OUTPUT);
  pinMode(CL, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(IPIN0), buzzerISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(IPIN1), lampTestISR, FALLING);
  digitalWrite(ALARM, LOW);
  digitalWrite(IALARM,LOW);
  digitalWrite(WL, HIGH);
  digitalWrite(CL, HIGH);
  delay(2000);
  digitalWrite(WL, LOW);
  digitalWrite(CL, LOW);
}

volatile byte enableAlarm = 0xF;
volatile byte prev = 0;
volatile byte stateCode = 0;
volatile bool lampTest = false;
volatile long lastValidChange = 0;


void loop() {
  lampTestFunction();  
  long timeMillis = millis();
  stateCode =  (digitalRead(S1) << 1) | digitalRead(S0);
  stateCode ^= 0b11;
  
  if (prev != stateCode && (timeMillis - lastValidChange) > 3000 ) {
    enableAlarm = 0xF;
    lastValidChange = millis();
  }
  int h = stateCode >> 1;

  bool clResult = h&0x01;
  bool wlResult = (stateCode & 0x1) && !clResult;

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
  
  delay(150);
  
  if(clResult){
    digitalWrite(CL, LOW);  
  }
  if(wlResult){
    digitalWrite(WL,LOW);  
  }
  

  prev = stateCode;
  delay(1000);
  wdt_reset();
}

void buzzerISR() {
  enableAlarm = 0x0;
  digitalWrite(ALARM, LOW);
  digitalWrite(IALARM, LOW);
}
void lampTestISR(){
  lampTest = true;
}

void lampTestFunction(){
  byte pin = 0;
  long lastDebounce = millis();
  while( lampTest){
    digitalWrite(WL,HIGH);
    digitalWrite(CL,HIGH);
    pin  = digitalRead(IPIN1);
    if(pin && (millis()-lastDebounce) > 50){
      lampTest = false;
    }
  }

}

