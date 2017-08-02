// Infinitag Libs
#include <SPI.h>
#include <Wire.h>
#include <sensor_dhcp_server.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Infinitag_Core.h>

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
const int8_t displayResetPin = 4;
const int8_t displayDcPin = 5;
const int8_t displayCsPin = 6;
bool alive = true;
unsigned long timeOfDeath = 0;
unsigned int currentScreen = 0; // 0 = home / 1 = inGame / 2 = gameStats

// Timings
unsigned int intensity = 255;

// Infinitag Inits
SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);
Infinitag_Core infinitagCore;

Adafruit_SH1106 display(51, 52, displayDcPin, displayResetPin, displayCsPin);

// Vendor Inits
IRsend irsend;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, muzzleLedPin, NEO_GRBW + NEO_KHZ800);

#include "Game.h"
Game game(irsend, infinitagCore, strip);

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
  display.begin(SH1106_SWITCHCAPVCC);
  display.display();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
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
        displayGameData();
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
  displayGameBasisInfo();
  currentScreen = 1;
}

void loopHomescreen() {
  display.clearDisplay();
  display.setTextSize(1);
  
  String text = "Homescreen";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  display.setCursor(0, 0);
  display.println(textBuf);
  display.writeFastHLine(0, 14, 128, WHITE);
  
  text = "Press [Enter] to play";
  text.toCharArray(textBuf, 50);
  display.setCursor(0, 30);
  display.println(textBuf);

  display.display();
  
  if (enterBtnState == HIGH) {
    startGame();
    return;
  }
  
  delay(100);
}

void receiveEvent() {
  Serial.println("receiveEvent");
  int byteCounter = 0;
  byte data[4] = {
    B0,
    B0,
    B0,
    B0,
  };
  
  while (Wire.available()) {
    data[byteCounter] = Wire.read();
    Serial.println(data[byteCounter]);
    byteCounter++;
  }
  
  game.receiveShot(data, byteCounter);
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

void displayGameTime() {
  display.setTextSize(1);
  
  // Bar
  int barMaxWidth = 94;
  
  display.writeFastHLine(0, 60, barMaxWidth, WHITE);
  display.writeLine(barMaxWidth, 60, barMaxWidth, 64, WHITE);

  display.writeFillRect(0, 61, barMaxWidth, 4, BLACK);
  int barSize = barMaxWidth - (game.timeDiff * barMaxWidth / game.timePlayTime);
  if (barSize < 0) {
    barSize = 0;
  }
  display.writeFillRect(0, 61, barSize, 4, WHITE);

  // Time
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  display.writeFillRect(97, 52, 31, 12, BLACK);
  String timeText = "";
  if (game.timeDiffMinutes < 10) {
    timeText += "0";
  }
  timeText += game.timeDiffMinutes;
  timeText += ":";
  if (game.timeDiffSeconds < 10) {
    timeText += "0";
  }
  timeText += game.timeDiffSeconds;
  char timeBuf[6];
  timeText.toCharArray(timeBuf, 6);
  display.setCursor(97, 57);
  display.println(timeBuf);

  display.display();
}

void displayGameData() {
  display.setTextSize(2);
  int posX3Left = 19;
  int posX3Right = 73;
  int posX2Left = 26;
  int posX2Right = 79;
  int posX1Left = 32;
  int posX1Right = 85;

  // links 16
  display.writeFillRect(posX3Left, 16, 38, 18, BLACK);
  display.setCursor((game.playerAmmo < 10) ? posX1Left : ((game.playerAmmo < 100) ? posX2Left : posX3Left), 16);
  display.println(game.playerAmmo);

  // rechts
  display.writeFillRect(posX3Right, 16, 38, 18, BLACK);
  display.setCursor((game.playerHealth < 10) ? posX1Right : ((game.playerHealth < 100) ? posX2Right : posX3Right), 16);
  display.println(game.playerHealth);
  
  display.display();
  
  displayGameTime();
}


void displayGameBasisInfo() {
  display.clearDisplay();
  display.setTextSize(1);

  // Infinitag Smybol
  // Kann noch nicht richtig mit der aktuellen Lib abgebildet werden
  display.writeLine(25, 2, 48, 2, WHITE);
  display.writeLine(25, 49, 48, 49, WHITE);
  display.writeLine(10, 24, 10, 27, WHITE);
  display.writeLine(25, 2, 10, 24, WHITE);
  display.writeLine(10, 27, 25, 49, WHITE);
  display.writeLine(48, 2, 59, 19, WHITE);
  
  display.writeLine(48, 49, 78, 2, WHITE);
  
  display.writeLine(78, 2, 101, 2, WHITE);
  display.writeLine(78, 49, 101, 49, WHITE);
  display.writeLine(116, 24, 116, 27, WHITE);
  display.writeLine(101, 2, 116, 24, WHITE);
  display.writeLine(116, 27, 101, 49, WHITE);
  display.writeLine(78, 49, 67, 32, WHITE);

  // Spieler Angabe
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  String displayPlayerText = "P";
  displayPlayerText += game.playerId;
  char charPlayerBuf[10];
  displayPlayerText.toCharArray(charPlayerBuf, 10);
  display.setCursor(0, 0);
  display.println(charPlayerBuf);

  // Team Angabe
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  String displayTeamText = "T";
  displayTeamText += game.playerTeamId;
  char charTeamBuf[10];
  displayTeamText.toCharArray(charTeamBuf, 10);
  display.setCursor(112, 0);
  display.println(charTeamBuf);

  display.display();

  displayGameData();
}

