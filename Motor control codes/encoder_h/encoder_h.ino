/* Encoder Library - Basic Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */

#include <Encoder.h>

#define MOT1_DIR  35   // Motor1 direction pin
#define MOT1_BRK  41   // Motor1 brake pin
#define MOT2_DIR  37   // Motor2 direction pin
#define MOT2_BRK  43   // Motor2 brake pin
#define MOT3_DIR  39   // Motor3 direction pin
#define MOT3_BRK  45   // Motor3 brake pin

#define MOT1_PWM  5   // Motor 1 PWM pin
#define MOT2_PWM  6   // Motor 2 PWM pin
#define MOT3_PWM  7   // Motor 3 PWM pin

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc1(20, 21);
Encoder myEnc2(18, 19);
Encoder myEnc3(2, 3);
//   avoid using pins with LEDs attached

void setup() {
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
  pinMode(MOT1_DIR, OUTPUT); 
  pinMode(MOT2_DIR, OUTPUT); 
  pinMode(MOT3_DIR, OUTPUT); 
  pinMode(MOT1_BRK, OUTPUT); 
  pinMode(MOT2_BRK, OUTPUT); 
  pinMode(MOT3_BRK, OUTPUT);
  pinMode(MOT1_PWM, OUTPUT);
  pinMode(MOT2_PWM, OUTPUT);
  pinMode(MOT3_PWM, OUTPUT);
  
  digitalWrite(MOT1_PWM,HIGH);
  digitalWrite(MOT2_PWM,HIGH);
  digitalWrite(MOT3_PWM,HIGH);
  digitalWrite(MOT1_BRK, LOW);
  digitalWrite(MOT2_BRK, LOW);
  digitalWrite(MOT3_BRK, LOW);
  
  digitalWrite(MOT1_DIR, HIGH);
  digitalWrite(MOT2_DIR, HIGH);
  digitalWrite(MOT3_DIR, HIGH);
}

long oldPosition1  = -999;
long oldPosition2  = -999;
long oldPosition3  = -999;
long newPosition1  = -999;
long newPosition2  = -999;
long newPosition3  = -999;

void loop() {
  newPosition1 = myEnc1.read();
  if (newPosition1 != oldPosition1) {
    oldPosition1 = newPosition1;
    Serial.print("MOTOR1: ");
    Serial.println(newPosition1);
  }
    newPosition2 = myEnc2.read();
  if (newPosition2 != oldPosition2) {
    oldPosition2 = newPosition2;
    Serial.print("MOTOR2: ");
    Serial.println(newPosition2);
  }
    newPosition3 = myEnc3.read();
  if (newPosition3 != oldPosition3) {
    oldPosition3 = newPosition3;
    Serial.print("MOTOR3: ");
    Serial.println(newPosition3);
  }
}
