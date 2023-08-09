#include <windows.h>
#include <cassert>

#include "DetourOADAStats.h"
#include "IPCMessage.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
#define OADA_UPDATE_INTERVAL 300

//=============================================================================
//=============================================================================
DetourFnData getPlayStats =       { "game.dll", NULL, NULL, SYM_CHAR_GETPLAYSTATS };
DetourFnData getLastMonstHit =    { "game.dll", NULL, NULL, SYM_PLAYSTATS_GETLASTMONSTERHIT };
DetourFnData getLastMonstHitBy =  { "game.dll", NULL, NULL, SYM_PLAYSTATS_GETLASTMONSTERHITBY };
DetourFnData charCalcOffAbility = { "game.dll", NULL, NULL, SYM_CHAR_DESIGNERCALCOFFENSIVEABILITY };
DetourFnData charCalcDefAbility = { "game.dll", NULL, NULL, SYM_CHAR_DESIGNERCALCDEFENSIVEABILITY };
DetourFnData charCalcProbHit =    { "game.dll", NULL, NULL, SYM_CHAR_DESIGNERCALCPROBTOHIT };
DetourFnData charCalcCritChance = { "game.dll", NULL, NULL, SYM_CHAR_DESIGNERCALCCRITCHANCE };

//=============================================================================
//=============================================================================
DetourOADAStats::DetourOADAStats()
{
    playerPtr_ = NULL;
    initialized_ = false;
    startTime_ = 0;
    playerInWorld_ = false;
 
}

bool DetourOADAStats::SetupDetour()
{
    int status = 0;

    status = HookDetour(getPlayStats);
	fnCharacterGetPlayStats_.SetFn(getPlayStats.realFn_);

    status += HookDetour(getLastMonstHit);
	fnPlayStatsGetLastMonsterHit_.SetFn(getLastMonstHit.realFn_);

    status += HookDetour(getLastMonstHitBy);
	fnPlayStatsGetLastMonsterHitBy_.SetFn(getLastMonstHitBy.realFn_);

    status += HookDetour(charCalcOffAbility);
	fnCharacterDesignerCalculateOffensiveAbility_.SetFn(charCalcOffAbility.realFn_);

    status += HookDetour(charCalcDefAbility);
	fnCharacterDesignerCalculateDefensiveAbility_.SetFn(charCalcDefAbility.realFn_);

    status += HookDetour(charCalcProbHit);
	fnCharacterDesignerCalculateProbabilityToHit_.SetFn(charCalcProbHit.realFn_);

    status += HookDetour(charCalcCritChance);
	fnCharacterDesignerCalculateCriticalChance_.SetFn(charCalcCritChance.realFn_);

    if (status != 0)
    {
        SetError("Error in DetourCommon::SetupDetour()");
    }

    return status == 0;
}

void DetourOADAStats::SetPlayerInWorld(bool inworld)
{
    playerInWorld_ = inworld;
    startTime_ = (unsigned int)timeGetTime();
}

void DetourOADAStats::Update(void *player, int)
{
    unsigned int curTime = (unsigned int)timeGetTime();
  
    if (playerPtr_ == NULL || playerPtr_ != player || !playerInWorld_
        || (curTime - startTime_ < OADA_UPDATE_INTERVAL))
    {
        return;
    }

    //reset timer
    startTime_ = curTime;

    //gather data
    unsigned int buf[32];
    float offAbil = 0.0f;
    float defAbil = 0.0f;
    float probToHit = 0.0f;
    float probToCrit = 0.0f;
    float probToBeHit = 0.0f;
    float probToBeCrit = 0.0f;
    float lastMonsterHit = 0.0f;
    float lastMonsterHitBy = 0.0f;

    memset(buf, 0, sizeof(buf));

    void *playStats = &fnCharacterGetPlayStats_.Fn_(playerPtr_);
	fnPlayStatsGetLastMonsterHit_.Fn_(playStats, (unsigned int&)buf, lastMonsterHit);
    offAbil = fnCharacterDesignerCalculateOffensiveAbility_.Fn_(playerPtr_, 0.0f);
    probToHit = fnCharacterDesignerCalculateProbabilityToHit_.Fn_(playerPtr_, offAbil, lastMonsterHit);
    probToCrit = fnCharacterDesignerCalculateCriticalChance_.Fn_(playerPtr_, probToHit);
    
    memset(buf, 0, sizeof(buf));

	fnPlayStatsGetLastMonsterHitBy_.Fn_(playStats, (unsigned int&)buf, lastMonsterHitBy);
    defAbil = fnCharacterDesignerCalculateDefensiveAbility_.Fn_(playerPtr_, 0.0f);
    probToBeHit = fnCharacterDesignerCalculateProbabilityToHit_.Fn_(playerPtr_, lastMonsterHitBy, defAbil);
    probToBeCrit = fnCharacterDesignerCalculateCriticalChance_.Fn_(playerPtr_, probToBeHit);

    // send msg
    IPCData ipc;
    ipc.msg_ = OADAstats;
    ipc.value_ = 0;
    ipc.u_.oada_.oa_ = (unsigned)offAbil;
    ipc.u_.oada_.pth_ = probToHit > 100.0f ? 100.0f : probToHit;
    ipc.u_.oada_.ptc_ = probToCrit * 100.0f;

    ipc.u_.oada_.da_ = (unsigned)defAbil;
    ipc.u_.oada_.ptbh_ = probToBeHit;
    ipc.u_.oada_.ptbc_ = probToBeCrit * 100.0f;
    IPCMessage::SendData(ipc);
}

