#include <Encoder.h>
Encoder motorLeft(2,3);
int dirPin=38, brk = 4,pwmPin = 7,pwm1 = 25,angle,eangle;
String angleStr;
  long pwm=0,t1,t2,t;
  double kp = 0.005, ki = 0.000003,kd = 0.000001;  
  int dir;
long encoder_const= 29250/360,error,integral,derivative;
long positionLeft  = -999;
#define LEFT 0
#define RIGHT 1
void setup() {
  Serial.begin(115200);
  Serial.println("Encoder Test:");
  
  pinMode(dirPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(brk,0);
  pinMode(brk,OUTPUT);
    while (!(Serial.available())){ }
    angleStr=Serial.readStringUntil(';');
    angle=angleStr.toInt();
    eangle = angle*encoder_const;
   // Serial.println("ho");
}
void loop() {
  long newLeft;
  newLeft = motorLeft.read();
  t1=micros();
  if ((Serial.available()))
  {angleStr=Serial.readStringUntil(';');
  angle=angleStr.toInt();
  eangle = angle*encoder_const;
  }
  t2=micros();
  t=t2-t1;
  //Serial.println(t);
  if(eangle != newLeft)
  {
    error = eangle-newLeft;
    posControl(error);
  }

}

int posControl(long error)
{

  integral = integral + error;
  if (abs(error)< 2*encoder_const)
  {   // 2 degree error
  integral=0;
  //Serial.println("REach");
  }
  derivative = error/0.001;
  pwm = (kp*error  );
  pwm = int(pwm+ ki*integral);
  //pwm = int(pwm+ kd*derivative);
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
  Serial.print(pwm);
  Serial.print(":PWM");
  
  Serial.print("Dir");
  Serial.println(dir);
  
  digitalWrite(dirPin, dir);
  analogWrite(pwmPin, pwm);
  return 0;
}
