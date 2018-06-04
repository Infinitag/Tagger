/*
  Settings.h - Settings File for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/
// Button Settings

#define RIGHT_BTN_PIN 34
int rightBtnState = 0;
#define LEFT_BTN_PIN 35
int leftBtnState = 0;
#define DOWN_BTN_PIN 36
int downBtnState = 0;
#define UP_BTN_PIN 37
int upBtnState = 0;
#define SPECIAL_BTN_PIN 44
int specialBtnState = 0;
#define INFO_BTN_PIN 45
int infoBtnState = 0;
#define RELOAD_BTN_PIN 46
int reloadBtnState = 0;
#define FIRE_BTN_PIN 47
int fireBtnState = 0;
#define ENTER_BTN_PIN 48
int enterBtnState = 0;
#define RESET_BTN_PIN 49
int resetBtnState = 0;

// Display
#define DISPLAY_RESET_PIN 4
#define DISPLAY_DC_PIN 5
#define DISPLAY_CS_PIN 5

// Additional
#define MUZZLE_LED_PIN 8
