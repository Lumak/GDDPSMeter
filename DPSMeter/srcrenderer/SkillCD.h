#pragma once
#include "BaseRender.h"

class SkillCD : public BaseRender
{
public:
  SkillCD();

  void InputMessage(IPCData &ipcData);
  void ClearMsgBuffs();

  virtual void ShowWin(bool &showWindow);
  virtual void Draw(const char* title, bool* p_open = NULL);

private:
  std::map<unsigned, skillBuffMsgData> skillBuffList_;

  bool dbgAfterEmpty_;

};

