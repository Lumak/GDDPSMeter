#pragma once
#include "BaseRender.h"

class OADAStats : public BaseRender
{
public:
  OADAStats();

  virtual void ShowWin(bool &showWindow);
  virtual void Draw(const char* title, bool* p_open = NULL);

  void ClearData();
  void SetStats(const OADAData &stats);

private:
  unsigned oa_;
  float oaPth_;
  float oaPtc_;
  unsigned da_;
  float daPth_;
  float daPtc_;
};

