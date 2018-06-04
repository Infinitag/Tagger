#include "Input.h"
#include "Settings.h"
#include "Arduino.h"

Input theInput;

Input::Input()
{
  myRightBtnState = 0;
  myLeftBtnState = 0;
  myDownBtnState = 0;
  myUpBtnState = 0;
  mySpecialBtnState = 0;
  myInfoBtnState = 0;
  myReloadBtnState = 0;
  myFireBtnState = 0;
  myEnterBtnState = 0;
  myResetBtnState = 0;
}

void Input::Fetch()
{
  myRightBtnState = digitalRead(RIGHT_BTN_PIN);
  myLeftBtnState = digitalRead(LEFT_BTN_PIN);
  myDownBtnState = digitalRead(DOWN_BTN_PIN);
  myUpBtnState = digitalRead(UP_BTN_PIN);
  mySpecialBtnState = digitalRead(SPECIAL_BTN_PIN);
  myInfoBtnState = digitalRead(INFO_BTN_PIN);
  myReloadBtnState = digitalRead(RELOAD_BTN_PIN);
  myFireBtnState = digitalRead(FIRE_BTN_PIN);
  myEnterBtnState = digitalRead(ENTER_BTN_PIN);
  myResetBtnState = digitalRead(RESET_BTN_PIN);
}

