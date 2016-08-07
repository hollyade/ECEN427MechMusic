// a lib that jim recomends for stepper motors, needs to be downloaded from http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h> 

//The defualt arduino servo lib
#include <Servo.h>

//MIDI lib has to be downloaded from https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2
#include <MIDI.h>

// use for debug testing and serial printing
#define DEBUG true

//***** Stepper Motor Setup *****
//Control Pins
#define stepperStepPin 26
#define stepperDirPin 24

//Function Pins
#define stepperRSTpin 30
#define stepperSleepPin 28

//Step Size Pins
#define stepperSize1Pin 36
#define stepperSize2Pin 34
#define stepperSize3Pin 32

//Consructor is as follows (Mode,stepPin,dirPin)
//Mode 1 indicates a driver interface
AccelStepper pickwheel(1,stepperStepPin,stepperDirPin);

//******************************



//***** RC Servo Setup *****
#define damperServoPin 47
#define pickServoPin 48

Servo damperServo;
Servo pickServo;

#define damperOnPos 30
#define damperOffPos 25

#define pickUpperLimit 84
#define pickLowerLimit 98

//************************

//***** MIDI Setup *****
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midi1)

//functions needed for handlling midi
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleNoteControlChange(byte channel, byte pitch, byte velocity);

//************

//***** Control Function Decleration *****
void dampOn();
void dampOff();

void pickFinger(int velocity);
void pickPlectrum(int velocity);
void setVelcoity(int velocity);
//**********



void setup() {
  //attatch the Servo classes to their respective pins
  damperServo.attach(damperServoPin);
  pickServo.attach(pickServoPin);

  //set speed and acleration for the pickwheel
  pickwheel.setMaxSpeed(1000);
  pickwheel.setAcceleration(10000);


  //Link the lib to the functions
  midi1.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  midi1.setHandleNoteOff(handleNoteOff);  // Put only the name of the function
  midi1.setHandleControlChange(handleNoteControlChange);  // Put only the name of the function

  // Initiate MIDI communications, listen to all channels
  midi1.begin(MIDI_CHANNEL_OMNI);

  if(DEBUG)Serial.begin(9600);
  if(DEBUG) Serial.println(F("Setup Compleete"));  
  //The F() operator stores the string in flash memory, thus freeing program memory
}

int pos = 50;

void loop() {

midi1.read();

//  pickServo.write(90);
//
//
//  if(pickwheel.distanceToGo()==0){
//    delay(500);
//    pos=-pos;
//    pickwheel.moveTo(pos);
//  }
//  pickwheel.run();
}



void handleNoteOn(byte channel, byte pitch, byte velocity){
  if(DEBUG) Serial.println(F("MIDI on recived"));
  dampOff();
  //This statment will have to be updated bassed on how we chose to distinguish picking methods in midi
  if(true){
    if(DEBUG) Serial.println(F("Finger Pick"));
    pickFinger((int) velocity);
  }
  else{
    if(DEBUG) Serial.println(F("Plectrum Pick"));
    pickPlectrum((int) velocity);
    }
}


void handleNoteOff(byte channel, byte pitch, byte velocity){
  if(DEBUG) Serial.println(F("MIDI off recived"));
  dampOn();
}

void handleNoteControlChange(byte channel, byte pitch, byte velocity){
    if(DEBUG) Serial.println(F("MIDI CC recived"));
}

void dampOn(){
  if(DEBUG) Serial.println(F("Damper Engaged"));
  damperServo.write(damperOnPos);
}
void dampOff(){
    if(DEBUG) Serial.println(F("Damper dis-engaged"));
    damperServo.write(damperOffPos);
}

int poss = 70;
void pickFinger(int velocity){
  setVelcoity(velocity);
  delay(15); // wait for damping and velocity servos to reach their possistions 
  pickwheel.moveTo(poss);
  while(pickwheel.distanceToGo() != 0){
    pickwheel.run();
  }
  poss=-poss;
  

  
  //Do some stuff with the pickwheel
}
void pickPlectrum(int velocity){
  setVelcoity(velocity);
  delay(15); // wait for damping and velocity servos to reach their possistions 
}


void setVelcoity(int velocity){
  //Maps the 0 to 127 MIDI input to the upper and lower limits of the servo possistion
  if(DEBUG){
    Serial.print(F("Velocoity is: "));
    Serial.println(velocity);
  }
  
  int servoPos = map(velocity , 0 , 127 , pickLowerLimit , pickUpperLimit);
  pickServo.write(servoPos);
}








