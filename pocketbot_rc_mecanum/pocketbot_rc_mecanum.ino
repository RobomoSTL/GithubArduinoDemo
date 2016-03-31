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

/* Struct to hold RC channel data */
struct RC {
  int rawValue;
  int filteredValue;
  int maxValue = INPUT_MAX;
  int minValue = INPUT_MIN;
};

RC rc;

RunningMedian runningMedian(3);

void setup() {
  //Start Serial for debuging
  Serial.begin(115200);
  //Set RC pins to input
  pinMode(CH1_PIN, INPUT);
}

void loop() {
  //Read raw RC values
  rc.rawValue = pulseIn(CH1_PIN, HIGH);
  //Use Median filter to remove spikes
  runningMedian.add(rc.rawValue);
  //Get filtered result
  rc.filteredValue = runningMedian.getMedian();
  //Convert to power
  int ch1Power = map(rc.filteredValue, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
  //Print result
  Serial.print(rc.filteredValue);
  Serial.print(" => ");
  Serial.println(ch1Power);
}
