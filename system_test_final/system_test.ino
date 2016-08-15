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
AccelStepper pickwheel(1, stepperStepPin, stepperDirPin);

//Pickwheel possistion variables
#define pickwheelSensePin 21

//******************************



//***** RC Servo Setup *****
#define damperServoPin 47
#define pickServoPin 48

Servo damperServo;
Servo pickServo;

#define damperOnPos 30
#define damperOffPos 25

#define pickUpperLimit 60
#define pickLowerLimit 75

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

void pickString();
void changePick();

void setVelcoity(int velocity);

void zeroPickwheel();
//**********

//***** Gobal Variable Decleration *****
bool fingerPicking = true;    // default to using the figer picking, can be changed with MIDI CC
bool fingerPickClose = true;  //default to using the finger closer - as set by the zero pickwheel method
bool pickUp = true;           //true if the pick is above the string
//int pos; //this is initlised in the zeroPickwheel method

void setup() {
  //attatch the Servo classes to their respective pins
  damperServo.attach(damperServoPin);
  pickServo.attach(pickServoPin);

  damperServo.write(damperOnPos);
  pickServo.write(pickLowerLimit);

  //set speed and acleration for the pickwheel
  pickwheel.setMaxSpeed(1000);
  pickwheel.setAcceleration(10000);

  zeroPickwheel();


  //Link the lib to the functions
  midi1.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  midi1.setHandleNoteOff(handleNoteOff);  // Put only the name of the function
  midi1.setHandleControlChange(handleNoteControlChange);  // Put only the name of the function

  // Initiate MIDI communications, listen to all channels
  midi1.begin(MIDI_CHANNEL_OMNI);

  if (DEBUG)Serial.begin(9600);
  if (DEBUG) Serial.println(F("Setup Compleete"));
  //The F() operator stores the string in flash memory, thus freeing program memory
}

int pos = 4000;

void loop() {
  midi1.read();

}



void handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (DEBUG) Serial.println(F("MIDI on recived"));
  dampOff();
  //This statment will have to be updated bassed on how we chose to distinguish picking methods in midi
  if (fingerPicking) {
    if (DEBUG) Serial.println(F("Finger Pick"));
    pickFinger((int) velocity);
  }
  else {
    if (DEBUG) Serial.println(F("Plectrum Pick"));
    pickPlectrum((int) velocity);
  }
}


void handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (DEBUG) Serial.println(F("MIDI off recived"));
  dampOn();
}

void handleNoteControlChange(byte channel, byte pitch, byte velocity) {
  if (DEBUG){
  Serial.println(F("MIDI CC recived"));
  Serial.print("Chan: ");
  Serial.print(channel,DEC);
  Serial.print(" Pitch: ");
  Serial.print(pitch,DEC);
  Serial.print(" velo: ");
  Serial.println(velocity,DEC);
  }
  if( (int) pitch == 1){
    fingerPicking = ((int)velocity) > 63;
  }
  if((int) pitch == 71){
    zeroPickwheel();
  }
  
}

void dampOn() {
  if (DEBUG) Serial.println(F("Damper Engaged"));
  damperServo.write(damperOnPos);
}
void dampOff() {
  if (DEBUG) Serial.println(F("Damper dis-engaged"));
  damperServo.write(damperOffPos);
}


void pickFinger(int velocity) {
  setVelcoity(velocity);
  delay(15); // wait for damping and velocity servos to reach their possistions
  if(!fingerPickClose){
    changePick();
  }
  pickString();
}


void pickPlectrum(int velocity) {
  setVelcoity(velocity);
  delay(15); // wait for damping and velocity servos to reach their possistions
  if(fingerPickClose){
    changePick();
  } 
  pickString();
}


void setVelcoity(int velocity) {
  //Maps the 0 to 127 MIDI input to the upper and lower limits of the servo possistion
  if (DEBUG) {
    Serial.print(F("Velocoity is: "));
    Serial.println(velocity);
  }
  int servoPos = map(velocity , 0 , 127 , pickLowerLimit , pickUpperLimit);
  pickServo.write(servoPos);
}

/*
   Picks the string using the selected plectrum
*/
void pickString() {
    if(DEBUG) Serial.println(F("Picking String"));
  if(pickUp){
    pickwheel.move(-50);
  }
  else{
    pickwheel.move(50);
  }
  while(pickwheel.distanceToGo() != 0){
    pickwheel.run();
  }
  pickUp=!pickUp;
}


/*
 * Changes the selected plectrum 
 */
 void changePick(){
    if(DEBUG) Serial.println(F("Changing picks"));
  if(pickUp){
    pickwheel.move(50);
  }
  else{
    pickwheel.move(-50);
  }


  while (pickwheel.distanceToGo() != 0) {
    pickwheel.run();
  }
  pickUp =! pickUp;
  fingerPickClose =! fingerPickClose;
  
 }



/*
   Zeros the pickwheel possistion so it the correct pick selection can be made
*/
void zeroPickwheel() {
  if(DEBUG) Serial.println(F("Zeroing Pickwheel"));
  pickServo.write(pickLowerLimit + 20); //move the pick away from the string so no sound is joined
  delay(500); //wait for the servo to move

  pickwheel.setSpeed(200);
  pickwheel.moveTo(400);

  // If the switch is already on, rotate till it isn't on.
  while (digitalRead(pickwheelSensePin) == 1) {
    pickwheel.run();
  }

  while (0 == digitalRead(pickwheelSensePin)) {
    pickwheel.run();
  }
  pickwheel.setCurrentPosition(0); //zero the motor
  pickServo.write(pickLowerLimit); // move the pick back to the string
  pickwheel.setSpeed(1000);

  pickUp = true;
  fingerPickClose = true;


  
 if(DEBUG) Serial.println(F("Pickwheel Zeroed"));
}





