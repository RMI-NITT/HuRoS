#include <Encoder.h>
Encoder motorLeft(2,3);
int dirPin=38, brk = 4,pwmPin = 7,pwm1 = 25,angle,eangle;
String angleStr;
long encoder_const= 29250/360;
void setup() {
  Serial.begin(9600);
  Serial.println("Encoder Test:");
  pinMode(dirPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(brk,0);
  pinMode(brk,OUTPUT);
    while (!(Serial.available())){ Serial.println("hi");}
    angleStr=Serial.readStringUntil(';');
    angle=angleStr.toInt();
    eangle = angle*encoder_const;
    Serial.println("ho");
    
}
void loop() {
  long newLeft;
  
  newLeft = motorLeft.read();
  Serial.print("leftangle:");
  Serial.println(newLeft);
  analogWrite(pwmPin,pwm1);
  Serial.print(eangle);
  
  if( newLeft >= eangle)
   {  digitalWrite(brk,1);
              /*while(1)
                   {Serial.println("leftangle:");
                    Serial.print(newLeft);
                   }*/
   }
}
