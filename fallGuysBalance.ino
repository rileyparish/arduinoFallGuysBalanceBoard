// (c) Michael Schoeffler 2017, http://www.mschoeffler.de

#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <Keyboard.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

float curX, curY, curZ; // variables for accelerometer raw data
float roll,pitch,rollF,pitchF=0;

float tolerance = 2;        // how many degrees of tolerance to register an input (so the movement doesn't have to be as precise)
// every character/key has these attributes
struct AccelInput {
    char keycode;
    float pitch;
    float roll;
    boolean wasPressed = false;
};



// this is an array of TouchInput structs called "Pins" that contains 4 items in it (wasd)
AccelInput Pins[4];


void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
}


void loop() {
    // this reads from the accelerometer and calculates the pitch and roll, which are stored in pitchF and rollF
    getPitchRoll();


    


    Serial.print(rollF);
    Serial.print("/");
    Serial.println(pitchF);

    delay(10);

    
}

// reads from the accelerometer calculates the pitch and roll of the board
void getPitchRoll(){
    
    // Read from the accelerometer to get the position of the balance board
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
    
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
  
    // Low-pass filter
    rollF = 0.94 * rollF + 0.06 * roll;
    pitchF = 0.94 * pitchF + 0.06 * pitch;
}


/*
If the circuit has been completed or if the balance board readings are within a given threshold, press the key
Otherwise, release the key

But I don't think I want to be sending "press" all the time because they'll be treated as separate presses.
So I need to keep track of whether the key is active or not and only press or release when the state has changed.


Data members:
    active - this is true if the button is currently being pressed
    keycode - 


I do need to use the pair of coords to detect a given key press so I can be sure there's no overlap.


If rollF ~ 0 && pitchF ~ -28
    press w
else
    release w


get accel data
determine if the key has changed state from the last iteration
if it has, press or release the associated key



*/




