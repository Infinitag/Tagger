// Infinitag Libs
#include <sensor_dhcp_server.h>
#include <Infinitag_SH1106.h>
#include <Infinitag_GFX.h>
#include <Infinitag_Core.h>
#include "Game.h"

// Vendor Libs
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

// Button Settings
int rightBtnPin = 34;
int rightBtnState = 0;
int leftBtnPin = 35;
int leftBtnState = 0;
int downBtnPin = 36;
int downBtnState = 0;
int upBtnPin = 37;
int upBtnState = 0;
int specialBtnPin = 44;
int specialBtnState = 0;
int infoBtnPin = 45;
int infoBtnState = 0;
int reloadBtnPin = 46;
int reloadBtnState = 0;
int fireBtnPin = 47;
int fireBtnState = 0;
int enterBtnPin = 48;
int enterBtnState = 0;
int resetBtnPin = 49;
int resetBtnState = 0;

// Settings
const int muzzleLedPin = 8;
const int displayResetPin = 4;
const int displayDcPin = 5;
const int displayCsPin = 6;
bool alive = true;
unsigned long timeOfDeath = 0;
unsigned int currentScreen = 0; // 0 = home / 1 = inGame / 2 = gameStats

// Timings
unsigned int intensity = 255;

// Infinitag Inits
SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);
Infinitag_Core infinitagCore;
sh1106_spi display = create_display(displayResetPin, displayDcPin, displayCsPin);
Framebuffer framebuffer;

// Vendor Inits
IRsend irsend;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, muzzleLedPin, NEO_GRBW + NEO_KHZ800);

Game game(framebuffer, display, irsend, infinitagCore, strip);

void setup() {
  Serial.begin(57600);
  
  Serial.println("booting Server...");
  SensorServer.initialize();
  
  Serial.println("booting Events...");
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  Serial.println("booting Pins...");
  pinMode(fireBtnPin, INPUT);
  pinMode(muzzleLedPin, OUTPUT);

  Serial.println("booting Display...");
  SPI.begin();
  initialize_display(&display);
  
  Serial.println("booting LEDs...");
  strip.begin();
  colorWipe(strip.Color(0,0,0,0));
  
  Serial.println("boot completed");

  game.initButtons(rightBtnPin, leftBtnPin, downBtnPin, upBtnPin, specialBtnPin, infoBtnPin, reloadBtnPin, fireBtnPin, enterBtnPin, resetBtnPin);

  game.updateSensorConfig();
}

void loop() {
  getButtonStates();

  switch(currentScreen) {
    case 2:
      if (enterBtnState == HIGH) {
        startGame();
        return;
      }
      game.loopStats();
      break;
    case 1:
      if (game.isRunning()) {
        game.loop();
      } else {
        currentScreen = 2;
      }
      break;
    case 0:
    default:
      loopHomescreen();
      break;
  }
  
  //SensorServer.scanIfNecessary();
}

void startGame() {
  game.start();
  currentScreen = 1;
}

void loopHomescreen() {
  framebuffer.clear(BLACK);
  
  String text = "Homescreen";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 0, WHITE);
  framebuffer.drawHorizontalLine(0, 14, 128, WHITE);
  
  text = "Press [Enter] to play";
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 30, WHITE);

  display_buffer(&display, framebuffer.getData());
  
  if (enterBtnState == HIGH) {
    startGame();
    return;
  }
  
  delay(100);
}

void receiveEvent() {
  Serial.println("receiveEvent");
  while (Wire.available()) {
    Serial.println(Wire.read());
  }
}

void requestEvent() {
  //maybe we have enough time to look for the next free address?
  Serial.println("requested");
  SensorServer.registerNewClient();
}

void colorWipe(uint32_t c) {
  for (uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void getButtonStates() {
  rightBtnState = digitalRead(rightBtnPin);
  leftBtnState = digitalRead(leftBtnPin);
  downBtnState = digitalRead(downBtnPin);
  upBtnState = digitalRead(upBtnPin);
  specialBtnState = digitalRead(specialBtnPin);
  infoBtnState = digitalRead(infoBtnPin);
  reloadBtnState = digitalRead(reloadBtnPin);
  fireBtnState = digitalRead(fireBtnPin);
  enterBtnState = digitalRead(enterBtnPin);
  resetBtnState = digitalRead(resetBtnPin);
}


void sendCmd(byte data[], unsigned int byteLength) {
  // Ziel muss noch bestimmt werden
  Wire.beginTransmission(0x22);
  Wire.write(data, byteLength);
  Wire.endTransmission();
}

void sendCmdSetGameId(unsigned int gameId) {
  byte data[2] = {
    0x01, 
    gameId
  };
  sendCmd(data, 2);
}

void sendCmdSetPlayerId(unsigned int playerId) {
  byte data[2] = {
    0x02, 
    playerId
  };
  sendCmd(data, 2);
}

void sendCmdSetSensorId(unsigned int sensorId) {
  byte data[2] = {
    0x03, 
    sensorId
  };
  sendCmd(data, 2);
}

void sendCmdSetAnimation(unsigned int animationId, unsigned int duration, unsigned int colorR, unsigned int colorG, unsigned int colorB, unsigned int colorW, unsigned int repeat) {
  byte data[9] = {
    0x04, 
    animationId, 
    duration, 
    duration >> 8, 
    colorR, 
    colorG, 
    colorB, 
    colorW, 
    repeat
  };
  sendCmd(data, 9);
}


void sendCmdPing(unsigned int senderId) {
  byte data[2] = {
    0x05, 
    senderId
  };
  sendCmd(data, 2);
}

