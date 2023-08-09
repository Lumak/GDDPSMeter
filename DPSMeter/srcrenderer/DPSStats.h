#pragma once
#include "BaseRender.h"

class DPSStats : public BaseRender
{
public:
  DPSStats();

  void ClearData();
  void InputStats(const dpsStatData &stats);

  virtual void ShowWin(bool &showWindow);
  virtual void Draw(const char* title, bool* p_open = NULL);

private:
  float statDps_;
  float statHp_;
  float statDamageDealth_;
  float statDamageTaken_;

};

