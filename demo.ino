#include <M5Stack.h>
#include "utility/MPU9250.h"

MPU9250 mpu; // create gyro-sensor object

typedef struct poistion
{
  int16_t x;
  int16_t y;
} Position;

Position pos;
// LCD width x height = 320 x 240 px
void setup()
{
  // Serial.begin(115200);
  M5.begin();
  Wire.begin();
  mpu.initMPU9250(); // init gyro-sensor
  mpu.calibrateMPU9250(mpu.gyroBias, mpu.accelBias);
  mpu.g
}

void loop()
{
}
