#pragma once
#include <vector>
#include "Mutex.h"
#include "Logger.h"

//=============================================================================
//=============================================================================
#define IPCNAME_SIZE 24
#define DETAIL_SIZE 10

enum DataMsgType
{
  None,
  
  SetHwnWindow,
  ShowUI,
  InputKey,

  OADAstats,
  DPSstats,

  PLayerInWorld,
  PlayerSkillDamage,
  AttackDamage,
  HPRecovery,

  ClearSkillBuffs,
  ActiveSkillBuff,
  ActiveSkillBuffRemove,
  SkillBuffCooldown,
  SkillBuffCooldownRemove,

  DispelBuffs,
  
  DataMsgEnd
};

struct OADAData
{
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }

  unsigned int oa_;
  unsigned int da_;
  float pth_;
  float ptc_;
  float ptbh_;
  float ptbc_;
};
struct detailDamageData
{
  unsigned int type_;
  float damage_;
};
struct attackDamageData
{
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }

  bool playerDealingDamage_;
  void *attackerPtr;
  bool isCrit;
  char name_[IPCNAME_SIZE];

  detailDamageData details_[DETAIL_SIZE];
};

struct dpsStatData
{
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  enum { typeDps = 1, typeDamageDealt = 2, typeDamageTaken = 4, typeHp = 8 };

  unsigned types_;
  float dps_;
  float damageDealt_;
  float damageTaken_;
  float hp_;
  unsigned int duration_;
};
struct skillDamageData
{
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }

  unsigned int skillId_;
  float damage_;
  bool isCrit_;
  char name_[IPCNAME_SIZE];
};
struct hpRecoveryData
{
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }
  void Copy(const hpRecoveryData& rhs)
  {
    memcpy(this, &rhs, sizeof(*this));
  }

  unsigned int typeId_;
  float hp_;
  float hpPerSec_;
  char name_[IPCNAME_SIZE];
};

struct skillBuffMsgData
{
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }

  unsigned int skillId_;
  int timeRemaining_;
  int startTime_;
  char name_[IPCNAME_SIZE];
};

struct IPCData
{
  IPCData() : msg_(0), value_(0)
  {
    memset(&u_, 0, sizeof(u_));
  }

  unsigned int msg_;
  unsigned int value_;

  union
  {
    OADAData oada_;
    attackDamageData attackDamage_;
    dpsStatData dpsStats_;
    hpRecoveryData hpRecovery_;
    skillDamageData skillDamage_;
    skillBuffMsgData skillBuff_;
  } u_;

  IPCData& operator = (const IPCData &rhs)
  {
      memcpy(this, &rhs, sizeof(IPCData));
      return *this;
  }
};

class IPCMessage
{
public:
  static void SendShortData(DataMsgType type, unsigned int val);
  static void SendData(IPCData& data);
  static void SendData(std::vector<IPCData>& msgList);
  static void ClearQueue();
};

class MessageHandler
{
public:
  static MessageHandler& GetQueue();

  void QueueMessage(IPCData& messageData);
  void QueueMessage(std::vector<IPCData> &msgList);
  void GetMessages(std::vector<IPCData> &veclist);
  void ClearQueue();

protected:
  MessageHandler() {}

private:
  static MessageHandler sMessageHandler_;

  Mutex mutexMessageLock_;
  std::vector<IPCData> messageQueue_;

};