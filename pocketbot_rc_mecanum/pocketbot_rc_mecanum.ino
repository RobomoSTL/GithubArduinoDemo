/*
* Demo app showing Pocket/RC control of a Mecanum Wheeled Robot
*/
#include <RunningMedian.h>

#define CH1_PIN 3
#define CH2_PIN 4
#define CH3_PIN 5
#define CH4_PIN 6
#define CH5_PIN 7
#define CH6_PIN 8

//Min and Max values from RC Channel
#define INPUT_MIN 993
#define INPUT_MAX 1977
//Min and Max values for Motors
#define OUTPUT_MIN -127
#define OUTPUT_MAX 127

#define CHANNELS 6

/* Struct to hold RC channel data */
struct RC {
  int pin;
  int rawValue;
  int filteredValue;
  int maxValue = INPUT_MAX;
  int minValue = INPUT_MIN;
};

/* Struct to hold motor states */
struct Motors{
  int frontLeft = 0;
  int frontRight = 0;
  int rearLeft = 0;
  int rearRight = 0;
};

Motors motorsCurrent;

RC rcChannels[CHANNELS];

RunningMedian runningMedian[CHANNELS]{
  RunningMedian(3),
  RunningMedian(3),
  RunningMedian(3),
  RunningMedian(3),
  RunningMedian(3),
  RunningMedian(3),
};

void setup() {
  //Start Serial for debuging
  Serial.begin(115200);
  //Start RC
  initRc();
}

void loop() {
  readRc();
  updateMotors(rcChannels[2].filteredValue, rcChannels[1].filteredValue, rcChannels[0].filteredValue, rcChannels[3].filteredValue);
  driveMotors();
}

void driveMotors(){
  Serial.print("Front Left: ");
  Serial.println(motorsCurrent.frontLeft);
  Serial.print("Front Right: ");
  Serial.println(motorsCurrent.frontRight);
  Serial.print("Rear Left: ");
  Serial.println(motorsCurrent.rearLeft);
  Serial.print("Rear Right: ");
  Serial.println(motorsCurrent.rearRight);
}

void updateMotors(int maxPower, int baseSpeed, int inDir, int inStrafe){
    int minSpeed =  constrain(map(maxPower, INPUT_MIN, INPUT_MAX, 0, OUTPUT_MIN), OUTPUT_MIN, OUTPUT_MAX);
    int maxSpeed =  constrain(map(maxPower, INPUT_MIN, INPUT_MAX, 0, OUTPUT_MAX), OUTPUT_MIN, OUTPUT_MAX);
    int mSpeed = map(baseSpeed, INPUT_MIN, INPUT_MAX, minSpeed, maxSpeed);
    int strafe = map(inStrafe, INPUT_MIN, INPUT_MAX, minSpeed, maxSpeed);
    int dir = map(inDir, INPUT_MAX, INPUT_MIN, minSpeed, maxSpeed);
    
    motorsCurrent.rearRight = constrain(mSpeed + dir + strafe, minSpeed, maxSpeed);
    motorsCurrent.rearLeft = constrain(mSpeed - dir - strafe, minSpeed, maxSpeed);
    motorsCurrent.frontRight = constrain(mSpeed + dir - strafe, minSpeed, maxSpeed);
    motorsCurrent.frontLeft = constrain(mSpeed - dir + strafe, minSpeed, maxSpeed);
}

void readRc(){
  //Loop through rc channels and get filtered value
  for(int i = 0; i < CHANNELS; i++){
    //Read raw RC values
    rcChannels[i].rawValue = pulseIn(rcChannels[i].pin, HIGH);
    //Use Median filter to remove spikes
    runningMedian[i].add(rcChannels[i].rawValue);
    //Get filtered result
    rcChannels[i].filteredValue = runningMedian[i].getMedian();
  }
}

void initRc(){
  //Set RC pins to input
  rcChannels[0].pin = CH1_PIN;
  rcChannels[1].pin = CH2_PIN;
  rcChannels[2].pin = CH3_PIN;
  rcChannels[3].pin = CH4_PIN;
  rcChannels[4].pin = CH5_PIN;
  rcChannels[5].pin = CH6_PIN;
  for(int i = 0; i < CHANNELS; i++){
    pinMode(rcChannels[i].pin, INPUT);
  }
}
