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
//----------------------------------------------------------------------------
// Fuction Declaration

void read_gyro(void);        // read gyro-sensor
void playerMove(void);       // เmove player after read gyro-sensor if value change and player not collision with the wall
void play_bg_music(void);    // play music background (Can you hear the music by Ludwig Göransson from Oppenheimer)
void waiting_page(void);     // show waiting screen while status is "WAIT"
void select_map_page(void);  // show select map screen when Flutter App send "CONNECTED" to M5stack Device
void rgb_on(uint32_t c);     // turn on RGB LED and change color by parameter -> uint32_t c <-
void wifi_connect(void);     // connect wifi
void firebase_connect(void); // connect firebase

// End Fuction Declaration
//-------------------------------------------------------------------------------
/*
            Details
 LCD width x height = 320 x 240 px
 gyro-sensor use gy->X-AIS , gz->Y-AXIS
*/

//-------------------------------------------------------------------
// Global Variable

unsigned long prev_time = 0;       // check time update gyro and player positon
unsigned long screen_time = 0;     // check timr to change screen frame
position_t playerPos = {160, 120}; // player start position
speed_t playerSpeed = {1.5, 1.5};  // player move pixel/time
uint8_t player_radius = 5;         // player radius to draw player in circle
uint8_t point = 0;                 // screen frame have 3 frame (draw ... while waiting)
int note_index = 0;                // index to run music note array
unsigned long note_duration = 0;   // check time chang note
uint8_t vol = 2;                   // Speaker Volume
uint8_t map_number = 0;            // map number -> 1(easy) 2(easy) 3(medium) 4(hard) 5(hard)
uint8_t play_state = 0;            // play_state change to true when status is "PLAY" and change to false when collision with end path of maze
/*
  3 status
  1. "WAIT" for connect mobile device
  2. "CONNECTED" -> selected map
  3. after select map from Flutter app status change to "PLAY"

  after end game status change from "PLAY" to "CONNECTED"
*/
String status = "WAIT";                                                                                      // defualt status is "WAIT"
const char *wifi_ssid = "Pi";                                                                                // wifi password
const char *wifi_password = "hehepassword";                                                                  // wifi password
const char *API_KEY = "AIzaSyBgy3jxmfAm9tbQf5lv8SuGFygwmKuSAew";                                             // Fisebase API key
const char *DATABASE_URL = "https://maze-game-demo-cbd12-default-rtdb.asia-southeast1.firebasedatabase.app"; // Firebase url realtime database
String device_name = "MAZE-GAME-Device";                                                                     // device name
String database_path = "";                                                                                   // data path
uint8_t isAuthen = 0;                                                                                        // status to check firebase is connected
String fuid = "";                                                                                            // store firebase uid
FirebaseData fbdo;                                                                                           // firebase data object read/write data from this object
FirebaseAuth auth;
FirebaseConfig config;

// MUSIC NOTE for background song play during play the game (Music name : Can you hear the music by Ludwig Göransson)
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

    NOTE_B1, NOTE_C2, NOTE_D2, NOTE_F2, NOTE_G2, NOTE_E2, // 17
    NOTE_C2, NOTE_D2, NOTE_E2, NOTE_G2, NOTE_B2, NOTE_F2, // 18
    NOTE_D2, NOTE_E2, NOTE_F2, NOTE_B2, NOTE_C3, NOTE_G2, // 19
    NOTE_E2, NOTE_F2, NOTE_G2, NOTE_C3, NOTE_D3, NOTE_G2, // 20
    NOTE_F2, NOTE_G2, NOTE_B2, NOTE_D3, NOTE_E3, NOTE_C3, // 21
    NOTE_B2, NOTE_C3, NOTE_D3, NOTE_F3, NOTE_G3, NOTE_E3, // 22
    NOTE_C3, NOTE_D3, NOTE_E3, NOTE_G3, NOTE_B3, NOTE_F3, // 23
    NOTE_D3, NOTE_E3, NOTE_F3, NOTE_B3, NOTE_C4, NOTE_G3  // 24
};

// duration of each note (ms)
int duration[144] = {
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

    333, 333, 333, 333, 333, 333, // 17
    333, 333, 333, 333, 333, 333, // 18
    333, 333, 333, 333, 333, 333, // 19
    324, 324, 324, 324, 324, 324, // 20
    330, 330, 330, 330, 330, 330, // 21
    331, 331, 331, 331, 331, 331, // 22
    331, 331, 331, 331, 331, 331, // 23
    331, 331, 331, 331, 331, 331  // 24
};
/*
 Maze map size 15 x 10 (put 10 x 15 because it easy to read)

 MAP 1 : EASY
 MAP 2 : EASY
 MAP 3 : MEDIUM
 MAP 4 : HARD
 MAP 5 : HARD

 0 -> path
 1 -> wall
 2 -> exit point (end map)
*/
uint8_t maze_maps[5][10][15] = {
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
     {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 2, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
     {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
     {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
     {1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1},
     {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1}},

    {{1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
     {1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
     {1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},

    {{0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
     {1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
     {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1},
     {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
     {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
     {1, 0, 2, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1}},

    {{0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
     {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1},
     {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},
     {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
     {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0},
     {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1},
     {1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1},
     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1}},

    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
     {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
     {1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 0, 1},
     {1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
     {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
     {1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1},
     {1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1},
     {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1}}};

// End Global Variable
//----------------------------------------------------------------------------
// Sensor and Componet Variable

MPU9250 mpu;                                                                                                              // gryo-sensor
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800); // RGB bar

// End Sensor Variable
//----------------------------------------------------------------------------

// Setup
void setup()
{
  Serial.begin(115200);
  M5.begin();                                        // M5stack begin
  M5.Lcd.begin();                                    // Lcd Begin
  M5.Speaker.begin();                                // Speaker begin
  M5.Speaker.setVolume(vol);                         // set default speaker volume
  Wire.begin();                                      //
  mpu.initMPU9250();                                 // init gyro-sensor
  mpu.calibrateMPU9250(mpu.gyroBias, mpu.accelBias); // calibrate gyro-sensor valu
  delay(1000);                                       // delay 1 second before setup wifi connect
  pixels.begin();                                    // RGB bar begin
  wifi_connect();                                    // connect wifi
  firebase_connect();                                // connect firebase
  send_default_status();                             // send defualt status ("WAIT") to realtime database
  note_duration = millis();                          // prepare to start music
  screen_time = millis();                            // prepare to start waiting screen
}
// End Setup
//------------------------------------------------------------------------------

// Main program
void loop()
{
  M5.update();
  BG.update();             // Update speaker
  while (status == "WAIT") // while status is "WAIT" show waiting screen
  {
    waiting_page();
  }
  while (status == "CONNECTED" || play_state == 0) // while status is "CONNECTED" and Flutter App has not send map number yet show select map screen
  {
    select_map_page();
  }
  if (status == "PLAY") // if status is play enter to the game
  {
    MAP.clear();                                                                 // clear screen
    draw_maze_map(map_number - 1);                                               // draw map according to map number that receive from Flutter app
    playerPos.x = 160;                                                           // set start position x
    playerPos.y = 120;                                                           // set start position y
    note_index = 0;                                                              // set start note index
    PLAYER.fillCircle((int)playerPos.x, (int)playerPos.y, player_radius, WHITE); // draw player
    BG.begin();                                                                  // speaker ready
    while (play_state)
    {
      play_bg_music();                // play bg music
      if (millis() - prev_time >= 16) // read gyro every 16 ms => 60Hz
      {
        read_gyro();  // read x and y  axis on gyro-sensor
        playerMove(); // if not collision move TARS
        prev_time = millis();
      }
    }
    MAP.clear();                                 // clear screen
    SCREEN.drawString("Good job!", 120, 100, 2); // show "Good job!" message
    status = "CONNECTED";                        // change status to "CONNECTED"
    send_default_status();                       // update status to realtime database
    BG.end();                                    // stop speaker
  }
}
// END main program
//-----------------------------------------------------------------------------

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
    if (PLAYER.readPixel((int)(playerPos.x + (player_radius + playerSpeed.x)), (int)playerPos.y + i) == 0x99E2)
      return 1;
    if (PLAYER.readPixel((int)(playerPos.x + (player_radius + playerSpeed.x)), (int)playerPos.y - i) == 0x99E2)
      return 1;
    // left collision
    if (PLAYER.readPixel((int)(playerPos.x - (player_radius + playerSpeed.x)) - playerSpeed.x, (int)playerPos.y + i) == 0x99E2)
      return 2;
    if (PLAYER.readPixel(((int)playerPos.x - (player_radius + playerSpeed.x)) - playerSpeed.x, (int)playerPos.y - i) == 0x99E2)
      return 2;
    // down collision
    if (PLAYER.readPixel((int)playerPos.x + i, ((int)playerPos.y + (player_radius + playerSpeed.x))) == 0x99E2)
      return 3;
    if (PLAYER.readPixel((int)playerPos.x - i, ((int)playerPos.y + (player_radius + playerSpeed.x))) == 0x99E2)
      return 3;
    // up collision
    if (PLAYER.readPixel((int)playerPos.x + i, ((int)playerPos.y - (player_radius + playerSpeed.x))) == 0x99E2)
      return 4;
    if (PLAYER.readPixel((int)playerPos.x - i, ((int)playerPos.y - (player_radius + playerSpeed.x))) == 0x99E2)
      return 4;

    if (PLAYER.readPixel((int)(playerPos.x + (player_radius + playerSpeed.x)), (int)playerPos.y + i) == GREEN)
      play_state = 0;
    else if (PLAYER.readPixel((int)(playerPos.x + (player_radius + playerSpeed.x)), (int)playerPos.y - i) == GREEN)
      play_state = 0;
    // left collision
    else if (PLAYER.readPixel((int)(playerPos.x - (player_radius + playerSpeed.x)) - playerSpeed.x, (int)playerPos.y + i) == GREEN)
      play_state = 0;
    else if (PLAYER.readPixel(((int)playerPos.x - (player_radius + playerSpeed.x)) - playerSpeed.x, (int)playerPos.y - i) == GREEN)
      play_state = 0;
    // down collision
    else if (PLAYER.readPixel((int)playerPos.x + i, ((int)playerPos.y + (player_radius + playerSpeed.x))) == GREEN)
      play_state = 0;
    else if (PLAYER.readPixel((int)playerPos.x - i, ((int)playerPos.y + (player_radius + playerSpeed.x))) == GREEN)
      play_state = 0;
    // up collision
    else if (PLAYER.readPixel((int)playerPos.x + i, ((int)playerPos.y - (player_radius + playerSpeed.x))) == GREEN)
      play_state = 0;
    else if (PLAYER.readPixel((int)playerPos.x - i, ((int)playerPos.y - (player_radius + playerSpeed.x))) == GREEN)
      play_state = 0;
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
  if (millis() - screen_time >= 100)
  {
    SCREEN.drawString(str, 120, 100, 2);
    if (Firebase.getString(fbdo, "/MAZE-GAME-Device/status"))
    {
      status = fbdo.stringData();
    }
    else
    {
      Serial.println("Error : " + fbdo.errorReason());
    }
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
  if (Firebase.getString(fbdo, "/MAZE-GAME-Device/status"))
  {
    status = fbdo.stringData();
  }
  if (Firebase.getString(fbdo, "/MAZE-GAME-Device/map"))
  {
    map_number = fbdo.intData();
    play_state = 1;
  }
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

void draw_maze_map(uint8_t index)
{

  for (int y = 0; y < 10; y++)
  {
    for (int x = 0; x < 15; x++)
    {
      if (maze_maps[index][y][x] == 2)
        MAP.fillRect(x * 21, y * 23, 10, 10, GREEN);
      if (maze_maps[index][y][x] == 1)
        drawShelf(x, y);
    }
  }
}
