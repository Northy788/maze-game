#include <M5Stack.h>
#include "utility/MPU9250.h"
#include "player.h"
#include <stdlib.h>

#define TONE_USE_INT
#define TONE_PITCH 440
#include <TonePitch.h>

#define PLAYER M5.Lcd
#define MAP M5.Lcd
#define BG M5.Speaker

// fuction decle
void read_gyro(void);
void playerMove(void);
void play_bg_music(void);
//
/*
--------- LCD width x height = 320 x 240 px
---------- gyro-sensor use gy->X-AIS , gz->Y-AXIS
*/

// Global Variable
unsigned long prev_time = 0;     // check time update gyro and player positon
position_t playerPos = {10, 10}; // player position
speed_t playerSpeed = {1, 1};    // player move pixel/time
uint8_t player_radius = 5;       // player size circle
int note_index = 0;              // index to run music note
unsigned long note_duration = 0; // check time chang note
uint16_t bg_music_note[144] = {
    NOTE_B1, NOTE_C2, NOTE_D2, NOTE_F2, NOTE_G2, NOTE_E2, // 1
    NOTE_C2, NOTE_D2, NOTE_E2, NOTE_G2, NOTE_B2, NOTE_F2, // 2
    NOTE_D2, NOTE_E2, NOTE_F2, NOTE_B2, NOTE_C3, NOTE_G2, // 3
    NOTE_E2, NOTE_F2, NOTE_G2, NOTE_C3, NOTE_D3, NOTE_G2, // 4
    NOTE_F2, NOTE_G2, NOTE_B2, NOTE_D3, NOTE_E3, NOTE_C3, // 5
    NOTE_B2, NOTE_C3, NOTE_D3, NOTE_F3, NOTE_G3, NOTE_E3, // 6
    NOTE_C3, NOTE_D3, NOTE_E3, NOTE_G3, NOTE_B3, NOTE_F3, // 7
    NOTE_D3, NOTE_E3, NOTE_F3, NOTE_B3, NOTE_C4, NOTE_G3, // 8
    NOTE_D4, NOTE_C4, NOTE_B4, NOTE_G4, NOTE_E3, NOTE_G4, // 9
    NOTE_C4, NOTE_B4, NOTE_A4, NOTE_F3, NOTE_D3, NOTE_F3, // 10
    NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E3, NOTE_C3, NOTE_E3, // 11
    NOTE_G4, NOTE_F3, NOTE_E3, NOTE_C3, NOTE_B2, NOTE_D3, // 12
    NOTE_F3, NOTE_E3, NOTE_D3, NOTE_B2, NOTE_G2, NOTE_B3, // 13
    NOTE_E3, NOTE_D3, NOTE_C3, NOTE_G2, NOTE_F2, NOTE_B2, // 14
    NOTE_D3, NOTE_C3, NOTE_B2, NOTE_G2, NOTE_E2, NOTE_G2, // 15
    NOTE_C3, NOTE_B2, NOTE_G2, NOTE_E2, NOTE_D2, NOTE_C2, // 16

    NOTE_B1, NOTE_C2, NOTE_D2, NOTE_F2, NOTE_G2, NOTE_E2, // 1
    NOTE_C2, NOTE_D2, NOTE_E2, NOTE_G2, NOTE_B2, NOTE_F2, // 2
    NOTE_D2, NOTE_E2, NOTE_F2, NOTE_B2, NOTE_C3, NOTE_G2, // 3
    NOTE_E2, NOTE_F2, NOTE_G2, NOTE_C3, NOTE_D3, NOTE_G2, // 4
    NOTE_F2, NOTE_G2, NOTE_B2, NOTE_D3, NOTE_E3, NOTE_C3, // 5
    NOTE_B2, NOTE_C3, NOTE_D3, NOTE_F3, NOTE_G3, NOTE_E3, // 6
    NOTE_C3, NOTE_D3, NOTE_E3, NOTE_G3, NOTE_B3, NOTE_F3, // 7
    NOTE_D3, NOTE_E3, NOTE_F3, NOTE_B3, NOTE_C4, NOTE_G3  // 8
};
int duration[] = {
    400, 400, 400, 400, 400, 400, // 1
    400, 400, 400, 400, 400, 400, // 2
    400, 400, 400, 400, 400, 400, // 3
    400, 400, 400, 400, 400, 400, // 4
    395, 395, 395, 395, 395, 395, // 5
    395, 395, 395, 395, 395, 395, // 6
    387, 387, 387, 387, 387, 387, // 7
    387, 387, 387, 387, 387, 387, // 8
    315, 315, 315, 315, 315, 315, // 9
    312, 312, 312, 312, 312, 312, // 10
    307, 307, 307, 307, 307, 307, // 11
    304, 304, 304, 304, 304, 304, // 12
    303, 303, 303, 303, 303, 303, // 13
    301, 301, 301, 301, 301, 301, // 14
    300, 300, 300, 300, 300, 300, // 15
    300, 300, 300, 300, 300, 300, // 16

    333, 333, 333, 333, 333, 333, // 1
    333, 333, 333, 333, 333, 333, // 2
    333, 333, 333, 333, 333, 333, // 3
    324, 324, 324, 324, 324, 324, // 4
    330, 330, 330, 330, 330, 330, // 5
    331, 331, 331, 331, 331, 331, // 6
    331, 331, 331, 331, 331, 331, // 7
    331, 331, 331, 331, 331, 331  // 8
};

// End Global Variable

// Sensor Variable
MPU9250 mpu; // gryo-sensor

// End Sensor Variable

void setup()
{
  Serial.begin(115200);
  M5.begin();     // M5stack begin
  M5.Lcd.begin(); // Lcd Begin
  M5.Speaker.begin();

  Wire.begin();
  mpu.initMPU9250();                                 // init gyro-sensor
  mpu.calibrateMPU9250(mpu.gyroBias, mpu.accelBias); // calibrate gyro-sensor valu

  note_duration = millis(); // prepare to start music
}

void loop()
{
  BG.update(); // Update speaker
  play_bg_music();

  // if (millis() - prev_time >= 16) // read gyro every 16 ms => 60Hz
  // {
  //   read_gyro();  // read x and y  axis on gyro-sensor
  //   playerMove(); // if not collision move TARS
  //   prev_time = millis();
  // }
}
// sensor read
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

// end about sensor read

// about player
void playerMove(void)
{
  float x = (mpu.gy) / 10;
  float y = (mpu.gx) / 10;
  // check collision with map
  if (isCollision() != 1 && (int)x > 0)
  {
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.x += playerSpeed.x;
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  else if (isCollision() != 2 && (int)x < 0)
  {
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.x -= playerSpeed.x;
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  if (isCollision() != 3 && (int)y > 0)
  {
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.y += playerSpeed.y;
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  else if (isCollision() != 4 && (int)y < 0)
  {
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.y -= playerSpeed.y;
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  // check collision with max width-height
  if (playerPos.x < (player_radius + 1))
  { // left
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.x = player_radius + 1;
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  if (playerPos.x > (M5.Lcd.width() - 1) - (player_radius + 1))
  { // right
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.x = (M5.Lcd.width() - 1) - (player_radius + 1);
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  if (playerPos.y < (player_radius + 1))
  { // up
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.y = player_radius + 1;
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
  if (playerPos.y > (M5.Lcd.height() - 1) - (player_radius + 1))
  { // down
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, BLACK);
    playerPos.y = (M5.Lcd.height() - 1) - (player_radius + 1);
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
  }
}

int isCollision()
{
  for (int i = 0; i <= player_radius; i++) // the player is circle but we check around it by squre
  {
    // right collision
    if (PLAYER.readPixel((int)(playerPos.x + (player_radius + 1)), (int)playerPos.y + i) == 0x99E2)
      return 1;
    if (PLAYER.readPixel((int)(playerPos.x + (player_radius + 1)), (int)playerPos.y - i) == 0x99E2)
      return 1;
    // left collision
    if (PLAYER.readPixel((int)(playerPos.x - (player_radius + 1)) - playerSpeed.x, (int)playerPos.y + i) == 0x99E2)
      return 2;
    if (PLAYER.readPixel(((int)playerPos.x - (player_radius + 1)) - playerSpeed.x, (int)playerPos.y - i) == 0x99E2)
      return 2;
    // down collision
    if (PLAYER.readPixel((int)playerPos.x + i, ((int)playerPos.y + (player_radius + 1))) == 0x99E2)
      return 3;
    if (PLAYER.readPixel((int)playerPos.x - i, ((int)playerPos.y + (player_radius + 1))) == 0x99E2)
      return 3;
    // up collision
    if (PLAYER.readPixel((int)playerPos.x + i, ((int)playerPos.y - (player_radius + 1))) == 0x99E2)
      return 4;
    if (PLAYER.readPixel((int)playerPos.x - i, ((int)playerPos.y - (player_radius + 1))) == 0x99E2)
      return 4;
  }
  return 0; // not collision
}
// end about player

// about map
//  draw map object at pos-x pos-y
void drawShelf(uint16_t x, uint16_t y)
{
  int index = 0; // index of book shelf array
  // book shelf bit map 21 x 23 (width x height)
  uint16_t book_shelf[483] = {0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7,
                              0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7,
                              0x9AC7, 0x9AC7, 0x99E2, 0x99E2, 0x8266, 0xFD16, 0xF1AA, 0xFBE0, 0xFBE0, 0xFF80,
                              0xFF80, 0xE8E4, 0x8266, 0x8266, 0x8266, 0x8266, 0x2589, 0x2589, 0x2589, 0x5BD,
                              0x5BD, 0x8266, 0x8266, 0x99E2, 0x99E2, 0x8266, 0xFD16, 0xF1AA, 0xFBE0, 0xFBE0,
                              0xFF80, 0xFF80, 0xE8E4, 0xE8E4, 0x8266, 0x8266, 0x8266, 0x2589, 0x2589, 0x2589,
                              0x5BD, 0x5BD, 0x8266, 0x8266, 0x99E2, 0x99E2, 0x8266, 0xFD16, 0xF1AA, 0xFBE0,
                              0xFBE0, 0xFFD7, 0xFFD7, 0xE8E4, 0xFD16, 0xFD16, 0x8266, 0x8266, 0xD7D7, 0xD7D7,
                              0xD7D7, 0x5BD, 0x5BD, 0x8266, 0x8266, 0x99E2, 0x99E2, 0x8266, 0x99A8, 0x99A8,
                              0xE54F, 0xE54F, 0xFF80, 0xFF80, 0xFD16, 0xFD16, 0xFD16, 0xFD16, 0x8266, 0x2589,
                              0x2589, 0x2589, 0x9EDD, 0x9EDD, 0x8266, 0x8266, 0x99E2, 0x99E2, 0x8266, 0x99A8,
                              0x99A8, 0xE54F, 0xE54F, 0xFF80, 0xFF80, 0x8266, 0xFD16, 0xFD16, 0xE8E4, 0xE8E4,
                              0x2589, 0x2589, 0x2589, 0x9EDD, 0x9EDD, 0x8266, 0x8266, 0x99E2, 0x99E2, 0x8266,
                              0x99A8, 0x99A8, 0xFBE0, 0xFBE0, 0xFF80, 0xFF80, 0x8266, 0x8266, 0x8266, 0xE8E4,
                              0xE8E4, 0x2589, 0x2589, 0x2589, 0x5BD, 0x5BD, 0x8266, 0x8266, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7,
                              0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7, 0x9AC7,
                              0x99E2, 0x99E2, 0x9AC7, 0x8266, 0x368C, 0x368C, 0xFA0C, 0xFD16, 0x8266, 0xFBE0,
                              0xFBE0, 0xFF80, 0xFF80, 0xAF23, 0xAF23, 0x2589, 0xFBE0, 0xFBE0, 0xE8E4, 0xE8E4,
                              0x8266, 0x99E2, 0x99E2, 0x9AC7, 0x8266, 0xAF23, 0xAF23, 0xFA0C, 0xFD16, 0x8266,
                              0xFBE0, 0xFBE0, 0xFF80, 0xFF80, 0xFFD7, 0xFFD7, 0x2589, 0xFBE0, 0xFBE0, 0x9806,
                              0x9806, 0x8266, 0x99E2, 0x99E2, 0x9AC7, 0x8266, 0xAF23, 0xAF23, 0xFA0C, 0xFD16,
                              0x8266, 0xFBE0, 0xFBE0, 0xFF80, 0xFF80, 0xAF23, 0xAF23, 0x2589, 0xF733, 0xF733,
                              0x9806, 0x9806, 0x8266, 0x99E2, 0x99E2, 0x9AC7, 0x8266, 0x368C, 0x368C, 0xFA0C,
                              0xFD16, 0xFD16, 0xFBE0, 0xFBE0, 0xFF80, 0xFFD7, 0xAF23, 0xAF23, 0x2589, 0xFBE0,
                              0xFBE0, 0xE8E4, 0xE8E4, 0x8266, 0x99E2, 0x99E2, 0x9AC7, 0x8266, 0x368C, 0x368C,
                              0xFA0C, 0xFA0C, 0xFD16, 0xFF80, 0xFF80, 0xFF80, 0xFFD7, 0xAF23, 0xAF23, 0x2589,
                              0xFBE0, 0xFBE0, 0xE8E4, 0xE8E4, 0x8266, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x9AC7, 0xFF80,
                              0xFE01, 0x5BD, 0x8266, 0x8266, 0x8266, 0x4B7E, 0x9EDD, 0x8266, 0xE54F, 0xE54F,
                              0xE8E4, 0xE8E4, 0xAF23, 0xAF23, 0x8266, 0x8266, 0x9AC7, 0x99E2, 0x99E2, 0x9AC7,
                              0xFF80, 0xFE01, 0x5BD, 0x5BD, 0x8266, 0x8266, 0x4B7E, 0x9EDD, 0xE54F, 0xE54F,
                              0x4228, 0xFF80, 0xFF80, 0xAF23, 0xAF23, 0xAF23, 0x8266, 0x8266, 0x99E2, 0x99E2,
                              0x9AC7, 0xFF80, 0xFE01, 0x9EDD, 0x5BD, 0x5BD, 0x8266, 0x4B7E, 0x9EDD, 0xE54F,
                              0x4228, 0x4228, 0xE8E4, 0xE8E4, 0x2589, 0x2589, 0xAF23, 0xAF23, 0x8266, 0x99E2,
                              0x99E2, 0x9AC7, 0xFF80, 0xFE01, 0x9EDD, 0x9EDD, 0x5BD, 0x5BD, 0x4228, 0x4228,
                              0xE54F, 0x4228, 0x8266, 0xE8E4, 0xE8E4, 0x8266, 0x2589, 0x2589, 0xAF23, 0x9AC7,
                              0x99E2, 0x99E2, 0x9AC7, 0xFF80, 0xFE01, 0x99E2, 0x99E2, 0x99E2, 0x9EDD, 0x4B7E,
                              0x9EDD, 0x4228, 0x4228, 0x8266, 0xE8E4, 0xE8E4, 0x8266, 0x8266, 0x8266, 0x2589,
                              0x9AC7, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2, 0x99E2,
                              0x99E2, 0x99E2, 0x99E2};
  for (int i = 0 + y; i < 23 + y; i++)
  {
    for (int j = 0 + x; j < 21 + x; j++)
    {
      if (index < 21 * 23)
        MAP.drawPixel(j, i, book_shelf[index]);
      index++;
    }
  }
}
// end about map

// about music
void play_bg_music()
{
  if (millis() - note_duration >= (int)((duration[note_index]) / 2))
  {
    BG.tone(bg_music_note[note_index] * 4); // play note
    note_index++;                           // change note
    note_duration = millis();               // check time
    if (note_index >= 144)                  // check is last note?
    {
      note_index = 48; // set to start note
    }
  }
}
// end about music

// about simple page

// end about simple page
