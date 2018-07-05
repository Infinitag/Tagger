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
#include "Input.h"

// Vendor Inits
// Should be initialized in the main file and then passed to the game class
int soundBusyPin = 38;
int soundDataPin = 39;
int soundClockPin = 41;
WTV020SD16P wtv020sd16p(soundClockPin,soundDataPin,soundBusyPin);

// Infinitag
#include "Game.h"

Game::Game(IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip)
{
  irSend = ir;
  infinitagCore = core;
  strip = ledStrip;
  
  timePlayTime = 30000;
  timePlayerRespawn = 3000;
}

void Game::loop() {

  if (timeToEnd <= 0) {
    return;
  }
  
  colorWipe(strip.Color(0, 0, ledIntensity, 0));
  
  calculateTime();
  demoFunctions();

  if (playerAlive) {
    if (theInput.GetFireBtnState() == HIGH) {
      shot();
    }
  
    if (theInput.GetReloadBtnState() == HIGH) {
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

void Game::start(bool isServer) {
  
  // Wifi
  if (isServer) {
    Serial.println("Send Game Config");
    // Game time
    sendWifiCmd(infinitagCore.wifiEncode(true, gameId, playerTeamId, playerId, 3, (timePlayTime / 1000)));
    // Respawn time
    sendWifiCmd(infinitagCore.wifiEncode(true, gameId, playerTeamId, playerId, 4, (timePlayerRespawn / 1000)));
    
    Serial.println("Game start");
    sendWifiCmd(infinitagCore.wifiEncode(true, gameId, 0, 0, 1, 1));
  }
  
  // Main
  timeStart = millis();
  timeEnd = timeStart + timePlayTime;
  
  statsShots = 0;
  statsDeath = 0;
  statsKills = 0;
  
  playerAmmoMax = 30;
  playerHealthMax = 100;

  // Tagger
  taggerDamage = 100;
  timeShotFrequence = 250;
  timeNextShot = millis();

  currentScreen = 1;
  
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

void Game::sendWifiCmd(unsigned long cmd) {
    Serial.println("Send Wifi Cmd");
    byte buf[4];
    buf[3] = cmd & 255;
    buf[2] = (cmd >> 8)  & 255;
    buf[1] = (cmd >> 16) & 255;
    buf[0] = (cmd >> 24) & 255;
    Serial1.write(buf, sizeof(buf));
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

void Game::demoFunctions() {
  if (theInput.GetUpBtnState() == HIGH) {
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
  if (theInput.GetResetBtnState() == HIGH) {
    setDamage(1000);
  }
}

void Game::updateSensorConfig() {
  infinitagCore.sendCmdSetTeamId(playerTeamId);
  infinitagCore.sendCmdSetPlayerId(playerId);
  infinitagCore.sendCmdSetAnimation(1, 1000, teamColors[playerTeamId - 1][0], teamColors[playerTeamId - 1][1], teamColors[playerTeamId - 1][2], teamColors[playerTeamId - 1][3], 0);
  displayBasisInfo();
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

void Game::receiveWifiCmd(unsigned long cmd) {
  Serial.println("Game CMD");
  Serial.println(cmd);

  infinitagCore.wifiDecode(cmd);

  // Check current game
  if (infinitagCore.wifiRecvGameId == gameId) {

    switch (infinitagCore.wifiRecvCmd) {
      
      // Game start
      case 1:
        if (infinitagCore.wifiRecvCmdValue == 1) {
          Serial.println("Remote Game start");
    
          // Change player and team for demo 1vs1
          playerId = 1;
          playerTeamId = 2;
          updateSensorConfig();
    
          // Start game without broadcast cmd
          start(false);
        }
        break;

      // Kill
      case 2:
        if (infinitagCore.wifiRecvCmdValue == 1) {
          if (infinitagCore.wifiRecvTeamId == playerTeamId && infinitagCore.wifiRecvPlayerId == playerId) {
            Serial.println("Kill ok");
            statsKills++;
          }
        }
        break;

      // Game Time
      case 3:
        Serial.println("Set game time");
        timePlayTime = infinitagCore.wifiRecvCmdValue * 1000;
        break;

      // Respawn Time
      case 4:
        Serial.println("Set respawn time");
        timePlayerRespawn = infinitagCore.wifiRecvCmdValue * 1000;
        break;
        
    }
  }
}

void Game::setDamage(int damage) {
  Serial.println("Damage");
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
    sendWifiCmd(infinitagCore.wifiEncode(true, gameId, infinitagCore.irRecvTeamId, infinitagCore.irRecvPlayerId, 2, 1));
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

  theFramebuffer.DrawHorizontalLine(0, 60, barMaxWidth, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(barMaxWidth, 60, barMaxWidth, 64, INFINITAG_GFX_WHITE);

  theFramebuffer.DrawRectFilled(0, 61, barMaxWidth, 4, INFINITAG_GFX_BLACK);
  int barSize = barMaxWidth - (timeDiff * barMaxWidth / timePlayTime);
  if (barSize < 0) {
    barSize = 0;
  }
  theFramebuffer.DrawRectFilled(0, 61, barSize, 4, INFINITAG_GFX_WHITE);

  // Time
  theFramebuffer.DrawRectFilled(97, 52, 31, 12, INFINITAG_GFX_BLACK);
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
  theFramebuffer.DisplayText(timeBuf, 97, 52, INFINITAG_GFX_WHITE);

  theDisplay.Display(theFramebuffer.GetData());
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
  theFramebuffer.DrawRectFilled(posX3Left, 18, 30, 18, INFINITAG_GFX_BLACK);
  theFramebuffer.DisplayText(playerAmmoBuffer, (playerAmmo < 10) ? posX1Left : ((playerAmmo < 100) ? posX2Left : posX3Left), 18, INFINITAG_GFX_WHITE);

  // Health
  char playerHealthBuffer[4];
  sprintf (playerHealthBuffer, "%i", playerHealth);
  theFramebuffer.DrawRectFilled(posX3Right, 18, 30, 18, INFINITAG_GFX_BLACK);
  if (playerAlive) {
    infinitagCore.sendCmdSetAnimation(1, 1000, teamColors[playerTeamId - 1][0], teamColors[playerTeamId - 1][1], teamColors[playerTeamId - 1][2], teamColors[playerTeamId - 1][3], 0);
    theFramebuffer.DisplayText(playerHealthBuffer, (playerHealth < 10) ? posX1Right : ((playerHealth < 100) ? posX2Right : posX3Right), 18, INFINITAG_GFX_WHITE);
  } else {
    infinitagCore.sendCmdSetAnimation(2, 1000, teamColors[playerTeamId - 1][0], teamColors[playerTeamId - 1][1], teamColors[playerTeamId - 1][2], teamColors[playerTeamId - 1][3], 0);
    theFramebuffer.DisplayText("X", posX1Right, 18, INFINITAG_GFX_WHITE);
  }
  
  theDisplay.Display(theFramebuffer.GetData());
  
  displayTime();
}

void Game::displayBasisInfo() {
  theFramebuffer.Clear(INFINITAG_GFX_BLACK);

  // Infinitag smybol
  theFramebuffer.DrawLine(25, 2, 48, 2, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(25, 49, 48, 49, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(10, 24, 10, 27, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(25, 2, 10, 24, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(10, 27, 25, 49, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(48, 2, 59, 19, INFINITAG_GFX_WHITE);
  
  theFramebuffer.DrawLine(48, 49, 78, 2, INFINITAG_GFX_WHITE);
  
  theFramebuffer.DrawLine(78, 2, 101, 2, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(78, 49, 101, 49, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(116, 24, 116, 27, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(101, 2, 116, 24, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(116, 27, 101, 49, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawLine(78, 49, 67, 32, INFINITAG_GFX_WHITE);

  // Player
  String displayPlayerText = "P";
  displayPlayerText += playerId;
  char charPlayerBuf[10];
  displayPlayerText.toCharArray(charPlayerBuf, 10);
  theFramebuffer.DisplayText(charPlayerBuf, 0, 0, INFINITAG_GFX_WHITE);

  // Team
  String displayTeamText = "T";
  displayTeamText += playerTeamId;
  char charTeamBuf[10];
  displayTeamText.toCharArray(charTeamBuf, 10);
  theFramebuffer.DisplayText(charTeamBuf, 112, 0, INFINITAG_GFX_WHITE);

  theDisplay.Display(theFramebuffer.GetData());

  displayData();
}

/*
 * Game Stats
 */
void Game::displayStats() {
  theFramebuffer.Clear(INFINITAG_GFX_BLACK);
  
  String text = "Game-Stats";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  theFramebuffer.DisplayText(textBuf, 0, 0, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawHorizontalLine(0, 14, 128, INFINITAG_GFX_WHITE);

  text = "Shots: ";
  text += statsShots;
  text.toCharArray(textBuf, 50);
  theFramebuffer.DisplayText(textBuf, 0, 17, INFINITAG_GFX_WHITE);

  text = "K / D: ";
  text += statsKills;
  text += " / ";
  text += statsDeath;
  text.toCharArray(textBuf, 50);
  theFramebuffer.DisplayText(textBuf, 0, 31, INFINITAG_GFX_WHITE);
  
  text = "Press [Enter] to restart";
  text.toCharArray(textBuf, 50);
  theFramebuffer.DisplayText(textBuf, 0, 49, INFINITAG_GFX_WHITE);
  theFramebuffer.DrawHorizontalLine(0, 48, 128, INFINITAG_GFX_WHITE);

  theDisplay.Display(theFramebuffer.GetData());
}
