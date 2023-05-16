// Basic code for running 3 motors forward and backward, and getting the encoder count

#define MOT1_DIR  35   // Motor1 direction pin
#define MOT1_BRK  41   // Motor1 brake pin
#define MOT2_DIR  37   // Motor2 direction pin
#define MOT2_BRK  43   // Motor2 brake pin
#define MOT3_DIR  39   // Motor3 direction pin
#define MOT3_BRK  45   // Motor3 brake pin

#define MOT1_PWM  5   // Motor 1 PWM pin
#define MOT2_PWM  6   // Motor 2 PWM pin
#define MOT3_PWM  7   // Motor 3 PWM pin

int MOT1_ECA = 20;   // Motor 1 encoder A pin
int MOT1_ECB = 21;   // Motor 1 encoder B pin
int MOT2_ECA = 18;   // Motor 2 encoder A pin
int MOT2_ECB = 19;   // Motor 2 encoder B pin
int MOT3_ECA = 2;   // Motor 3 encoder A pin
int MOT3_ECB = 3;   // Motor 3 encoder B pin

volatile int lastEncoded1 = 0, lastEncoded2 = 0, lastEncoded3 = 0; // Here updated value of encoder store.
volatile long encoderValue1 = 0, encoderValue2 = 0, encoderValue3 = 0; // Raw encoder value

int MSB, LSB, encoded, sum;
void setup() {

  pinMode(MOT1_DIR, OUTPUT); 
  pinMode(MOT2_DIR, OUTPUT); 
  pinMode(MOT3_DIR, OUTPUT); 
  pinMode(MOT1_BRK, OUTPUT); 
  pinMode(MOT2_BRK, OUTPUT); 
  pinMode(MOT3_BRK, OUTPUT);
  pinMode(MOT1_PWM, OUTPUT);
  pinMode(MOT2_PWM, OUTPUT);
  pinMode(MOT3_PWM, OUTPUT);
  
  Serial.begin(9600); //initialize serial comunication
  
  pinMode(MOT1_ECA, INPUT_PULLUP);
  pinMode(MOT1_ECB, INPUT_PULLUP);
  pinMode(MOT2_ECA, INPUT_PULLUP);
  pinMode(MOT2_ECB, INPUT_PULLUP);
  pinMode(MOT3_ECA, INPUT_PULLUP);
  pinMode(MOT3_ECB, INPUT_PULLUP); 

  digitalWrite(MOT1_ECA, HIGH); //turn pullup resistor on
  digitalWrite(MOT1_ECB, HIGH); //turn pullup resistor on
  digitalWrite(MOT2_ECA, HIGH); //turn pullup resistor on
  digitalWrite(MOT2_ECB, HIGH); //turn pullup resistor on
  digitalWrite(MOT3_ECA, HIGH); //turn pullup resistor on
  digitalWrite(MOT3_ECB, HIGH); //turn pullup resistor on

  attachInterrupt(2, updateEncoder1, CHANGE); 
  attachInterrupt(3, updateEncoder1, CHANGE);
  attachInterrupt(4, updateEncoder2, CHANGE); 
  attachInterrupt(5, updateEncoder2, CHANGE);
  attachInterrupt(0, updateEncoder3, CHANGE); 
  attachInterrupt(1, updateEncoder3, CHANGE); 

//  digitalWrite(MOT1_PWM,HIGH);
//  digitalWrite(MOT2_PWM,HIGH);
//  digitalWrite(MOT3_PWM,HIGH);
  analogWrite(MOT1_PWM,100);
  analogWrite(MOT2_PWM,100);
  analogWrite(MOT3_PWM,100);
}

void loop() {
  digitalWrite(MOT1_BRK, LOW);
  digitalWrite(MOT2_BRK, LOW);
  digitalWrite(MOT3_BRK, LOW);
  
  digitalWrite(MOT1_DIR, HIGH); 
  digitalWrite(MOT2_DIR, HIGH);
  digitalWrite(MOT3_DIR, HIGH);
  for (int i = 0; i <= 300; i++){
    Serial.print(encoderValue1);
    Serial.print(" | ");
    Serial.print(encoderValue2);
    Serial.print(" | ");
    Serial.println(encoderValue3);
}
  digitalWrite(MOT1_BRK, HIGH);
  digitalWrite(MOT2_BRK, HIGH);
  digitalWrite(MOT3_BRK, HIGH);
  
  delay(2000);
  
  digitalWrite(MOT1_BRK, LOW);
  digitalWrite(MOT2_BRK, LOW);
  digitalWrite(MOT3_BRK, LOW);
  digitalWrite(MOT1_DIR, LOW); 
  digitalWrite(MOT2_DIR, LOW);
  digitalWrite(MOT3_DIR, LOW);
  
  for (int i = 0; i <= 300; i++){
    Serial.print(encoderValue1);
    Serial.print(" | ");
    Serial.print(encoderValue2);
    Serial.print(" | ");
    Serial.println(encoderValue3);
}
  digitalWrite(MOT1_BRK, HIGH);
  digitalWrite(MOT2_BRK, HIGH); 
  digitalWrite(MOT3_BRK, HIGH);
    
  delay(2000);
} 

void updateEncoder1(){
//  Serial.println("ENCODER1");
  MSB = digitalRead(MOT1_ECA); //MSB = most significant bit
  LSB = digitalRead(MOT1_ECB); //LSB = least significant bit

  encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  sum  = (lastEncoded1 << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue1 --;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue1 ++;

  lastEncoded1 = encoded; //store this value for next time

}

void updateEncoder2(){
//  Serial.println("ENCODER2");
  MSB = digitalRead(MOT2_ECA); //MSB = most significant bit
  LSB = digitalRead(MOT2_ECB); //LSB = least significant bit

  encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  sum  = (lastEncoded2 << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue2 --;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue2 ++;

  lastEncoded2 = encoded; //store this value for next time

}

void updateEncoder3(){
//  Serial.println("ENCODER3");

  MSB = digitalRead(MOT3_ECA); //MSB = most significant bit
  LSB = digitalRead(MOT3_ECB); //LSB = least significant bit

  encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  sum  = (lastEncoded3 << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue3 --;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue3 ++;

  lastEncoded3 = encoded; //store this value for next time

}
