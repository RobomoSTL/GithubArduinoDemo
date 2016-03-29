/*
* Demo app showing Pocket/RC control of a Mecanum Wheeled Robot
*/
#include <RunningMedian.h>

#define CH1_PIN 3

//Min and Max values from RC Channel
#define INPUT_MIN 993
#define INPUT_MAX 1977
//Min and Max values for Motors
#define OUTPUT_MIN -127
#define OUTPUT_MAX 127

/** Median filter to remove spikes with 5 samples */
RunningMedian channel1RunningMedian(3);

void setup() {
  //Start Serial for debuging
  Serial.begin(115200);
  //Set RC pins to input
  pinMode(CH1_PIN, INPUT);
}

void loop() {
  //Read raw RC values
  int ch1Val = pulseIn(CH1_PIN, HIGH);
  //Convert to power
  int ch1Power = map(ch1Val, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
  //Use Median filter to remove spikes
  channel1RunningMedian.add(ch1Power);
  //Print result
  Serial.print(ch1Val);
  Serial.print(" => ");
  Serial.println(channel1RunningMedian.getMedian());
}
