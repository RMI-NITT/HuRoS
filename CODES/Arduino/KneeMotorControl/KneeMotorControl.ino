/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */
#include <Encoder.h>

// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability

Encoder motorLeft(20,21);
//Encoder motorRight(7, 8);
int dirPin=34, pwmPin=5;
long integral=0;
long eAngle=0;
#define LEFT 0
#define RIGHT 1
long encoderConstant = 29520/360; //Counts per revolution for 100RPM motor
//   avoid using pins with LEDs attached

void setup() {
  Serial.begin(9600);
  Serial.println("Encoder Test:");
  pinMode(dirPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
}

long positionLeft  = -999;
long positionRight = -999;

int posControl(long error){
  long pwm=0;
  double kp = 0.001, ki = 0.00001;
  int dir;
  
  //if(abs(error)<25000){
  integral = integral + error;
//  }

  
  //Serial.print("error:");
  //Serial.print(error/encoderConstant);

  
  Serial.print(" integral:");
  Serial.println(integral);
  
  if (abs(error)<2*encoderConstant){   // 2 degree error
    integral=0;
    Serial.println("REach");
  }
  
  pwm = int(kp*error + ki*integral);
  
  
  if(pwm<0){
    dir=LEFT;
  }
  else{
    dir=RIGHT;
  }
  pwm=abs(pwm);
  if(pwm>255){
    pwm=255;
  }
  /*
  if (abs(diff) < 40*encoderConstant && abs(diff)>0){
    //Serial.println("low");
    pwm=20;
  }
  else{
    pwm=abs(diff)/200;
  }
  
  */
  Serial.print("PWM:");
  Serial.println(pwm);
  Serial.print("Dir");
  Serial.println(dir);
  
  digitalWrite(dirPin, dir);
  analogWrite(pwmPin, pwm);
  //Serial.print("Dir:");
  //Serial.println(dir);
 // Serial.println("Run");
  return 0;
  } 

void loop() {
  long newLeft, newRight, error;
  long angle;
  String angleStr;
  newLeft = motorLeft.read();
  //newRight = motorRight.read();
  if (newLeft != positionLeft || newRight != positionRight) {
    //Serial.print("Left = ");
    //Serial.print(newLeft/encoderConstant);
    //Serial.print(", Right = ");
    //Serial.print(newRight);
    //Serial.print(" error:");
  /*Serial.print(error);
  Serial.print(" integral:");
  Serial.println(integral);*/
    //Serial.println();
    positionLeft = newLeft;
    //positionRight = newRight;
  }
  //Serial.flush();
  // angle reading from serial monitor
  if (Serial.available()) {
    //Serial.println("Reading....");
    angleStr=Serial.readStringUntil(';');
    angle=angleStr.toInt();
    //Serial.flush();
    eAngle=angle*encoderConstant;
    //Serial.print("Angle:");
    //Serial.println(angle);
    //knobLeft.write(0);
    //knobRight.write(0);
  }

  if(eAngle != newLeft){
    error = eAngle-newLeft;
    //Serial.println(diff);
    posControl(error);
}
delay(10);
}
