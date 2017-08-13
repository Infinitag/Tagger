/*
  Game.cpp - Game Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include "Arduino.h"

// Infinitag
#include "Game.h"


Game::Game(IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip)
{
  irSend = ir;
  infinitagCore = core;
  strip = ledStrip;
  
  timePlayTime = 60000;//0;
}

void Game::loop() {
  getButtonStates();
  
  if (timeToEnd <= 0) {
    end();
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
}

void Game::end() {
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

  // Color
  colorWipe(strip.Color(0, ledIntensity, 0, 0));

  // Calcs
  playerAmmo--;
  timeNextShot = millis() + timeShotFrequence;

  // Stats
  statsShots++;
}

void Game::respawn() {
  setAlive(true);
  playerHealth = playerHealthMax;
}

void Game::reload() {
  playerAmmo = playerAmmoMax;
  
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
}

void Game::updateSensorConfig() {
  infinitagCore.sendCmdSetTeamId(playerTeamId);
  infinitagCore.sendCmdSetPlayerId(playerId);
  reloadDisplay = true;
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
  resetBtnPin = rsP;
  resetBtnState = 0;
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
  resetBtnState = digitalRead(resetBtnPin);
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
  }
}

void Game::setAlive(bool alive) {
  playerAlive = alive;
  infinitagCore.sendCmdPingSetAlive(alive);
}

