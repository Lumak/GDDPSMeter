#include <cstdio>
#include "OADAStats.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
// fwd decl
int ImFormatString(char* buf, size_t buf_size, const char* fmt, ...);

//=============================================================================
//=============================================================================
OADAStats::OADAStats()
{
  oa_ = 0;
  oaPth_ = 0.0f;
  oaPtc_ = 0.0f;
  da_ = 0;
  daPth_ = 0.0f;
  daPtc_ = 0.0f;
}

void OADAStats::ClearData()
{
}

void OADAStats::SetStats(const OADAData &stats)
{
  oa_ = stats.oa_;
  oaPth_ = stats.pth_;
  oaPtc_ = stats.ptc_;
  da_ = stats.da_;
  daPth_ = stats.ptbh_;
  daPtc_ = stats.ptbc_;
}

void OADAStats::ShowWin(bool &showWindow)
{
  ImGui::SetNextWindowPos(ImVec2(10, 300), ImGuiCond_FirstUseEver, ImVec2(0, 0));

  Draw("OADA stats", &showWindow);
}

void OADAStats::Draw(const char* title, bool* p_open)
{
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (moveUI_)
  {
    window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
  }

  if (!ImGui::Begin(title, p_open, window_flags))
  {
    ImGui::End();
    return;
  }

  const ImVec4 GREEN(0.1f, 0.9f, 0.1f, 1.0f);
  const ImVec4 WHITE(0.8f, 0.9f, 0.8f, 1.0f);

  char oashort[5];
  char dashort[5];
  char oabuf[32];
  char dabuf[32];
  
  ImFormatString(oashort, 5, "%4d", oa_);
  ImFormatString(dashort, 5, "%4d", da_);

  ImFormatString(oabuf, 32, "%5.1f, %5.1f", oaPth_, oaPtc_);
  ImFormatString(dabuf, 32, "%5.1f, %5.1f", daPth_, daPtc_);

  if (boldFont_)
  {
    ImGui::PushFont(boldFont_);
  }
  // OA
  ImGui::TextColored(GREEN, "OA ");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, oashort);
  ImGui::SameLine();
  ImGui::TextColored(GREEN, " PTH/C(");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, oabuf);
  ImGui::SameLine();
  ImGui::TextColored(GREEN, ")");

  // DA
  ImGui::TextColored(GREEN, "DA ");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, dashort);
  ImGui::SameLine();
  ImGui::TextColored(GREEN, " PTBH/C(");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, dabuf);
  ImGui::SameLine();
  ImGui::TextColored(GREEN, ")");

  if (boldFont_)
  {
    ImGui::PopFont();
  }


#if 0
  if (ImGui::BeginTable("split", 2))
  {
    //ImGui::TextUnformatted(line_start, line_end);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show OA/DA stats", &showOADA_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show DPS/hit/HP recovery", &showDPSHitHpRecovery_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show active skill buffs", &showActiveSkillBuffs_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show Dev skill cooldowns", &showDevCD_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show combat log", &showCombatLog_);
    ImGui::TableNextColumn(); ImGui::Checkbox("Show DPS meter", &showDPSMeter_);
    ImGui::EndTable();
  }
#endif
  ImGui::End();
}
