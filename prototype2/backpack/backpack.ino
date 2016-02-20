// RF setup
#include <SPI.h>
#include "RF24.h"

bool radioNumber = 0;
RF24 radio(9,10);
byte addresses[][6] = {"1Node","2Node"};

// EL switches
#define LEFT_LIGHT 3
#define RIGHT_LIGHT 4
#define CENTER_LIGHT 5

// Accelerometer
#define INT 2
#define SCL A4
#define SDA A5

// LEDs
#define LED1 A0
#define LED2 A1

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

  Serial.begin(38400);
  
  // rf
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  radio.startListening();  
}

void loop() {
  // put your main code here, to run repeatedly:
  _lightStatusUpdate();

  // get YPR readings

  radio.startListening();

    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
    
    while ( ! radio.available() ){                             // While nothing is received
      if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){                                             // Describe the results
        Serial.println(F("Failed, response timed out."));
    }else{
        char ypr[sizeof(float)*3];                                 // Get the response       
        byte yaw_bytes[sizeof(float)];
        byte pitch_bytes[sizeof(float)];
        byte roll_bytes[sizeof(float)];
        
        radio.read( &ypr, sizeof(float)*3 );
        unsigned long time = micros();

        memcpy(yaw_bytes,ypr,sizeof(float));
        float yaw = *((float *) yaw_bytes);

        memcpy(pitch_bytes,&(ypr[sizeof(float)]),sizeof(float));
        float pitch = *((float *) pitch_bytes);
        
        memcpy(roll_bytes,&(ypr[sizeof(float)*2]),sizeof(float));
        float roll = *((float *) roll_bytes);
        
        Serial.print("yaw: ");
        Serial.print(yaw);
        Serial.print(" pitch: ");
        Serial.print(pitch);
        Serial.print(" roll: ");
        Serial.println(roll);       
    }

    // Try again 1s later
    delay(1000);
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
