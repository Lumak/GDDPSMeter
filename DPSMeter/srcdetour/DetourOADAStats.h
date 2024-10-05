#pragma once
#include "DetourBase.h"

//=============================================================================
//=============================================================================
#define SYM_CHAR_GETPLAYSTATS "?GetPlayStats@Character@GAME@@QAEAAVPlayStats@2@XZ"
#define SYM_PLAYSTATS_GETLASTMONSTERHIT "?GetLastMonsterHit@PlayStats@GAME@@QAEXAAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AAM@Z" 
#define SYM_PLAYSTATS_GETLASTMONSTERHITBY "?GetLastMonsterHitBy@PlayStats@GAME@@QAEXAAV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AAM@Z" 
#define SYM_CHAR_DESIGNERCALCOFFENSIVEABILITY "?DesignerCalculateOffensiveAbility@Character@GAME@@QAEMM@Z"
#define SYM_CHAR_DESIGNERCALCDEFENSIVEABILITY "?DesignerCalculateDefensiveAbility@Character@GAME@@QAEMM@Z" 
#define SYM_CHAR_DESIGNERCALCPROBTOHIT "?DesignerCalculateProbabilityToHit@Character@GAME@@QBEMMM@Z"
#define SYM_CHAR_DESIGNERCALCCRITCHANCE "?DesignerCalculateCriticalChance@Character@GAME@@QBEMM@Z" 

//=============================================================================
//=============================================================================
class DetourOADAStats : public DetourBase
{
public:
  DetourOADAStats();
  
  virtual bool SetupDetour();

  virtual void Update(void *player, int idx);
  virtual void SetPlayer(void* player) { playerPtr_ = player; }

  void SetPlayerInWorld(bool inworld);

private:
	ThisFunc<unsigned int&, void*> fnCharacterGetPlayStats_;
	ThisFunc<void, void*, unsigned int&, float&> fnPlayStatsGetLastMonsterHit_;
	ThisFunc<void, void*, unsigned int&, float&> fnPlayStatsGetLastMonsterHitBy_;
	ThisFunc<float, void*, float> fnCharacterDesignerCalculateOffensiveAbility_;
	ThisFunc<float, void*, float> fnCharacterDesignerCalculateDefensiveAbility_;
	ThisFunc<float, void*, float, float> fnCharacterDesignerCalculateProbabilityToHit_;
	ThisFunc<float, void*, float> fnCharacterDesignerCalculateCriticalChance_;

	void* playerPtr_;
	bool initialized_;
	unsigned int startTime_;
	bool playerInWorld_;
};

