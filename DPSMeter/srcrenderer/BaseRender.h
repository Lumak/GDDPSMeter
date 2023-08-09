#pragma once
#include <map>
#include <imgui.h>
#include "IPCMessage.h"

class BaseRender
{
public:
  BaseRender();

  void SetMoveUI(bool set);
  void SetBoldFont(ImFont *bold);

  virtual void ShowWin(bool &showWindow);
  virtual void Draw(const char* title, bool* p_open = NULL);

protected:
  bool moveUI_;
  ImFont* boldFont_;

};
