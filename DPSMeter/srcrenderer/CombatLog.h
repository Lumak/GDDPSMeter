#pragma once
#include "BaseRender.h"

class CombatLog : public BaseRender
{
public:
  CombatLog();
  CombatLog(int lines);

  void Clear();
  void SetTotalLinesToShow(int lines);

  void AddLog(const char* fmt, ...);
  virtual void ShowWin(bool &showWindow);
  virtual void Draw(const char* title, bool* p_open = NULL);

protected:
  int maxLines_;
  ImGuiTextBuffer     Buf;
  //ImGuiTextFilter     Filter;
  ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
  bool                AutoScroll;  // Keep scrolling if already at the bottom.

};

