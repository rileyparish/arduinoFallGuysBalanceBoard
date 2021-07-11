#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <Keyboard.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

float curX, curY, curZ; // variables for accelerometer raw data
float roll,pitch,curRoll,curPitch=0;

const int numKeys = 4;

const char keyCodes[numKeys] = {'w', 'a', 's','d'};
const float rollThresholds[numKeys] = {0, 20, -4, -23};
const float pitchThresholds[numKeys] = {-28, -1, 14, -9};


float tolerance = 5;        // how many degrees of tolerance to register an input (so the movement doesn't have to be as precise)
// every character/key has these attributes
struct AccelInput {
    char keycode;
    float roll;
    float pitch;
    boolean wasActive = false;
};


boolean shouldActivate = false;     // this keeps track of whether a given key should be pressed or not given the current readings

// this is an array of AccelInput structs called "Inputs" that contains numKeys items in it (wasd)
AccelInput Inputs[numKeys];


void setup() {
    Serial.begin(115200);
    // set up the accelerometer
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    // initialize the wasd structs
    for(int i = 0; i < numKeys; i++){
        Inputs[i].keycode = keyCodes[i];
        Inputs[i].roll = rollThresholds[i];
        Inputs[i].pitch = pitchThresholds[i];
    }
}


void loop() {
    // this function reads from the accelerometer and calculates the roll and pitch, which are stored in curRoll and curPitch
    getRollPitch();

    // now determine whether each wasd key needs to be pressed or released
    for(int i = 0; i < numKeys; i++){
        shouldActivate = Inputs[i].wasActive;       // assume that the state will not change
        // determine whether the key is within the range to register a press
        if(isWithinRange(curRoll, Inputs[i].roll) && isWithinRange(curPitch, Inputs[i].pitch)){
            // if the values are within range, the key should be pressed
            shouldActivate = true;
        }else{
            shouldActivate = false;
        }
        
        // if the activity state on this iteration is different from the previous iteration, press or release the key
        if(shouldActivate != Inputs[i].wasActive){
            if(shouldActivate){
                 Keyboard.press(Inputs[i].keycode);
            }else{
                 Keyboard.release(Inputs[i].keycode);
            }
            Inputs[i].wasActive = shouldActivate;
        }
    }
}

// is the current roll/pitch within range of the target values for a given key?
bool isWithinRange(float curVal, float targetVal){
    return ((curVal > targetVal - tolerance) && (curVal < targetVal + tolerance));
}

// reads from the accelerometer and calculate the roll and pitch of the board
void getRollPitch(){
    // Read from the accelerometer to get the position of the balance board
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 3*2, true); // used to be 7 * 2 but since I only need 3 pieces of data
    
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
    // .94 and .06 were the initial values. Sometimes newer values would come in that knocked the reading out of range so we got multiple key presses at the start
    curRoll = 0.98 * curRoll + 0.02 * roll;
    curPitch = 0.98 * curPitch + 0.02 * pitch;
}

