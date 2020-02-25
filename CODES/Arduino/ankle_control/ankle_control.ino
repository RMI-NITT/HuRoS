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
Encoder motorLeft(19,18);
Encoder motorRight(21,20);
int dirPin1=37, pwmPin1=6, dirPin2=35, pwmPin2=5;
long integral1=0,integral2=0;
long eAngle1=0, eAngle2=0;
//double kp = 0.0025, ki = 0.00001; // double kp = 0.0025, ki = 0.00008;
//double kp = 0.003, ki = 0.00015;
double kp1 = 0.0058, ki1 = 0.0000015;
double kp2 = 0.0065, ki2 = 0.0000010;
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

long pwm1=0;
int dir1;
long pwm2=0;
int dir2;
  
int posControl1(long error1)
{  
  if(abs(error1)<25000){
    integral1 = integral1 + error1;
    }  
  if (abs(error1)<2*encoderConstant){   // 2 degree error
    integral1=0;
    }
  pwm1 = int(kp1*error1 + ki1*integral1);
   
  if(pwm1<0){
    dir1=LEFT;
    }
  else{
    dir1=RIGHT;
    }
  pwm1=abs(pwm1);
  if(pwm1>255){
    pwm1=255;
  }
  digitalWrite(dirPin1, dir1);
  analogWrite(pwmPin1, pwm1);
  return 0;
}

int posControl2(long error2){

  //double kp = 0.0015, ki = 0.00001;
  
  if(abs(error2)<25000){
  integral2 = integral2 + error2; 
  }

  if (abs(error2)<2*encoderConstant){   // 2 degree error
    integral2=0;
  } 
  pwm2 = int(kp2*error2 + ki2*integral2);  
  if(pwm2<0){
    dir2=LEFT;
  }
  else{
    dir2=RIGHT;
  }
  pwm2=abs(pwm2);
  if(pwm2>255){
    pwm2=255;
  }
  digitalWrite(dirPin2, dir2);
  analogWrite(pwmPin2, pwm2);
  return 0;
  }

void loop() {
  long newLeft, newRight,error1, error2;
  long angle1, angle2;
  String angleStr1, angleStr2;
  newLeft = motorLeft.read();
  newRight = motorRight.read();
  if (newLeft != positionLeft || newRight != positionRight) {
    Serial.print("Left = ");
    Serial.print(newLeft/encoderConstant);
    Serial.print(", Right = ");
    Serial.print(newRight/encoderConstant);
    Serial.println(" Wait....");
    positionLeft = newLeft;
    positionRight = newRight;
  }
  
  // angle reading from serial monitor
  if (Serial.available()) {
    angleStr1=Serial.readStringUntil(',');
    angle1=angleStr1.toInt();
    Serial.println(angle1);
    //Serial.flush();
    eAngle1=angle1*encoderConstant;
    
    angleStr2=Serial.readStringUntil('\0');
    angle2=angleStr2.toInt();
    Serial.println(angle2);
    eAngle2=angle2*encoderConstant;
    
  }
  error1=eAngle1-newLeft;
  posControl1(error1);
  
  error2=eAngle2-newRight;
  posControl2(error2);
}
