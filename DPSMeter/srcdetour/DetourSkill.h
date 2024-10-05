#pragma once
#include <vector>
#include <map>

#include "DetourBase.h"
#include "IPCMessage.h"

//=============================================================================
//=============================================================================
class DetourMain;
class DetourCommon;

//=============================================================================
//=============================================================================
#define SYM_CHARACTER_DISPELSKILLBUFFS "?DispelSkillBuffs@Character@GAME@@UAEXXZ"
#define SYM_SKILLBUFF_INSTALL "?Install@SkillBuff@GAME@@UAEXPAVCharacter@2@@Z"
#define SYM_SKILLBUFF_UNINSTALL "?UnInstall@SkillBuff@GAME@@UAEXPAVCharacter@2@@Z"
#define SYM_SKILLBUFFSELFDURATION_ACTIVATENOW "?ActivateNow@Skill_BuffSelfDuration@GAME@@UAEXAAVCharacter@2@ABVName@2@IABVWorldVec3@2@@Z"
#define SYM_SKILLBUFFSELFDURATION_REMOVESELFBUFF "?RemoveSelfBuff@Skill_BuffSelfDuration@GAME@@UAEXXZ"
#define SYM_SKILLBUFF_GETPARENT "?GetParentSkillId@SkillBuff@GAME@@QBEIXZ"
#define SYM_SKILL_ACTIVATESECONDARYSKILLS "?ActivateSecondarySkills@Skill@GAME@@UAEXAAVCharacter@2@ABVName@2@IABV?$vector@I@mem@@ABVWorldVec3@2@@Z"
#define SYM_SKILL_PRIMARYSTOPSECONDARYSKILLS "?PrimaryStopSecondarySkills@Skill@GAME@@QAEXAAVCharacter@2@@Z"
#define SYM_SKILL_GETSECONDARYSKILL "?GetSecondarySkills@Skill@GAME@@QBEABV?$vector@I@mem@@XZ"
#define SYM_SKILL_GETBASESKILLS "?GetBaseSkills@Skill@GAME@@QBEABV?$vector@I@mem@@XZ" 
#define SYM_SKILL_GETMANAGER "?GetManager@Skill@GAME@@QAEPAVSkillManagerBase@2@XZ"
#define SYM_SKILLMGR_GETPARENT "?GetParent@SkillManager@GAME@@UBEPAVCharacter@2@XZ" 
#define SYM_SKILLSERVCHAR_SENDSKILLACTIVEUPDATE "?SendSkillActiveUpdate@SkillServices_Character@GAME@@UAEXPBVSkill@2@ABVSkillActiveState@2@@Z" 
#define SYM_SKILLBUFFDEBUF_UPDATE "?Update@SkillBuff_Debuf@GAME@@UAEXAAVCharacter@2@H@Z" 
#define SYM_SKILLBUFF_SETTIMETOLIVE "?SetTimeToLive@SkillBuff@GAME@@UAEXH@Z" 

//=============================================================================
//=============================================================================
struct skillBuffData
{
    skillBuffData() : parentId_(0), isDevotionSkill_(false), parentSkill_(NULL), CDRemaining_(0), prevtime_(0){}

    void *skillptr_;
    unsigned int parentId_;
    void* parentSkill_;
    unsigned int CDRemaining_;
    int prevtime_;
    bool isDevotionSkill_;
};

struct petData
{
    petData() : petPtr_(NULL), removeTimer_(0) {}

    std::map<unsigned int, void*> skillMap_;
    void* petPtr_;
    unsigned int removeTimer_;
};

struct skillActivated
{
    skillActivated() :parentSkillPtr_(NULL), parentSkillId_(0) {}

    void *parentSkillPtr_;
    unsigned int parentSkillId_;
};

struct tempSkillBuffData
{
    tempSkillBuffData() : skillPtr_(NULL), expirationTime_()
    {
        memset(name, 0, IPCNAME_SIZE);
    }

    void* skillPtr_;
    int expirationTime_;
    char name[IPCNAME_SIZE];

    tempSkillBuffData& operator = (const tempSkillBuffData &rhs)
    {
        memcpy(this, &rhs, sizeof(*this));
        return *this;
    }
};

//=============================================================================
//=============================================================================
class DetourSkill : public DetourBase
{
public:
    DetourSkill();

    virtual bool SetupDetour();
    virtual void Update(void *player, int idx);
    virtual void SetPlayer(void* player);

    void ClearMaps();
    void SetSkillMap();
    void SetItemSkillMap();
    void SetDetourMain(DetourMain* detourMain);
    void SetCommon(DetourCommon* common);
    void* GetSkillPtr(unsigned int) const;
    void* GetDevSkillPtr(unsigned int) const;
    void* GetTempSkillPtr(unsigned int, std::string &) const;
    void* GetItemSkillPtr(unsigned int) const;
    unsigned int GetParentSkillId(void* skillPtr) const;
    void* GetAlternateSkillPtr(unsigned int skillId);

    void AddPetSpawned(unsigned int);
    void RemovePetSpawned(void*);
    void SetActivatePetSkill(unsigned int, void* skillPtr, void* entityPtr);
    void SetPetPtr(unsigned int petId, void* entityPtr, unsigned int skillId);
    void* PetHasSkillId(unsigned int skillId);
    void* GetPetPtr(unsigned int);
    void* GetPetParentSkill(unsigned int petId, unsigned int skillId);
    void* GetPetSkillPtr(unsigned int petId, unsigned int skillId);
    void* GetSkillActivated(unsigned int skillId);
    void NotifyPlayerIsDying();

private:
    void AddSkillActivated(unsigned int skillId, void* parentSkillPtr);
    void RemoveSkillActivated(void *skillPtr);
    void AddTempSkill(void* skillPtr);
    void SetBuffExpirationTimer(void *skillPtr, int duration);
    void RemoveTempBuff();
    bool GetConsumableSkillId(unsigned int skillId) const;

    void CharacterDispelSkillBuffs(void*);
    bool IsValidSkillToMonitor(void *skillPtr);
    void SkillBuffInstall(void* skillptr, void* charPtr);
    void SkillBuffSelfDurationActivateNow(void* This, unsigned int& charRef, unsigned int& nameRef, unsigned int a, unsigned int& vec3Ref);
    void SkillBuffSelfDurationRemoveSelfBuff(void*);
    void SkillActivateSecondarySkill(void*, unsigned int &charRef, unsigned int &nameRef, unsigned int, unsigned int &vecRef, unsigned int &vec3Ref);
    void SkillPrimaryStopSecondarySkills(void *This, unsigned int &charRef);
    void SkillBuffUnInstall(void*, void*);
    void SendSkillBuffMessage(void* skillPtr, int timeRemaining, bool isActiveBuff, bool remove);
    void GetPetSkills(void* skillPtr);
    void* SkillGetManager(void* skillPtr);
    void* SkillManagerGetParent(void* skillMgr);

    void SkillSvcsCharSendSkillActiveUpdate(void* This, void *skillPtr, unsigned int &skillActiveStateRef);
    void SkillBuffDebufUpdate(void*, unsigned int&, int);
    void SkillBuffSetTimeToLive(void*, int);

public:
    static void __fastcall DTCharacterDispelSkillBuffs(void*);
    static void __fastcall DTSkillBuffInstall(void* This, void*, void* charPtr);
    static void __fastcall DTSkillBuffUnInstall(void*, void*, void*);
    static void __fastcall DTSkillBuffSelfDurationActivateNow(void* This, void*, unsigned int& charRef, unsigned int& nameRef, unsigned int a, unsigned int& vec3Ref);
    static void __fastcall DTSkillBuffSelfDurationRemoveSelfBuff(void*);
    static void __fastcall DTSkillActivateSecondarySkill(void*, void*, unsigned int &charRef, unsigned int &nameRef, unsigned int, unsigned int &vecRef, unsigned int &vec3Ref);
    static void __fastcall DTSkillPrimaryStopSecondarySkills(void *This, void*, unsigned int &charRef);

    static void __fastcall DTSkillSvcsCharSendSkillActiveUpdate(void*, void*, void *skillPtr, unsigned int &skillActiveStateRef);
    static void __fastcall DTSkillBuffDebufUpdate(void*, void*, unsigned int&, int);
    static void __fastcall DTSkillBuffSetTimeToLive(void*, void*, int);

private:
    void* playerPtr_;
    unsigned int playerId_;
    bool initialized_;
    unsigned int startTime_;

    DetourMain *pDetourMain_;
    DetourCommon *pDetourCommon_;

    std::map<unsigned int, void*> skillMap_;
    std::map<unsigned int, void*> devSkillMap_;
    std::vector<unsigned int> consumableSkillList_;
    std::map<unsigned int, void*> itemSkillMap_;
    std::map<unsigned int, tempSkillBuffData> tempSkillBuffMap_;
    std::map<unsigned int, skillActivated> skillActivatedMap_;
    std::map<unsigned int, skillBuffData> skillBuffMap_;
    std::map<unsigned int, skillBuffData> skillSelfBuffMap_;
    std::map<unsigned int, skillBuffData> skillBuffCDRemainingMap_;
    std::map<unsigned int, petData> petSkillMap_;

	//fn var
	ThisFunc<unsigned int &, void*> fnSkillGetSecondarySkills_;
	ThisFunc<void*, void*> fnSkillGetManager_;
	ThisFunc<void*, void*> fnSkillManagerGetParent_;
	ThisFunc<unsigned int &, void*> fnSkillGetBaseSkills_;

    //self static var
    static DetourSkill *sDetourSkill_;

	//static fn vars
	static ThisFunc<void, void*, void*> fnSkillBuffInstall_;
	static ThisFunc<void, void*, void*> fnSkillBuffUnInstall_;
	static ThisFunc<void, void*, unsigned int&, unsigned int&, unsigned int, unsigned int&> fnSkillBuffSelfDurationActivateNow_;
	static ThisFunc<void, void*> fnCharacterDispelSkillBuffs_;
	static ThisFunc<void, void*> fnSkillBuffSelfDurationRemoveSelfBuff_;
	static ThisFunc<unsigned int, void*> fnSkillBuffGetParentSkillId_;

	static ThisFunc<void, void*, unsigned int&, unsigned int&, unsigned int, unsigned int&, unsigned int&> fnSkillActivateSecondarySkill_;
	static ThisFunc<void, void *, unsigned int&> fnSkillPrimaryStopSecondarySkills_;
	static ThisFunc<void, void*, void*, unsigned int&> fnSkillServicesCharacterSendSkillActiveUpdate_;

	static ThisFunc<void, void*, unsigned int&, int> fnSkillBuffDebufUpdate_;
	static ThisFunc<void, void*, int> fnSkillBuffSetTimeToLive_;
};