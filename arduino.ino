
#define LEFT_LIGHT 2
#define RIGHT_LIGHT 3
#define CENTER_LIGHT 4

#define LEFT_SWITCH A0
#define RIGHT_SWITCH A1
#define PHOTOCELL A2

#define STATUS_ARDUINO 13
#define STATUS_SHIELD 10

#define BLINK_DELAY_ON 100
#define BLINK_DELAY_OFF 100
#define BLINK_LONG_DELAY_OFF 500

int mode = 0;
int incomingByte = 0;

// Accelerometer Setup
#include<Wire.h>
#include<Average.h>
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup() {                
  // The EL channels are on pins 2 through 9
  // Initialize the pins as outputs
  pinMode(LEFT_LIGHT, OUTPUT);  // channel A  
  pinMode(RIGHT_LIGHT, OUTPUT);  // channel B   
  pinMode(CENTER_LIGHT, OUTPUT);  // channel C

  pinMode(LEFT_SWITCH, INPUT);
  pinMode(RIGHT_SWITCH, INPUT);
  
  // We also have two status LEDs, pin 10 on the Escudo, 
  // and pin 13 on the Arduino itself
  pinMode(STATUS_SHIELD, OUTPUT);     
  pinMode(STATUS_ARDUINO, OUTPUT); 

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Serial.begin(9600);
}

int count = 0;
int AcX_buffer[8];
int AcY_buffer[8];

int AcX_avg_buffer[8];
int AcY_avg_buffer[8];

int AcX_diff_buffer[8];
int AcY_diff_buffer[8];

int blink_center_delay_on = 100;
int blink_center_delay_off = 0;

void loop() 
{

// Serial.println(analogRead(PHOTOCELL)); //400 darkness cutoff
// delay(100);

  if (Serial.available() > 0) {

      incomingByte = Serial.read();
      //delay(10);
      //Serial.println(incomingByte);

      // if (analogRead(PHOTOCELL) > 250) { // check that light is dark enough
          
          // mode selection
          if (incomingByte == 49){ // 1, left
            mode = 1;
          } else if (incomingByte == 50){ // 2, right
            mode = 2;
          } else if (incomingByte == 51) { // 3, center
            mode = 0;
          } else if (incomingByte == 52) { // 4. middle blink
            mode = 3;
          } else {
            mode = -1;
          }
      
      //  } else {
      //    mode = -1; // all off
      //  }
      
  }

  // mode actions
  if (mode == -1){
    digitalWrite(CENTER_LIGHT, 0);
    digitalWrite(LEFT_LIGHT, 0);
    digitalWrite(RIGHT_LIGHT, 0);
  //} else if (mode == 0){
    //digitalWrite(CENTER_LIGHT, 1);
  } else {
    if (mode == 1){
      for(int i=0; i<2; i++){
        digitalWrite(LEFT_LIGHT, 1);
        digitalWrite(CENTER_LIGHT, 1);
        delay(BLINK_DELAY_ON);
        digitalWrite(LEFT_LIGHT, 0);
        delay(BLINK_DELAY_OFF);
      }
    } else if (mode == 2){
      for(int i=0; i<2; i++){
        digitalWrite(RIGHT_LIGHT, 1);
        digitalWrite(CENTER_LIGHT, 1);
        delay(BLINK_DELAY_ON);
        digitalWrite(RIGHT_LIGHT, 0);
        delay(BLINK_DELAY_OFF);
      }
    }
    int centerBlinkDuration = 0;
    while (centerBlinkDuration < BLINK_LONG_DELAY_OFF){
          digitalWrite(CENTER_LIGHT, 1);
          delay(blink_center_delay_on);
          centerBlinkDuration += blink_center_delay_on;
          digitalWrite(CENTER_LIGHT, 0);
          delay(blink_center_delay_off);
          centerBlinkDuration += blink_center_delay_off;
    }
    //delay(BLINK_LONG_DELAY_OFF);    
//  } else if (mode == 3){
//
//    digitalWrite(CENTER_LIGHT, 1);
//    delay(BLINK_DELAY_OFF);
//    digitalWrite(CENTER_LIGHT, 0);
//    delay(BLINK_DELAY_OFF);
  }

  // read accelerometer data

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers

  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L) 

  AcX_buffer[count] = AcX;
  AcY_buffer[count] = AcY;

  int AcX_average = (AcX_buffer[0]+AcX_buffer[1]+AcX_buffer[2]+AcX_buffer[3]+AcX_buffer[4]+AcX_buffer[5]+AcX_buffer[6]+AcX_buffer[7])/8;
  int AcY_average = (AcY_buffer[0]+AcY_buffer[1]+AcY_buffer[2]+AcY_buffer[3]+AcY_buffer[4]+AcY_buffer[5]+AcY_buffer[6]+AcY_buffer[7])/8;

  AcX_avg_buffer[count] = AcX_average;
  AcY_avg_buffer[count] = AcY_average;

  AcX_diff_buffer[count] = AcX_average - AcX_avg_buffer[(count+7)%8];
  AcY_diff_buffer[count] = AcY_average - AcY_avg_buffer[(count+7)%8];

  int blink_rate = AcX_average;
  blink_rate = (-blink_rate + abs(blink_rate))/2;
  blink_center_delay_off = max(min(1600000/blink_rate, 400),100);

  count = (count+1) %8;

  // for debug
  /*Serial.print("blink_rate: "); Serial.print(blink_rate);
  Serial.print(" | blink_center_delay_off: "); Serial.print(blink_center_delay_off);
  Serial.print(" | AcX_average "); Serial.print(AcX_average);
  Serial.print(" | AcY_average "); Serial.print(AcY_average);
  Serial.print(" | AcX_diff "); Serial.print(AcX_diff_buffer[count]);
  Serial.print(" | AcY_diff "); Serial.print(AcY_diff_buffer[count]);
  Serial.print(" || AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.println(AcZ);*/

  //delay(50);
}
