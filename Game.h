/*
  Game.h - Game Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/
#include <Infinitag_Core.h>

// Vendor
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#ifndef Game_h
  #define Game_h

  #include "Arduino.h"

  class Game
  {
    public:
      Game(IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip);
      
      void start();
      void end();
      bool isRunning();
      void loop();
      void shot();
      void reload();
      void respawn();
      
      void calculateTime();
      void displayTime();
      void displayBasisInfo();
      
      void demoFunktions();
      
      void colorWipe(uint32_t c);
      void updateSensorConfig();
      void initButtons(int rP, int lP, int dP, int uP, int sP, int iP, int rlP, int fP, int eP, int rsP);
      void getButtonStates();

      void receiveShot(byte *data, int byteCounter);
      void setDamage(int damage);

      // Display
      bool reloadDisplay = false;
    
      // Time
      unsigned long timeStart;
      unsigned long timeEnd;
      unsigned long timePlayTime;
      unsigned long timeToEnd;
      unsigned long timeDiff;
      int timeDiffMinutes;
      int timeDiffSeconds;

      // Stats
      unsigned int statsShots;
      unsigned int statsDeath;
      
      // Player
      bool playerAlive;
      byte playerTeamId = 1;
      byte playerId = 1;
      int playerAmmo;
      int playerAmmoMax;
      int playerHealth;
      int playerHealthMax;
      int timePlayerRespawn;
      unsigned long timeNextRespawn;

      // Tagger
      int timeShotFrequence;
      unsigned long timeNextShot;
      int taggerDamage;
      
    private:
      // Basic
      Infinitag_Core infinitagCore;
      
      // IR
      IRsend irsend;
      
      // LED
      unsigned int ledIntensity = 255;
      Adafruit_NeoPixel strip;
      
      // Button Settings
      int rightBtnPin;
      int rightBtnState;
      int leftBtnPin;
      int leftBtnState;
      int downBtnPin;
      int downBtnState;
      int upBtnPin;
      int upBtnState;
      int specialBtnPin;
      int specialBtnState;
      int infoBtnPin;
      int infoBtnState;
      int reloadBtnPin;
      int reloadBtnState;
      int fireBtnPin;
      int fireBtnState;
      int enterBtnPin;
      int enterBtnState;
      int resetBtnPin;
      int resetBtnState;
  };

#endif
