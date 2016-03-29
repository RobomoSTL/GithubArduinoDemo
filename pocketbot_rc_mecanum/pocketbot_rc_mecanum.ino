/*
* Demo app showing Pocket/RC control of a Mecanum Wheeled Robot
*/

#define CH1_PIN 3

void setup() {
  Serial.begin(115200);
  pinMode(CH1_PIN, INPUT);
}

void loop() {
  int ch1Val = pulseIn(CH1_PIN, HIGH);
  Serial.println(ch1Val);
}
