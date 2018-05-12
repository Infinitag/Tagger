/*
  Game.cpp - Game Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include "Arduino.h"

// Vendor Libs
#include <WTV020SD16P.h>

// Vendor Inits
// Should be initialized in the main file and then passed to the game class
int soundBusyPin = 38;
int soundDataPin = 39;
int soundClockPin = 41;
WTV020SD16P wtv020sd16p(soundClockPin,soundDataPin,soundBusyPin);

// Infinitag
#include "Game.h"

Game::Game(Framebuffer& fb, sh1106_spi& dp, IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip)
{
  framebuffer = fb;
  display = dp;
  irSend = ir;
  infinitagCore = core;
  strip = ledStrip;
  
  timePlayTime = 60000;//0;
}

void Game::loop() {
  getButtonStates();

  if (timeToEnd <= 0) {
    return;
  }
  
  colorWipe(strip.Color(0, 0, ledIntensity, 0));
  
  calculateTime();
  demoFunktions();

  if (playerAlive) {
    if (fireBtnState == HIGH) {
      shot();
    }
  
    if (reloadBtnState == HIGH) {
      reload();
    }
  } else {
    if (timeNextRespawn <= millis()) {
      respawn();
    }
  }
  
  displayData();
}

void Game::loopStats() {
  displayStats();
}

void Game::start() {
  timeStart = millis();
  timeEnd = timeStart + timePlayTime;
  
  statsShots = 0;
  statsDeath = 0;
  playerAmmoMax = 30;
  playerHealthMax = 100;
  timePlayerRespawn = 3000;

  // Tagger
  taggerDamage = 25;
  timeShotFrequence = 250;
  timeNextShot = millis();

  reload();
  respawn();
  calculateTime();
  displayBasisInfo();
}

void Game::end() {
  // Sound
  wtv020sd16p.asyncPlayVoice(600);
}


void Game::shot() {
  if (playerAmmo <= 0) {
    return;
  }

  if (timeNextShot > millis()) {
    return;
  }
  
  // IR signal
  unsigned long shotValue = infinitagCore.irEncode(false, 0, playerTeamId, playerId, 1, taggerDamage);
  irSend.sendRC5(shotValue, 24);

  // Sound
  wtv020sd16p.asyncPlayVoice(100);

  // Color
  colorWipe(strip.Color(0, ledIntensity, 0, 0));

  // Calcs
  playerAmmo--;
  timeNextShot = millis() + timeShotFrequence;

  // Stats
  statsShots++;

  // Autoreload
  if (playerAmmo <= 0 && autoReload) {
    reload();
  }
}

void Game::respawn() {
  setAlive(true);
  playerHealth = playerHealthMax;
}

void Game::reload() {
  playerAmmo = playerAmmoMax;
  wtv020sd16p.asyncPlayVoice(400);
}

bool Game::isRunning() {
  return (timeToEnd > 0);
}

void Game::calculateTime() {
  timeDiff = millis() - timeStart;
  if (timeDiff <= timePlayTime) {
    timeToEnd = timePlayTime - timeDiff;
    if (timeToEnd > 0) {
      timeDiffMinutes = (timeToEnd / 60000);
      timeDiffSeconds = (timeToEnd - (timeDiffMinutes * 60000)) / 1000;
      return;
    }
  }
  timeToEnd = 0;
  timeDiffMinutes = 0;
  timeDiffSeconds = 0;
}


void Game::colorWipe(uint32_t c) {
  for (uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void Game::demoFunktions() {
  if (upBtnState == HIGH) {
    if (playerId == 1) {
      playerId = 2;
    } else {
      playerId = 1;
      if (playerTeamId == 1) {
        playerTeamId = 2;
      } else {
        playerTeamId = 1;
      }
    }
    updateSensorConfig();
    delay(100);
  }
  if (backBtnState == HIGH) {
    setDamage(1000);
  }
}

void Game::updateSensorConfig() {
  infinitagCore.sendCmdSetTeamId(playerTeamId);
  infinitagCore.sendCmdSetPlayerId(playerId);
  infinitagCore.sendCmdSetAnimation(1, 1000, teamColors[playerTeamId - 1][0], teamColors[playerTeamId - 1][1], teamColors[playerTeamId - 1][2], teamColors[playerTeamId - 1][3], 0);
  displayBasisInfo();
}

void Game::initButtons(int rP, int lP, int dP, int uP, int sP, int iP, int rlP, int fP, int eP, int rsP) {
  rightBtnPin = rP;
  rightBtnState = 0;
  leftBtnPin = lP;
  leftBtnState = 0;
  downBtnPin = dP;
  downBtnState = 0;
  upBtnPin = uP;
  upBtnState = 0;
  specialBtnPin = sP;
  specialBtnState = 0;
  infoBtnPin = iP;
  infoBtnState = 0;
  reloadBtnPin = rlP;
  reloadBtnState = 0;
  fireBtnPin = fP;
  fireBtnState = 0;
  enterBtnPin = eP;
  enterBtnState = 0;
  backBtnPin = rsP;
  backBtnState = 0;
}

// ToDo: double function in Tagger.cpp and here... how to combine?!
// maybe in the Core lib?
void Game::getButtonStates() {
  rightBtnState = digitalRead(rightBtnPin);
  leftBtnState = digitalRead(leftBtnPin);
  downBtnState = digitalRead(downBtnPin);
  upBtnState = digitalRead(upBtnPin);
  specialBtnState = digitalRead(specialBtnPin);
  infoBtnState = digitalRead(infoBtnPin);
  reloadBtnState = digitalRead(reloadBtnPin);
  fireBtnState = digitalRead(fireBtnPin);
  enterBtnState = digitalRead(enterBtnPin);
  backBtnState = digitalRead(backBtnPin);
}

void Game::receiveShot(byte *data, int byteCounter) {
  switch (data[0]) {
    case 0x06:
      if (byteCounter == 4) {
        infinitagCore.irDecode(data);
        if (infinitagCore.irRecvCmd == 1) {
          setDamage(infinitagCore.irRecvCmdValue);
        }
      }
      break;
  }
}

void Game::setDamage(int damage) {
  if (! playerAlive) {
    return;
  }
  
  playerHealth -= damage;

  if (playerHealth <= 0) {
    playerHealth = 0;
    setAlive(false);
    timeNextRespawn = millis() + timePlayerRespawn;
    statsDeath++;
    wtv020sd16p.asyncPlayVoice(899);
  } else {
    wtv020sd16p.asyncPlayVoice(random(800, 803));
  }
}

void Game::setAlive(bool alive) {
  playerAlive = alive;
  infinitagCore.sendCmdPingSetAlive(alive);
}



/*
 * Display Game-Informations
 */
void Game::displayTime() {
  // Bar
  int barMaxWidth = 94;

  framebuffer.drawHorizontalLine(0, 60, barMaxWidth, WHITE);
  framebuffer.drawLine(barMaxWidth, 60, barMaxWidth, 64, WHITE);

  framebuffer.drawRectFilled(0, 61, barMaxWidth, 4, BLACK);
  int barSize = barMaxWidth - (timeDiff * barMaxWidth / timePlayTime);
  if (barSize < 0) {
    barSize = 0;
  }
  framebuffer.drawRectFilled(0, 61, barSize, 4, WHITE);

  // Time
  framebuffer.drawRectFilled(97, 52, 31, 12, BLACK);
  String timeText = "";
  if (timeDiffMinutes < 10) {
    timeText += "0";
  }
  timeText += timeDiffMinutes;
  timeText += ":";
  if (timeDiffSeconds < 10) {
    timeText += "0";
  }
  timeText += timeDiffSeconds;
  char timeBuf[6];
  timeText.toCharArray(timeBuf, 6);
  framebuffer.displayText(timeBuf, 97, 52, WHITE);

  display_buffer(&display, framebuffer.getData());
}

void Game::displayData() {
  int posX3Left = 26;
  int posX3Right = 80;
  int posX2Left = 30;
  int posX2Right = 83;
  int posX1Left = 34;
  int posX1Right = 87;

  // Ammo  
  char playerAmmoBuffer[4];
  sprintf (playerAmmoBuffer, "%i", playerAmmo);
  framebuffer.drawRectFilled(posX3Left, 18, 30, 18, BLACK);
  framebuffer.displayText(playerAmmoBuffer, (playerAmmo < 10) ? posX1Left : ((playerAmmo < 100) ? posX2Left : posX3Left), 18, WHITE);

  // Health
  char playerHealthBuffer[4];
  sprintf (playerHealthBuffer, "%i", playerHealth);
  framebuffer.drawRectFilled(posX3Right, 18, 30, 18, BLACK);
  if (playerAlive) {
    infinitagCore.sendCmdSetAnimation(1, 1000, teamColors[playerTeamId - 1][0], teamColors[playerTeamId - 1][1], teamColors[playerTeamId - 1][2], teamColors[playerTeamId - 1][3], 0);
    framebuffer.displayText(playerHealthBuffer, (playerHealth < 10) ? posX1Right : ((playerHealth < 100) ? posX2Right : posX3Right), 18, WHITE);
  } else {
    infinitagCore.sendCmdSetAnimation(2, 1000, teamColors[playerTeamId - 1][0], teamColors[playerTeamId - 1][1], teamColors[playerTeamId - 1][2], teamColors[playerTeamId - 1][3], 0);
    framebuffer.displayText("X", posX1Right, 18, WHITE);
  }
  
  display_buffer(&display, framebuffer.getData());
  
  displayTime();
}

void Game::displayBasisInfo() {
  framebuffer.clear(BLACK);

  // Infinitag smybol
  framebuffer.drawLine(25, 2, 48, 2, WHITE);
  framebuffer.drawLine(25, 49, 48, 49, WHITE);
  framebuffer.drawLine(10, 24, 10, 27, WHITE);
  framebuffer.drawLine(25, 2, 10, 24, WHITE);
  framebuffer.drawLine(10, 27, 25, 49, WHITE);
  framebuffer.drawLine(48, 2, 59, 19, WHITE);
  
  framebuffer.drawLine(48, 49, 78, 2, WHITE);
  
  framebuffer.drawLine(78, 2, 101, 2, WHITE);
  framebuffer.drawLine(78, 49, 101, 49, WHITE);
  framebuffer.drawLine(116, 24, 116, 27, WHITE);
  framebuffer.drawLine(101, 2, 116, 24, WHITE);
  framebuffer.drawLine(116, 27, 101, 49, WHITE);
  framebuffer.drawLine(78, 49, 67, 32, WHITE);

  // Player
  String displayPlayerText = "P";
  displayPlayerText += playerId;
  char charPlayerBuf[10];
  displayPlayerText.toCharArray(charPlayerBuf, 10);
  framebuffer.displayText(charPlayerBuf, 0, 0, WHITE);

  // Team
  String displayTeamText = "T";
  displayTeamText += playerTeamId;
  char charTeamBuf[10];
  displayTeamText.toCharArray(charTeamBuf, 10);
  framebuffer.displayText(charTeamBuf, 112, 0, WHITE);

  display_buffer(&display, framebuffer.getData());

  displayData();
}

/*
 * Game Stats
 */
void Game::displayStats() {
  framebuffer.clear(BLACK);
  
  String text = "Game-Stats";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 0, WHITE);
  framebuffer.drawHorizontalLine(0, 14, 128, WHITE);

  text = "Shots: ";
  text += statsShots;
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 17, WHITE);

  text = "Death: ";
  text += statsDeath;
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 31, WHITE);
  
  text = "Press [Enter] to restart";
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 49, WHITE);
  framebuffer.drawHorizontalLine(0, 48, 128, WHITE);

  display_buffer(&display, framebuffer.getData());
  
  delay(100);
}
