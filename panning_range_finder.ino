#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial XBee(0,1); // Rx and Tx of Fio

float mmReading = 0.00;
int sonarPin = A0;
int  sonarValue;

Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
int from = 0;
int to = 0;
int rotateBy = 0;
int newFrom = 0;
int newTo = 0;
int newRotateBy = 0;


void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(0,INPUT);
  pinMode(1,OUTPUT);  
  pinMode(sonarPin,INPUT);
  XBee.begin(9600);
}

void loop() {
  readSerialInput();
  updateConfiguration();
  performCycle();
}

void readSerialInput() {
  if (XBee.available() > 0) { // no error tolerance
    newFrom  = (XBee.readStringUntil(',')).toInt();
    newTo = (XBee.readStringUntil(',')).toInt();
    newRotateBy  = (XBee.readStringUntil('\n')).toInt();
  }
}

void updateConfiguration() {
  if (newFrom && newTo) {
    from = newFrom;
    to = newTo;
    rotateBy = newRotateBy;
    newFrom = 0;
    newTo= 0;
    newRotateBy = 0;
  }
}

void performCycle() { // pan taking a distance measurement during each step 
  if(!(from && to)) { 
    return; 
  } 
//  Serial.print("from & to != 0");
  
  for (pos = from; pos <= to; pos += rotateBy) {
    performStepAtPosition(pos);
  }
  
  for (pos = to; pos >= from; pos -= rotateBy) {
    performStepAtPosition(pos); 
  }
}

void performStepAtPosition(int pos) {
  myservo.write(pos);
  delay(500);
  readDistance();
  reportDistance();
}

void readDistance() {
  sonarValue =  analogRead(sonarPin);
  mmReading = (sonarValue*5);
}

void reportDistance() {
  XBee.print(pos);
  XBee.print(",");
  XBee.println(mmReading);  
}
