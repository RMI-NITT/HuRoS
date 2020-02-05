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
Encoder motorknee(20,21);

int dirPin1=38, pwmPin1=7, dirPin2=36, pwmPin2=6, dirPin2=37, pwmPin2=5;
long integral1=0,integral2=0;
long eAngle1=0, eAngle2=0, eAnglek=0;
//double kp = 0.0025, ki = 0.00001; // double kp = 0.0025, ki = 0.00008;
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
  pinMode(dirPin3, OUTPUT);
  pinMode(pwmPin3, OUTPUT);  
}

long positionLeft  = -999;
long positionRight = -999;
long positionknee = -999;

int posControl1(long error)
{
  long pwm=0;
  int dir;
  
  if(abs(error)<25000){
    integral1 = integral1 + error;
    }  
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
  digitalWrite(dirPin1, dir);
  analogWrite(pwmPin1, pwm);
  return 0;
}

int posControl2(long error){
  long pwm=0;
  //double kp = 0.0015, ki = 0.00001;
  int dir;
  
  if(abs(error)<25000){
  integral2 = integral2 + error; 
  }

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
  digitalWrite(dirPin2, dir);
  analogWrite(pwmPin2, pwm);
  return 0;
  }

int posControlk(long error){
  long pwm=0;
  //double kp = 0.0015, ki = 0.00001;
  int dir;
  
  if(abs(error)<25000){
  integralk = integralk + error; 
  }

  if (abs(error)<2*encoderConstant){   // 2 degree error
    integralk=0;
  } 
  pwm = int(kp*error + ki*integralk);  
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
  digitalWrite(dirPink, dir);
  analogWrite(pwmPink, pwm);
  return 0;
  }

void loop() {
  long newLeft, newRight,newknee, error;
  long angle;
  String angleStr;
  newLeft = motorLeft.read();
  newRight = motorRight.read();
  newknee = motorknee.read();
  if (newLeft != positionLeft || newRight != positionRight) {
    Serial.print("Left = ");
    Serial.print(newLeft/encoderConstant);
    Serial.print(", Right = ");
    Serial.print(newRight/encoderConstant);
    Serial.print(", knee = ");
    Serial.print(newknee/encoderConstant);
    Serial.println(" Wait....");
    positionLeft = newLeft;
    positionRight = newRight;
    positionknee = newknee;
  }
  
  // angle reading from serial monitor
  if (Serial.available()) {
    angleStr=Serial.readStringUntil(',');
    angle=angleStr.toInt();
    Serial.println(angle);
    eAngle1=angle*encoderConstant;
    
    angleStr=Serial.readStringUntil(';');
    angle=angleStr.toInt();
    Serial.println(angle);
    eAngle2=angle*encoderConstant;

    angleStr=Serial.readStringUntil('\0');
    angle=angleStr.toInt();
    Serial.println(angle);
    eAnglek=angle*encoderConstant;
    
  }
  error=eAngle1-newLeft;
  posControl1(error);
  
  error=eAngle2-newRight;
  posControl2(error);

  error= eAnglek-newknee;
  posControlk(error);
}
