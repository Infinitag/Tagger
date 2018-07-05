/*
  Game.h - Game Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/
#include <Infinitag_Core.h>

// Basic Libs
#include <Wire.h>

// Infinitag Libs
#include <Infinitag_SH1106.h>
#include <Infinitag_GFX.h>

// Vendor Libs
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

#ifndef Game_h
  #define Game_h

  #include "Arduino.h"

  class Game
  {
    // ToDo: Maybe access the most vars only over a getter?
    public:
      Game(IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip);
      
      void start(bool isServer);
      void end();
      bool isRunning();
      void loop();
      void loopStats();
      void shot();
      void reload();
      void respawn();
      
      void calculateTime();
      
      void displayTime();
      void displayData();
      void displayBasisInfo();
      void displayStats();
      
      void demoFunctions();
      
      void colorWipe(uint32_t c);
      void updateSensorConfig();

      void receiveShot(byte *data, int byteCounter);
      void receiveWifiCmd(unsigned long cmd);
      
      void setDamage(int damage);
      void setAlive(bool alive);

      // Wifi
      void sendWifiCmd(unsigned long cmd);

      // Display
      unsigned int currentScreen = 0; // 0 = home / 1 = inGame / 2 = gameStats

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
      unsigned int statsKills;

      // Game
      int gameId = 1;
      
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
      bool autoReload = true;
      
    private:
      // Basic
      Infinitag_Core infinitagCore;
      
      // IR
      IRsend irSend;
      
      // LED
      unsigned int ledIntensity = 255;
      Adafruit_NeoPixel strip;
      uint32_t teamColors[6][4] = {{255,0,0,0},{0,255,0,0},{0,0,255,0},{255,255,0,0},{0,255,255,0},{255,0,255,0}};
  };

#endif
