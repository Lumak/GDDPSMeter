#include <windows.h>
#include <string>
#include <cassert>

#include "DetourCommon.h"
#include "DetourUtil.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
DetourFnData datGenerateUISkillInfo = { "game.dll", NULL, NULL, SYM_GAMEENGINE_GENERATEUISKILLINFO };
DetourFnData datGetObjectId = { "engine.dll", NULL, NULL, SYM_OBJECT_GETOBJECTID };
DetourFnData datGetObjectName = { "engine.dll", NULL, NULL, SYM_OBJECT_GETOBJECTNAME };
DetourFnData datCharacterGetSkillList = { "game.dll", NULL, NULL, SYM_CHAR_GETSKILLLIST };
DetourFnData datSkillMgrGetItemSkillList = { "game.dll", NULL, NULL, SYM_SKILLMGR_GETITEMSKILLLIST };
DetourFnData datCharGetSkillMgr = { "game.dll", NULL, NULL, SYM_CHAR_GETSKILLMGR };
DetourFnData datCharGetCharBio = { "game.dll", NULL, NULL,SYM_CHAR_GETCHARBIO  };
DetourFnData datCharBioGetBonusLifeAmt = { "game.dll", NULL, NULL,  SYM_CHARBIO_GETBONUSLIFEAMOUNT };
DetourFnData datCharGetCombatMgr = { "game.dll", NULL, NULL, SYM_CHAR_GETCOMBATMGR };
DetourFnData datCombatMgrGetAttackerId = { "game.dll", NULL, NULL, SYM_COMBATMGR_GETATTACKERID };
DetourFnData datCombatMgrGetCharacter = { "game.dll", NULL, NULL, SYM_COMBATMGR_GETCHARACTER };
DetourFnData datSkillTrackableTotalTime = { "game.dll", NULL, NULL, SYM_SKILLBUFFSELFDURATION_TRACKABLETOTALTIME };

DetourFnData datSkillGetCooldownCompletion = { "game.dll", NULL, NULL, SYM_SKILL_GETCOOLDOWNCOMPLETION };
DetourFnData datSkillGetCooldownRemaining = { "game.dll", NULL, NULL, SYM_SKILL_GETCOOLDOWNREMAINING };
DetourFnData datSkillGetCooldownTime = { "game.dll", NULL, NULL, SYM_SKILL_GETCOOLDOWNTIME };
DetourFnData datSkillGetCooldownTotal = { "game.dll", NULL, NULL, SYM_SKILL_GETCOOLDOWNTOTAL };
DetourFnData datGetGameTime = { "engine.dll", NULL, NULL, SYM_GETGAMETIME };
DetourFnData datCharGetPortraitName = { "game.dll", NULL, NULL, SYM_CHAR_GETPORTRAITNAME };

//=============================================================================
//=============================================================================
DetourCommon::DetourCommon()
{
  initialized_ = false;
}

bool DetourCommon::SetupDetour()
{
    int status = 0;

    status = HookDetour(datGetObjectId);
    fnGetObjectId_.SetFn(datGetObjectId.realFn_);

    status += HookDetour(datGetObjectName);
    fnObjectGetObjectName_.SetFn(datGetObjectName.realFn_);

    status += HookDetour(datCharacterGetSkillList);
	fnCharacterGetSkillList_.SetFn(datCharacterGetSkillList.realFn_);

    status += HookDetour(datSkillMgrGetItemSkillList);
    fnSkillManagerGetItemSkillList_.SetFn(datSkillMgrGetItemSkillList.realFn_);

    status += HookDetour(datCharGetSkillMgr);
	fnCharacterGetSkillManager_.SetFn(datCharGetSkillMgr.realFn_);

    status += HookDetour(datCharGetCharBio);
	fnCharacterGetCharacterBio_.SetFn(datCharGetCharBio.realFn_);

    status += HookDetour(datCharBioGetBonusLifeAmt);
	fnCharacterBioGetBonusLifeAmount_.SetFn(datCharBioGetBonusLifeAmt.realFn_);

    status += HookDetour(datCharGetCombatMgr);
    fnCharacterGetCombatManager_.SetFn(datCharGetCombatMgr.realFn_);

    status += HookDetour(datCombatMgrGetAttackerId);
    fnCombatMgrGetAttackerId_.SetFn(datCombatMgrGetAttackerId.realFn_);

    status += HookDetour(datCombatMgrGetCharacter);
    fnCombatManagerGetCharacter_.SetFn(datCombatMgrGetCharacter.realFn_);

    status += HookDetour(datSkillTrackableTotalTime);
    fnSkillTrackableTotalTime_.SetFn(datSkillTrackableTotalTime.realFn_);

    status += HookDetour(datSkillGetCooldownCompletion);
    fnSkillGetCooldownCompletion_.SetFn(datSkillGetCooldownCompletion.realFn_);

    status += HookDetour(datSkillGetCooldownRemaining);
	fnSkillGetCooldownRemaining_.SetFn(datSkillGetCooldownRemaining.realFn_);

    status += HookDetour(datSkillGetCooldownTime);
    fnSkillGetCooldownTime_.SetFn(datSkillGetCooldownTime.realFn_);

    status += HookDetour(datSkillGetCooldownTotal);
    fnSkillGetCooldownTotal_.SetFn(datSkillGetCooldownTotal.realFn_);

	status += HookDetour(datCharGetPortraitName);
	fnCharGetPortraitName_.SetFn(datCharGetPortraitName.realFn_);

	status += HookDetour(datGenerateUISkillInfo);
	fnGenerateUISkillInfo_ = (FnGenerateUISkillInfo)datGenerateUISkillInfo.realFn_;

	status += HookDetour(datGetGameTime);
    fnGetGameTime_ = (FnGetGameTime)datGetGameTime.realFn_;

    if (status != 0)
    {
        SetError("Error in DetourCommon::SetupDetour()");
    }

    return status == 0;
}

void DetourCommon::Update(void *player, int idx)
{
}

unsigned int DetourCommon::GetObjectId(void* obj) const
{
    return fnGetObjectId_.Fn_(obj);
}

void DetourCommon::GetObjectName(void* obj, std::string &name)
{
    const char *objname = fnObjectGetObjectName_.Fn_(obj);

    if (objname)
    {
        std::string strName = std::string(objname);
        std::size_t pos = strName.find_last_of("/");
        if (pos != std::string::npos)
        {
        //omit the path and the extension(.dbr): len - (pos + 1) - 4
        name = strName.substr(pos + 1, strName.length() - pos - 5);
        }
        else
        {
        name = "entity";
        }
    }
    else
    {
        name = "*";
        LOGF("  obj name not found\n");
    }
}


const char* DetourCommon::GetSkillName(void* skillptr)
{
    if (!skillptr)
    {
        return "null";
    }

    unsigned char skillbuf[STR_BUF_SIZE] = { 0 };

    fnGenerateUISkillInfo_(skillptr, (unsigned int&)skillbuf);

    //what's returned are double wide chars, convert this to single wide char
    const unsigned short *uname = (const unsigned short*)(((unsigned int*)&skillbuf)[0]);
    uname += 0x2;

    unsigned int* uptr = (unsigned int*)uname;
    unsigned int memval = (unsigned int)uptr[0];
    bool isWideChar = (memval & 0xff000000) != 0;

    std::wstring &wname = *(std::wstring*)uname;

    //convert to char string
    if (isWideChar)
    {
        char *converted = DetourUtil::WStr2CharStr("%ls", wname);
        sprintf_s(wcharbuff, STR_BUF_SIZE - 1, "%s", converted);
    }
    else
    {
        //copy unsigned as char
        for (int i = 0; i < STR_BUF_SIZE; ++i)
        {
            const unsigned short v = uname[i];
            char c = (char)v;
            wcharbuff[i] = c;
            if (v == 0)
            break;
        }
    }

    //strip out '(' 
    for (unsigned int i = 0; i < STR_BUF_SIZE; ++i)
    {
        if (wcharbuff[i] == '(')
        {
            if (wcharbuff[i - 1] == ' ')
            {
                wcharbuff[i - 1] = '\0';
            }
            else
            {
                wcharbuff[i] = '\0';
            }
            break;
        }
    }

    return wcharbuff;
}

unsigned int& DetourCommon::CharGetSkillList(void *charPtr) const
{
    return fnCharacterGetSkillList_.Fn_(charPtr);
}

unsigned int& DetourCommon::SkillMgrGetItemSkillList(void *This) const
{
    return fnSkillManagerGetItemSkillList_.Fn_(This);
}

unsigned int& DetourCommon::CharGetSkillMgr(void *charPtr) const
{
    return fnCharacterGetSkillManager_.Fn_(charPtr);
}

unsigned int& DetourCommon::CharGetCharacterBio(void* charPtr) const
{
    return fnCharacterGetCharacterBio_.Fn_(charPtr);

}

float DetourCommon::CharBioGetBonusLifeAmount(void *charBio, unsigned int& bonusRef) const
{
    return fnCharacterBioGetBonusLifeAmount_.Fn_(charBio, bonusRef);
}

void* DetourCommon::CharacterGetCombatManager(void* charPtr) const
{
    return fnCharacterGetCombatManager_.Fn_(charPtr);
}
unsigned int DetourCommon::CombatMgrGetAttackerId(void* This) const
{
    return fnCombatMgrGetAttackerId_.Fn_(This);
}

void* DetourCommon::CombatManagerGetCharacter(void* This) const
{
    return fnCombatManagerGetCharacter_.Fn_(This);
}

int DetourCommon::SkillTrackableTotalTime(void* This, unsigned int a) const
{
    return fnSkillTrackableTotalTime_.Fn_(This, a);
}


float DetourCommon::SkillGetCooldownCompletion(void* This) const
{
    return fnSkillGetCooldownCompletion_.Fn_(This);
}

int DetourCommon::SkillGetCooldownRemaining(void* This) const
{
    return fnSkillGetCooldownRemaining_.Fn_(This);
}

float DetourCommon::SkillGetCooldownTime(void* This) const
{
    return fnSkillGetCooldownTime_.Fn_(This);
}

int DetourCommon::SkillGetCooldownTotal(void* This) const
{
    return fnSkillGetCooldownTotal_.Fn_(This);
}

int DetourCommon::GetGameTime() const
{
    return fnGetGameTime_();
}

unsigned int& DetourCommon::CharGetPortraitName(void* charPtr) const
{
    return fnCharGetPortraitName_.Fn_(charPtr);
}

