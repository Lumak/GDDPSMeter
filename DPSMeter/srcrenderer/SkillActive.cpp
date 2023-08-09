#include <windows.h>
#include "SkillActive.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
SkillActive::SkillActive()
{
}

void SkillActive::ClearMsgBuffs()
{
  skillBuffList_.clear();
}

void SkillActive::InputMessage(IPCData &ipcData)
{
  skillBuffMsgData &skillbuffNew = ipcData.u_.skillBuff_;
  bool remove = (ipcData.msg_ == DataMsgType::ActiveSkillBuffRemove);
  
  //LOGF("skillactiveinputmsg: skillid=%d (%s), remove=%d, size=%d\n", 
  //  skillbuffNew.skillId_, skillbuffNew.name_, remove?1:0, skillBuffList_.size());

  //update
  std::map<unsigned, skillBuffMsgData>::iterator found = skillBuffList_.find(skillbuffNew.skillId_);

  if (!remove)
  {
    if (found == skillBuffList_.end())
    {
      skillBuffMsgData skillbuff;

      memset(&skillbuff, 0, sizeof(skillbuff));

      skillbuff.skillId_ = skillbuffNew.skillId_;
      skillbuff.timeRemaining_ = skillbuffNew.timeRemaining_;
      skillbuff.startTime_ = (int)timeGetTime();
      strcpy_s(skillbuff.name_, skillbuffNew.name_);

      //insert
      skillBuffList_[skillbuff.skillId_] = skillbuff;
    }
    else
    {
      found->second.timeRemaining_ = skillbuffNew.timeRemaining_;
    }
  }
  else if (found != skillBuffList_.end())
  {
    skillBuffList_.erase(found);
  }
}

void SkillActive::ShowWin(bool &showWindow)
{
  ImGui::SetNextWindowPos(ImVec2(10, 420), ImGuiCond_FirstUseEver, ImVec2(0, 0));

  Draw("Temp active buffs", &showWindow);
}

void SkillActive::Draw(const char* title, bool* p_open)
{
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

  if (moveUI_)
  {
    window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Always);
    p_open = NULL;
  }

  if (!ImGui::Begin(title, p_open, window_flags))
  {
    ImGui::End();
    return;
  }

  if (skillBuffList_.size() == 0)
  {
    ImGui::End();
    return;
  }


  const ImVec4 DYEL(1.0f, 0.9f, 0.0f, 1.0f);

  if (boldFont_)
  {
    ImGui::PushFont(boldFont_);
  }
  int curTime = (int)timeGetTime();

  for (std::map<unsigned, skillBuffMsgData>::iterator itr = skillBuffList_.begin();
    itr != skillBuffList_.end(); ++itr)
  {
    ImGui::TextColored(DYEL, "%s(", itr->second.name_);
    ImGui::SameLine();
    float ftime = (float)itr->second.timeRemaining_ / 1000.0f;
    int time = (int)ceil(ftime);

    ImGui::TextColored(DYEL, "%d", time);
    ImGui::SameLine();
    ImGui::TextColored(DYEL, ")");
  }

#if 0
  ImGui::TextColored(DYEL, "Dryad's Blessing(");
  ImGui::SameLine();
  ImGui::TextColored(LYEL, "9");
  ImGui::SameLine();
  ImGui::TextColored(DYEL, ")");
  
  ImGui::TextColored(DYEL, "Maul(");
  ImGui::SameLine();
  ImGui::TextColored(LYEL, "5");
  ImGui::SameLine();
  ImGui::TextColored(DYEL, ")");
#endif
  if (boldFont_)
  {
    ImGui::PopFont();
  }

  ImGui::End();
}


