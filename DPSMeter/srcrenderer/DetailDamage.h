#pragma once
#include "BaseRender.h"
#include "Logger.h"

//===================================================================
//===================================================================
class MainUIController;

struct detailskillDamageData
{
    detailskillDamageData()
    {
        memset(this, 0, sizeof(*this));
    }

    unsigned int skillId_;

    float totalDamage_;
    float hitDamageTotal_;
    float critDamageTotal_;

    int hitCount_;
    int critCount_;

    char name_[IPCNAME_SIZE];

    bool operator()(detailskillDamageData const &a, detailskillDamageData const &b)
    {
        return (a.totalDamage_ > b.totalDamage_);
    }

    detailskillDamageData& operator = (const detailskillDamageData &rhs)
    {
        memcpy(this, &rhs, sizeof(*this));
        return *this;
    }
};

struct detailHpRecoveryData
{
    detailHpRecoveryData()
    {
        memset(this, 0, sizeof(*this));
    }
    hpRecoveryData hpRecovery_;
    unsigned count_;
  
    bool operator()(detailHpRecoveryData const &a, detailHpRecoveryData const &b)
    {
        return (a.hpRecovery_.hp_ > b.hpRecovery_.hp_);
    }
};

struct detailAttackDamageData
{
    unsigned type_;
    float damage_;
    unsigned count_;

    detailAttackDamageData() : type_(0), damage_(0.0f), count_(0) {}

    detailAttackDamageData& operator = (const detailAttackDamageData& rhs)
    {
        type_ = rhs.type_;
        damage_ = rhs.damage_;
        count_ = rhs.count_;
        return *this;
    }

    bool operator()(detailAttackDamageData const &a, detailAttackDamageData const &b)
    {
        return (a.damage_ > b.damage_);
    }
};

class DetailDamage : public BaseRender
{
    enum DetailType
    {
        DetailDPS,
        DetailSkills,
        DetailDamageDealt,
        DetailDamageTaken,
        DetailHpRecovery
    };
public:
    DetailDamage();

    void SetMainUIController(MainUIController *mainController);
    void ClearLists();
    void InputDPSstat(const dpsStatData &stats);
    void InputHpRecoveryMsg(const hpRecoveryData &hpRecovery);
    void InputAttackDamage(const attackDamageData &attackDamage);
    void InputSkillDamage(const skillDamageData &skillDamage);

    virtual void ShowWin(bool &showWindow);
    virtual void Draw(const char* title, bool* p_open = NULL);

private:
    void PrepSkillDamageForView();
    void PrepAttackDamageForView();
    void PrepHpRecoveryForView();
    void InsertCollapsableTree(DetailType detail);

private:
    MainUIController *mainUIController_;
    float dps_;
    float totalDamage_;
    float maxDamage_;
    float avgDamage_;
    unsigned int hitCount_;
    unsigned int duration_;

    float totalSkillDamage_;
    std::vector<detailskillDamageData> skillDamageList_;
    float totalDealtAttackDamage_;
    std::vector<detailAttackDamageData> dealtAttackDamageList_;
    float totalTakenAttackDamage_;
    std::vector<detailAttackDamageData> takenAttackDamageList_;

    float totalHpRecovery_;
    std::vector<detailHpRecoveryData> hpRecoveryIds_;
    //std::map<unsigned, detailHpRecoveryData> hpRecoveryList_;

    float lastDetailDmg_;
    float lastSkillDmg_;

};

