#include <algorithm>
#include "DetailDamage.h"
#include "MainUIController.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
#define MAX_ATTRIB_CNT 61

const char *AttribTypeDef[MAX_ATTRIB_CNT] =
{
    "unknown",                                 // 0
    "phy pierce",                              // 1 
    "physical",                                // 2 
    "pierce ratio",                            // 3 
    "pierce",                                  // 4 
    "cold",                                    // 5 
    "fire",                                    // 6 
    "poison/acid",                             // 7 
    "lightning",                               // 8 
    "vitality",                                // 9 
    "chaos",                                   // 10 
    "aether",                                  // 11 
    "mana burn",                               // 12 
    "disruption",                              // 13 
    "percent cur life",                        // 14 
    "bleeding",                                // 15 
    "total speed",                             // 16 
    "attack speed",                            // 17 
    "spell cast speed",                        // 18 
    "run speed",                               // 19 
    "life leech",                              // 20 
    "mana leech",                              // 21 
    "offensive ability",                       // 22 
    "defensive ability",                       // 23 
    "offensive reduction",                     // 24 
    "defensive reduction",                     // 25 
    "fumble",                                  // 26 
    "projectile fumble",                       // 27 
    "total damage reduction percent",          // 28 
    "total damage reduction absolute",         // 29 
    "total resistance reduction percent",      // 30 
    "total resistance reduction absolute",     // 31 
    "physical resistance reduction percent",   // 32 
    "physical resistance reduction absolute",  // 33 
    "elemental resistance reduction percent",  // 34 
    "elemental resistance reduction absolute", // 35 
    "physical damage reduction percent",       // 36 
    "elemental damage reduction percent",      // 37 
    "absorption protection",                   // 38 
    "absorption",                              // 39 
    "protection",                              // 40 
    "armor rating",                            // 41 
    "stun",                                    // 42 
    "sleep",                                   // 43 
    "trap",                                    // 44 
    "freeze",                                  // 45 
    "petrify",                                 // 46 
    "immobilize",                              // 47 
    "knockdown",                               // 48 
    "take hit",                                // 49 
    "taunt",                                   // 50 
    "convert",                                 // 51 
    "fear",                                    // 52 
    "confusion",                               // 53 
    "block modifier",                          // 54 
    "block amount modifier",                   // 55 
    "reflect",                                 // 56 
    "elemental group",                         // 57 
    "total damage modifier",                   // 58 
    "crit damage modifier",                    // 59 
    "mana burn ratio"                          // 60 
};


//=============================================================================
//=============================================================================
DetailDamage::DetailDamage()
{
    mainUIController_ = NULL;
    boldFont_ = NULL;
    moveUI_ = false;

    dps_ = 0.0f;
    totalSkillDamage_ = 0.0f;
    totalDealtAttackDamage_ = 0.0f;
    totalTakenAttackDamage_ = 0.0f;
    totalHpRecovery_ = 0.0f;

    lastDetailDmg_ = 0.0f;
    lastSkillDmg_ = 0.0f;
}

void DetailDamage::SetMainUIController(MainUIController *mainController)
{
    mainUIController_ = mainController;
}

void DetailDamage::ClearLists()
{
    dps_ = 0.0f;
    totalDamage_ = 0.0f;
    maxDamage_ = 0.0f;
    avgDamage_ = 0.0f;
    hitCount_ = 0;
    duration_ = 0;

    totalHpRecovery_ = 0.0f;
    hpRecoveryIds_.clear();

    totalDealtAttackDamage_ = 0.0f;
    dealtAttackDamageList_.clear();

    totalTakenAttackDamage_ = 0.0f;

    totalSkillDamage_ = 0.0f;
    skillDamageList_.clear();
}

void DetailDamage::InputDPSstat(const dpsStatData &stats)
{
    if (stats.duration_ < duration_)
    {
        dps_ = 0.0f;
        totalDamage_ = 0.0f;
        maxDamage_ = 0.0f;
        avgDamage_ = 0.0f;
        hitCount_ = 0;
        duration_ = 0;
    }

    hitCount_++;
    dps_ = stats.dps_;
    totalDamage_ += stats.damageDealt_;

    if (stats.damageDealt_ > maxDamage_)
    {
        maxDamage_ = stats.damageDealt_;
    }
    avgDamage_ = totalDamage_ / (float)hitCount_;
    duration_ = stats.duration_;
}

void DetailDamage::InputHpRecoveryMsg(const hpRecoveryData &hpRecovery)
{
    static int cnt = 0;
    cnt++;
    totalHpRecovery_ += hpRecovery.hp_;
    int foundIdx = -1;

    for (unsigned i = 0; i < hpRecoveryIds_.size(); ++i)
    {
        if (hpRecoveryIds_[i].hpRecovery_.typeId_ == hpRecovery.typeId_)
        {
            foundIdx = (int)i;
            break;
        }
    }

    if (foundIdx != -1)
    {
        detailHpRecoveryData &detailHpRecovery = hpRecoveryIds_[foundIdx];
        detailHpRecovery.hpRecovery_.hp_ += hpRecovery.hp_;
        detailHpRecovery.count_++;
    }
    else
    {
        detailHpRecoveryData detailHpRecovery;
        detailHpRecovery.hpRecovery_.Copy(hpRecovery);
        detailHpRecovery.count_ = 1;

        hpRecoveryIds_.push_back(detailHpRecovery);
    }
}

void DetailDamage::InputAttackDamage(const attackDamageData &attackDamage)
{
    if (attackDamage.playerDealingDamage_)
    {
        for (unsigned j = 0; j < DETAIL_SIZE; ++j)
        {
            if (attackDamage.details_[j].type_ != 0)
            {
                //see if we have the damage type already stored
                unsigned idx = 0xffff;
                for (unsigned i = 0; i < dealtAttackDamageList_.size(); ++i)
                {
                    if (attackDamage.details_[j].type_ == dealtAttackDamageList_[i].type_)
                    {
                        idx = i;
                        break;
                    }
                }

                //if found add it to the existing list, otherwise, create new
                if (idx != 0xffff)
                {
                    totalDealtAttackDamage_ += attackDamage.details_[j].damage_;

                    dealtAttackDamageList_[idx].damage_ += attackDamage.details_[j].damage_;
                    dealtAttackDamageList_[idx].count_++;
                }
                else
                {
                    totalDealtAttackDamage_ += attackDamage.details_[j].damage_;

                    detailAttackDamageData detAtkDmg;
                    detAtkDmg.type_ = attackDamage.details_[j].type_;
                    detAtkDmg.damage_ = attackDamage.details_[j].damage_;
                    detAtkDmg.count_ = 1;

                    dealtAttackDamageList_.push_back(detAtkDmg);

                }
                //LOGF("InputAttackDamage: type=%d, dmg=%1.0f\n", attackDamage.details_[j].type_, attackDamage.details_[j].damage_);
            }
            else
            {
                break;
            }
        }
    }
}

void DetailDamage::InputSkillDamage(const skillDamageData &skillDamage)
{
    unsigned idx = 0xffff;
  
    totalSkillDamage_ += skillDamage.damage_;

    for (unsigned i = 0; i < skillDamageList_.size(); ++i)
    {
        if (skillDamage.skillId_ == skillDamageList_[i].skillId_)
        {
            idx = i;
            //LOGF("InputSkillDamage: idx=%d\n", i);
            break;
        }
    }

    if (idx != 0xffff)
    {
        skillDamageList_[idx].totalDamage_ += skillDamage.damage_;

        if (skillDamage.isCrit_)
        {
            skillDamageList_[idx].critCount_++;
            skillDamageList_[idx].critDamageTotal_ += skillDamage.damage_;
        }
        else
        {
            skillDamageList_[idx].hitCount_++;
            skillDamageList_[idx].hitDamageTotal_ += skillDamage.damage_;
        }
        //LOGF("InputSkillDamage: hit cnt=%d, critdnt=%d, dmg=%1.0f\n", skillDamageList_[idx].hitCount_, skillDamageList_[idx].critCount_, skillDamage.damage_);

    }
    else
    {
        detailskillDamageData skillDmg;
        skillDmg.skillId_ = skillDamage.skillId_;
        skillDmg.totalDamage_ += skillDamage.damage_;
        strcpy_s(skillDmg.name_, skillDamage.name_);

        if (skillDamage.isCrit_)
        {
            skillDmg.critCount_ = 1;
            skillDmg.critDamageTotal_ = skillDamage.damage_;
        }
        else
        {
            skillDmg.hitCount_ = 1;
            skillDmg.hitDamageTotal_ = skillDamage.damage_;
        }
        skillDamageList_.push_back(skillDmg);
    }
}

void DetailDamage::PrepSkillDamageForView()
{
  if (skillDamageList_.size() > 0)
  {
    detailskillDamageData &prt = skillDamageList_[0];
    std::sort(skillDamageList_.begin(), skillDamageList_.end(), prt);
  }
}

void DetailDamage::PrepAttackDamageForView()
{
  if (dealtAttackDamageList_.size() > 0)
  {
    detailAttackDamageData &prt = dealtAttackDamageList_[0];
    std::sort(dealtAttackDamageList_.begin(), dealtAttackDamageList_.end(), prt);
  }
}

void DetailDamage::PrepHpRecoveryForView()
{
  if (hpRecoveryIds_.size() > 0)
  {
    detailHpRecoveryData &prt = hpRecoveryIds_[0];
    std::sort(hpRecoveryIds_.begin(), hpRecoveryIds_.end(), prt);
  }
}

void DetailDamage::ShowWin(bool &showWindow)
{
  ImGui::SetNextWindowPos(ImVec2(300, 500), ImGuiCond_FirstUseEver, ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

  Draw("Damage details win", &showWindow);
}

void DetailDamage::Draw(const char* title, bool* p_open)
{
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
  ImGui::SetNextWindowBgAlpha(0.85f); // semi transparent background

  if (moveUI_)
  {
    window_flags &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
  }

  if (!ImGui::Begin(title, 0, window_flags))
  {
    ImGui::End();
    return;
  }

 
  const ImVec4 BACK(0.0f, 0.0f, 0.06f, 1.0f);
  const ImVec4 LYEL(0.9f, 0.9f, 0.7f, 1.0f);

  ImGui::PushStyleColor(ImGuiCol_Button, BACK);
  ImGui::PushStyleColor(ImGuiCol_Text, LYEL);

  ImVec2 wsize = ImGui::GetWindowSize();
  float rightJustified = wsize.x - 50.0f;
  if (rightJustified < 0.0f) rightJustified = 0.0f;
  ImGui::SameLine(rightJustified);

  if (ImGui::SmallButton("reset"))
  {
    ClearLists();

    //let's not clear the dps stat as those stats are timed by 
    //detour main and clearing it wouldn't do much
    //if (mainUIController_)
    //{
    //    mainUIController_->ClearStats();
    //}
  }
  
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();

  ImGui::Separator();

  //ImGui::BeginChild("tabsgroups");

  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("DetailTabBar", tab_bar_flags))
  {
    ImGui::PushStyleColor(ImGuiCol_Tab, BACK);
    ImGui::PushStyleColor(ImGuiCol_Text, LYEL);

    if (ImGui::BeginTabItem("dps"))
    {
        InsertCollapsableTree(DetailDPS);
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("skills"))
    {
        InsertCollapsableTree(DetailSkills);
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("dmg type"))
    {
        InsertCollapsableTree(DetailDamageDealt);
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("heals"))
    {
        InsertCollapsableTree(DetailHpRecovery);
        ImGui::EndTabItem();
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    
    ImGui::EndTabBar();
  }

  ImGui::End();
}

void DetailDamage::InsertCollapsableTree(DetailType detail)
{
  const int buffSize = 100;
  char buff[buffSize];
  const ImVec4 LYEL(0.9f, 0.9f, 0.7f, 1.0f);

  ImGui::BeginChild("skillscrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

  if (detail == DetailDPS)
  {
	  ImGui::TextUnformatted("*dps updates when you strike a target and resets");
	  ImGui::TextUnformatted("  after 2 sec. of inactivity");

      sprintf_s(buff, buffSize, "DPS: %1.0f", dps_);

      if (ImGui::TreeNode(buff))
      {
          sprintf_s(buff, buffSize, "   max damage %1.0f", maxDamage_);
          ImGui::TextUnformatted(buff);

          sprintf_s(buff, buffSize, "   avg damage %1.0f", avgDamage_);
          ImGui::TextUnformatted(buff);

          if (duration_ / 3600 > 0)
          {
              unsigned int secs = duration_;
              unsigned int hrs = secs / 3600;
              secs -= hrs * 3600;
              unsigned int mins = secs / 60;
              secs -= mins * 60;

              sprintf_s(buff, buffSize, "   duraton %02d:%02d:%02d sec.", hrs, mins, secs);
          }
          else if (duration_ / 60 > 0)
          {
              unsigned int secs = duration_;
              unsigned int mins = secs / 60;
              secs -= mins * 60;
              sprintf_s(buff, buffSize, "   duraton %02d:%02d sec.", mins, secs);
          }
          else
          {
              sprintf_s(buff, buffSize, "   duraton %02d sec.", duration_);
          }

          ImGui::TextUnformatted(buff);

          ImGui::TreePop();
      }
  }
  else if (detail == DetailSkills)
  {
    PrepSkillDamageForView();
    sprintf_s(buff, buffSize, "Skills damage total: %1.0f", totalSkillDamage_);
    if (ImGui::TreeNode(buff))
    {
      for (unsigned i = 0; i < skillDamageList_.size(); ++i)
      {
        //note: do this in detourMain using #define
        //skip listing unknown skills
        //if (strncmp(skillDamageList_[i].name_, "id:", 3) == 0)
        //{
        //    continue;
        //}

        sprintf_s(buff, buffSize, "%1.0f (%1.0f%c) %s [%d]",
          skillDamageList_[i].totalDamage_,
          (skillDamageList_[i].totalDamage_ / totalDealtAttackDamage_) * 100.0f,
          '%',
          skillDamageList_[i].name_,
          skillDamageList_[i].hitCount_ + skillDamageList_[i].critCount_);

        if (ImGui::CollapsingHeader(buff))
        {
			//the order is normal damage then crit, looks odd if crit came first
            sprintf_s(buff, buffSize, "   %1.0f (%1.0f%c) %s [%d]",
                skillDamageList_[i].hitDamageTotal_,
                (skillDamageList_[i].hitDamageTotal_ / totalDealtAttackDamage_)*100.0f,
                '%',
                skillDamageList_[i].name_,
                skillDamageList_[i].hitCount_);

            ImGui::TextUnformatted(buff);

            sprintf_s(buff, buffSize, "   %1.0f (%1.0f%c) Crit %s [%d]",
                skillDamageList_[i].critDamageTotal_,
                (skillDamageList_[i].critDamageTotal_ / totalDealtAttackDamage_)*100.0f,
                '%',
                skillDamageList_[i].name_,
                skillDamageList_[i].critCount_);
			
			ImGui::TextUnformatted(buff);
        }

      }
      ImGui::TreePop();
    }

  }
  else if (detail == DetailDamageDealt)
  {
    PrepAttackDamageForView();

    sprintf_s(buff, buffSize, "Damage types total: %1.0f", totalDealtAttackDamage_);
    if (ImGui::TreeNode(buff))
    {
      for (unsigned i = 0; i < dealtAttackDamageList_.size(); ++i)
      {
        sprintf_s(buff, buffSize, "%1.0f (%1.0f%c) %s [%d]",
          dealtAttackDamageList_[i].damage_,
          (dealtAttackDamageList_[i].damage_ / totalDealtAttackDamage_) * 100.0f,
          '%',
          AttribTypeDef[dealtAttackDamageList_[i].type_],
          dealtAttackDamageList_[i].count_);

        if (ImGui::CollapsingHeader(buff))
        {
          if (dealtAttackDamageList_[i].count_ > 0)
          {
            sprintf_s(buff, buffSize, "     average %1.0f", dealtAttackDamageList_[i].damage_ / (float)dealtAttackDamageList_[i].count_);
          }
          else
          {
            strcpy_s(buff, "     average 0");
          }
          ImGui::TextUnformatted(buff);
        }
      }
      ImGui::TreePop();
    }

  }
  else if (detail == DetailHpRecovery)
  {
    PrepHpRecoveryForView();

    sprintf_s(buff, buffSize, "Hp recovery total: %1.0f", totalHpRecovery_);
    
    if (ImGui::TreeNode(buff))
    {
      for (unsigned i = 0; i < hpRecoveryIds_.size(); ++i)
      {
        sprintf_s(buff, buffSize, "%1.0f (%1.0f%c) %s [%d]",
          hpRecoveryIds_[i].hpRecovery_.hp_,
          (hpRecoveryIds_[i].hpRecovery_.hp_ / totalHpRecovery_) * 100.0f,
          '%',
          hpRecoveryIds_[i].hpRecovery_.name_,
          hpRecoveryIds_[i].count_);

        if (ImGui::CollapsingHeader(buff))
        {
          if (hpRecoveryIds_[i].count_)
          {
            sprintf_s(buff, buffSize, "     average %1.0f", hpRecoveryIds_[i].hpRecovery_.hp_ / (float)hpRecoveryIds_[i].count_);
          }
          else
          {
            strcpy_s(buff, "     average 0");
          }
          ImGui::Text(buff);
        }
      }
      ImGui::TreePop();
    }
  }
  else
  {
    if (ImGui::TreeNode("Collapsing Headers"))
    {
      if (ImGui::CollapsingHeader("Header", ImGuiTreeNodeFlags_None))
      {
        ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
        for (int i = 0; i < 5; i++)
          ImGui::Text("Some content %d", i);
      }
      if (ImGui::CollapsingHeader("Header with a close button"))
      {
        ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
        for (int i = 0; i < 5; i++)
          ImGui::Text("More content %d", i);
      }
      ImGui::TreePop();
    }
  }

  ImGui::EndChild();
}


