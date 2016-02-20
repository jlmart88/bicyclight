
// EL switches
#define LEFT_LIGHT 3
#define RIGHT_LIGHT 4
#define CENTER_LIGHT 5

// Accelerometer


// RF Module

// timing variables
int centerBlinkOnInterval = 500;
int centerBlinkOffInterval = 1000;

int sideBlinkOnInterval;
int sideBlinkOffInterval;

// state variables
enum TurnState {
  Left,
  Right,
  Center
};
TurnState turnState = Center;
int centerState = LOW;
int centerBlinkMillis = 0;
int sideState = LOW;
int sideBlinkMillis = 0;

void setup() {
  // setup light pins
  pinMode(LEFT_LIGHT, OUTPUT);  // channel A  
  pinMode(RIGHT_LIGHT, OUTPUT);  // channel B   
  pinMode(CENTER_LIGHT, OUTPUT);  // channel C
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  _lightStatusUpdate();
}

// call frequently throughout the loop() to simulate asynchronous light updating
void _lightStatusUpdate() {
  unsigned long currentMillis = millis();
  int sideLightPin;
  
  // control center blink state
  if (currentMillis - centerBlinkMillis >= (centerState == HIGH ? centerBlinkOnInterval : centerBlinkOffInterval)) {
   centerBlinkMillis = currentMillis;
   centerState = !centerState;
  } 
  
  // control side blink state
  if (turnState == Center) {
    sideBlinkMillis = currentMillis;
    sideState = LOW;
    digitalWrite(LEFT_LIGHT, sideState);
    digitalWrite(RIGHT_LIGHT, sideState);
  } else {
    sideLightPin = (turnState == Left ? LEFT_LIGHT : RIGHT_LIGHT);
    if (currentMillis - sideBlinkMillis >= (sideState == HIGH ? sideBlinkOnInterval : sideBlinkOffInterval)) {
      sideBlinkMillis = currentMillis;
      sideState = !sideState;
      digitalWrite(sideLightPin, sideState);
    }
  }
  
  digitalWrite(CENTER_LIGHT, centerState);
}
