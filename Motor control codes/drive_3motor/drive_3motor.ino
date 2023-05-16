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

String str;
int dur1 = 0, dur2 = 0, dur3 = 0;

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
  
  Serial.begin(115200); //initialize serial comunication
  
  digitalWrite(MOT1_PWM,HIGH);
  digitalWrite(MOT2_PWM,HIGH);
  digitalWrite(MOT3_PWM,HIGH);
}

void loop() {
  Serial.println("DUR1");
  while(!Serial.available());
  str = Serial.readStringUntil('\r');
  dur1 = str.toInt();
  Serial.println("DUR2");
  while(!Serial.available());
  str = Serial.readStringUntil('\r');
  dur2 = str.toInt();
  Serial.println("DUR3");
  while(!Serial.available());
  str = Serial.readStringUntil('\r');
  dur3 = str.toInt();
  Serial.println(dur1);
  Serial.println(dur2);
  Serial.println(dur3);
  digitalWrite(MOT1_BRK, LOW);
  digitalWrite(MOT2_BRK, LOW);
  digitalWrite(MOT3_BRK, LOW);
  
  digitalWrite(MOT1_DIR, HIGH); 
  digitalWrite(MOT2_DIR, HIGH);
  digitalWrite(MOT3_DIR, HIGH);
  for (int i = 0; i <= 300; i++){
//    Serial.print(encoderValue1);
//    Serial.print(" | ");
//    Serial.print(encoderValue2);
//    Serial.print(" | ");
//    Serial.println(encoderValue3);
}
  digitalWrite(MOT1_BRK, HIGH);
  digitalWrite(MOT2_BRK, HIGH);
  digitalWrite(MOT3_BRK, HIGH);
  
  delay(dur1);
  
  digitalWrite(MOT1_BRK, LOW);
  digitalWrite(MOT2_BRK, LOW);
  digitalWrite(MOT3_BRK, LOW);
  digitalWrite(MOT1_DIR, LOW); 
  digitalWrite(MOT2_DIR, LOW);
  digitalWrite(MOT3_DIR, LOW);
  
  for (int i = 0; i <= 300; i++){
//    Serial.print(encoderValue1);
//    Serial.print(" | ");
//    Serial.print(encoderValue2);
//    Serial.print(" | ");
//    Serial.println(encoderValue3);
}
  digitalWrite(MOT1_BRK, HIGH);
  digitalWrite(MOT2_BRK, HIGH); 
  digitalWrite(MOT3_BRK, HIGH);
    
  delay(2000);
} 
