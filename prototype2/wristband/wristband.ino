#include "freeram.h"
#include "mpu.h"
#include "I2Cdev.h"

#include <SPI.h>
#include "RF24.h"

// RF Setup
/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(9,10);

/**********************************************************/
byte addresses[][6] = {"1Node","2Node"};

// Accelerometer
#define INT 2
#define SCL A4
#define SDA A5

// Vibe
#define VIBE D5

// LEDs
#define LED1 A0
#define LED2 A1

int ret;
void setup() {
    Fastwire::setup(400,0);
    Serial.begin(38400);
    ret = mympu_open(200);
    Serial.print("MPU init: "); Serial.println(ret);
    Serial.print("Free mem: "); Serial.println(freeRam());
    
    radio.begin();

    // Set the PA Level low to prevent power supply related issues since this is a
    // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
    radio.setPALevel(RF24_PA_LOW);
    
    // Open a writing and reading pipe on each radio, with opposite addresses
    if(radioNumber){
      radio.openWritingPipe(addresses[1]);
      radio.openReadingPipe(1,addresses[0]);
    }else{
      radio.openWritingPipe(addresses[0]);
      radio.openReadingPipe(1,addresses[1]);
    }
    
    // Start the radio listening for data
    radio.startListening();
  
}

unsigned int c = 0; //cumulative number of successful MPU/DMP reads
unsigned int np = 0; //cumulative number of MPU/DMP reads that brought no packet back
unsigned int err_c = 0; //cumulative number of MPU/DMP reads that brought corrupted packet
unsigned int err_o = 0; //cumulative number of MPU/DMP reads that had overflow bit set

void loop() {
    ret = mympu_update();

    switch (ret) {
    case 0: c++; break;
    case 1: //np++;
    return;
    case 2: //err_o++; 
    return;
    case 3: //err_c++;
    return;
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
      
      char transmitString[sizeof(float)*3];
      byte * yaw = (byte *) &(mympu.ypr[0]);
      byte * pitch = (byte *) &(mympu.ypr[1]);
      byte * roll = (byte *) &(mympu.ypr[2]);
      memcpy(transmitString, yaw, sizeof(float)); // yaw
      memcpy(&(transmitString[sizeof(float)*1]), pitch, sizeof(float)); // pitch
      memcpy(&(transmitString[sizeof(float)*2]), roll, sizeof(float)); // roll   
       
      radio.stopListening();
      radio.write(transmitString, 12);
      radio.startListening();
      delay(1000);
   }
}

