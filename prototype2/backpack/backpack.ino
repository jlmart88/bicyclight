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

//#define LEFT_LIGHT A0
//#define RIGHT_LIGHT A2
//#define CENTER_LIGHT A1

// Accelerometer
#include "freeram.h"
#include "mpu.h"
#include "I2Cdev.h"
#define INT 2
#define SCL A4
#define SDA A5
#define INIT_YAW 0
#define INIT_PITCH 0
#define INIT_ROLL 90

// LEDs
#define LED1 A0
#define LED2 A1

// timing variables
int centerBlinkOnInterval = 500;
int centerBlinkOffInterval = 500;

int sideBlinkOnInterval = 500;
int sideBlinkOffInterval = 500;

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

int ret;

void setup() {

  Fastwire::setup(400,0);
  Serial.begin(38400);
  ret = mympu_open(200);
  Serial.print("MPU init: "); Serial.println(ret);
  Serial.print("Free mem: "); Serial.println(freeRam());
    
  // setup light pins
  pinMode(LEFT_LIGHT, OUTPUT);  // channel A  
  pinMode(RIGHT_LIGHT, OUTPUT);  // channel B   
  pinMode(CENTER_LIGHT, OUTPUT);  // channel C
  
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

unsigned int c = 0; //cumulative number of successful MPU/DMP reads
unsigned int np = 0; //cumulative number of MPU/DMP reads that brought no packet back
unsigned int err_c = 0; //cumulative number of MPU/DMP reads that brought corrupted packet
unsigned int err_o = 0; //cumulative number of MPU/DMP reads that had overflow bit set


void loop() {
  // put your main code here, to run repeatedly:
  _lightStatusUpdate();

  // get YPR readings

        ret = mympu_update();

    switch (ret) {
  case 0: c++; break;
  case 1: np++; return;
  case 2: err_o++; return;
  case 3: err_c++; return;
  
  default: 
    Serial.print("READ ERROR!  ");
    Serial.println(ret);
    return;
    }

    if (!(c%100)) {
      c = 0;    
      Serial.print(np); Serial.print("  "); Serial.print(err_c); Serial.print(" "); Serial.print(err_o);
      Serial.print(" Y: "); Serial.print(mympu.ypr[0]);
      Serial.print(" P: "); Serial.print(mympu.ypr[1]);
      Serial.print(" R: "); Serial.print(mympu.ypr[2]);
      Serial.print("\tgy: "); Serial.print(mympu.gyro[0]);
      Serial.print(" gp: "); Serial.print(mympu.gyro[1]);
      Serial.print(" gr: "); Serial.println(mympu.gyro[2]);

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

        float yaw_diff = int((yaw - mympu.ypr[0]));
        yaw_diff = (yaw_diff < 0 ? yaw_diff + 360 : yaw_diff) - INIT_YAW;
        yaw_diff = (yaw_diff < 0 ? yaw_diff + 360 : yaw_diff);
        float pitch_diff = int((pitch - mympu.ypr[1]));
        pitch_diff = (pitch_diff < 0 ? pitch_diff + 360 : pitch_diff) - INIT_PITCH;
        float roll_diff = int((roll - mympu.ypr[2]));
        roll_diff = (roll_diff < 0 ? roll_diff + 360 : roll_diff) - INIT_ROLL;
        roll_diff = (roll_diff < 0 ? roll_diff + 360 : roll_diff);
        
        if(roll_diff<315 && roll_diff > 225){
          turnState = Right;
         
        } 
        else if(yaw_diff<315 && yaw_diff > 225){
          turnState = Left;
        }
        else{
          turnState = Center;
        }
      
        
        Serial.print("yaw: ");
        Serial.print(yaw);
        Serial.print(" pitch: ");
        Serial.print(pitch);
        Serial.print(" roll: ");
        Serial.println(roll);   

        Serial.print("yaw_delta: ");
        Serial.print(yaw - mympu.ypr[0]);
        Serial.print(" pitch_delta: ");
        Serial.print(pitch - mympu.ypr[1]);
        Serial.print(" roll_delta ");
        Serial.println(roll - mympu.ypr[2]);

        Serial.print("yaw_diff: ");
        Serial.print(yaw_diff);
        Serial.print(" pitch_diff: ");
        Serial.print(pitch_diff);
        Serial.print(" roll_diff ");
        Serial.println(roll_diff);
    }  

    // Try again 1s later
    //delay(100);
    }
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
