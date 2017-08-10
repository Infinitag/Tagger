/*
  Tagger.ino - Tagger Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/
// Basic Libs
#include <SPI.h>
#include <Wire.h>

// Infinitag Libs
#include <sensor_dhcp_server.h>
#include <Infinitag_Core.h>

// Vendor Libs
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

// Settings
#include "Settings.h"

// Infinitag Inits
SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);
Infinitag_Core infinitagCore;

// Vendor Inits
Adafruit_SH1106 display(51, 52, displayDcPin, displayResetPin, displayCsPin);
IRsend irSend;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, muzzleLedPin, NEO_GRBW + NEO_KHZ800);

#include "Game.h"
Game game(irSend, infinitagCore, strip);

void setup() {
  Serial.begin(57600);
  
  Serial.println("booting DHCP-Server...");
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
  
  Serial.println("booting Game");
  game.initButtons(rightBtnPin, leftBtnPin, downBtnPin, upBtnPin, specialBtnPin, infoBtnPin, reloadBtnPin, fireBtnPin, enterBtnPin, resetBtnPin);
  game.updateSensorConfig();
}

/*
 * Loops
 */
void loop() {
  getButtonStates();

  switch(currentScreen) {
    case 2:
      if (enterBtnState == HIGH) {
        startGame();
        return;
      }
      displayGameStats();
      break;
    case 1:
      if (game.isRunning()) {
        game.loop();
        displayGameData();
        if (game.reloadDisplay) {
          displayGameBasisInfo();
          game.reloadDisplay = false;
        }
      } else {
        currentScreen = 2;
      }
      break;
    case 0:
    default:
      loopHomescreen();
      break;
  }
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

/*
 * Events
 */
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

/*
 * Display Game-Informations
 * ToDo: At this poit no idea how to call the display
 * fron this file and also from the Game lib
 */
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

  // Ammo
  display.writeFillRect(posX3Left, 16, 38, 18, BLACK);
  display.setCursor((game.playerAmmo < 10) ? posX1Left : ((game.playerAmmo < 100) ? posX2Left : posX3Left), 16);
  display.println(game.playerAmmo);

  // Health
  display.writeFillRect(posX3Right, 16, 38, 18, BLACK);
  display.setCursor((game.playerHealth < 10) ? posX1Right : ((game.playerHealth < 100) ? posX2Right : posX3Right), 16);
  if (game.playerAlive) {
    display.println(game.playerHealth);
  } else {
    display.println("X");
  }
  
  display.display();
  
  displayGameTime();
}

void displayGameBasisInfo() {
  display.clearDisplay();
  display.setTextSize(1);

  // Infinitag smybol
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

  // Player
  String displayPlayerText = "P";
  displayPlayerText += game.playerId;
  char charPlayerBuf[10];
  displayPlayerText.toCharArray(charPlayerBuf, 10);
  display.setCursor(0, 0);
  display.println(charPlayerBuf);

  // Team
  String displayTeamText = "T";
  displayTeamText += game.playerTeamId;
  char charTeamBuf[10];
  displayTeamText.toCharArray(charTeamBuf, 10);
  display.setCursor(112, 0);
  display.println(charTeamBuf);

  display.display();

  displayGameData();
}

/*
 * Game Stats
 */
void displayGameStats() {
  display.clearDisplay();
  
  String text = "Game-Stats";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  display.setCursor(0, 0);
  display.println(textBuf);
  display.writeFastHLine(0, 14, 128, WHITE);

  text = "Shots: ";
  text += game.statsShots;
  text.toCharArray(textBuf, 50);
  display.setCursor(0, 17);
  display.println(textBuf);

  text = "Death: ";
  text += game.statsDeath;
  text.toCharArray(textBuf, 50);
  display.setCursor(0, 31);
  display.println(textBuf);
  
  text = "Press [Enter] to restart";
  text.toCharArray(textBuf, 50);
  display.setCursor(0, 49);
  display.println(textBuf);
  display.writeFastHLine(0, 48, 128, WHITE);

  display.display();
  
  delay(100);
}

/*
 * Commands
 */
void sendCmd(byte data[], unsigned int byteLength) {
  // ToDo: correct destination
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

/*
 * Helpers
 */
void startGame() {
  game.start();
  displayGameBasisInfo();
  currentScreen = 1;
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
