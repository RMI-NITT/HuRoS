/* Encoder Library - TwoKnobs Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */
#include <Encoder.h>
Encoder motorLeft(20,21);
int dirPin=34, pwmPin=5;
long integral=0;
long eAngle=0;
#define LEFT 0
#define RIGHT 1
long encoderConstant = 29520/360; //Counts per revolution for 100RPM motor
//   avoid using pinswith LEDs attached

void setup() {
  Serial.begin(9600);
  Serial.println("Encoder Test:");
  pinMode(dirPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  if (Serial.available()) 
  {
    angleStr=Serial.readStringUntil(';');
    angle=angleStr.toInt();
    eAngle=angle*encoderConstant;
  }
}

long positionLeft  = -999;
long positionRight = -999;
 

void loop() 
{
  long newLeft, newRight, error;
  long angle;
  String angleStr;
  newLeft = motorLeft.read();

  if (newLeft != positionLeft) 
    {
      positionLeft = newLeft;
    }



  if(eAngle != newLeft)
    {
      error = eAngle-newLeft;
      posControl(error);
    }
delay(10);
}




int posControl(long error)
{
  long pwm=0;
  double kp = 0.001, ki = 0.00001;
  int dir;
  integral = integral + error; 
  Serial.print(" integral:");
  Serial.println(integral);
  
  

  Serial.print("PWM:");
  Serial.println(pwm);
  Serial.print("Dir");
  Serial.println(dir);
  
  digitalWrite(dirPin, dir);
  analogWrite(pwmPin, pwm);
  return 0;
  }
  
  /*if (abs(error)<2*encoderConstant)
    {   // 2 degree error
      integral=0;
      Serial.println("REach");
    }
  pwm = int(kp*error + ki*integral); 
  
  if(pwm<0)
    {
      dir=LEFT;
    }
  else
    {
      dir=RIGHT;
    }
  pwm=abs(pwm);
  
  if(pwm>255)
    {
      pwm=255;
    }
    */
