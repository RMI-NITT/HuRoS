void setup() {
  // put your setup code here, to run once:
Serial.begin(38400);
}
int a;
void loop() {
  if(digitalRead(3))
  a++;
  Serial.println(a);
  // put your main code here, to run repeatedly:

}
