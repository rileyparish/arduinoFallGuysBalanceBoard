// This is just a quick and dirty script to print out the current roll and pitch of the balance board.
// Tilt the board in various positions and record the output, then copy those values to the corresponding keypresses in fallGuysBalance.ino

#include "Wire.h" // This library allows you to communicate with I2C devices.

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

float curX, curY, curZ; // variables for accelerometer raw data
float roll,pitch,yaw,rollF,pitchF,yawF=0;


void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
}


void loop() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 3*2, true); // request a total of 7*2=14 registers
    
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

    Serial.print(rollF);
    Serial.print("/");
    Serial.println(pitchF);

    delay(10);
}
