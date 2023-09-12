#include <M5Stack.h>
#include "utility/MPU9250.h"
#include "player.h"

#define PLAYER M5.Lcd

/*
--------- LCD width x height = 320 x 240 px
---------- gyro-sensor use gy->X-AIS , gz->Y-AXIS
*/

// Global Variable
unsigned long prev_time = 0;
position_t playerPos = {160, 120};
speed_t playerSpeed = {2.5, 2.5};

// End Global Variable

// Sensor Variable
MPU9250 mpu; // gryo-sensor

// End Sensor Variable

void setup()
{
  Serial.begin(115200);
  M5.begin();     // M5stack begin
  M5.Lcd.begin(); // Lcd Begin
  Wire.begin();
  mpu.initMPU9250(); // init gyro-sensor
  mpu.calibrateMPU9250(mpu.gyroBias, mpu.accelBias);
  PLAYER.drawCircle((int)playerPos.x, (int)playerPos.y, 5, RED); // calibrate gyro-sensor value
}

void loop()
{
  if (millis() - prev_time >= 33) // read gyro every 33 ms => 30Hz
  {
    read_gyro();
    // Serial.printf("X = %f, Y = %f\n", (mpu.gy * mpu.ay) / 10, (mpu.gz * mpu.az) / 10);
    PLAYER.drawCircle((int)playerPos.x, (int)playerPos.y, 5, BLACK);
    playerMove();
    Serial.println(playerPos.y);
    prev_time = millis();
  }
  PLAYER.drawCircle((int)playerPos.x, (int)playerPos.y, 5, RED);
}

void read_gyro()
{
  if (mpu.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    mpu.readGyroData(mpu.gyroCount);
    mpu.readAccelData(mpu.accelCount);
    mpu.getGres();
    mpu.getAres();
    mpu.gx = (float)mpu.gyroCount[0] * mpu.gRes;
    mpu.gy = (float)mpu.gyroCount[1] * mpu.gRes;
    mpu.gz = (float)mpu.gyroCount[2] * mpu.gRes;
    mpu.ax = (float)mpu.accelCount[0] * mpu.aRes;
    mpu.ay = (float)mpu.accelCount[1] * mpu.aRes;
    mpu.az = (float)mpu.accelCount[2] * mpu.aRes;
  }
}

void playerMove()
{
  float x = (mpu.gy) / 10;
  float y = (mpu.gx) / 10;
  playerPos.x += (int)x;
  playerPos.y += (int)y;
  if (playerPos.x < 0)
    playerPos.x = 0;
  if (playerPos.x > M5.Lcd.width())
    playerPos.x = M5.Lcd.width();
  if (playerPos.y < 0)
    playerPos.y = 0;
  if (playerPos.y > M5.Lcd.height())
    playerPos.y = M5.Lcd.height();
}
