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


Game::Game(Framebuffer& fb, sh1106_spi& dp, IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip)
{
  framebuffer = fb;
  display = dp;
  irsend = ir;
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
  displayTime();
  demoFunktions();
  
  if (fireBtnState == HIGH) {
    unsigned long shotValue = infinitagCore.ir_encode(false, 0, playerTeamId, playerId, 1, 100);
    irsend.sendRC5(shotValue, 24);
    colorWipe(strip.Color(0, ledIntensity, 0, 0));
    statsShots++;
  }
  
  delay(100);
}

void Game::start() {
  timeStart = millis();
  timeEnd = timeStart + timePlayTime;
  
  statsShots = 0;
  statsDeath = 0;
  
  displayBasisInfo();
  calculateTime();
  displayTime();
}

void Game::end() {
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
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
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
    displayBasisInfo();
    updateSensorConfig();
    delay(100);
  }
}

void Game::displayBasisInfo() {
  framebuffer.clear(BLACK);

  // Infinitag Smybol
  // Kann noch nicht richtig mit der aktuellen Lib abgebildet werden
  framebuffer.drawLine(25, 2, 48, 2, WHITE);
  framebuffer.drawLine(25, 49, 48, 49, WHITE);
  framebuffer.drawLine(10, 24, 10, 27, WHITE);
  
  framebuffer.drawLine(78, 2, 101, 2, WHITE);
  framebuffer.drawLine(78, 49, 101, 49, WHITE);
  framebuffer.drawLine(116, 24, 116, 27, WHITE);

  // Spieler Angabe
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  String displayPlayerText = "P";
  displayPlayerText += playerId;
  char charPlayerBuf[10];
  displayPlayerText.toCharArray(charPlayerBuf, 10);
  framebuffer.displayText(charPlayerBuf, 0, 0, WHITE);

  // Team Angabe
  // Schrift muss noch kleiner werden, geht mit der aktuellen lib nicht
  String displayTeamText = "T";
  displayTeamText += playerTeamId;
  char charTeamBuf[10];
  displayTeamText.toCharArray(charTeamBuf, 10);
  framebuffer.displayText(charTeamBuf, 112, 0, WHITE);

  display_buffer(&display, framebuffer.getData());

  displayTime();
}

void Game::updateSensorConfig() {
  Wire.beginTransmission(0x22);
  Wire.write(playerTeamId);
  Wire.write(playerId);
  Wire.endTransmission();
  Wire.beginTransmission(0x24);
  Wire.write(playerTeamId);
  Wire.write(playerId);
  Wire.endTransmission();
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

void Game::loopStats() {
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

void Game::receiveShot(byte *data, int byteCounter) {
  Serial.println("receiveShot");
  switch (data[0]) {
    case 0x06:
      if (byteCounter == 4) {
        infinitagCore.ir_decode(data);
        if (infinitagCore.ir_recv_cmd == 1) {
          setDamage(infinitagCore.ir_recv_cmd_value);
        }
      }
      break;
      
    default:
      Serial.println("No Command found");
      break;
  }
}

void Game::setDamage(int damage) {
  Serial.println("Damage");
  Serial.println(damage);
  statsDeath++;
}

