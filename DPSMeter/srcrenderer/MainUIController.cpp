#include "MainImgui.h"
#include "MainUIController.h"
#include "CombatLog.h"
#include "IPCMessage.h"
#include "Logger.h"
#include "resource.h"

//=============================================================================
//=============================================================================
#define MAIN_MENU_BOTTON    IDB_PNG1

//=============================================================================
//=============================================================================
BOOL FileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

MainUIController::MainUIController()
{
  showWindow_ = false;
  showOADA_ = false;
  showSkillCD_ = false;
  showSkillActive_ = false;
  showDPSStats_ = false;
  showDetailDamage_ = false;
  showDispelWarning_ = false;

  checkboxesRead_ = false;
  showOnceAtStartup_ = true;

  arialFont_ = NULL;
  arialBoldFont_ = NULL;
  
  combatLog_.SetTotalLinesToShow(MAX_COMBATLOG_LINES);
}

void MainUIController::SetFonts()
{
  ImGuiIO& io = ImGui::GetIO();
  arialFont_ = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Arial.ttf", 14.0f, NULL, NULL);
  arialBoldFont_ = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Arialbd.ttf", 16.0f, NULL, NULL);

  oaDaStats_.SetBoldFont(arialBoldFont_);
  dDPSStats_.SetBoldFont(arialBoldFont_);
  dispelBuffsWarning_.SetBoldFont(arialBoldFont_);
  skillActive_.SetBoldFont(arialBoldFont_);
  skillCD_.SetBoldFont(arialBoldFont_);

  detailDamage_.SetMainUIController(this);

  //show options window on very first run
  if (io.IniFilename && !FileExists(io.IniFilename))
  {
      LOGF("*** main ui setfont, show win");
    showWindow_ = true;
  }
}

void MainUIController::QueryMessages(bool &menuShow)
{
  std::vector<IPCData> list;

  MessageHandler::GetQueue().GetMessages(list);

  for (unsigned i = 0; i < list.size(); ++i)
  {
    switch (list[i].msg_)
    {
    case DataMsgType::SetHwnWindow:
      LOGF("QueryMessages  SetHwnWindow=0x%X", list[i].value_);
      ImGuiMain::SetHwnWindow((void*)list[i].value_);
      break;

    case DataMsgType::ShowUI:
      menuShow = (list[i].value_ == 1 ? true : false );
      break;

    case DataMsgType::InputKey:
      SendKeyInput(list[i].value_);
      break;

    case DataMsgType::OADAstats:
      oaDaStats_.SetStats(list[i].u_.oada_);
      break;

    case DataMsgType::DispelBuffs:
        dispelBuffsWarning_.ShowWarning();
        break;
    
    case DataMsgType::ClearSkillBuffs:
      LOGF("MainUI clear accumulators\n");
      dDPSStats_.ClearData();
      skillActive_.ClearMsgBuffs();
      skillCD_.ClearMsgBuffs();
      detailDamage_.ClearLists();
      break;

    case DataMsgType::ActiveSkillBuff:
    case DataMsgType::ActiveSkillBuffRemove:
      skillActive_.InputMessage(list[i]);
      break;

    case DataMsgType::SkillBuffCooldown:
    case DataMsgType::SkillBuffCooldownRemove:
      skillCD_.InputMessage(list[i]);
      break;

    case DataMsgType::AttackDamage:
      detailDamage_.InputAttackDamage(list[i].u_.attackDamage_);
      break;

    case DataMsgType::PlayerSkillDamage:
      detailDamage_.InputSkillDamage(list[i].u_.skillDamage_);
      break;

    case DataMsgType::HPRecovery:
      DistributeHPRecoveryMsg(list[i].u_.hpRecovery_);
      break;

    case DataMsgType::DPSstats:
      dDPSStats_.InputStats(list[i].u_.dpsStats_);
      detailDamage_.InputDPSstat(list[i].u_.dpsStats_);
      break;
    }
  }
}

void MainUIController::ClearStats()
{
    dDPSStats_.ClearData();
}


void MainUIController::DistributeHPRecoveryMsg(hpRecoveryData &hpRecovery)
{
  dpsStatData dpsStat;
  dpsStat.Clear();
  dpsStat.types_ = dpsStat.typeHp;
  dpsStat.hp_ = hpRecovery.hpPerSec_;

  dDPSStats_.InputStats(dpsStat);
  detailDamage_.InputHpRecoveryMsg(hpRecovery);
}

void MainUIController::CopyCheckbox(bool toSelf)
{
  bool checkboxes[ImGuiCheckboxSize_Int8] = { 0 };

  if (toSelf)
  {
    ImGui::GetCheckboxArray(OPTWIN_TITLE, checkboxes[0]);

    showOADA_ = checkboxes[0];
    showSkillCD_ = checkboxes[1];
    showDPSStats_ = checkboxes[2];
    showSkillActive_ = checkboxes[3];
    showDetailDamage_ = checkboxes[4];
    showDispelWarning_ = checkboxes[5];
    checkboxesRead_ = true;
  }
  else
  {
    checkboxes[0] = showOADA_;
    checkboxes[1] = showSkillCD_;
    checkboxes[2] = showDPSStats_;
    checkboxes[3] = showSkillActive_;
    checkboxes[4] = showDetailDamage_;
    checkboxes[5] = showDispelWarning_;

    ImGui::SetCheckboxArray(OPTWIN_TITLE, checkboxes, ImGuiCheckboxSize_Int8);
  }
}

void MainUIController::SendKeyInput(int key)
{
  if (key == VK_ESCAPE)
  {
    if (showWindow_)
    {
      showWindow_ = false;
    }
  }
}

void MainUIController::SetStats(const OADAData &stats)
{
  oaDaStats_.SetStats(stats);
}

void MainUIController::ShowWin(void* pd3dDevice)
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
  ImVec2 work_size = viewport->WorkSize;
  ImVec2 window_pos;
  window_pos.x = work_size.x * 0.5f - 25.0f;
  window_pos.y = work_size.y - 102.0f;
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver);

  // setup button texture
  Texture &buttonText = GetButtonTexture();
  if (buttonText.GetDevice() == NULL)
  {
    buttonText.SetDevice(pd3dDevice);
  }
  if (buttonText.GetTexture() == NULL)  
  {
      LOGF("load button resource");
      buttonText.LoadTextureFromResource(MAIN_MENU_BOTTON);
      if (buttonText.GetTexture() == NULL)
      {
          LOGF("button texture NULL");
      }
  }
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
                                  ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (moveUI_)
  {
    window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
  }

  if (ImGui::Begin("DPSOptions button", 0, window_flags))
  {
    if (buttonText.GetTexture())
    {
      const ImVec2 uv0(0.0f, 0.0f), uv1(1.0f, 1.0f);
      const int padding = 0;
      const ImVec4 bgCol(0.0f, 0.0f, 0.0f, -1.0f);
      ImVec4 tintCol(0.9f, 0.9f, 0.9f, 1.0f);

      if (ImGui::IsWindowHovered(0))
      {
        tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
      }

      if (ImGui::ImageButton((ImTextureID)buttonText.GetTexture(), ImVec2(35, 35), uv0, uv1, padding, bgCol, tintCol))
      {
        showWindow_ = !showWindow_;
        if (!showWindow_)
        {
          moveUI_ = false;
        }
      }
    }
    //no button texture found, draw generic button box
    else
    {
      if (ImGui::Button("DPS Meter"))
      {
        showWindow_ = !showWindow_;
      }
    }
  }
  ImGui::End();

  if (ImGui::IsKeyPressed(ImGuiKey_Escape) && showWindow_)
  {
    LOGF("escape key negates showWindow\n");
    showWindow_ = false;
  }
  
  if (showWindow_ || showOnceAtStartup_)
  {
    Draw(OPTWIN_TITLE);
    showOnceAtStartup_ = false;
  }

  if (showOADA_)
  {
    oaDaStats_.SetMoveUI(moveUI_);
    oaDaStats_.ShowWin(showOADA_);
  }

  if (showSkillCD_)
  {
    skillCD_.SetMoveUI(moveUI_);
    skillCD_.ShowWin(showSkillCD_);
  }
  if (showSkillActive_)
  {
    skillActive_.SetMoveUI(moveUI_);
    skillActive_.ShowWin(showSkillActive_);
  }

  if (showDPSStats_)
  {
    dDPSStats_.SetMoveUI(moveUI_);
    dDPSStats_.ShowWin(showDPSStats_);
  }
  if (showDetailDamage_)
  {
    detailDamage_.SetMoveUI(moveUI_);
    detailDamage_.ShowWin(showDetailDamage_);
  }
  if (showDispelWarning_)
  {
      dispelBuffsWarning_.SetMoveUI(moveUI_);
      dispelBuffsWarning_.ShowWin(showDispelWarning_);
  }

}

void MainUIController::Draw(const char* title, bool* p_open)
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
  ImVec2 work_size = viewport->WorkSize;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
  ImGui::SetNextWindowPos(ImVec2(work_size.x * 0.5f, work_size.y - 200.0f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

  if (moveUI_)
  {
    window_flags &= ~ImGuiWindowFlags_NoMove;
  }

  if (!ImGui::Begin(title, p_open, window_flags))
  {
    ImGui::End();
    return;
  }

  if (!checkboxesRead_)
  {
    CopyCheckbox(true);
  }

  if (ImGui::BeginTable("split", 2))
  {
    ImGui::TableNextColumn(); ImGui::Checkbox("Show OA/DA stats", &showOADA_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show temp active skill buffs", &showSkillActive_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show DPS stats", &showDPSStats_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show skill buff cooldowns", &showSkillCD_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show damage details win", &showDetailDamage_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show dispel buffs warning", &showDispelWarning_);
    ImGui::EndTable();

  }
  ImGui::Separator();
  ImGui::Checkbox("Move/Adjust UI", &moveUI_);

  //copy checkboxes to window
  CopyCheckbox(false);

  ImGui::End();
}

void MainUIController::SetPathName(const std::string& path)
{
  exePathName_ = path;
}