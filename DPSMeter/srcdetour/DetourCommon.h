#pragma once
#include <string>
#include "DetourBase.h"

//=============================================================================
//=============================================================================
#define STR_BUF_SIZE 64

#define SYM_GAMEENGINE_GENERATEUISKILLINFO "?GenerateUISkillInfo@GameEngine@GAME@@SAXPBVSkill@2@AAV?$vector@UGameTextLine@GAME@@@mem@@@Z"
#define SYM_OBJECT_GETOBJECTID "?GetObjectId@Object@GAME@@QBEIXZ" 
#define SYM_OBJECT_GETOBJECTNAME "?GetObjectName@Object@GAME@@QBEPBDXZ"
#define SYM_CHAR_GETSKILLLIST "?GetSkillList@Character@GAME@@QBEABV?$vector@PAVSkill@GAME@@@mem@@XZ"
#define SYM_SKILLMGR_GETITEMSKILLLIST "?GetItemSkillList@SkillManager@GAME@@QBEABV?$vector@PAVSkill@GAME@@@mem@@XZ" 
#define SYM_CHAR_GETSKILLMGR "?GetSkillManager@Character@GAME@@QAEAAVSkillManager@2@XZ" 
#define SYM_CHAR_GETCHARBIO "?GetCharacterBio@Character@GAME@@QAEAAVCharacterBio@2@XZ"
#define SYM_CHARBIO_GETBONUSLIFEAMOUNT "?GetBonusLifeAmount@CharacterBio@GAME@@QBEMABVBonus@2@@Z"
#define SYM_CHAR_GETCOMBATMGR "?GetCombatManager@Character@GAME@@QAEAAVCombatManager@2@XZ"
#define SYM_COMBATMGR_GETATTACKERID "?GetAttackerId@CombatManager@GAME@@QBE?BIXZ"
#define SYM_COMBATMGR_GETCHARACTER "?GetCharacter@CombatManager@GAME@@QAEPAVCharacter@2@XZ" 
#define SYM_SKILLBUFFSELFDURATION_TRACKABLETOTALTIME "?TrackableTotalTime@Skill_BuffSelfDuration@GAME@@UBE?BHI@Z"
#define SYM_SKILL_GETCOOLDOWNCOMPLETION "?GetCooldownCompletion@Skill@GAME@@QBEMXZ"
#define SYM_SKILL_GETCOOLDOWNREMAINING "?GetCooldownRemaining@Skill@GAME@@QBEHXZ" 
#define SYM_SKILL_GETCOOLDOWNTIME "?GetCooldownTime@Skill@GAME@@QBEMXZ"
#define SYM_SKILL_GETCOOLDOWNTOTAL "?GetCooldownTotal@Skill@GAME@@QBEHXZ" 
#define SYM_GETGAMETIME "?GetGameTime@GAME@@YAHXZ"
#define SYM_CHAR_GETPORTRAITNAME "?GetPortraitName@Character@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ" 

//=============================================================================
//=============================================================================
typedef void(__cdecl *FnGenerateUISkillInfo)(void *skillPtr, unsigned int &vectorRef);
typedef int(__cdecl *FnGetGameTime)(void);


//=============================================================================
//=============================================================================
class DetourCommon : public DetourBase
{
public:
  DetourCommon();

  virtual bool SetupDetour();
  virtual void Update(void *player, int idx);
  virtual void SetPlayer(void* player) {}

  unsigned int GetObjectId(void* charptr) const;
  void GetObjectName(void*, std::string &name);
  const char* GetSkillName(void* skillptr);
  unsigned int& CharGetSkillList(void *charPtr) const;
  unsigned int& SkillMgrGetItemSkillList(void *charPtr) const;
  unsigned int& CharGetSkillMgr(void *charPtr) const;
  unsigned int& CharGetCharacterBio(void* charPtr) const;
  float CharBioGetBonusLifeAmount(void *charBio, unsigned int& bonusRef) const;
  void* CharacterGetCombatManager(void* charPtr) const;
  unsigned int CombatMgrGetAttackerId(void*) const;
  void* CombatManagerGetCharacter(void*) const;
  int SkillTrackableTotalTime(void*, unsigned int) const;

  float SkillGetCooldownCompletion(void*) const;
  int SkillGetCooldownRemaining(void*) const;
  float SkillGetCooldownTime(void*) const;
  int SkillGetCooldownTotal(void*) const;
  int GetGameTime() const;
  unsigned int& CharGetPortraitName(void* charPtr) const;

private:
  bool initialized_;
  char wcharbuff[STR_BUF_SIZE];

  // fn ptrs
  RealFunc<int, void*> fnGetObjectId_;
  RealFunc<char const*, void*> fnObjectGetObjectName_;
  RealFunc<unsigned int&, void*> fnCharacterGetSkillList_;
  RealFunc<unsigned int&, void*> fnSkillManagerGetItemSkillList_;
  RealFunc<unsigned int&, void*> fnCharacterGetCharacterBio_;
  RealFunc<unsigned int&, void*> fnCharacterGetSkillManager_;
  RealFunc<float, void*, unsigned int&> fnCharacterBioGetBonusLifeAmount_;
  RealFunc<void*, void*> fnCharacterGetCombatManager_;
  RealFunc<unsigned int, void*> fnCombatMgrGetAttackerId_;
  RealFunc<void*, void*> fnCombatManagerGetCharacter_;
  RealFunc<int, void*, unsigned int> fnSkillTrackableTotalTime_;

  RealFunc<float, void*> fnSkillGetCooldownCompletion_;
  RealFunc<int, void*> fnSkillGetCooldownRemaining_;
  RealFunc<float, void*> fnSkillGetCooldownTime_;
  RealFunc<int, void*> fnSkillGetCooldownTotal_;
  RealFunc<void*, void*> fnSkillGetManager_;
  RealFunc<unsigned int&, void*> fnCharGetPortraitName_;

  FnGenerateUISkillInfo fnGenerateUISkillInfo_;
  FnGetGameTime fnGetGameTime_;

};

