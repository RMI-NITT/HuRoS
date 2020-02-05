/*
 * Encoder example sketch
 * by Andrew Kramer
 * 1/1/2016
 *
 * Records encoder ticks for each wheel
 * and prints the number of ticks for
 * each encoder every 500ms
 *
 *Motor - RMCS-3013
 *Black - GND
 *Brown - Vcc
 *Red - EncA
 *Orange - EncB
 *Yellow - Motor+
 *Green - Motor-
 */
 
// pins for the encoder inputs
#define ENCODER_A 2 
#define ENCODER_B 3
#define ENABLE 13
#define MOTOR_A1 9
#define MOTOR_A2 10
//A1 - HIGH A2 - LOW ---> count  ++
//A1 - LOW A2 - HIGH ---> count  --
// variables to store the number of encoder pulses
// for each motor
volatile long Count = 0;
int reached = 1,angle;
float Kp=0.1,Kd=0.05;
int error=0, prevError=0;
 
void setup() {
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(ENABLE, OUTPUT);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  
  // initialize hardware interrupts
  attachInterrupt(1, EncoderEvent, CHANGE);
  
  Serial.begin(9600);
}
 
void loop() {
  if(reached == 1)
  {
    while (Serial.available());
    Serial.println("Enter angle:");
    angle = Serial.parseInt(); 
    Serial.println(angle);
    reached = 0;
  }
  servo_reach();
  Serial.print("Count: ");
  Serial.println(Count);
  Serial.print("Reached: ");
  Serial.println(reached);
  Serial.println();
  delay(500);
}
//servo motor function
void servo_reach()
{
  double setPoint = angle*40.888889;
  Serial.println(setPoint);
  error = setPoint - Count;
  Serial.print("Error: ");
  Serial.println(error);
  float u = Kp*error - Kd*(error - prevError) ;
  Serial.print("u: ");
  Serial.println(u);
    if(u>0)
    {
     if(u>255) u=255;
     if(u<130) u=130;
     analogWrite(ENABLE, u);
     digitalWrite(MOTOR_A1, HIGH);
     digitalWrite(MOTOR_A2, LOW);
    }
    else if (u<0)
    {
     u *= -1;
     if(u>255) u=255;
     if(u<120) u=120;
     analogWrite(ENABLE, u);
     digitalWrite(MOTOR_A1, LOW);
     digitalWrite(MOTOR_A2, HIGH);
    } 
    if(abs(error)<100)
    {
      digitalWrite(MOTOR_A1, LOW);
      digitalWrite(MOTOR_A2, LOW);
      reached = 1;    
    }
    prevError = error;
} 
// encoder event for the interrupt call
void EncoderEvent() {
  if (digitalRead(ENCODER_A) == HIGH) {
    if (digitalRead(ENCODER_B) == LOW) {
      Count++;
    } else {
      Count--;
    }
  } else {
    if (digitalRead(ENCODER_B) == LOW) {
      Count--;
    } else {
      Count++;
    }
  }
}
