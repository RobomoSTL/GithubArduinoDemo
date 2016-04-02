/*
* Demo app showing Pocket/RC control of a Mecanum Wheeled Robot
*/
#include <RunningMedian.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h>


#define CH1_PIN 3
#define CH2_PIN 4
#define CH3_PIN 5
#define CH4_PIN 6
#define CH5_PIN 7
#define CH6_PIN 8

#define USE_POCKET_BOT 1
#define USE_RC 0

#if USE_POCKET_BOT
  #include <PocketBot.h> // https://github.com/frankjoshua/PocketBot
  /* This will be used to decode messages from the Android device */
  PocketBot pocketBot;
  /* Allocate space for the decoded message. */
  PocketBotMessage message = PocketBotMessage_init_zero;
  //Min and Max values from input
  #define INPUT_MIN -1
  #define INPUT_MAX 1
#endif
#if USE_RC
  //Min and Max values from input
  #define INPUT_MIN 993
  #define INPUT_MAX 1977
  #define CHANNELS 6

  /* Struct to hold RC channel data */
  struct RC {
  int pin;
  int rawValue;
  int filteredValue;
  int maxValue = INPUT_MAX;
  int minValue = INPUT_MIN;
  };
  
  RC rcChannels[CHANNELS];
  
  #define SAMPLES 1
  RunningMedian runningMedian[CHANNELS]{
    RunningMedian(SAMPLES),
    RunningMedian(SAMPLES),
    RunningMedian(SAMPLES),
    RunningMedian(SAMPLES),
    RunningMedian(SAMPLES),
    RunningMedian(SAMPLES),
  };
#endif


//Min and Max values for Motors
#define OUTPUT_MIN -127
#define OUTPUT_MAX 127

/* Struct to hold motor states */
struct Motors{
  int frontLeft = 0;
  int frontRight = 0;
  int rearLeft = 0;
  int rearRight = 0;
};

//Hold current state of the motors
Motors motorsCurrent;

//Setup Adafruit motor shield
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motorRearLeft = AFMS.getMotor(1);
Adafruit_DCMotor *motorRearRight = AFMS.getMotor(2);
Adafruit_DCMotor *motorFrontLeft = AFMS.getMotor(4);
Adafruit_DCMotor *motorFrontRight = AFMS.getMotor(3);

//Time since last motor update
int mLastMotorUpdate = 0;

/*
* Helper function to map Floats, based on Arduino map()
*/
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void driveMotors(){
    //Set motors to FORWARD or BACKWARD
    if(motorsCurrent.rearLeft < 0){
      motorRearLeft->run(BACKWARD);
    } else {
      motorRearLeft->run(FORWARD);
    }
    if(motorsCurrent.rearRight < 0){
      motorRearRight->run(BACKWARD);
    } else {
      motorRearRight->run(FORWARD);
    }
    if (motorsCurrent.frontLeft < 0) {
      motorFrontLeft->run(BACKWARD);
    } else {
      motorFrontLeft->run(FORWARD);
    }
    if (motorsCurrent.frontRight < 0) {
      motorFrontRight->run(BACKWARD);
    } else {
      motorFrontRight->run(FORWARD);
    }
    
    //Adjust power from -127:127 to 1:255
    int frontRight = abs(motorsCurrent.frontRight) * 2 + 1;
    int frontLeft = abs(motorsCurrent.frontLeft) * 2 + 1;
    int rearRight = abs(motorsCurrent.rearRight) * 2 + 1;
    int rearLeft = abs(motorsCurrent.rearLeft) * 2 + 1;
    
    //Set motor speeds
    motorRearRight->setSpeed(rearRight);
    motorRearLeft->setSpeed(rearLeft);		
    motorFrontRight->setSpeed(frontRight);
    motorFrontLeft->setSpeed(frontLeft);
}

void updateMotors(float maxPower, float baseSpeed, float inDir, float inStrafe){
    int minSpeed =  constrain(mapfloat(maxPower, INPUT_MIN, INPUT_MAX, 0, OUTPUT_MIN), OUTPUT_MIN, OUTPUT_MAX);
    int maxSpeed =  constrain(mapfloat(maxPower, INPUT_MIN, INPUT_MAX, 0, OUTPUT_MAX), OUTPUT_MIN, OUTPUT_MAX);
    int mSpeed = mapfloat(baseSpeed, INPUT_MIN, INPUT_MAX, minSpeed, maxSpeed);
    int strafe = mapfloat(inStrafe, INPUT_MIN, INPUT_MAX, minSpeed, maxSpeed);
    int dir = mapfloat(inDir, INPUT_MAX, INPUT_MIN, minSpeed, maxSpeed);
    
    motorsCurrent.rearRight = constrain(mSpeed + dir + strafe, minSpeed, maxSpeed);
    motorsCurrent.rearLeft = constrain(mSpeed - dir - strafe, minSpeed, maxSpeed);
    motorsCurrent.frontRight = constrain(mSpeed + dir - strafe, minSpeed, maxSpeed);
    motorsCurrent.frontLeft = constrain(mSpeed - dir + strafe, minSpeed, maxSpeed);
}

#if USE_RC
  void readRc(){
    //Loop through rc channels and get filtered value
    for(int i = 0; i < CHANNELS; i++){
      //Read raw RC values
      rcChannels[i].rawValue = constrain(pulseIn(rcChannels[i].pin, HIGH), INPUT_MIN, INPUT_MAX);
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
#endif

void setup() {
  //Start Serial for PocketBot
  Serial.begin(115200);
  #if USE_RC
    //Start RC
    initRc();
  #endif
  //Start Adafruit motor shield
  AFMS.begin();
}

void loop() {

  if(millis() - mLastMotorUpdate > 50){
    mLastMotorUpdate = millis();
    driveMotors();
  } else {
    #if USE_RC
    readRc();
    updateMotors(rcChannels[2].filteredValue, rcChannels[1].filteredValue,
                rcChannels[0].filteredValue, rcChannels[3].filteredValue);
    #endif
    #if USE_POCKET_BOT
      //Check for PocketBot message
      if(pocketBot.read(Serial, message)){
        /* This code will only be called if a complete message is received*/
        updateMotors(message.control.joy2.Y, message.control.joy1.Y,
                    message.control.joy1.X, message.control.joy2.X);
      }
    #endif
  }
}
