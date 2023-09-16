#include <M5Stack.h>
#include "utility/MPU9250.h"
#include "player.h"
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define TONE_USE_INT
#define TONE_PITCH 440
#include <TonePitch.h>

#include "addons/TokenHelper.h" //Provide the token generation process info.
#include "addons/RTDBHelper.h"  //Provide the RTDB payload printing info and other helper functions.

#define SCREEN M5.Lcd
#define PLAYER M5.Lcd
#define MAP M5.Lcd
#define BG M5.Speaker
#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15

// fuction declaration
void read_gyro(void);
void playerMove(void);
void play_bg_music(void);
void waiting_page(void);
void select_map_page(void);
void rgb_on(uint32_t c);
void wifi_connect(void);
void firebase_connect(void);
//
/*
--------- LCD width x height = 320 x 240 px
---------- gyro-sensor use gy->X-AIS , gz->Y-AXIS
*/

// Global Variable
// RGB bar
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
unsigned long prev_time = 0;       // check time update gyro and player positon
unsigned long screen_time = 0;     // check screen change frame
position_t playerPos = {160, 120}; // player position
speed_t playerSpeed = {1, 1};      // player move pixel/time
uint8_t player_radius = 5;         // player size circle
uint8_t point = 0;
int note_index = 0;              // index to run music note
unsigned long note_duration = 0; // check time chang note
uint8_t vol = 2;
uint8_t song_state = 1;
/*
  4 status
  1. wait for connect mobile device
  2. connected -> selected map/mode
  3. play
  4. end -> selected map/mode
*/
String status = "WAIT";
const char *wifi_ssid = "Pi";               // wifi password
const char *wifi_password = "hehepassword"; // wifi password
const char *API_KEY = "AIzaSyBgy3jxmfAm9tbQf5lv8SuGFygwmKuSAew";
const char *DATABASE_URL = "https://maze-game-demo-cbd12-default-rtdb.asia-southeast1.firebasedatabase.app";
String device_name = "MAZE-GAME-Device";
String database_path = "";
uint8_t isAuthen = 0;
String fuid = "";
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

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

// map
uint8_t maze_map1[10][15] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1}};
uint8_t maze_map2[10][15] = {
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
uint8_t maze_map3[10][15] = {
    {0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    {1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1}};
uint8_t maze_map4[10][15] = {
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1}};
uint8_t maze_map5[10][15] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1}};

uint8_t maze_maps[5][10][15] = {maze_map1, maze_map2, maze_map3, maze_map4, maze_map5};
// end map

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
  M5.Speaker.setVolume(vol);
  Wire.begin();
  mpu.initMPU9250();                                 // init gyro-sensor
  mpu.calibrateMPU9250(mpu.gyroBias, mpu.accelBias); // calibrate gyro-sensor valu
  delay(1000);
  pixels.begin();
  // wifi_connect();
  // firebase_connect();
  // send_default_status();
  note_duration = millis(); // prepare to start music
  screen_time = millis();
}

void loop()
{
  M5.update();
  BG.update(); // Update speaker
  play_bg_music();
  if (status == "WAIT")
  {
    waiting_page();
  }
  else if (status == "CONNECTED")
  {
    select_map_page();
  }
  else if (status == "PLAY")
  {
    BG.end();
    draw_maze_map();
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE);
    while (1)
    {

      if (millis() - prev_time >= 16) // read gyro every 16 ms => 60Hz
      {
        read_gyro();  // read x and y  axis on gyro-sensor
        playerMove(); // if not collision move TARS
        prev_time = millis();
      }
    }
  }
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
  x *= 21;
  y *= 23;
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
void waiting_page()
{
  String str = "Connecting";
  if (millis() - screen_time >= 400)
  {
    SCREEN.drawString(str, 120, 100, 2);
    screen_time = millis();
    if (point < 3)
    {
      SCREEN.drawChar('.', 190 + (point * 5), 100, 2);
      point++;
    }
    else
    {
      SCREEN.fillRect(190, 100, 30, 20, BLACK);
      point = 0;
    }
  }
}

void select_map_page()
{
  String str = "Selecte map";
  if (millis() - screen_time >= 400)
  {
    SCREEN.drawString(str, 120, 100, 2);
    screen_time = millis();
    if (point < 3)
    {
      SCREEN.drawChar('.', 200 + (point * 5), 100, 2);
      point++;
    }
    else
    {
      SCREEN.fillRect(200, 100, 30, 20, BLACK);
      point = 0;
    }
  }
}
// end about simple page

void rgb_on(uint32_t c)
{
  for (int i = 0; i < 9; i++)
  {
    pixels.setPixelColor(i, c);
    pixels.show();
  }
}

void wifi_connect(void)
{
  WiFi.mode(WIFI_STA);                  // Wifi station mode
  WiFi.begin(wifi_ssid, wifi_password); // Wifi connecting 2.4 Ghz only
  rgb_on(pixels.Color(255, 0, 0));      // not connect display red colr
  while (WiFi.status() != WL_CONNECTED)
  {
  }                                  //
  rgb_on(pixels.Color(255, 128, 0)); // connected wifi display yellow color
}

void firebase_connect(void)
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  if (Firebase.signUp(&config, &auth, "", ""))
  { // anonymus mode
    isAuthen = 1;
    database_path += "/" + device_name;
    fuid = auth.token.uid.c_str();
    rgb_on(pixels.Color(128, 255, 0));
  }
  else
  {
    config.signer.signupError.message.c_str();
    isAuthen = 0;
    rgb_on(pixels.Color(0, 0, 255));
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
}

void send_default_status()
{
  if (isAuthen && Firebase.ready())
  {
    String node = database_path + "/status";
    if (Firebase.set(fbdo, node.c_str(), status))
    {
      rgb_on(pixels.Color(128, 255, 0));
    }
    else
    {
      rgb_on(pixels.Color(255, 128, 0));
    }
  }
}

void draw_maze_map()
{

  for (int y = 0; y < 10; y++)
  {
    for (int x = 0; x < 15; x++)
    {
      if (maze_map2[y][x])
        drawShelf(x, y);
    }
  }
}