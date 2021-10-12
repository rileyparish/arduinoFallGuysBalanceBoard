#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <Keyboard.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

float curX, curY, curZ; // variables for accelerometer raw data
float roll,pitch,curRoll,curPitch=0;        // variables for the processed accerlerometer data

// the number of keys that I'll code the accelerometer to output
const int numKeys = 8;

int divePin = 7;
int jumpPin = 4;

// these are the roll and pitch thresholds for how far the board should be tilted in a given direction before the keypress registers.
// these values are derived after attaching the accelerometer to the board and observing the output of the calibration script as the board is tilted.
const char keyCodes[numKeys] =          {'w', 'a', 's','d',     'w', 'a', 's','d'};
const char keyCodesDouble[numKeys] =    {'-', '-','-', '-',     'a', 's', 'd', 'w'};
const float rollThresholds[numKeys] =   {-0.5, 20, -2, -23,        15, 14, -18.5, -17.8};
const float pitchThresholds[numKeys] =  {-23, -3, 19, -3,        -15, 11, 10, -16};


float tolerance = 5;        // how many degrees of tolerance to register an input (so the movement doesn't have to be as precise)

// every character/key has these attributes. Some ranges represent two simulaneous key presses (wa, wd, sa, sd)
struct AccelInput {
    char keycode;
    char keycode2;
    float roll;
    float pitch;
    boolean wasActive = false;
};

// inputs for jumping/diving
struct PipeInput {
    char keycode;
    int pin;
    boolean wasActive = false;
};


boolean shouldActivateAccel = false;     // this keeps track of whether a given key should be pressed or not given the current readings
boolean shouldActivatePipe = false;

// this is an array of AccelInput structs called "Inputs" that contains numKeys items in it (wasd)
AccelInput Inputs[numKeys];
PipeInput JumpDive[2];


void setup() {
    Serial.begin(115200);
    // set up the accelerometer
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    // initialize the wasd structs, matching roll/pitch values with the keys I want to press
    for(int i = 0; i < numKeys; i++){
        Inputs[i].keycode = keyCodes[i];
        Inputs[i].keycode2 = keyCodesDouble[i];
        Inputs[i].roll = rollThresholds[i];
        Inputs[i].pitch = pitchThresholds[i];
    }

    // jump and dive pins. There are only two so it's just as easy to assign them directly.
    pinMode(jumpPin, INPUT_PULLUP);
    pinMode(divePin, INPUT_PULLUP);

    JumpDive[0].keycode = ' ';
    JumpDive[0].pin = jumpPin;

    JumpDive[1].keycode = KEY_LEFT_CTRL;
    JumpDive[1].pin = divePin;
}


void loop() {
    // this function reads from the accelerometer and calculates the roll and pitch, which are stored in curRoll and curPitch
    getRollPitch();

    // now determine whether each wasd key needs to be pressed or released
    for(int i = 0; i < numKeys; i++){
        shouldActivateAccel = Inputs[i].wasActive;       // assume that the state will not change
        // determine whether the key is within the range to register a press
        if(isWithinRange(curRoll, Inputs[i].roll) && isWithinRange(curPitch, Inputs[i].pitch)){
            // if the values are within range, the key should be pressed
            shouldActivateAccel = true;
        }else{
            shouldActivateAccel = false;
        }
        
        // if the activity state on this iteration is different from the previous iteration, press or release the key
        if(shouldActivateAccel != Inputs[i].wasActive){
            if(shouldActivateAccel){
                Keyboard.press(Inputs[i].keycode);
                // if this is an input range that represents a double key press, press the other key as well
                if(Inputs[i].keycode2 != '-'){
                    Keyboard.press(Inputs[i].keycode2);
                }
            }else{
                Keyboard.release(Inputs[i].keycode);
                if(Inputs[i].keycode2 != '-'){
                    Keyboard.release(Inputs[i].keycode2);
                }
            }
            Inputs[i].wasActive = shouldActivateAccel;
        }
    }

    // now check if the jump/dive keys need to be pressed or released based on the status of the pins
    for(int i = 0; i < 2; i++){
        shouldActivatePipe = JumpDive[i].wasActive;

        if(digitalRead(JumpDive[i].pin) == LOW){
            shouldActivatePipe = true;
        }else{
            shouldActivatePipe = false;
        }

        if(shouldActivatePipe != JumpDive[i].wasActive){
            if(shouldActivatePipe){
                Keyboard.press(JumpDive[i].keycode);
            }else{
                Keyboard.release(JumpDive[i].keycode);
            }
            JumpDive[i].wasActive = shouldActivatePipe;
        }
    }
}

// is the current roll/pitch of the board within range of the target values for a given key?
bool isWithinRange(float curVal, float targetVal){
    return ((curVal > targetVal - tolerance) && (curVal < targetVal + tolerance));
}

// reads from the accelerometer and calculates the roll and pitch of the board
void getRollPitch(){
    // Read from the accelerometer to get the position of the balance board
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 2*2, true); // used to be 7 * 2 but since I only need 3 pieces of data, I don't need to process the rest
    
    // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
    curX = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
    curY = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
    curZ = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
    curX = curX / 256;
    curY = curY / 256;
    curZ = curZ / 256;


    // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
    roll = atan(curY / sqrt(pow(curX, 2) + pow(curZ, 2))) * 180 / PI;
    pitch = atan(-1 * curX / sqrt(pow(curY, 2) + pow(curZ, 2))) * 180 / PI;
  
    // Low-pass filter (to reduce noise). This is really important because the sensors are pretty noisy
    // .94 and .06 were the initial values. I adjusted the values to reduce multiple keypresses when moving to a new tilt
    curRoll = 0.98 * curRoll + 0.02 * roll;
    curPitch = 0.98 * curPitch + 0.02 * pitch;
}
