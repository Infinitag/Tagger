#include "Input.h"
#include "Settings.h"
#include "Arduino.h"

Input theInput;

extern "C" void __NoInputFunction(void)
{
}

#define InputFunction(FunctionName) void FunctionName(void) __attribute__((weak, alias("__NoInputFunction")))
//went from !down to down
InputFunction(OnRightButtonPressed);
InputFunction(OnLeftButtonPressed);
InputFunction(OnDownButtonPressed);
InputFunction(OnUpButtonPressed);
InputFunction(OnSpecialButtonPressed);
InputFunction(OnInfoButtonPressed);
InputFunction(OnReloadButtonPressed);
InputFunction(OnFireButtonPressed);
InputFunction(OnEnterButtonPressed);
InputFunction(OnResetButtonPressed);

//went from down to !down
InputFunction(OnRightButtonReleased);
InputFunction(OnLeftButtonReleased);
InputFunction(OnDownButtonReleased);
InputFunction(OnUpButtonReleased);
InputFunction(OnSpecialButtonReleased);
InputFunction(OnInfoButtonReleased);
InputFunction(OnReloadButtonReleased);
InputFunction(OnFireButtonReleased);
InputFunction(OnEnterButtonReleased);
InputFunction(OnResetButtonReleased);

//down
InputFunction(OnRightButtonDown);
InputFunction(OnLeftButtonDown);
InputFunction(OnDownButtonDown);
InputFunction(OnUpButtonDown);
InputFunction(OnSpecialButtonDown);
InputFunction(OnInfoButtonDown);
InputFunction(OnReloadButtonDown);
InputFunction(OnFireButtonDown);
InputFunction(OnEnterButtonDown);
InputFunction(OnResetButtonDown);

//up
InputFunction(OnRightButtonUp);
InputFunction(OnLeftButtonUp);
InputFunction(OnDownButtonUp);
InputFunction(OnUpButtonUp);
InputFunction(OnSpecialButtonUp);
InputFunction(OnInfoButtonUp);
InputFunction(OnReloadButtonUp);
InputFunction(OnFireButtonUp);
InputFunction(OnEnterButtonUp);
InputFunction(OnResetButtonUp);

//TODO[Failee]: Do we want to also consider 'clicked' up->down->up in a given time frame?

#define HandleButton(button, pin, downFun, upFun, pressedFun, releasedFun) \
  { \
    uint8_t curBtnState = digitalRead(pin); \
    if(curBtnState != button) \
    { \
      if(curBtnState == HIGH) \
      { \
        pressedFun(); \
      } \
      else \
      { \
        releasedFun(); \
      } \
    } \
    if(curBtnState) \
    { \
      downFun(); \
    } \
    else \
    { \
      upFun(); \
    } \
    button = curBtnState; \
  }
  
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
  HandleButton(myRightBtnState, RIGHT_BTN_PIN, OnRightButtonDown, OnRightButtonUp, OnRightButtonPressed, OnRightButtonReleased);
  HandleButton(myLeftBtnState, LEFT_BTN_PIN, OnLeftButtonDown, OnLeftButtonUp, OnLeftButtonPressed, OnLeftButtonReleased);
  HandleButton(myDownBtnState, DOWN_BTN_PIN, OnDownButtonDown, OnDownButtonUp, OnDownButtonPressed, OnDownButtonReleased);
  HandleButton(myUpBtnState, UP_BTN_PIN, OnUpButtonDown, OnUpButtonUp, OnUpButtonPressed, OnUpButtonReleased);
  HandleButton(mySpecialBtnState, SPECIAL_BTN_PIN, OnSpecialButtonDown, OnSpecialButtonUp, OnSpecialButtonPressed, OnSpecialButtonReleased);
  HandleButton(myInfoBtnState, INFO_BTN_PIN, OnInfoButtonDown, OnInfoButtonUp, OnInfoButtonPressed, OnInfoButtonReleased);
  HandleButton(myReloadBtnState, RELOAD_BTN_PIN, OnReloadButtonDown, OnReloadButtonUp, OnReloadButtonPressed, OnReloadButtonReleased);
  HandleButton(myFireBtnState, FIRE_BTN_PIN, OnFireButtonDown, OnFireButtonUp, OnFireButtonPressed, OnFireButtonReleased);
  HandleButton(myEnterBtnState, ENTER_BTN_PIN, OnEnterButtonDown, OnEnterButtonUp, OnEnterButtonPressed, OnEnterButtonReleased);
  HandleButton(myResetBtnState, RESET_BTN_PIN, OnResetButtonDown, OnResetButtonUp, OnResetButtonPressed, OnResetButtonReleased);
}

