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
Encoder motorLeft(2,3);
Encoder motorRight(18,19);
int dirPin1=38, pwmPin1=7, dirPin2=36, pwmPin2=6;
long integral1=0,integral2=0;
long eAngle1=0, eAngle2=0;
//double kp = 0.0025, ki = 0.00001;
double kp = 0.003, ki = 0.00015;
#define LEFT 0
#define RIGHT 1
long encoderConstant = 29520/360; //Counts per revolution for 100RPM motor
//   avoid using pins with LEDs attached

void setup() {
  Serial.begin(9600);
  Serial.println("Encoder Test:");
  pinMode(dirPin1, OUTPUT);
  pinMode(pwmPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin2, OUTPUT);
}

long positionLeft  = -999;
long positionRight = -999;

int posControl1(long error){
  long pwm=0;
 // double kp = 0.0025, ki = 0.00008;
  int dir;
  
  if(abs(error)<25000){
  integral1 = integral1 + error;
  }  
  //Serial.print("error:");
  //Serial.print(error/encoderConstant);

  if (abs(error)<2*encoderConstant){   // 2 degree error
    integral1=0;
  }
  pwm = int(kp*error + ki*integral1);
   
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
  
  //Serial.print("PWM:");
  //Serial.println(pwm);
  
  digitalWrite(dirPin1, dir);
  analogWrite(pwmPin1, pwm);
  //Serial.print("Dir:");
  //Serial.println(dir);
 // Serial.println("Run");
  return 0;
}

int posControl2(long error){
  long pwm=0;
  //double kp = 0.0015, ki = 0.00001;
  int dir;
  
  if(abs(error)<25000){
  integral2 = integral2 + error;
  
  }

  /*
  Serial.print("error:");
  Serial.print(error/encoderConstant);

  
  Serial.print(" integral:");
  Serial.println(integral);
  */
  if (abs(error)<2*encoderConstant){   // 2 degree error
    integral2=0;
  }
  
  pwm = int(kp*error + ki*integral2);
  
  
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
  
  
  Serial.print("PWM:");
  Serial.println(pwm);
  */
  digitalWrite(dirPin2, dir);
  analogWrite(pwmPin2, pwm);
  //Serial.print("Dir:");
  //Serial.println(dir);
 // Serial.println("Run");
  return 0;
  }

void loop() {
  long newLeft, newRight,error;
  long angle;
  String angleStr;
  newLeft = motorLeft.read();
  newRight = motorRight.read();
  if (newLeft != positionLeft || newRight != positionRight) {
    Serial.print("Left = ");
    Serial.print(newLeft/encoderConstant);
    Serial.print(", Right = ");
    Serial.print(newRight/encoderConstant);
    //Serial.print(" error:");
  //Serial.print(error);
  //Serial.print(" integral:");
  //Serial.println(integral);
    Serial.println(" Wait....");
    positionLeft = newLeft;
    positionRight = newRight;
  }
  //Serial.flush();
  // angle reading from serial monitor
  if (Serial.available()) {
    //Serial.println("Reading....");
    angleStr=Serial.readStringUntil(',');
    angle=angleStr.toInt();
    Serial.println(angle);
    //Serial.flush();
    eAngle1=angle*encoderConstant;
    
    angleStr=Serial.readStringUntil(';');
    angle=angleStr.toInt();
    Serial.println(angle);
    //Serial.flush();
    eAngle2=angle*encoderConstant;
    
    //Serial.print("Angle:");
    //Serial.println(angle);
    //knobLeft.write(0);
    //knobRight.write(0);
  }
  error=eAngle1-newLeft;
  posControl1(error);
  
  error=eAngle2-newRight;
  //Serial.println(error);
  posControl2(error);
}
