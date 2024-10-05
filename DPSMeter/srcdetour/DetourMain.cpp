#include <windows.h>
#include <process.h>
#include <cassert>
#include <algorithm>

#include "DetourMain.h"
#include "MainImgui.h"
#include "DetourCommon.h"
#include "DetourSkill.h"
#include "DetourOADAStats.h"
#include "DetourScreen.h"
#include "IPCMessage.h"
#include "DetourUtil.h"
#include "AttribTypeDef.h"
#include "Logger.h"
#include "proc.h"

//=============================================================================
//=============================================================================
#define PLAYER_UDATE_INTERVAL 100
#define NOACTIVITY_REFRESH_TIMER 2000
#define CONSOLIDATE_UNIDENTIFIED_SKILLS

//=============================================================================
//=============================================================================
DetourFnData datProcessMsgHwndWindow = { "engine.dll", NULL, (VoidFn)&DetourMain::DTProcessMsgHwndWindow, SYM_PROCESSMSG_HWNDWINDOE };
DetourFnData datGetSystemWindow = { "engine.dll", NULL, NULL, SYM_GETSYSTEMWINDOW };

DetourFnData datPlayerUpdateSelf = { "game.dll", NULL, (VoidFn)&DetourMain::DTPlayerUpdateSelf, SYM_PLAYER_UPDATESELF };
DetourFnData datPlayerOnAttack = { "game.dll", NULL, (VoidFn)&DetourMain::DTPlayerOnAttack, SYM_PLAYER_ONATTACK };
DetourFnData datCharBioTakeBonus = { "game.dll", NULL, (VoidFn)&DetourMain::DTCharacterBioTakeBonus, SYM_CHARBIO_TAKEBONUS };
DetourFnData datCharacterAttackTarget = { "game.dll", NULL, (VoidFn)&DetourMain::DTCharacterAttackTarget,SYM_CHAR_ATTACKTARGET  };

DetourFnData datCombatMgrApplyDamage = { "game.dll", NULL, (VoidFn)&DetourMain::DTCombatgMgrApplyDamage, SYM_COMBATMGR_APPLYDAMAGE };
DetourFnData datCombatMgrTakeAttack = { "game.dll", NULL, (VoidFn)&DetourMain::DTCombatManagerTakeAttack, SYM_COMBATMGR_TAKEATTACK };

DetourFnData datPlayerRegisterCombatTextHit = { "game.dll", NULL, (VoidFn)&DetourMain::DTPlayerRegisterCombatTextHit,  SYM_PLAYER_REGISTERCOMBATTEXTHIT};
DetourFnData datPlayerRegisterCombatTextCrit = { "game.dll", NULL, (VoidFn)&DetourMain::DTPlayerRegisterCombatTextCrit, SYM_PLAYER_REGISTERCOMBATTEXTCRIT };

DetourFnData datGameEngineRegisterDamage = { "game.dll", NULL, (VoidFn)&DetourMain::DTGameEngineRegisterDamage, SYM_GAMEENGINE_REGISTERDAMAGE };
DetourFnData datCombatAttribAccExeDamage = { "game.dll", NULL, (VoidFn)&DetourMain::DTCombatAttribAccuExeDamage, SYM_COMBATATTRIBACC_EXEDAMAGE };
DetourFnData datCtrlPlayerRequestMoveAction = { "game.dll", NULL, (VoidFn)&DetourMain::DTCtrlPlayerStateDfltRequestMoveAction, SYM_CTRLPLAYER_REQUESTMOVEACTION };

DetourFnData datPlayerPostSpawnPet = { "game.dll", NULL, (VoidFn)&DetourMain::DTPlayerPostSpawnPet, SYM_PLAYER_POSTSPAWNPET };
DetourFnData datCharCharacterIsDying = { "game.dll", NULL, (VoidFn)&DetourMain::DTCharCharacterIsDying, SYM_CHAR_CHARACTERISDYING };

ThisFunc<void, void*, void*> DetourMain::fnProcessMsgHwndWindow_;
ThisFunc<struct HWND__*, void*> DetourMain::fnGetSystemWindow_;

ThisFunc<void, void*, int> DetourMain::fnPlayerUpdateSelf_;
ThisFunc<void, void*, void*> DetourMain::fnPlayerOnAttack_;
ThisFunc<void, void*, unsigned int&, bool> DetourMain::fnCharacterBioTakeBonus_;
ThisFunc<bool, void*, unsigned int, unsigned int&, unsigned int&, bool, unsigned int, unsigned int, unsigned int> DetourMain::fnCharacterAttackTarget_;

ThisFunc<float, void*, float> DetourMain::fnPlayerRegisterCombatTextHit_;
ThisFunc<float, void*, float> DetourMain::fnPlayerRegisterCombatTextCrit_;
ThisFunc<void, void*, unsigned int, unsigned int, float> DetourMain::fnGameEngineRegisterDamage_;
ThisFunc<bool, void*, float, unsigned int&, unsigned int, unsigned int&> DetourMain::fnCombatManagerApplyDamage_;
ThisFunc<bool, void*, unsigned int&, unsigned int&, unsigned int&> DetourMain::fnCombatManagerTakeDamage_;

ThisFunc<int*, void*, unsigned int &> DetourMain::fnCombatAttribAccuExeDamage_;
ThisFunc<void, void*, bool, bool, unsigned int&> DetourMain::fnCtrlPlayerStateDfltRequestMoveAction_;
ThisFunc<void, void*, unsigned int&, unsigned int, unsigned int, unsigned int, bool> DetourMain::fnPlayerPostSpawnPet_;
ThisFunc<void, void*> DetourMain::fnCharCharacterIsDying_;

DetourMain *DetourMain::sDetourMain_ = NULL;
//=============================================================================
//=============================================================================
unsigned __stdcall RunImgui(void* argss)
{
    ImGuiMain::ImGuiStartup();
    return 0;
}

//=============================================================================
//=============================================================================
void DetourMain::Init()
{
    if (!sDetourMain_)
    {
        sDetourMain_ = new DetourMain();
    }

    _beginthreadex(NULL, 0, &RunImgui, NULL, 0, NULL);

    //sleep to init imgui
    Sleep(1000);

    sDetourMain_->SetupDetour();

    //signal to ensure UI is not shown at start
    IPCMessage::SendShortData(DataMsgType::ShowUI, 0);
}

void DetourMain::Close()
{
    if (sDetourMain_)
    {
        delete sDetourMain_;
        sDetourMain_ = NULL;
    }
}

//=============================================================================
//=============================================================================
DetourMain::DetourMain()
{
    Logger::SetLogLevel(LogCombatMgr | LogCharAttack | LogSkillBase);

    hwndWindow_ = NULL;
    playerPtr_ = NULL;
    playerId_ = 0;
    playerSkillMgr_ = NULL;
    playerCharacterBio_ = NULL;
    playerInitialized_ = false;
	ingameRendered_ = false;

    lastActivatedSkill_ = NULL;
    lastActivatedSkillActive_ = false;

    initialized_ = false;
    screenEnabled_ = false;
    childProcessSet_ = false;

    dpsStatStartTime_ = 0;
    dpsStatLastActionTime_ = 0;
    accumulatedDamage_ = 0.0f;
    dpsStatDPS_ = 0.0f;

    hpRecoveryStartTime_ = 0;
    hpRecoveryLastActionTime_ = 0;
    accumulatedHpRecovery_ = 0.0f;
    hpRecoveryPS_ = 0.0f;
      
    pDetourCommon_ = new DetourCommon();
    subDetourClassList_.push_back(pDetourCommon_);
  
    pDetourSkill_ = new DetourSkill();
    subDetourClassList_.push_back(pDetourSkill_);
    pDetourSkill_->SetDetourMain(this);
    pDetourSkill_->SetCommon(pDetourCommon_);

    pDetourOADAStats_ = new DetourOADAStats();
    subDetourClassList_.push_back(pDetourOADAStats_);

    pDetourScreen_ = new DetourScreen();
    subDetourClassList_.push_back(pDetourScreen_);
    pDetourScreen_->SetParent(this);
}

DetourMain::~DetourMain()
{
    for (unsigned i = 0; i < subDetourClassList_.size(); ++i)
    {
        delete subDetourClassList_[i];
    }
    subDetourClassList_.clear();
}

bool DetourMain::SetupDetour()
{
    int status = 0;

    status = HookDetour(datProcessMsgHwndWindow);
    fnProcessMsgHwndWindow_.SetFn(datProcessMsgHwndWindow.realFn_);

    status = HookDetour(datGetSystemWindow);
    fnGetSystemWindow_.SetFn(datGetSystemWindow.realFn_);

    status += HookDetour(datPlayerUpdateSelf);
    fnPlayerUpdateSelf_.SetFn(datPlayerUpdateSelf.realFn_);

    status += HookDetour(datCharacterAttackTarget);
    fnCharacterAttackTarget_.SetFn(datCharacterAttackTarget.realFn_);

    status += HookDetour(datCharBioTakeBonus);
	fnCharacterBioTakeBonus_.SetFn(datCharBioTakeBonus.realFn_);

    status += HookDetour(datCombatMgrApplyDamage);
    fnCombatManagerApplyDamage_.SetFn(datCombatMgrApplyDamage.realFn_);

    status += HookDetour(datCombatMgrTakeAttack);
    fnCombatManagerTakeDamage_.SetFn(datCombatMgrTakeAttack.realFn_);

    status += HookDetour(datPlayerRegisterCombatTextHit);
    fnPlayerRegisterCombatTextHit_.SetFn(datPlayerRegisterCombatTextHit.realFn_);

    status += HookDetour(datPlayerRegisterCombatTextCrit);
    fnPlayerRegisterCombatTextCrit_.SetFn(datPlayerRegisterCombatTextCrit.realFn_);

    status += HookDetour(datCombatAttribAccExeDamage);
    fnCombatAttribAccuExeDamage_.SetFn(datCombatAttribAccExeDamage.realFn_);

    status += HookDetour(datCtrlPlayerRequestMoveAction);
    fnCtrlPlayerStateDfltRequestMoveAction_.SetFn(datCtrlPlayerRequestMoveAction.realFn_);

    status += HookDetour(datPlayerPostSpawnPet);
    fnPlayerPostSpawnPet_.SetFn(datPlayerPostSpawnPet.realFn_);

    status += HookDetour(datCharCharacterIsDying);
    fnCharCharacterIsDying_.SetFn(datCharCharacterIsDying.realFn_);

    if (status != 0)
    {
        SetError("Error in DetourMain::SetupDetour()");
    }

    //init child classes
    InitSubClasses();
  
    return status == 0;
}

void DetourMain::SetSkillActivate(void *skillPtr, bool active)
{
    lastActivatedSkill_ = skillPtr;
    lastActivatedSkillActive_ = active;
}

void DetourMain::SetScreenEnable(bool enable)
{
    screenEnabled_ = enable;
    LOGF("DetourMain::SetScreenEnable: enable=%d\n", enable ? 1 : 0);
    if (!enable)
    {
        playerInitialized_ = false;
		ingameRendered_ = false;
        playerId_ = 0;
        playerCharacterBio_ = NULL;

        SetPlayer(NULL);
        pDetourSkill_->ClearMaps();
        petList_.clear();
        pDetourOADAStats_->SetPlayerInWorld(false);
    }
}

void DetourMain::SetInGameRendered(bool ingame)
{
	ingameRendered_ = ingame;
}

void DetourMain::InitSubClasses()
{
    for (unsigned i = 0; i < subDetourClassList_.size(); ++i)
    {
        subDetourClassList_[i]->SetupDetour();
    }
}

void DetourMain::UpdateSubClasses(void *player, int idx)
{
    for (unsigned i = 0; i < subDetourClassList_.size(); ++i)
    {
        subDetourClassList_[i]->Update(player, idx);
    }
}

void DetourMain::ProcessMsgHwndWindow(void *This, void *canvass)
{
    struct HWND__* hwnd = fnGetSystemWindow_.Fn_(This);

    if (hwndWindow_ = hwnd)
    {
        hwndWindow_ = hwnd;
        IPCMessage::SendShortData(DataMsgType::SetHwnWindow, (unsigned int)hwndWindow_);
    }
}

void DetourMain::SetupPlayer()
{
    if (!playerInitialized_ && playerPtr_)
    {
		LOGF("SetupPlayer - clear main ui\n");
		IPCMessage::SendShortData(DataMsgType::ClearSkillBuffs, 0);

        playerId_ = pDetourCommon_->GetObjectId(playerPtr_);
        playerCharacterBio_ = &pDetourCommon_->CharGetCharacterBio(playerPtr_);

        SetPlayer(playerPtr_);

        pDetourOADAStats_->SetPlayerInWorld(true);
        pDetourSkill_->SetSkillMap();

        playerInitialized_ = true;

        IPCMessage::SendShortData(DataMsgType::ShowUI, 1);
    }
}

void DetourMain::SetPlayer(void* player)
{
	for (unsigned i = 0; i < subDetourClassList_.size(); ++i)
	{
		subDetourClassList_[i]->SetPlayer(player);
	}
}

void DetourMain::Update(void *player, int proc)
{
    //update subs
    UpdateSubClasses(playerPtr_, proc);

    //check update interval
    unsigned int curTime = (unsigned int)timeGetTime();

    if (curTime - startTime_ < PLAYER_UDATE_INTERVAL)
    {
        return;
    }

    // update timer
    startTime_ = curTime;

    if (playerPtr_ != player)
    {
        playerPtr_ = NULL;
    }

    if (playerPtr_ == NULL)
    {
        if (DetourUtil::IsPlayerInWorld(player))
        {
            playerPtr_ = player;

            LOGF("**found player in the world: player=0x%p\n", playerPtr_);
        }

    }
	if (ingameRendered_)
	{
		SetupPlayer();
	}
}

void DetourMain::PreCharAttackTarget(void *charPtr, unsigned int a, unsigned int& entity, unsigned int& paramcombat, bool b, unsigned int c, unsigned int d, unsigned int e)
{
    // arg meaning:
    // charPtr - attacker
    // a - skill id
    // entity - entity being attacked
    // paramcombat - ?unknown
    // b - ?unknown
    // c - direct dmg type
    // d - duration dmg type
    // e - damage multiplier, > 1.0 is crit?
    unsigned int id = pDetourCommon_->GetObjectId(charPtr);
    unsigned int defenderId = pDetourCommon_->GetObjectId(&entity);
    unsigned int petId = 0;

    //check if pet
    for (unsigned int i = 0; i < petList_.size(); ++i)
    {
        if (petList_[i] == id)
        {
            petId = id;
            //set pet ptr in case it's not set yet
            pDetourSkill_->SetPetPtr(petId, charPtr, a);
            break;
        }
    }

    if (playerPtr_ == charPtr || petId == id)
    {
        std::string defenderName = "defender";
        pDetourCommon_->GetObjectName(&entity, defenderName);

        //fill
        memset(&playerAttackInProgress_, 0, sizeof(playerAttackInProgress_));
        playerAttackInProgress_.attackerIsPlayer_ = true;
        playerAttackInProgress_.attackerPetId_ = petId;
        playerAttackInProgress_.attackerId_ = id;
        playerAttackInProgress_.skillId_ = a;
        playerAttackInProgress_.defender_ = defenderId;
        playerAttackInProgress_.totalDamage_ = 0.0f;
        snprintf(playerAttackInProgress_.name_, IPCNAME_SIZE - 1, "%s", defenderName.c_str());
        void *skillPtr = pDetourSkill_->GetSkillPtr(a);
        const char *skname = pDetourCommon_->GetSkillName(skillPtr);

        DLOG(LogCharAttack, "*PreCharAttackTarget %s this=0x%p id=%d, entity=0x%p id=%d, skid=%d(%X)(%s), b=%d c=%d, d=%d, e=%d\n",
            (petId != 0) ? ":**pet" : ":", charPtr, id, &entity, defenderId, a, a, skname, b?1:0, c, d, e);
    }
    else
    {
        const char *skname = "null";
        if (a)
        {
            void *skillPtr = pDetourSkill_->GetSkillPtr(a);
            skname = pDetourCommon_->GetSkillName(skillPtr);
        }

        DLOG(LogCharAttack, "enemy PreCharAttackTarget: skid=%d(%s)\n", a, skname);
    }

}

void DetourMain::PostCharAttackTarget()
{
    if (playerAttackInProgress_.attackerId_ || playerAttackInProgress_.attackerPetId_)
    {
        DLOG(LogCharAttack, "*PostCharAttackTarget\n");
        playerAttackInProgress_.attackerId_ = 0;
        playerAttackInProgress_.attackerPetId_ = 0;
    }
    else
    {
        DLOG(LogCharAttack, "enemy PostCharAttackTarget\n");
    }
}

void DetourMain::PrePlayerOnAttack(void* player, void* entity)
{
    if (!initialized_ || player != playerPtr_)
    {
        return;
    }
    LOGF("   DTPlayerOnAttack entity=%p\n", entity);
}

void DetourMain::CombatgMgrApplyDamage(void *This, float a, unsigned int &playstats, unsigned int cmbtAttrType, unsigned int &vlist)
{
    // arg meaning:
    // This - entity taking damage
    // a - damage amount
    // playstats - ?unknown
    // cmbtAttrType - dmg type
    // vlist - skill list
    void *ownerChar = pDetourCommon_->CombatManagerGetCharacter(This);
    unsigned int ownerId = pDetourCommon_->GetObjectId(ownerChar);
    unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(This);
    unsigned int petId = 0;
    
    for (unsigned i = 0; i < petList_.size(); ++i)
    {
        if (petList_[i] == attackerId)
        {
            petId = attackerId;
            break;
        }
    }

    //combat attr type = 0 means it's a miss, ignore
    if (cmbtAttrType != 0)
    {
        if (attackerId == playerId_ || attackerId == petId)
        {
            std::vector<unsigned int> newlist;
            DetourUtil::VectorMemoryToVector0<unsigned int>((unsigned int)&vlist, newlist);
            unsigned int skid = 0;
            unsigned int parentSkid = 0;
            void *parentSkillPtr = 0;

            for (unsigned n = 0; n < newlist.size(); ++n)
            {
                if (skid == 0)
                {
                    skid = newlist[n];
                    break;
                }
            }
            void* devSkill = pDetourSkill_->GetDevSkillPtr(skid);
            if (devSkill)
            {
                //DLOG(LogCombatMgr, "CombatgMgrApplyDamage: skillId=%d (%s)\n", skid);
                const char* devSkname = pDetourCommon_->GetSkillName(devSkill);

                DLOG(LogCombatMgr, "CombatgMgrApplyDamage: owner=0x%p(%d), skid=%d(%s)\n  attackerId=%d, petId=%d, a=%1.1f, type=%d\n",
                    ownerChar, ownerId, skid, devSkname, attackerId, petId, a, cmbtAttrType);
            }

            if (skid)
            {
                void* skillPtr = pDetourSkill_->GetSkillPtr(skid);

                if (skillPtr)
                {
                    const char* mainSkname = pDetourCommon_->GetSkillName(skillPtr);
                    unsigned int parentSkillId = pDetourSkill_->GetParentSkillId(skillPtr);
                    void* parentPtr = NULL;
                    if (parentSkillId)
                    {
                        parentPtr = pDetourSkill_->GetSkillPtr(parentSkillId);
                        void* devSkillPtr = pDetourSkill_->GetDevSkillPtr(parentSkillId);
                        if (parentPtr && !devSkillPtr)
                        {
                            const char *sname = pDetourCommon_->GetSkillName(parentPtr);
                            DLOG(LogCombatMgr, "   skname (%s), parent skname(%s)\n", mainSkname, sname);
                            if (_strnicmp(sname, "Weapon", 6) != 0)
                            {
                                skid = parentSkillId;
                            }
                        }
                    }
                }
                else
                {
                    skillPtr = pDetourSkill_->GetItemSkillPtr(skid);
                    if (skillPtr)
                    {
                        skid = pDetourCommon_->GetObjectId(skillPtr);
                    }
                    else
                    {
                        LOGF("  skillptr = null\n");
                    }
                }
            }

            if (playerAttackInProgress_.attackerId_ == attackerId || playerAttackInProgress_.attackerPetId_)
            {
                for (int i = 0; i < DETAIL_SIZE; ++i)
                {
                    if (playerAttackInProgress_.detailDmg_[i].type_ == 0 ||
                        playerAttackInProgress_.detailDmg_[i].type_ == cmbtAttrType)
                    {
                        //round off
                        float cf = ceil(a);

                        playerAttackInProgress_.detailDmg_[i].type_ = cmbtAttrType;
                        playerAttackInProgress_.detailDmg_[i].damage_ += cf;
                        playerAttackInProgress_.totalDamage_ += cf;
                        break;
                    }
                }

            }
            else
            {
                AttackerDamageData *attackerDamagePtr = NULL;
                for (unsigned i = 0; i < applyDamageProgress_.size(); ++i)
                {
                    if (applyDamageProgress_[i].skillId_ == skid)
                    {
                        attackerDamagePtr = &applyDamageProgress_[i];
                        break;
                    }
                }
                if (attackerDamagePtr == NULL)
                {
                    AttackerDamageData dat(attackerId, skid, ownerId);
                    dat.attackerIsPlayer_ = true;
                    applyDamageProgress_.push_back(dat);
                    attackerDamagePtr = &applyDamageProgress_.back();
                }
                if (attackerId == petId)
                {
                    attackerDamagePtr->attackerPetId_ = petId;
                }

                for (int i = 0; i < DETAIL_SIZE; ++i)
                {
                    if (attackerDamagePtr->detailDmg_[i].type_ == 0 ||
                        attackerDamagePtr->detailDmg_[i].type_ == cmbtAttrType)
                    {
                        //round off
                        float cf = ceil(a);

                        attackerDamagePtr->detailDmg_[i].type_ = cmbtAttrType;
                        attackerDamagePtr->detailDmg_[i].damage_ += cf;
                        attackerDamagePtr->totalDamage_ += cf;
                        break;
                    }
                }
            }
        }
    }
}

void DetourMain::CombatManagerTakeDamage(void* This, unsigned int& paramCombat, unsigned int& skillMgr, unsigned int& charBio)
{
    void *ownerChar = pDetourCommon_->CombatManagerGetCharacter(This);
    unsigned int ownerId = pDetourCommon_->GetObjectId(ownerChar);
    unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(This);

    DLOG(LogCombatMgr, "CombatManagerTakeDamage: owner=0x%p(%d), attackerId=%d\n",
        ownerChar, ownerId, attackerId);
}


void DetourMain::TakeBonus(void* charBio, unsigned int& bonusRef, bool a)
{
    if (charBio != playerCharacterBio_)
    {
        return;
    }

    float hp = pDetourCommon_->CharBioGetBonusLifeAmount(charBio, bonusRef);

    if (hp > 1.0f)
    {
        unsigned int* ubonus = (unsigned int*)&bonusRef;
        unsigned int typeId = ubonus[2];
        //index 2 seems to have specific fixed values:
        //hp pot            41C80000
        //dryad             40C00000
        //giant's blood     41900000
        //healing rain      41200000
        //ghoulish hunger   00000000 (life leech)
        //tip the scales    00000000 (life leech)
        //maul              00000000 (life leech)
        //living shadow     00000000 (life leech)
        //weindigo's mark   00000000 (life leech)
        const char *typeName = NULL;
        switch (typeId)
        {
        case 0x0:
            typeName = "Life leech";
            break;
        case 0x41C80000:
            typeName = "HP pot";
            break;
        default:
            //activate skillactive fn is called immediately before this fn is called
            //and that activated skill causes bonus health to be added
            if (lastActivatedSkillActive_ && lastActivatedSkill_)
            {
                typeName = pDetourCommon_->GetSkillName(lastActivatedSkill_);
            }
            break;
        }

        // hp recovery/s calc
        unsigned int curTime = (unsigned int)pDetourCommon_->GetGameTime();
        unsigned int elapsedTime = curTime - hpRecoveryLastActionTime_;

        // refresh if no activity after certain time
        if (curTime - hpRecoveryLastActionTime_ > NOACTIVITY_REFRESH_TIMER)
        {
            accumulatedHpRecovery_ = 0.0f;
            hpRecoveryLastActionTime_ = curTime;
            hpRecoveryStartTime_ = curTime;
            hpRecoveryPS_ = 0.0f;

            elapsedTime = 0;
        }
        else
        {
            hpRecoveryLastActionTime_ = curTime;
        }

        // update dps
        float elapsed = (float)elapsedTime / 1000.0f;
        accumulatedHpRecovery_ += hp;

        if (elapsed < 1.0f)
        {
            hpRecoveryPS_ = accumulatedHpRecovery_;
        }
        else
        {
            hpRecoveryPS_ = accumulatedHpRecovery_ / elapsed;
        }


        IPCData ipc;
        ipc.msg_ = HPRecovery;
        ipc.u_.hpRecovery_.typeId_ = typeId;
        ipc.u_.hpRecovery_.hp_ = hp;
        ipc.u_.hpRecovery_.hpPerSec_ = hpRecoveryPS_;

        if (typeName)
        {
            std::string strName = typeName;
            ipc.u_.hpRecovery_.typeId_ = std::hash<std::string>{}(strName);
            snprintf(ipc.u_.hpRecovery_.name_, IPCNAME_SIZE - 1, "%s", strName.c_str());
        }
        else
        {
            strcpy_s(ipc.u_.hpRecovery_.name_, "Unknown");
        }
        IPCMessage::SendData(ipc);
    }
}

void DetourMain::CalculateDPS(float damage)
{
    unsigned int curTime = (unsigned int)pDetourCommon_->GetGameTime();
    unsigned int elapsedTime = curTime - dpsStatStartTime_;

    // refresh if no activity after certain time
    if (curTime - dpsStatLastActionTime_ > NOACTIVITY_REFRESH_TIMER)
    {
        accumulatedDamage_ = 0.0f;
        dpsStatStartTime_ = curTime;
        dpsStatLastActionTime_ = curTime;

        dpsStatDPS_ = 0.0f;
        elapsedTime = 0;
    }
    else
    {
        dpsStatLastActionTime_ = curTime;
    }
  
    // update dps
    float elapsed = (float)elapsedTime / 1000.0f;
    accumulatedDamage_ += damage;

    if (elapsed < 1.0f)
    {
        dpsStatDPS_ = accumulatedDamage_;
    }
    else
    {
        dpsStatDPS_ = accumulatedDamage_ / elapsed;
    }

    IPCData ipc;
    ipc.msg_ = DPSstats;
    ipc.u_.dpsStats_.dps_ = dpsStatDPS_;
    ipc.u_.dpsStats_.damageDealt_ = damage;
    ipc.u_.dpsStats_.duration_ = elapsedTime / 1000;
    ipc.u_.dpsStats_.types_ = ipc.u_.dpsStats_.typeDps | ipc.u_.dpsStats_.typeDamageDealt;

    IPCMessage::SendData(ipc);
}

bool DetourMain::GetAttackDataSkillName(const AttackerDamageData &attackDamage, IPCData &ipcskill)
{
    void* skillPtr = pDetourSkill_->GetSkillPtr(attackDamage.skillId_);
    void* devSkillPtr = pDetourSkill_->GetDevSkillPtr(attackDamage.skillId_);
    void *petPtr = pDetourSkill_->PetHasSkillId(attackDamage.skillId_);
    void* activeSkillPtr = pDetourSkill_->GetSkillActivated(attackDamage.skillId_);
    void* altSkillPtr = pDetourSkill_->GetAlternateSkillPtr(attackDamage.skillId_);
    
    std::string skillName;
    void* tempSkillPtr = pDetourSkill_->GetTempSkillPtr(attackDamage.skillId_, skillName);
    
    if (attackDamage.attackerPetId_ && !petPtr)
    {
        petPtr = pDetourSkill_->GetPetPtr(attackDamage.attackerPetId_);
    }

    if (devSkillPtr)
    {
        std::string name = pDetourCommon_->GetSkillName(devSkillPtr);
        ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(name);
        snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", name.c_str());
        return true;
    }

    if (petPtr)
    {
        std::string petName;
        pDetourCommon_->GetObjectName(petPtr, petName);
        std::transform(petName.begin(), petName.begin()+1, petName.begin(), ::toupper);
        ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(petName);
        snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "*%s", petName.c_str());
        return true;
    }

    if (activeSkillPtr)
    {
        std::string name = pDetourCommon_->GetSkillName(activeSkillPtr);
        ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(name);
        snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", name.c_str());
        return true;
    }

    if (skillPtr)
    {
        std::string name = pDetourCommon_->GetSkillName(skillPtr);
        ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(name);
        snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", name.c_str());
        return true;
    }

    if (tempSkillPtr)
    {
        ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(skillName);
        snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", skillName.c_str());
        return true;
    }

    if (altSkillPtr)
    {
        std::string name = pDetourCommon_->GetSkillName(altSkillPtr);
        ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(name);
        snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", name.c_str());
        return true;
    }

    return false;
}

void DetourMain::BuildAttackDamageData(const AttackerDamageData &attackDamage, float registeredDamage, std::vector<IPCData> &msgList)
{
    if (attackDamage.attackerIsPlayer_)
    {
        //attack damage
        IPCData ipcdmg;
        ipcdmg.msg_ = AttackDamage;
        memcpy(&ipcdmg.u_.attackDamage_.details_, &attackDamage.detailDmg_, sizeof(attackDamage.detailDmg_));
        strcpy_s(ipcdmg.u_.attackDamage_.name_, attackDamage.name_);
        ipcdmg.u_.attackDamage_.playerDealingDamage_ = true;

        msgList.push_back(ipcdmg);

        //skill damage
        IPCData ipcskill;
        ipcskill.msg_ = PlayerSkillDamage;
        ipcskill.u_.skillDamage_.isCrit_ = attackDamage.isCrit_;
        ipcskill.u_.skillDamage_.damage_ = registeredDamage;
        ipcskill.u_.skillDamage_.skillId_ = attackDamage.skillId_;

        if (!GetAttackDataSkillName(attackDamage, ipcskill))
        {
            DLOG(LogUnknownSkill, "*****SendAttackDamageData: skill id=%d NOT found\n", attackDamage.skillId_);

            //check if realiation dmg
            if (attackDamage.skillId_ == 0)
            {
                std::string strName = "*Retaliation damage";
                ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(strName);
                snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", strName.c_str());
            }
            else
            {
#ifdef CONSOLIDATE_UNIDENTIFIED_SKILLS
                std::string strName = "unidentified skillname";
                ipcskill.u_.skillDamage_.skillId_ = std::hash<std::string>{}(strName);
                snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "%s", strName.c_str());
#else
                snprintf(ipcskill.u_.skillDamage_.name_, IPCNAME_SIZE - 1, "id: %d", attackDamage.skillId_);
#endif
            }
            DetourUtil::GetBackTrace();
       }

        msgList.push_back(ipcskill);
    }
}

void DetourMain::PlayerRegisterCombatTextHit(void* This, float a, float f)
{
    CalculateDPS(a);

    DLOG(LogDamageAccum, "^^RegisterCombatTextHit\n");

    //damage derived from player's direct attack or it's a duration dmg
    std::vector<IPCData> msgList;
    if (playerAttackInProgress_.attackerId_ == playerId_ || playerAttackInProgress_.attackerPetId_)
    {
        playerAttackInProgress_.isCrit_ = false;
        BuildAttackDamageData(playerAttackInProgress_, playerAttackInProgress_.totalDamage_, msgList);
        IPCMessage::SendData(msgList);

        playerAttackInProgress_.ClearDetails();
    }
    else
    {
        if (applyDamageProgress_.size())
        {
            for (unsigned i = 0; i < applyDamageProgress_.size(); ++i)
            {
                applyDamageProgress_[i].isCrit_ = false;
                BuildAttackDamageData(applyDamageProgress_[i], applyDamageProgress_[i].totalDamage_, msgList);
            }

            applyDamageProgress_.clear();
            IPCMessage::SendData(msgList);
        }
    }
}

void DetourMain::PlayerRegisterCombatTextCrit(void* This, float a, float f)
{
    CalculateDPS(a);
    DLOG(LogDamageAccum, "^^RegisterCombatTextCrit\n");

    //damage derived from player's direct attack or it's a duration dmg
    std::vector<IPCData> msgList;
    if (playerAttackInProgress_.attackerId_ == playerId_ || playerAttackInProgress_.attackerPetId_)
    {
        playerAttackInProgress_.isCrit_ = true;
        BuildAttackDamageData(playerAttackInProgress_, playerAttackInProgress_.totalDamage_, msgList);
        IPCMessage::SendData(msgList);

        playerAttackInProgress_.ClearDetails();
    }
    else
    {
        if (applyDamageProgress_.size())
        {
            for (unsigned i = 0; i < applyDamageProgress_.size(); ++i)
            {
                applyDamageProgress_[i].isCrit_ = true;
                BuildAttackDamageData(applyDamageProgress_[i], applyDamageProgress_[i].totalDamage_, msgList);
            }

            applyDamageProgress_.clear();
            IPCMessage::SendData(msgList);
        }
    }
}

void DetourMain::GameEngineRegisterDamage(void* This, unsigned int a, unsigned int b, float c)
{
    //this fn is a specific damage type registry
    //this fn is called by DTCombatgMgrApplyDamage, where the attacker id, skill id, and combatAttribTypes are known
    //however, this function gets called multiple times then some manager tallies all damages called into this fn 
    // a - entity id being attacked
    // b - attacker id
    // c - damage - specific damage type not a total damage
    //LOGF("    DTGameEngineRegisterDamage: defenderId=%d, attackerId=%d, c=%4.1f\n", a, b, c);
}

void DetourMain::CombatAttribAccuExeDamage(void *This, unsigned int& charRef)
{
    //unsigned int ownerId = pDetourCommon_->GetObjectId(ownerChar);
    void *charCmbatMgr = pDetourCommon_->CharacterGetCombatManager(&charRef);
    unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(charCmbatMgr);

    if (playerPtr_ == &charRef || attackerId == playerId_)
    {
        //LOGF("  DTCombatAttribAccuExeDamage: char=%p, attackerId=%d, playerid=%d\n", &charRef, attackerId, playerId_);
    }
}

void DetourMain::CtrlPlayerStateDfltRequestMoveAction(void* This, bool a, bool b, unsigned int& v3Ref)
{
}

void DetourMain::PlayerPostSpawnPet(void* This, unsigned int& ve3Ref, unsigned int a, unsigned int b, unsigned int c, bool d)
{
    LOGF("PlayerPostSpawnPet: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d ? 1 : 0);
    //arg meaning:
    //a - petId
    //b - ?unknown
    //c - ?unknown
    //d - ?unknown
    pDetourSkill_->AddPetSpawned(a);
    petList_.push_back(a);
}

void DetourMain::CharCharacterIsDying(void* This)
{
    //notify
    if (This == playerPtr_)
    {
        LOGF("CharCharacterIsDying: this=0x%p\n", This);
        pDetourSkill_->NotifyPlayerIsDying();
        petList_.clear();
    }
    else
    {
        //remove pets (only if found)
        pDetourSkill_->RemovePetSpawned(This);

        unsigned int entityId = pDetourCommon_->GetObjectId(This);
        for (std::vector<unsigned int>::iterator itr = petList_.begin(); itr != petList_.end(); ++itr)
        {
            if (*itr == entityId)
            {
                petList_.erase(itr);
                break;
            }
        }
    }
}

//==============================================================
// static fns
//==============================================================
void DetourMain::DTProcessMsgHwndWindow(void* This, void*, void *canvass)
{
    DBGLOG("DTProcessMsgHwndWindow\n");
    sDetourMain_->ProcessMsgHwndWindow(This, canvass);

    fnProcessMsgHwndWindow_.Fn_(This, canvass);
    DBGLOG("end DTProcessMsgHwndWindow\n");
}

void DetourMain::DTPlayerUpdateSelf(void* This, void*, int proc)
{
    DBGLOG("DTPlayerUpdateSelf\n");
    sDetourMain_->Update(This, proc);

    fnPlayerUpdateSelf_.Fn_(This, proc);
    DBGLOG("end DTPlayerUpdateSelf\n");
}

void DetourMain::DTPlayerOnAttack(void *This, void *, void *entity)
{
    DBGLOG("PlayerOnAttack\n");

    sDetourMain_->PrePlayerOnAttack(This, entity);

    fnPlayerOnAttack_.Fn_(This, entity);
    DBGLOG("end PlayerOnAttack\n");
}

void DetourMain::DTCharacterBioTakeBonus(void* This, void*, unsigned int &bonusRef, bool a)
{
    DBGLOG("CharacterBioTakeBonus\n");

    sDetourMain_->TakeBonus(This, bonusRef, a);

	fnCharacterBioTakeBonus_.Fn_(This, bonusRef, a);
    DBGLOG("end CharacterBioTakeBonus\n");
}

bool DetourMain::DTCharacterAttackTarget(void *This, void*, unsigned int a, unsigned int& entity, unsigned int& paramcombat, bool b, unsigned int c, unsigned int d, unsigned int e)
{
    sDetourMain_->PreCharAttackTarget(This, a, entity, paramcombat, b, c, d, e);

    bool r = fnCharacterAttackTarget_.Fn_(This, a, entity, paramcombat, b, c, d, e);

    sDetourMain_->PostCharAttackTarget();

    return r;
}

bool DetourMain::DTCombatgMgrApplyDamage(void *This, void*, float a, unsigned int &playstats, unsigned int cmbtAttrType, unsigned int &vlist)
{
    DBGLOG("DTCombatgMgrApplyDamage\n");

    sDetourMain_->CombatgMgrApplyDamage(This, a, playstats, cmbtAttrType, vlist);

    bool b = fnCombatManagerApplyDamage_.Fn_(This, a, playstats, cmbtAttrType, vlist);
    DBGLOG("end DTCombatgMgrApplyDamage\n");

    return b;
}

bool DetourMain::DTCombatManagerTakeAttack(void* This, void*, unsigned int& paramCombat, unsigned int& skillMgr, unsigned int& charBio)
{
    DBGLOG("DTCombatManagerTakeAttack\n");
    sDetourMain_->CombatManagerTakeDamage(This, paramCombat, skillMgr, charBio);
    bool val = fnCombatManagerTakeDamage_.Fn_(This, paramCombat, skillMgr, charBio);
    DBGLOG("end DTCombatManagerTakeAttack\n");

    return val;
}

float DetourMain::DTPlayerRegisterCombatTextHit(void* This, void*, float a)
{
    DBGLOG("DTPlayerRegisterCombatTextHit\n");
    float f = fnPlayerRegisterCombatTextHit_.Fn_(This, a);
    sDetourMain_->PlayerRegisterCombatTextHit(This, a, f);
    DBGLOG("end DTPlayerRegisterCombatTextHit\n");

    return f;
}

float DetourMain::DTPlayerRegisterCombatTextCrit(void* This, void*, float a)
{
    DBGLOG("DTPlayerRegisterCombatTextCrit\n");
    float f = fnPlayerRegisterCombatTextCrit_.Fn_(This, a);
    sDetourMain_->PlayerRegisterCombatTextCrit(This, a, f);
    DBGLOG("end DTPlayerRegisterCombatTextCrit\n");

    return f;
}

void DetourMain::DTGameEngineRegisterDamage(void* This, void*, unsigned int a, unsigned int b, float c)
{
    DBGLOG("GameEngineRegisterDamage\n");

    sDetourMain_->GameEngineRegisterDamage(This, a, b, c);
    fnGameEngineRegisterDamage_.Fn_(This, a, b, c);
    DBGLOG("end GameEngineRegisterDamage\n");
}

void DetourMain::DTCombatAttribAccuExeDamage(void *This, void*, unsigned int& charRef)
{
    DBGLOG("CombatAttribAccuExeDamage\n");

    sDetourMain_->CombatAttribAccuExeDamage(This, charRef);
    fnCombatAttribAccuExeDamage_.Fn_(This, charRef);
    DBGLOG("end CombatAttribAccuExeDamage\n");
}

void DetourMain::DTCtrlPlayerStateDfltRequestMoveAction(void* This, void*, bool a, bool b, unsigned int& v3Ref)
{
    DBGLOG("CtrlPlayerStateDfltRequestMoveAction\n");
    sDetourMain_->CtrlPlayerStateDfltRequestMoveAction(This, a, b, v3Ref);

    fnCtrlPlayerStateDfltRequestMoveAction_.Fn_(This, a, b, v3Ref);
    DBGLOG("end CtrlPlayerStateDfltRequestMoveAction\n");
}

void DetourMain::DTPlayerPostSpawnPet(void* This, void*, unsigned int& v3Ref, unsigned int a, unsigned int b, unsigned int c, bool d)
{
    DBGLOG("DTPlayerPostSpawnPet:\n");
    sDetourMain_->PlayerPostSpawnPet(This, v3Ref, a, b, c, d);

    fnPlayerPostSpawnPet_.Fn_(This, v3Ref, a, b, c, d);
    DBGLOG("end DTPlayerPostSpawnPet:\n");
}

void DetourMain::DTCharCharacterIsDying(void* This)
{
    sDetourMain_->CharCharacterIsDying(This);
    fnCharCharacterIsDying_.Fn_(This);
}


