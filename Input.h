#pragma once

#include <stdint.h>
class Input
{
public:
  Input();

  void Fetch();

  uint8_t GetRightBtnState() const { return myRightBtnState;}
  uint8_t GetLeftBtnState() const { return myLeftBtnState;}
  uint8_t GetDownBtnState() const { return myDownBtnState;}
  uint8_t GetUpBtnState() const { return myUpBtnState;}
  uint8_t GetSpecialBtnState() const { return mySpecialBtnState;}
  uint8_t GetInfoBtnState() const {return myInfoBtnState;}
  uint8_t GetReloadBtnState() const {return myReloadBtnState;}
  uint8_t GetFireBtnState() const {return myFireBtnState;}
  uint8_t GetEnterBtnState() const {return myEnterBtnState;}
  uint8_t GetResetBtnState() const {return myResetBtnState;}
private:
  uint8_t myRightBtnState : 1;
  uint8_t myLeftBtnState : 1;
  uint8_t myDownBtnState : 1;
  uint8_t myUpBtnState : 1;
  uint8_t mySpecialBtnState : 1;
  uint8_t myInfoBtnState : 1;
  uint8_t myReloadBtnState : 1;
  uint8_t myFireBtnState : 1;
  uint8_t myEnterBtnState : 1;
  uint8_t myResetBtnState : 1;
};

extern Input theInput;

