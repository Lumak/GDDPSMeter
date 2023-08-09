#include "DPSStats.h"

//=============================================================================
//=============================================================================
DPSStats::DPSStats()
{
  ClearData();
}

void DPSStats::ClearData()
{
  statDps_ = 0.0f;
  statHp_ = 0.0f;
  statDamageDealth_ = 0.0f;
  statDamageTaken_ = 0.0f;
}

void DPSStats::InputStats(const dpsStatData &stats)
{
  if (stats.types_ & dpsStatData::typeDps)
  {
    statDps_ = stats.dps_;
  }
  if (stats.types_ & dpsStatData::typeDamageDealt)
  {
    statDamageDealth_ = stats.damageDealt_;

  }
  if (stats.types_ & dpsStatData::typeDamageTaken)
  {
    statDamageTaken_ = stats.damageTaken_;

  }
  if (stats.types_ & dpsStatData::typeHp)
  {
    statHp_ = stats.hp_;
  }
}

void DPSStats::ShowWin(bool &showWindow)
{
  ImGui::SetNextWindowPos(ImVec2(10, 350), ImGuiCond_FirstUseEver, ImVec2(0, 0));

  Draw("DPS stats", &showWindow);
}

void DPSStats::Draw(const char* title, bool* p_open)
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

  const ImVec4 BLUE(0.5f, 0.5f, 1.0f, 1.0f);
  const ImVec4 LBLUE(0.4f, 0.4f, 0.9f, 1.0f);
  const ImVec4 DRED(1.0f, 0.2f, 0.2f, 1.0f);
  const ImVec4 DGRN(0.2f, 1.0f, 0.2f, 1.0f);

  if (boldFont_)
  {
    ImGui::PushFont(boldFont_);
  }

  ImGui::TextColored(GREEN, "DPS: ");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, "%1.0f", statDps_);

  ImGui::TextColored(GREEN, "Damage: ");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, "%1.0f", statDamageDealth_);

  ImGui::TextColored(GREEN, "Hp recovery/s: ");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, "%1.0f", statHp_);

  if (boldFont_)
  {
    ImGui::PopFont();
  }

  ImGui::End();
}

