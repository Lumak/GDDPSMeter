#pragma once
#include <vector>
#include <map>
#include "DetourBase.h"
#include "Mutex.h"
#include "IPCMessage.h"

//=============================================================================
//=============================================================================
//fwd decl
class DetourCommon;
class DetourSkill;
class DetourOADAStats;
class DetourScreen;

//=============================================================================
//=============================================================================
#define SYM_PLAYER_UPDATESELF "?UpdateSelf@Player@GAME@@UAEXH@Z"
#define SYM_PLAYER_ONATTACK	"?OnAttack@Player@GAME@@UAEXPAVEntity@2@@Z"
#define SYM_CHARBIO_TAKEBONUS "?TakeBonus@CharacterBio@GAME@@QAEXABVBonus@2@_N@Z"
#define SYM_CHAR_ATTACKTARGET "?AttackTarget@Character@GAME@@UAE_NIAAVEntity@2@AAVParametersCombat@2@_NIII@Z"
#define SYM_COMBATMGR_APPLYDAMAGE "?ApplyDamage@CombatManager@GAME@@QAE_NMABUPlayStatsDamageType@2@W4CombatAttributeType@2@ABV?$vector@I@mem@@@Z"
#define SYM_COMBATMGR_TAKEATTACK "?TakeAttack@CombatManager@GAME@@QAE_NAAVParametersCombat@2@AAVSkillManager@2@AAVCharacterBio@2@@Z" 
#define SYM_PLAYER_REGISTERCOMBATTEXTHIT "?RegisterCombatTextHit@Player@GAME@@UAEMM@Z"
#define SYM_PLAYER_REGISTERCOMBATTEXTCRIT "?RegisterCombatTextCrit@Player@GAME@@UAEMM@Z"
#define SYM_GAMEENGINE_REGISTERDAMAGE "?RegisterDamage@GameEngine@GAME@@QAEXIIM@Z"
#define SYM_COMBATATTRIBACC_EXEDAMAGE "?ExecuteDamage@CombatAttributeAccumulator@GAME@@QAEXAAVCharacter@2@@Z"
#define SYM_CTRLPLAYER_REQUESTMOVEACTION "?DefaultRequestMoveAction@ControllerPlayerState@GAME@@IAEX_N0ABVWorldVec3@2@@Z"
#define SYM_PLAYER_POSTSPAWNPET "?PostPetSpawn@Player@GAME@@UAEXABVWorldVec3@2@III_N@Z"
#define SYM_CHAR_CHARACTERISDYING "?CharacterIsDying@Character@GAME@@UAEXXZ"

//=============================================================================
//=============================================================================
class DetourMain : public DetourBase
{
  struct DamageData
  {
    DamageData() : enemyId_(0), combatAttrType_(0), skillId_(0), damage_(0.0f) {}

    unsigned int enemyId_;
    unsigned int combatAttrType_;
    unsigned int skillId_;
    float damage_;
  };

  struct CombatAttribDamageData
  {
    unsigned int combatAttrType_;
    float damage_;
  };

  struct AttackerDamageData
  {
    AttackerDamageData()
    {
      memset(this, 0, sizeof(AttackerDamageData));
    }

    AttackerDamageData(unsigned int attakerId, unsigned int skid, unsigned int defender) 
      : attackerId_(attakerId), skillId_(skid), defender_(defender), 
        attackerIsPlayer_(false), attackerPetId_(0), parentSkillPtr_(NULL), totalDamage_(0.0f)
    {
      memset(detailDmg_, 0, sizeof(detailDmg_));
      memset(name_, 0, sizeof(name_));
    }

    void Clear()
    {
        memset(this, 0, sizeof(AttackerDamageData));
    }

    void ClearDetails()
    {
        memset(detailDmg_, 0, sizeof(detailDmg_));
    }

    bool attackerIsPlayer_;
    unsigned int attackerPetId_;
    unsigned int attackerId_;
    unsigned int skillId_;
    unsigned int defender_;
    void *parentSkillPtr_;
    bool isCrit_;
    float totalDamage_;
    char name_[IPCNAME_SIZE];

    detailDamageData detailDmg_[DETAIL_SIZE];

  };

public:
    DetourMain();
    ~DetourMain();

    static void Init();
    static void Close();

    static DetourMain& GetInstance(){ return *sDetourMain_; }
  
    virtual bool SetupDetour();
    virtual void Update(void *player, int idx);
    virtual void SetPlayer(void* player);

    void SetSkillActivate(void *skillPtr, bool active);
    void SetScreenEnable(bool enable);
	void SetInGameRendered(bool ingame);

private:
    void SetupPlayer();
    void PreCharAttackTarget(void*, unsigned int, unsigned int&, unsigned int&, bool, unsigned int, unsigned int, unsigned int);
    void PostCharAttackTarget();

    void CalculateDPS(float damage);
    bool GetAttackDataSkillName(const AttackerDamageData &attackDamage, IPCData &ipcskill);
    void BuildAttackDamageData(const AttackerDamageData &damageData, float registeredDamage, std::vector<IPCData> &msgList);
    void PrePlayerOnAttack(void* player, void* entity);
    void TakeBonus(void* charBio, unsigned int&bonusRef, bool a);
    
    void CombatgMgrApplyDamage(void *This, float a, unsigned int &playstats, unsigned int cmbtAttrType, unsigned int &vlist);
    void CombatManagerTakeDamage(void*, unsigned int&, unsigned int&, unsigned int&);
	
    void PlayerRegisterCombatTextHit(void* This, float a, float f);
    void PlayerRegisterCombatTextCrit(void* This, float a, float f);

    void GameEngineRegisterDamage(void* This, unsigned int a, unsigned int b, float c);
    void CombatAttribAccuExeDamage(void *This, unsigned int& charRef);
    void CtrlPlayerStateDfltRequestMoveAction(void*, bool, bool, unsigned int&);

    void PlayerPostSpawnPet(void*, unsigned int&, unsigned int, unsigned int, unsigned int, bool);
    void CharCharacterIsDying(void*);

public:
    //static fns
    static void __fastcall DTPlayerUpdateSelf(void*, void*, int);
    static void __fastcall DTPlayerOnAttack(void *This, void *, void *entity);
    static void __fastcall DTCharacterBioTakeBonus(void* This, void*, unsigned int &bonusRef, bool a);
    static bool __fastcall DTCharacterAttackTarget(void*, void*, unsigned int, unsigned int&, unsigned int&, bool, unsigned int, unsigned int, unsigned int);
    static bool __fastcall DTCombatgMgrApplyDamage(void *This, void*, float a, unsigned int &playstats, unsigned int cmbtAttrType, unsigned int &vlist);
    static bool __fastcall DTCombatManagerTakeAttack(void*, void*, unsigned int&, unsigned int&, unsigned int&);


    static float __fastcall DTPlayerRegisterCombatTextHit(void* This, void*, float a);
    static float __fastcall DTPlayerRegisterCombatTextCrit(void* This, void*, float a);

    static void __fastcall DTGameEngineRegisterDamage(void* This, void*, unsigned int a, unsigned int b, float c);
    static void __fastcall DTCombatAttribAccuExeDamage(void *This, void*, unsigned int& charRef);
    static void __fastcall DTCtrlPlayerStateDfltRequestMoveAction(void*, void*, bool, bool, unsigned int&);

    static void __fastcall DTPlayerPostSpawnPet(void*, void*, unsigned int&, unsigned int, unsigned int, unsigned int, bool);
    static void __fastcall DTCharCharacterIsDying(void*);

protected:
    void InitSubClasses();
    void UpdateSubClasses(void *player, int idx);
  
private:
  
	bool ingameRendered_;
    bool screenEnabled_;
    bool childProcessSet_;

    void *playerPtr_;
    unsigned int playerId_;
    void *playerSkillMgr_;
    void *playerCharacterBio_;
    bool playerInitialized_;
    void *lastActivatedSkill_;
    bool lastActivatedSkillActive_;

    unsigned int dpsStatStartTime_;
    unsigned int dpsStatLastActionTime_;
    float accumulatedDamage_;
    float dpsStatDPS_;

    unsigned int hpRecoveryStartTime_;
    unsigned int hpRecoveryLastActionTime_;
    float accumulatedHpRecovery_;
    float hpRecoveryPS_;

    DetourCommon *pDetourCommon_;
    DetourSkill *pDetourSkill_;
    DetourOADAStats *pDetourOADAStats_;
    DetourScreen *pDetourScreen_;
    std::vector<DetourBase*> subDetourClassList_;

    std::map<unsigned int, void*> skillMap_;
    std::vector<unsigned int> petList_;

    AttackerDamageData playerAttackInProgress_;
    std::vector<AttackerDamageData> applyDamageProgress_;
    std::vector<AttackerDamageData> enemyAttackInProgress_;

    std::vector<DamageData> enemyDamageDealtList_;
    std::vector<DamageData> playerDamageDealtList_;
    std::vector<attackDamageData> runningAttackDamageList_;

    unsigned int startTime_;
    bool initialized_;

    // static vars
    static DetourMain *sDetourMain_;

    // static fn vars
	static RealFunc<void, void*, int> fnPlayerUpdateSelf_;
	static RealFunc<void, void*, void*> fnPlayerOnAttack_;
	static RealFunc<void, void*, unsigned int&, bool> fnCharacterBioTakeBonus_;
	static RealFunc<bool, void*, unsigned int, unsigned int&, unsigned int&, bool, unsigned int, unsigned int, unsigned int> fnCharacterAttackTarget_;

	static RealFunc<float, void*, float> fnPlayerRegisterCombatTextHit_;
	static RealFunc<float, void*, float> fnPlayerRegisterCombatTextCrit_;
	static RealFunc<void, void*, unsigned int, unsigned int, float> fnGameEngineRegisterDamage_;
	static RealFunc<bool, void*, float, unsigned int&, unsigned int, unsigned int&> fnCombatManagerApplyDamage_;
	static RealFunc<bool, void*, unsigned int&, unsigned int&, unsigned int&> fnCombatManagerTakeDamage_;

	static RealFunc<int*, void*, unsigned int &> fnCombatAttribAccuExeDamage_;
	static RealFunc<void, void*, bool, bool, unsigned int&> fnCtrlPlayerStateDfltRequestMoveAction_;
	static RealFunc<void, void*, unsigned int&, unsigned int, unsigned int, unsigned int, bool> fnPlayerPostSpawnPet_;
	static RealFunc<void, void*> fnCharCharacterIsDying_;
};



