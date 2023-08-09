#pragma once
#include <string>
#include "BaseRender.h"
#include "Texture.h"
#include "DispelBuffsWarning.h"
#include "OADAStats.h"
#include "DPSStats.h"
#include "DetailDamage.h"
#include "SkillActive.h"
#include "SkillCD.h"
#include "CombatLog.h"


#define MAX_COMBATLOG_LINES 50
#define OPTWIN_TITLE "DPSOptions win"

class MainUIController : public BaseRender
{
public:
  MainUIController();


  virtual void ShowWin(void *pd3dDevice);
  virtual void Draw(const char* title, bool* p_open = NULL);
  
  void QueryMessages(bool &menuShow);
  void ClearStats();
  void SetFonts();

  Texture& GetButtonTexture()
  {
    return buttonTexture_;
  }

  void SetPathName(const std::string& path);
  void SendKeyInput(int key);
  void SetStats(const OADAData &stats);

private:
  void CopyCheckbox(bool toSelf);
  void DistributeHPRecoveryMsg(hpRecoveryData &hpRecovery);

public:
  bool showWindow_;
  unsigned int menuButtonClicks_;

protected:
  DispelBuffsWarning dispelBuffsWarning_;
  CombatLog combatLog_;
  OADAStats oaDaStats_;
  DPSStats dDPSStats_;
  DetailDamage detailDamage_;
  SkillActive skillActive_;
  SkillCD skillCD_;

  bool showOADA_;
  bool showSkillCD_;
  bool showSkillActive_;
  bool showDPSStats_;
  bool showDetailDamage_;
  bool showDispelWarning_;

  bool checkboxesRead_;
  bool showOnceAtStartup_;

  std::string exePathName_;
  Texture buttonTexture_;

  ImFont* arialFont_;
  ImFont* arialBoldFont_;


};

