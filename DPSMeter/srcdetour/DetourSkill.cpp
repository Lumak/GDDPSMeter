#include <windows.h>
#include <cassert>

#include "DetourSkill.h"
#include "DetourMain.h"
#include "DetourCommon.h"
#include "DetourUtil.h"
#include "IPCMessage.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
#define SKILL_UPDATE_INTERVAL 100

//=============================================================================
//=============================================================================
DetourFnData datCharacterDispelSkillBuffs = { "game.dll", NULL, (VoidFn)&DetourSkill::DTCharacterDispelSkillBuffs,SYM_CHARACTER_DISPELSKILLBUFFS  };
DetourFnData datSkillBuffInstall = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillBuffInstall, SYM_SKILLBUFF_INSTALL };
DetourFnData datSkillBuffUnInstall = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillBuffUnInstall, SYM_SKILLBUFF_UNINSTALL };
DetourFnData datSkillBuffSelfDurationActivateNow = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillBuffSelfDurationActivateNow,  SYM_SKILLBUFFSELFDURATION_ACTIVATENOW};
DetourFnData datSkillBuffSelfDurRemoveSelf = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillBuffSelfDurationRemoveSelfBuff, SYM_SKILLBUFFSELFDURATION_REMOVESELFBUFF };
DetourFnData datSkillBuffSelfGetParentsSkillIdD = { "game.dll", NULL, NULL, SYM_SKILLBUFF_GETPARENT };
DetourFnData datSkillActivateSecondarySkill = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillActivateSecondarySkill, SYM_SKILL_ACTIVATESECONDARYSKILLS };
DetourFnData datSkillPrimaryStopSecondarySkill = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillPrimaryStopSecondarySkills,SYM_SKILL_PRIMARYSTOPSECONDARYSKILLS  };
DetourFnData datSkillGetSecondarySkills = { "game.dll", NULL, NULL, SYM_SKILL_GETSECONDARYSKILL };
DetourFnData datSkillGetBaseSkills = { "game.dll", NULL, NULL, SYM_SKILL_GETBASESKILLS };
DetourFnData datSkillGetManager = { "game.dll", NULL, NULL, SYM_SKILL_GETMANAGER };
DetourFnData datSkillMangerGetParent = { "game.dll", NULL, NULL, SYM_SKILLMGR_GETPARENT };

DetourFnData datSkillServCharSendSkillActiveUpdate = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillSvcsCharSendSkillActiveUpdate,SYM_SKILLSERVCHAR_SENDSKILLACTIVEUPDATE };
DetourFnData datSkillBuffDebufUpdate = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillBuffDebufUpdate, SYM_SKILLBUFFDEBUF_UPDATE};
DetourFnData datSkillBuffSetTimeToLive = { "game.dll", NULL, (VoidFn)&DetourSkill::DTSkillBuffSetTimeToLive, SYM_SKILLBUFF_SETTIMETOLIVE };

ThisFunc<void, void*, void*> DetourSkill::fnSkillBuffUnInstall_;
ThisFunc<void, void*, unsigned int&, unsigned int&, unsigned int, unsigned int&> DetourSkill::fnSkillBuffSelfDurationActivateNow_;
ThisFunc<void, void*> DetourSkill::fnCharacterDispelSkillBuffs_;
ThisFunc<void, void*> DetourSkill::fnSkillBuffSelfDurationRemoveSelfBuff_;
ThisFunc<unsigned int, void*> DetourSkill::fnSkillBuffGetParentSkillId_;
ThisFunc<void, void*, void*> DetourSkill::fnSkillBuffInstall_;

ThisFunc<void, void*, unsigned int&, unsigned int&, unsigned int, unsigned int&, unsigned int&> DetourSkill::fnSkillActivateSecondarySkill_;
ThisFunc<void, void *, unsigned int&> DetourSkill::fnSkillPrimaryStopSecondarySkills_;

ThisFunc<void, void*, void*, unsigned int&> DetourSkill::fnSkillServicesCharacterSendSkillActiveUpdate_;
ThisFunc<void, void*, unsigned int&, int> DetourSkill::fnSkillBuffDebufUpdate_;
ThisFunc<void, void*, int> DetourSkill::fnSkillBuffSetTimeToLive_;


DetourSkill *DetourSkill::sDetourSkill_ = NULL;
//=============================================================================
//=============================================================================
DetourSkill::DetourSkill()
{
    sDetourSkill_ = this;

    playerPtr_ = NULL;
    initialized_ = false;
    startTime_ = 0;
}

bool DetourSkill::SetupDetour()
{
    int status = 0;

    status = HookDetour(datSkillBuffInstall);
    fnSkillBuffInstall_.SetFn(datSkillBuffInstall.realFn_);

    status += HookDetour(datSkillBuffUnInstall);
    fnSkillBuffUnInstall_.SetFn(datSkillBuffUnInstall.realFn_);

    status += HookDetour(datSkillBuffSelfDurationActivateNow);
    fnSkillBuffSelfDurationActivateNow_.SetFn(datSkillBuffSelfDurationActivateNow.realFn_);

    status += HookDetour(datCharacterDispelSkillBuffs);
    fnCharacterDispelSkillBuffs_.SetFn(datCharacterDispelSkillBuffs.realFn_);

    status += HookDetour(datSkillBuffSelfDurRemoveSelf);
    fnSkillBuffSelfDurationRemoveSelfBuff_.SetFn(datSkillBuffSelfDurRemoveSelf.realFn_);

    status += HookDetour(datSkillBuffSelfGetParentsSkillIdD);
    fnSkillBuffGetParentSkillId_.SetFn(datSkillBuffSelfGetParentsSkillIdD.realFn_);

    status += HookDetour(datSkillActivateSecondarySkill);
    fnSkillActivateSecondarySkill_.SetFn(datSkillActivateSecondarySkill.realFn_);

    status += HookDetour(datSkillServCharSendSkillActiveUpdate);
    fnSkillServicesCharacterSendSkillActiveUpdate_.SetFn(datSkillServCharSendSkillActiveUpdate.realFn_);

    status += HookDetour(datSkillPrimaryStopSecondarySkill);
    fnSkillPrimaryStopSecondarySkills_.SetFn(datSkillPrimaryStopSecondarySkill.realFn_);

    status += HookDetour(datSkillGetSecondarySkills);
    fnSkillGetSecondarySkills_.SetFn(datSkillGetSecondarySkills.realFn_);

    status += HookDetour(datSkillGetBaseSkills);
    fnSkillGetBaseSkills_.SetFn(datSkillGetBaseSkills.realFn_);

    status += HookDetour(datSkillGetManager);
    fnSkillGetManager_.SetFn(datSkillGetManager.realFn_);

    status += HookDetour(datSkillMangerGetParent);
    fnSkillManagerGetParent_.SetFn(datSkillMangerGetParent.realFn_);

    status += HookDetour(datSkillBuffDebufUpdate);
    fnSkillBuffDebufUpdate_.SetFn(datSkillBuffDebufUpdate.realFn_);

    status += HookDetour(datSkillBuffSetTimeToLive);
    fnSkillBuffSetTimeToLive_.SetFn(datSkillBuffSetTimeToLive.realFn_);

    if (status != 0)
    {
        SetError("Error in DetourSkill::SetupDetour()");
    }

    return status == 0;
}

void DetourSkill::SendSkillBuffMessage(void* skillPtr, int timeRemaining, bool isActiveBuff, bool remove)
{
    IPCData ipc;

    ipc.msg_ = isActiveBuff ? ActiveSkillBuff : SkillBuffCooldown;

    if (remove)
    {
        ipc.msg_ = isActiveBuff ? ActiveSkillBuffRemove : SkillBuffCooldownRemove;
    }
 
    unsigned int id = pDetourCommon_->GetObjectId(skillPtr);
    snprintf(ipc.u_.skillBuff_.name_, IPCNAME_SIZE - 1, "%s", pDetourCommon_->GetSkillName(skillPtr));
    ipc.u_.skillBuff_.skillId_ = id;
    ipc.u_.skillBuff_.timeRemaining_ = timeRemaining;
  
    IPCMessage::SendData(ipc);
}

void DetourSkill::Update(void *player, int idx)
{
    int curTime = (int)timeGetTime();

    //update
    if (player == NULL || curTime - startTime_ < SKILL_UPDATE_INTERVAL)
    {
        return;
    }

    startTime_ = curTime;

    //skill self buffs
    if (skillSelfBuffMap_.size() > 0)
    {
        for (std::map<unsigned int, skillBuffData>::iterator itr = skillSelfBuffMap_.begin();
            itr != skillSelfBuffMap_.end(); )
        {
            unsigned int skillId = itr->first;
            skillBuffData &data = itr->second;

            //skillbuffself timer offset = 226 (per uint alignment, 4 bytes ea.)
            unsigned int *uptr = (unsigned int*)data.skillptr_;
            int durtime = (int)uptr[226];

            if (durtime > 0)
            {
                int cdr = pDetourCommon_->SkillGetCooldownRemaining(data.skillptr_);
                data.CDRemaining_ = durtime;
                SendSkillBuffMessage(data.skillptr_, data.CDRemaining_, true, false);
                ++itr;
            }
            else
            {
                SendSkillBuffMessage(data.skillptr_, 0, true, true);

                if (data.parentSkill_)
                {
                    if (IsValidSkillToMonitor(data.parentSkill_))
                    {
                        int cdr = pDetourCommon_->SkillGetCooldownRemaining(data.parentSkill_);
                        std::map<unsigned int, skillBuffData>::iterator findParent = skillBuffCDRemainingMap_.find(data.parentId_);
                        if (cdr > 0 && findParent != skillBuffCDRemainingMap_.end())
                        {
                            skillBuffData parentDat;
                            parentDat.skillptr_ = data.parentSkill_;
                            parentDat.parentId_ = data.parentId_;
                            parentDat.isDevotionSkill_ = data.isDevotionSkill_;
                            skillBuffCDRemainingMap_[parentDat.parentId_] = parentDat;
                        }
                    }
                }
                itr = skillSelfBuffMap_.erase(itr);
            }
        }
    }

    //dbg skill buffs
    if (skillBuffMap_.size() > 0)
    {
        for (std::map<unsigned int, skillBuffData>::iterator itr = skillBuffMap_.begin(); 
            itr != skillBuffMap_.end(); )
        {
            //skillbuff timer offset = 219 (per uint alignment, 4 bytes ea.)
            unsigned int skillId = itr->first;
            skillBuffData &data = itr->second;
            unsigned int *uptr = (unsigned int*)data.skillptr_;
            int durtime = (int)uptr[219];
            int cdr = pDetourCommon_->SkillGetCooldownRemaining(data.skillptr_);

            if (durtime > 0)
            {
                data.CDRemaining_ = durtime;
                SendSkillBuffMessage(data.skillptr_, data.CDRemaining_, true, false);
                ++itr;
            }
            else
            {
                SendSkillBuffMessage(data.skillptr_, 0, true, true);

                if (data.parentSkill_)
                {
                    if (IsValidSkillToMonitor(data.parentSkill_))
                    {
                        int cdr = pDetourCommon_->SkillGetCooldownRemaining(data.parentSkill_);
                        std::map<unsigned int, skillBuffData>::iterator findParent = skillBuffCDRemainingMap_.find(data.parentId_);

                        if (cdr > 0 && findParent == skillBuffCDRemainingMap_.end())
                        {
                            skillBuffData parentDat;
                            parentDat.skillptr_ = data.parentSkill_;
                            parentDat.parentId_ = data.parentId_;
                            parentDat.isDevotionSkill_ = data.isDevotionSkill_;
                            skillBuffCDRemainingMap_[parentDat.parentId_] = parentDat;
                        }
                    }
                }
                itr = skillBuffMap_.erase(itr);
            }
        }
    }


    //skill cd
    if (skillBuffCDRemainingMap_.size())
    {
        for (std::map<unsigned int, skillBuffData>::iterator itr = skillBuffCDRemainingMap_.begin(); 
            itr != skillBuffCDRemainingMap_.end(); )
        {
            //skillbuff timer offset = 219 (per uint alignment, 4 bytes ea.)
            unsigned int skillId = itr->first;
            skillBuffData &data = itr->second;
            int cdr = pDetourCommon_->SkillGetCooldownRemaining(data.skillptr_);
            if (cdr > 0)
            {
                SendSkillBuffMessage(data.skillptr_, cdr, false, false);
                ++itr;
            }
            else
            {
                SendSkillBuffMessage(data.skillptr_, 0, false, true);

                itr = skillBuffCDRemainingMap_.erase(itr);
            }
        }
    }
}

void DetourSkill::ClearMaps()
{
    skillMap_.clear();
    devSkillMap_.clear();
    consumableSkillList_.clear();
    skillActivatedMap_.clear();
    skillBuffMap_.clear();
    skillSelfBuffMap_.clear();
    itemSkillMap_.clear();
    tempSkillBuffMap_.clear();
    petSkillMap_.clear();
}

void DetourSkill::SetPlayer(void* player)
{
    playerPtr_ = player;
    if (playerPtr_)
    {
        playerId_ = pDetourCommon_->GetObjectId(playerPtr_);
    }
}

void DetourSkill::SetSkillMap()
{
    if (skillMap_.size() == 0)
    {
        DLOG(LogSkillBase, "SetSkillMap:\n");

		consumableSkillList_.clear();

        std::vector<unsigned int*> &vskillList = pDetourCommon_->CharGetSkillList(playerPtr_);
        //std::vector<unsigned int*> &vskillList = *(std::vector<unsigned int*>*)&skillList;
        LOGF("SetSkillMap() vec size=%u", vskillList.size());

        for (unsigned i = 0; i < vskillList.size(); ++i)
        {
            unsigned int* uskptr = vskillList[i];
            std::string recStr = (const char*)uskptr[1];

            if (recStr.find("skills") != std::string::npos)
            {
                unsigned int uskillId = pDetourCommon_->GetObjectId(uskptr);

                //not storing consumable skills
                if (recStr.find("consumable") == std::string::npos)
                {
                    skillMap_[uskillId] = uskptr;

                    if (recStr.find("devotion") != std::string::npos)
                    {
                        devSkillMap_[uskillId] = uskptr;
                    }
                }
                else
                {
                    consumableSkillList_.push_back(uskillId);
                }

                //DLOG(LogSkillBase, "[%04u]id=%u, %s\n", i, uskillId, recStr.c_str());
            }
        }
#if 0
        unsigned int *skillArray = (unsigned int*)skillList;
        unsigned int skillId = 0;
        DLOG(LogSkillBase,"  skillList=0x%X\n", skillList);

        // max size is unknown buf it should be around 920 for now
        for (unsigned int i = 0; i < 1000; ++i)
        {
 		    bool memvalid = DetourUtil::MemValidity((void*)skillArray[i]);
			if (!memvalid)
            {
                DLOG(LogSkillBase,"  reached invalid ptr\n");
                break;
            }

            unsigned int *skptr = (unsigned int*)skillArray[i];

            if (skptr[1] == 0)
            {
                continue;
            }

            const char *recordPtr = (const char*)skptr[1];
			std::string recStr = recordPtr;

            //skptr[1] points to a skill record filename
            if (recStr.find("skills") != std::string::npos)
            {
                skillId = pDetourCommon_->GetObjectId(skptr);
                //DLOG(LogSkillBase, "  %s\n", recStr.c_str());

                //not storing consumable skills in the skillmap
                if (recStr.find("consumable") == std::string::npos)
                {
                    skillMap_[skillId] = skptr;

                    if (recStr.find("devotion") != std::string::npos)
                    {
                        devSkillMap_[skillId] = skptr;
                    }
                }
                else
                {
                    consumableSkillList_.push_back(skillId);
                }
            }
            else
            {
                DLOG(LogSkillBase,"  end of skills\n");
                break;
            }
        }
#endif
        DLOG(LogSkillBase,"  total %d\n", skillMap_.size());

        //get item skills
        SetItemSkillMap();
    }
}

void DetourSkill::SetItemSkillMap()
{
    if (itemSkillMap_.size() == 0)
    {
        DLOG(LogSkillBase,"SetItemSkillMap:\n");

        void *skillMgr = &pDetourCommon_->CharGetSkillMgr(playerPtr_);
        if (!skillMgr)
        {
            DLOG(LogSkillBase,"  skillMgr = null\n");
            return;
        }

        std::vector<unsigned int*> &skillList = pDetourCommon_->SkillMgrGetItemSkillList(skillMgr);
        unsigned int skillId = 0;
        LOGF("  item skill list size=%u", skillList.size());

        if (skillList.size() == 0)
        {
            DLOG(LogSkillBase,"  skillList=%d, total = 0\n", skillList);
            return;
        }

        // max size is unknown but should be a few dozen if that
        for (unsigned int i = 0; i < skillList.size(); ++i)
        {
            //bool memvalid = DetourUtil::MemValidity((void*)skillArray[i]);
            
			//if (!memvalid)
            //{
            //    DLOG(LogSkillBase,"  [%i]item skill bad mem\n", i);
            //    break;
            //}
            //unsigned int* uskptr = skillList[i];
            //std::string recStr = (const char*)uskptr[1];


            unsigned int *skptr = (unsigned int*)skillList[i];
            //memvalid = DetourUtil::MemValidity((void*)skptr[1]);
            //if (skptr[1] == 0 || !memvalid)
            //{
             //   DLOG(LogSkillBase,"  .[%i]item skill bad mem\n", i);
             //   break;
            //}
            const char *recordPtr = (const char*)skptr[1];
            std::string recStr = recordPtr;

            //skptr[1] points to a skill record filename
            if (recStr.find("skills") != std::string::npos)
            {
                skillId = pDetourCommon_->GetObjectId(skptr);
                itemSkillMap_[skillId] = skptr;
            }
            else
            {
                DLOG(LogSkillBase,"  end skills\n");
                break;
            }
        }
        DLOG(LogSkillBase,"  total=%d\n", itemSkillMap_.size());
    }
}

void DetourSkill::SetDetourMain(DetourMain* detourMain)
{
    pDetourMain_ = detourMain;
}

void DetourSkill::SetCommon(DetourCommon* common)
{
    pDetourCommon_ = common;
}

void* DetourSkill::GetSkillPtr(unsigned int skillId) const
{
    void *skillPtr = NULL;

    if (skillId == 0)
    {
        return skillPtr;
    }

    std::map<unsigned int, void*>::const_iterator found = skillMap_.find(skillId);
    if (found != skillMap_.end())
    {
        skillPtr = found->second;
    }

    return skillPtr;
}

void* DetourSkill::GetDevSkillPtr(unsigned int skillId) const
{
    void *skillPtr = NULL;

    if (skillId == 0)
    {
        return skillPtr;
    }

    std::map<unsigned int, void*>::const_iterator found = devSkillMap_.find(skillId);
    if (found != devSkillMap_.end())
    {
        skillPtr = found->second;
    }

    return skillPtr;
}

void* DetourSkill::GetItemSkillPtr(unsigned int skillId) const
{
    void *skillPtr = NULL;

    std::map<unsigned int, void*>::const_iterator found = itemSkillMap_.find(skillId);
    if (found != itemSkillMap_.end())
    {
        skillPtr = found->second;
    }

    return skillPtr;
}

unsigned int DetourSkill::GetParentSkillId(void* skillPtr) const
{
    return fnSkillBuffGetParentSkillId_.Fn_(skillPtr);
}

void* DetourSkill::GetAlternateSkillPtr(unsigned int skillId)
{
    void* altSkillPtr = NULL;

    altSkillPtr = GetItemSkillPtr(skillId);
    if (altSkillPtr)
    {
        return altSkillPtr;
    }
    std::map<unsigned int, skillBuffData> skillBuffMap_;
    std::map<unsigned int, skillBuffData>::const_iterator itrBuffMap = skillBuffMap_.find(skillId);
    if (itrBuffMap != skillBuffMap_.end())
    {
        if (itrBuffMap->second.parentSkill_)
        {
            return itrBuffMap->second.parentSkill_;
        }
        else
        {
            return itrBuffMap->second.skillptr_;
        }
    }

    std::map<unsigned int, skillBuffData>::const_iterator itrSeflBuffMap = skillSelfBuffMap_.find(skillId);
    if (itrSeflBuffMap != skillSelfBuffMap_.end())
    {
        if (itrSeflBuffMap->second.parentSkill_)
        {
            return itrSeflBuffMap->second.parentSkill_;
        }
        else
        {
            return itrSeflBuffMap->second.skillptr_;
        }
    }


    return NULL;
}

void DetourSkill::AddPetSpawned(unsigned int petId)
{
    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(petId);
    if (itr == petSkillMap_.end())
    {
        petData data;
        petSkillMap_[petId] = data;
        petSkillMap_[petId].skillMap_.clear();

        DLOG(LogSkillBase,"*AddPetSpawned: added pet=%d\n", petId);
    }

    //delay remove
    unsigned int curTime = timeGetTime();

    for (std::map<unsigned int, petData>::iterator itr = petSkillMap_.begin(); 
        itr != petSkillMap_.end(); )
    {
        if (itr->second.removeTimer_ != 0 && itr->second.removeTimer_ > curTime)
        {
            DLOG(LogSkillBase,"*RemovePetSpawned: id=%d expired, removing\n", petId);
            itr->second.skillMap_.clear();

            itr = petSkillMap_.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}

void DetourSkill::RemovePetSpawned(void* petPtr)
{
    unsigned int curTime = timeGetTime();
    unsigned int petId = pDetourCommon_->GetObjectId(petPtr);
    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(petId);

    //only mark it for removal, set expiration timer (10s)
    //do this bc their duration skill damage will extend beyond their death
    if (itr != petSkillMap_.end())
    {
        itr->second.removeTimer_ = curTime + 10000;
    }
}

void DetourSkill::SetPetPtr(unsigned int petId, void* petPtr, unsigned int skillId)
{
    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(petId);
    if (itr != petSkillMap_.end())
    {
        DLOG(LogSkillBase,"*SetPetPtr: petid=%d found, skid=%d\n", petId, skillId);
        petData &pdata = itr->second;
        pdata.petPtr_ = petPtr;
        std::map<unsigned int, void*>::iterator itrSkill = pdata.skillMap_.find(skillId);

        //insert the id w/o the skillPtr (might or might not get it later)
        if (itrSkill == pdata.skillMap_.end())
        {
            pdata.skillMap_[skillId] = NULL;
        }
        else
        {
            DLOG(LogSkillBase,"  already in map\n");
        }
    }
    else
    {
        DLOG(LogSkillBase,"  petId=%d NOT found\n", petId);
    }
}

void* DetourSkill::PetHasSkillId(unsigned int skillId)
{
    void* petPtr = NULL;

    for (std::map<unsigned int, petData>::iterator itr = petSkillMap_.begin(); itr != petSkillMap_.end(); ++itr)
    {
        petData &pdata = itr->second;
        std::map<unsigned int, void*>::const_iterator itrSkill = pdata.skillMap_.find(skillId);
        if (itrSkill != pdata.skillMap_.end())
        {
            petPtr = pdata.petPtr_;
            break;
        }
    }
    return petPtr;
}

void DetourSkill::SetActivatePetSkill(unsigned int entityId, void* skillPtr, void* entityPtr)
{
    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(entityId);
    if (itr != petSkillMap_.end())
    {
        DLOG(LogSkillBase,"*SetActivatePetSkill: petid=%d found\n", entityId);
        unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);
        petData &pdata = itr->second;
        pdata.petPtr_ = entityPtr;

        std::map<unsigned int, void*>::iterator itrSkill = pdata.skillMap_.find(skillId);
        if (itrSkill == pdata.skillMap_.end())
        {
            pdata.skillMap_[skillId] = skillPtr;
            std::string petName;
            pDetourCommon_->GetObjectName(entityPtr, petName);

            DLOG(LogSkillBase,"   skill=%d(%s) inserted, pet name(%s)\n", skillId, pDetourCommon_->GetSkillName(skillPtr), petName.c_str());
        }
    }
}

void* DetourSkill::SkillGetManager(void* skillPtr)
{
    void* manager = NULL;
    if (skillPtr)
    {
        manager = fnSkillGetManager_.Fn_(skillPtr);
    }
    return manager;
}

void* DetourSkill::SkillManagerGetParent(void* skillMgr)
{
    void* character = NULL;
    if (skillMgr)
    {
        character = fnSkillManagerGetParent_.Fn_(skillMgr);
    }
    return character;
}

void DetourSkill::GetPetSkills(void* skillPtr)
{
    DLOG(LogSkillBase,"GetPetSkills: skillptr=0x%p\n", skillPtr);

    unsigned int &skillList = fnSkillGetBaseSkills_.Fn_(skillPtr);
    DLOG(LogSkillBase,"  skilllist = %x\n", skillList);
    if (skillList == 0)
    {
        return;
    }
    unsigned int *skillArray = (unsigned int*)skillList;
    unsigned int skillId = 0;

    // max size is unknown buf as of April 2023 it's around 919
    for (unsigned int i = 0; i < 1000; ++i)
    {
        //DLOG(LogSkillBase," skillArray=0x%X\n", skillArray[i]);
        bool memvalid = DetourUtil::MemValidity((void*)skillArray[i]);

        if (!memvalid || skillArray[i] == 0 || skillArray[i] == 0xffffffff || (skillArray[i] & 0x80000000) != 0)
        {
            DLOG(LogSkillBase,"  1item skills found %d\n", i);
            break;
        }

        unsigned int *skptr = (unsigned int*)skillArray[i];
        memvalid = DetourUtil::MemValidity((void*)skptr[1]);
        if (skptr[1] == 0 || !memvalid)
        {
            DLOG(LogSkillBase,"  2item skills found %d\n", i);
            break;
        }
        const char *recordPtr = (const char*)skptr[1];

        //DLOG(LogSkillBase,"  [%03d] skptr=0x%p, 0x%p(%s)\n", i, skptr, recordPtr, recordPtr);
       //skptr[1] points to a skill record filename
        if (recordPtr[0] == 'r' && recordPtr[1] == 'e' && recordPtr[2] == 'c')
        {
            skillId = pDetourCommon_->GetObjectId(skptr);
            //pdata.skillMap_[skillId] = skptr;
            DLOG(LogSkillBase,"  [%03d] insert skptr=0x%p, id=%d\n", i, skptr, skillId);
        }
        else
        {
            DLOG(LogSkillBase,"  3item skills found %d\n", i);
            break;
        }
    }

}

void* DetourSkill::GetPetPtr(unsigned int petId)
{
    void* petPtr = NULL;
    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(petId);
    if (itr != petSkillMap_.end())
    {
        petData &pdata = itr->second;
        petPtr = pdata.petPtr_;
    }

    return petPtr;
}

void* DetourSkill::GetPetParentSkill(unsigned int petId, unsigned int skillId)
{
    void* parentSkillPtr = NULL;

    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(petId);
    if (itr != petSkillMap_.end())
    {
        petData &pdata = itr->second;
        for (std::map<unsigned int, void*>::iterator itrSkill = pdata.skillMap_.begin();
            itrSkill != pdata.skillMap_.end(); ++itrSkill)
        {
        }
    }
    return parentSkillPtr;
}

void* DetourSkill::GetPetSkillPtr(unsigned int petId, unsigned int skillId)
{
    void* skillPtr = NULL;

    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(petId);
    if (itr != petSkillMap_.end())
    {
        unsigned int skillId = pDetourCommon_->GetObjectId(playerPtr_);
        std::map<unsigned int, void*>::iterator itrIn = itr->second.skillMap_.find(skillId);
        if (itrIn != itr->second.skillMap_.end())
        {
            skillPtr = itr->second.skillMap_[skillId];
        }
    }

    return skillPtr;
}

void DetourSkill::CharacterDispelSkillBuffs(void* charPtr)
{
    if (charPtr == playerPtr_)
    {
        unsigned int charId = pDetourCommon_->GetObjectId(charPtr);
        IPCMessage::SendShortData(DispelBuffs, 1);
        DLOG(LogSkillBase,"****CharacterDispelSkillBuffs: char=%p(%d)\n", charPtr, charId);
    }
}

bool DetourSkill::IsValidSkillToMonitor(void *skillPtr)
{
    unsigned int *uskptr = (unsigned int*)skillPtr;
    const char *recordPtr = (const char*)uskptr[1];
    bool valid = true;

    const char *skname = pDetourCommon_->GetSkillName(skillPtr);
    unsigned int skillid = pDetourCommon_->GetObjectId(skillPtr);
    std::string strSkillName = skname;

    if (strSkillName.find("devotion") == std::string::npos)
    {
        valid = false;
    }
    else
    {
        //ignore 'current skill level' and 'not learned' buff names, 
        //don't know what they are but definitely not gonna show them
        if (strSkillName.find("Current") != std::string::npos || 
            strSkillName.find("Not Learned") != std::string::npos)
        {
            valid = false;
        }
    }

    return valid;
}

void DetourSkill::AddSkillActivated(unsigned int skillId, void* parentSkillPtr)
{
    std::map<unsigned int, skillActivated>::iterator itr = skillActivatedMap_.find(skillId);
    if (itr == skillActivatedMap_.end())
    {
        unsigned int parentSkillId = pDetourCommon_->GetObjectId(parentSkillPtr);
        skillActivated skillActiv;
        skillActiv.parentSkillPtr_ = parentSkillPtr;
        skillActiv.parentSkillId_ = parentSkillId;

        skillActivatedMap_[skillId] = skillActiv;
    }

}

void DetourSkill::RemoveSkillActivated(void *skillPtr)
{
    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);
    for (std::map<unsigned int, skillActivated>::iterator itr = skillActivatedMap_.begin();
        itr != skillActivatedMap_.end(); )
    {
        skillActivated &skillActiv = itr->second;
        if (skillId == itr->first || skillId == skillActiv.parentSkillId_)
        {
            itr = skillActivatedMap_.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}

void DetourSkill::NotifyPlayerIsDying()
{
    for (std::map<unsigned int, skillBuffData>::iterator itr = skillSelfBuffMap_.begin();
        itr != skillSelfBuffMap_.end(); ++itr)
    {
        skillBuffData &data = itr->second;
        SendSkillBuffMessage(data.skillptr_, 0, true, true);
    }
    skillSelfBuffMap_.clear();

    for (std::map<unsigned int, skillBuffData>::iterator itr = skillBuffMap_.begin();
        itr != skillBuffMap_.end(); ++itr)
    {
        skillBuffData &data = itr->second;
        SendSkillBuffMessage(data.skillptr_, 0, true, true);
    }
    skillBuffMap_.clear();


    for (std::map<unsigned int, skillBuffData>::iterator itr = skillBuffCDRemainingMap_.begin();
        itr != skillBuffCDRemainingMap_.end(); ++itr)
    {
        skillBuffData &data = itr->second;
        SendSkillBuffMessage(data.skillptr_, 0, false, true);
    }
    skillBuffCDRemainingMap_.clear();

    tempSkillBuffMap_.clear();
    skillActivatedMap_.clear();
    petSkillMap_.clear();
}

void* DetourSkill::GetSkillActivated(unsigned int skillId)
{
    void* skillPtr = NULL;
    std::map<unsigned int, skillActivated>::const_iterator itr = skillActivatedMap_.find(skillId);
    if (itr != skillActivatedMap_.end())
    {
        skillPtr = itr->second.parentSkillPtr_;
    }

    return skillPtr;
}

void* DetourSkill::GetTempSkillPtr(unsigned int skillId, std::string &name) const
{
    void *skillPtr = NULL;

    std::map<unsigned int, tempSkillBuffData>::const_iterator found = tempSkillBuffMap_.find(skillId);
    if (found != tempSkillBuffMap_.end())
    {
        skillPtr = found->second.skillPtr_;
        name = found->second.name;
        DLOG(LogSkillBase," -- temp skill found=%d\n", skillId);
    }

    return skillPtr;
}

void DetourSkill::AddTempSkill(void* skillPtr)
{
    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);

    std::map<unsigned int, tempSkillBuffData>::const_iterator itr = tempSkillBuffMap_.find(skillId);
    if (itr == tempSkillBuffMap_.end())
    {
        tempSkillBuffData data;
        data.skillPtr_ = skillPtr;
        snprintf(data.name, IPCNAME_SIZE - 1, "%s", pDetourCommon_->GetSkillName(skillPtr));

        // arbitrary 20s duration
        data.expirationTime_ = pDetourCommon_->GetGameTime() + 20000;
        tempSkillBuffMap_[skillId] = data;
        DLOG(LogSkillBase,"AddTempSkill: id=%d\n", skillId);
    }
}

void DetourSkill::SetBuffExpirationTimer(void *skillPtr, int duration)
{
    void *skillMgr = SkillGetManager(skillPtr);
    void *charPtr = SkillManagerGetParent(skillMgr);
    void *charRefCmbatMgr = pDetourCommon_->CharacterGetCombatManager(charPtr);
    unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(charRefCmbatMgr);

    if (playerId_ == attackerId)
    {
        unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);
        std::map<unsigned int, tempSkillBuffData>::iterator itr = tempSkillBuffMap_.find(skillId);
        
        int exp = pDetourCommon_->GetGameTime() + duration;

        if (itr != tempSkillBuffMap_.end())
        {
            itr->second.expirationTime_ = exp;
        }
        else
        {
            tempSkillBuffData data;
            data.skillPtr_ = skillPtr;
            data.expirationTime_ = exp;
            snprintf(data.name, IPCNAME_SIZE - 1, "%s", pDetourCommon_->GetSkillName(skillPtr));
            tempSkillBuffMap_[skillId] = data;
        }
    }
}

void DetourSkill::RemoveTempBuff()
{
    int curTime = pDetourCommon_->GetGameTime();
    for (std::map<unsigned int, tempSkillBuffData>::iterator itr = tempSkillBuffMap_.begin();
        itr != tempSkillBuffMap_.end(); )
    {
        if (curTime > itr->second.expirationTime_)
        {
            itr = tempSkillBuffMap_.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}

bool DetourSkill::GetConsumableSkillId(unsigned int skillId) const
{
    bool found = false;
    for (unsigned i = 0; i < consumableSkillList_.size(); ++i)
    {
        if (consumableSkillList_[i] == skillId)
        {
            found = true;
            break;
        }
    }
    return found;
}


void DetourSkill::SkillBuffInstall(void* skillptr, void* charPtr)
{
    void *skillMgr = SkillGetManager(skillptr);
    void *skillcharPtr = SkillManagerGetParent(skillMgr);

    if (playerPtr_ == skillcharPtr)
    {
        void *charCmbatMgr = pDetourCommon_->CharacterGetCombatManager(charPtr);
        unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(charCmbatMgr);
        // insert player's skillBuff if skillmap is initialized
        unsigned int skillId = pDetourCommon_->GetObjectId(skillptr);
        unsigned int parentSkillId = fnSkillBuffGetParentSkillId_.Fn_(skillptr);
        void* parentSkillPtr = NULL;
        void* skillPtr = GetSkillPtr(skillId);
        const char *parentName = "null";
        bool isParentDevSkill = false;
        std::map<unsigned int, void*>::const_iterator itrParentSkill = skillMap_.end();
    
        if (parentSkillId)
        {
            itrParentSkill = skillMap_.find(parentSkillId);
            if (itrParentSkill != skillMap_.end())
            {
                parentSkillPtr = itrParentSkill->second;
                DLOG(LogSkillBase,"  parentptr=0x%p\n", parentSkillPtr);
                parentName = pDetourCommon_->GetSkillName(parentSkillPtr);
                isParentDevSkill = IsValidSkillToMonitor(parentSkillPtr);
            }
        }

        DLOG(LogSkillBase,"SkillBuff Install: skillId=%d, parent:0x%p(%d), isdevskill=%d, (%s)\n",
            skillId, parentSkillPtr, parentSkillId, isParentDevSkill?1:0, parentName ? parentName : " ");

		const char *skname = pDetourCommon_->GetSkillName(skillptr);
        std::string strSkillName = skname;

        //insert parent skill instead of the child's
        if (parentSkillPtr)
        {
            DLOG(LogSkillBase,"  using parent skill id=%d\n", parentSkillId);
            skillId = parentSkillId;
        }
        else
        {
            DLOG(LogSkillBase,"SkillBuff Install: no parent found, exit id=%d\n", parentSkillId);
            return;
        }

        std::map<unsigned int, skillBuffData>::iterator found = skillBuffMap_.find(skillId);
        if (found == skillBuffMap_.end())
        {
            DLOG(LogSkillBase,"SkillBuff Install: inserting %d (%s) into the map, parent id=%d\n", skillId, skname, parentSkillId);
            skillBuffData data;
            data.parentId_ = parentSkillId;
            data.parentSkill_ = parentSkillPtr;

            data.skillptr_ = skillptr;
            skillBuffMap_[skillId] = data;

            std::map<unsigned int, skillBuffData>::iterator cdfound = skillBuffCDRemainingMap_.find(skillId);
            if (cdfound != skillBuffCDRemainingMap_.end())
            {
                DLOG(LogSkillBase,"  removing from cd buff list\n");
                skillBuffData &data = cdfound->second;
                SendSkillBuffMessage(data.skillptr_, 0, false, true);
                skillBuffCDRemainingMap_.erase(cdfound);
            }
        }
    }
}

void DetourSkill::SkillBuffUnInstall(void* skillptr, void* charPtr)
{
    void *skillMgr = SkillGetManager(skillptr);
    void *skillcharPtr = SkillManagerGetParent(skillMgr);

    if (playerPtr_ && playerPtr_ == skillcharPtr)
    {
        unsigned int skillId = pDetourCommon_->GetObjectId(skillptr);
        const char *skname = pDetourCommon_->GetSkillName(skillptr);
        unsigned int parentSkillId = fnSkillBuffGetParentSkillId_.Fn_(skillptr);
        void *parentSkillPtr = NULL;
        std::map<unsigned int, void*>::const_iterator itrParentSkill = skillMap_.find(parentSkillId);
        if (itrParentSkill != skillMap_.end())
        {
            parentSkillPtr = itrParentSkill->second;
        }
        DLOG(LogSkillBase,"SkillBuff UnInstall: char=0x%p, skillId=%d (%s)\n", charPtr, skillId, skname);

        if (parentSkillPtr)
        {
            DLOG(LogSkillBase,"  using parent skillid %d\n", parentSkillId);
            skillId = parentSkillId;
        }
        std::map<unsigned int, skillBuffData>::iterator found = skillBuffMap_.find(skillId);

        if (found != skillBuffMap_.end())
        {
            skillBuffData &skillDat = found->second;

            //send msg
            SendSkillBuffMessage(skillDat.skillptr_, 0, true, true);

            if (skillDat.parentSkill_)
            {
                int cdr = pDetourCommon_->SkillGetCooldownRemaining(skillDat.parentSkill_);
                std::map<unsigned int, skillBuffData>::iterator cdfound = skillBuffCDRemainingMap_.find(skillDat.parentId_);
                if (cdr > 0 && cdfound == skillBuffCDRemainingMap_.end())
                {
                    skillBuffData parentDat;
                    parentDat.skillptr_ = skillDat.parentSkill_;
                    parentDat.parentId_ = skillDat.parentId_;
                    parentDat.isDevotionSkill_ = skillDat.isDevotionSkill_;
                    skillBuffCDRemainingMap_[parentDat.parentId_] = parentDat;
                    DLOG(LogSkillBase,"  inserting parent cd buff\n");
                }
            }
            DLOG(LogSkillBase,"  removing skill from map\n");
            //SendSkillBuffMessage(skillptr, 0, true, true);
            skillBuffMap_.erase(found);
        }
        else
        {
            DLOG(LogSkillBase,"  not found\n");
        }
    }
}

void DetourSkill::SkillBuffSelfDurationActivateNow(void* This, unsigned int& charRef, unsigned int& nameRef, unsigned int a, unsigned int& vec3Ref)
{
    void *skillMgr = SkillGetManager(This);
    void *charPtr = SkillManagerGetParent(skillMgr);

    if (playerPtr_ && playerPtr_ == charPtr)
    {
        unsigned int skillId = pDetourCommon_->GetObjectId(This);
        const char *skname = pDetourCommon_->GetSkillName(This);

        //ignore consumables
        if (!GetConsumableSkillId(skillId))
        {
            std::map<unsigned int, skillBuffData>::iterator found = skillSelfBuffMap_.find(skillId);
            if (found == skillSelfBuffMap_.end())
            {
                DLOG(LogSkillBase,"lBuffSelfDurationActivate, inserting %d (%s) into the map\n", skillId, skname);
                skillBuffData data;
                data.parentId_ = 0;
                data.skillptr_ = This;
                skillSelfBuffMap_[skillId] = data;
            }
        }
    }
}

void DetourSkill::SkillBuffSelfDurationRemoveSelfBuff(void* skillPtr)
{
    if (playerPtr_ == NULL)
    {
        return;
    }

    void *skillMgr = SkillGetManager(skillPtr);
    void *charPtr = SkillManagerGetParent(skillMgr);

    if (playerPtr_ == charPtr)
    {
        unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);

        std::map<unsigned int, skillBuffData>::iterator found = skillSelfBuffMap_.find(skillId);
        if (found != skillSelfBuffMap_.end())
        {
            skillSelfBuffMap_.erase(found);
            DLOG(LogSkillBase,"SkillBuffSelfDurationRemoveSelfBuff: id=%d\n", skillId);
        }
    }
}

//secondary skill is not necessary a buff skill, and when inserting it as a buff, it's cdr is always 0 and gets 
//removed immediately, however, if it's actually a buff then it will call skillbuff install
//remove this hook eventually
void DetourSkill::SkillActivateSecondarySkill(void* skillPtr, unsigned int &charRef, unsigned int &nameRef, unsigned int a, unsigned int &vecRef, unsigned int &v3Ref)
{
    //arg meaning:
    //skillPtr 
    //charRef 
    //nameRef 
    //a - defender, if a == 0 then self
    //vecRef - if not 0 then vector list of skills
    //v3Ref 
#if 0
    if (playerPtr_ == NULL)
    {
        return;
    }
    if (!DetourUtil::MemValidity(skillPtr))
    {
        DLOG(LogSkillBase,"Skill ActivateSecondarySkill: skptr=%p mem invalie\n", skillPtr);
        return;
    }

    DLOG(LogSkillBase,"Skill ActivateSecondarySkill: skptr=0x%p\n", skillPtr);

    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);
    void *skillMgr = SkillGetManager(skillPtr);
    void *charPtr = SkillManagerGetParent(skillMgr);


    DLOG(LogSkillBase,"  charRef=%p (%s)\n", &charRef, pDetourCommon_->GetSkillName(skillPtr));
    
    if (playerPtr_ == &charRef || playerPtr_ == charPtr)
    {
        //DLOG(LogSkillBase,"Skill ActivateSecondarySkill: skptr=0x%p, id=%d, charRef=%p\n", skillPtr, skillId, &charRef);
        unsigned int parentSkillId = GetParentSkillId(skillPtr);
        void* parentSkillPtr = GetSkillPtr(parentSkillId);
        void* skillPtrRef = parentSkillPtr ? parentSkillPtr : skillPtr;
    }
#endif
    //NOTE: some skill names cannot be accessed w/o crashing. need to figure out how to circumvent this
    //const char *skname = pDetourCommon_->GetSkillName(skillPtr);
    //DLOG(LogSkillBase,"  ActivateSecondarySkill: (%s)\n", skname);
}

void DetourSkill::SkillPrimaryStopSecondarySkills(void *skillPtr, unsigned int &charRef)
{
    if (playerPtr_ == NULL)
    {
        return;
    }
    void *skillMgr = SkillGetManager(skillPtr);
    void *charPtr = SkillManagerGetParent(skillMgr);
    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);

    if (playerPtr_ == &charRef || playerPtr_ == charPtr)
    {
        DLOG(LogSkillBase,"SkillPrimary StopSecondarySkills: skill=%p, id=%d, charRef=%p\n", skillPtr, skillId, &charRef);
        std::map<unsigned int, skillBuffData>::iterator found = skillSelfBuffMap_.find(skillId);
        if (found != skillSelfBuffMap_.end())
        {
            SendSkillBuffMessage(skillPtr, 0, true, true);
            skillSelfBuffMap_.erase(found);
        }
    }
}

void DetourSkill::SkillSvcsCharSendSkillActiveUpdate(void* This, void *skillPtr, unsigned int &activeStateRef)
{
    if (playerPtr_ == NULL)
    {
        return;
    }
    //skill svcs char ptr[3] is the owner char ptr
    void *ownerCharPtr = (void*)((unsigned int*)This)[3];
    unsigned int ownerId = pDetourCommon_->GetObjectId(ownerCharPtr);
    unsigned int *activeState = (unsigned int*)&activeStateRef;
    bool active = activeState[0] & 0xf || activeState[1] & 0xf;
    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);

    if (ownerCharPtr == playerPtr_)
    {
        DLOG(LogSkillBase,"SkillSvcsCharSendSkillActiveUpdate : active=%d\n", active?1:0)
        pDetourMain_->SetSkillActivate(skillPtr, active);

        // insert skill if missing
        if (active)
        {
            if (!GetConsumableSkillId(skillId))
            {
                AddSkillActivated(skillId, skillPtr);
            }
            else
            {
                return;
            }
        }
        else
        {
            RemoveSkillActivated(skillPtr);
        }

        //possibly could be a pet
        if (active)
        {
            SetActivatePetSkill(ownerId, skillPtr, ownerCharPtr);
        }

        //if (oWnerCharPtr == playerPtr_)
        {
            const char *skname = pDetourCommon_->GetSkillName(skillPtr);

            DLOG(LogSkillBase,"SkillSvcsChar SendSkillActiveUpdate: char=%p, skill=%p(%d) active=%d (%s)\n",
                ownerCharPtr, skillPtr, skillId, active ? 1 : 0, skname);

            DLOG(LogSkillBase,"  player=%p, owner=%p, ownerid=%d\n", playerPtr_, ownerCharPtr, ownerId);
        }
    }

}

void  DetourSkill::SkillBuffDebufUpdate(void* skillPtr, unsigned int& charRef, int a)
{
    //arg meaning:
    //skillPtr - owned by charRef and is applying debuf on itself
    //charRef - character applying debuf on itself
    //a - duration?
    if (playerPtr_ == NULL)
    {
        return;
    }
    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);
    unsigned int charRefId = pDetourCommon_->GetObjectId(&charRef);
    unsigned int parentSkid = GetParentSkillId(skillPtr);
    void *charRefCmbatMgr = pDetourCommon_->CharacterGetCombatManager(&charRef);
    unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(charRefCmbatMgr);
    unsigned int petId = 0;

    //possibly might be a pet
    std::map<unsigned int, petData>::iterator itr = petSkillMap_.find(attackerId);
    if (itr != petSkillMap_.end())
    {
        petId = attackerId;
    }

    if (playerId_ == attackerId || petId != 0 || attackerId == 0)
    {
        const char *skname = pDetourCommon_->GetSkillName(skillPtr);

        DLOG(LogSkillDispel, "SkillBuffDebufUpdate: playerId=%d, charRefId=%d, attacker=%d, this=%p(%d)(%s)\n", 
            playerId_, charRefId, attackerId, skillPtr, skillId, skname);

        AddTempSkill(skillPtr);

    }
}

void DetourSkill::SkillBuffSetTimeToLive(void* skillPtr, int a)
{
    if (playerPtr_ == NULL)
    {
        return;
    }
    unsigned int skillId = pDetourCommon_->GetObjectId(skillPtr);
    void *skillMgr = SkillGetManager(skillPtr);
    void *charPtr = SkillManagerGetParent(skillMgr);
    void *charRefCmbatMgr = pDetourCommon_->CharacterGetCombatManager(charPtr);
    unsigned int attackerId = pDetourCommon_->CombatMgrGetAttackerId(charRefCmbatMgr);

    if (playerId_ == attackerId)
    {
        DLOG(LogSkillDispel, "SkillBuffSetTimeToLive: skillId=%d, time to live=%d\n", skillId, a);
        SetBuffExpirationTimer(skillPtr, a);
    }
}


//=====================================
// static fns
//=====================================
void DetourSkill::DTCharacterDispelSkillBuffs(void* This)
{
    SKILLLOG("DTCharacterDispelSkillBuffs\n");

    sDetourSkill_->CharacterDispelSkillBuffs(This);

    fnCharacterDispelSkillBuffs_.Fn_(This);
}

void DetourSkill::DTSkillBuffInstall(void* This, void*, void* charPtr)
{
    SKILLLOG("DTSkillBuffInstall\n");

    sDetourSkill_->SkillBuffInstall(This, charPtr);

    fnSkillBuffInstall_.Fn_(This, charPtr);
    SKILLLOG("end DTSkillBuffInstall\n");
}

void DetourSkill::DTSkillBuffUnInstall(void* This, void*, void* charPtr)
{
    SKILLLOG("DTSkillBuffUnInstall\n");

    sDetourSkill_->SkillBuffUnInstall(This, charPtr);

    fnSkillBuffUnInstall_.Fn_(This, charPtr);

    SKILLLOG("end DTSkillBuffUnInstall\n");
}

void DetourSkill::DTSkillBuffSelfDurationActivateNow(void* This, void*, unsigned int& charRef, unsigned int& nameRef, unsigned int a, unsigned int& vec3Ref)
{
    SKILLLOG("DTSkillBuffSelfDurationActivateNow\n");

    sDetourSkill_->SkillBuffSelfDurationActivateNow(This, charRef, nameRef, a, vec3Ref);

    fnSkillBuffSelfDurationActivateNow_.Fn_(This, charRef, nameRef, a, vec3Ref);
    SKILLLOG("end DTSkillBuffSelfDurationActivateNow\n");
}

void DetourSkill::DTSkillBuffSelfDurationRemoveSelfBuff(void* This)
{
    sDetourSkill_->SkillBuffSelfDurationRemoveSelfBuff(This);

    fnSkillBuffSelfDurationRemoveSelfBuff_.Fn_(This);
}

void DetourSkill::DTSkillSvcsCharSendSkillActiveUpdate(void* This, void*, void *skillPtr, unsigned int &skillActiveStateRef)
{
    SKILLLOG("DTSkillSvcsCharSendSkillActiveUpdate\n");

    sDetourSkill_->SkillSvcsCharSendSkillActiveUpdate(This, skillPtr, skillActiveStateRef);

    fnSkillServicesCharacterSendSkillActiveUpdate_.Fn_(This, skillPtr, skillActiveStateRef);
    SKILLLOG("end DTSkillSvcsCharSendSkillActiveUpdate\n");
}

void DetourSkill::DTSkillActivateSecondarySkill(void* This, void*, unsigned int &charRef, unsigned int &nameRef, unsigned int a, unsigned int &vecRef, unsigned int &vec3Ref)
{
    SKILLLOG("DTSkillActivateSecondarySkill\n");

    sDetourSkill_->SkillActivateSecondarySkill(This, charRef, nameRef, a, vecRef, vec3Ref);
    fnSkillActivateSecondarySkill_.Fn_(This, charRef, nameRef, a, vecRef, vec3Ref);

    SKILLLOG("end DTSkillActivateSecondarySkill\n");
}

void DetourSkill::DTSkillPrimaryStopSecondarySkills(void *This, void*, unsigned int &charRef)
{
    SKILLLOG("DTSkillPrimaryStopSecondarySkills\n");
    sDetourSkill_->SkillPrimaryStopSecondarySkills(This, charRef);
    fnSkillPrimaryStopSecondarySkills_.Fn_(This, charRef);
    SKILLLOG("end DTSkillPrimaryStopSecondarySkills\n");
}

void DetourSkill::DTSkillBuffDebufUpdate(void* This, void*, unsigned int& charRef, int a)
{
    SKILLLOG("DTSkillBuffDebufUpdate\n");
    sDetourSkill_->SkillBuffDebufUpdate(This, charRef, a);
    fnSkillBuffDebufUpdate_.Fn_(This, charRef, a);
    SKILLLOG("end DTSkillBuffDebufUpdate\n");
}


void DetourSkill::DTSkillBuffSetTimeToLive(void* This, void*, int a)
{
    SKILLLOG("DTSkillBuffSetTimeToLive\n");
    sDetourSkill_->SkillBuffSetTimeToLive(This, a);
    fnSkillBuffSetTimeToLive_.Fn_(This, a);
    SKILLLOG("end DTSkillBuffSetTimeToLive\n");
}


