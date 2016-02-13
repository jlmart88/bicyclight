
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

  Serial.begin(9600);
}

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
  } else if (mode == 0){
    digitalWrite(CENTER_LIGHT, 1);
  } else if (mode == 1){
    for(int i=0; i<2; i++){
      digitalWrite(LEFT_LIGHT, 1);
      digitalWrite(CENTER_LIGHT, 1);
      delay(BLINK_DELAY_ON);
      digitalWrite(LEFT_LIGHT, 0);
      delay(BLINK_DELAY_OFF);
    }
    delay(BLINK_LONG_DELAY_OFF);
  } else if (mode == 2){
    for(int i=0; i<2; i++){
      digitalWrite(RIGHT_LIGHT, 1);
      digitalWrite(CENTER_LIGHT, 1);
      delay(BLINK_DELAY_ON);
      digitalWrite(RIGHT_LIGHT, 0);
      delay(BLINK_DELAY_OFF);
    }
    delay(BLINK_LONG_DELAY_OFF);    
  } else if (mode == 3){

    digitalWrite(CENTER_LIGHT, 1);
    delay(BLINK_DELAY_OFF);
    digitalWrite(CENTER_LIGHT, 0);
    delay(BLINK_DELAY_OFF);
  }


}
