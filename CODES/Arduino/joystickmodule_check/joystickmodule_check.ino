void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  Serial.print("x:");
  Serial.print(analogRead(A0));// put your main code here, to run repeatedly:
  Serial.print("y:");
  Serial.println(analogRead(A1));

}
