#pragma once
#include <stdio.h>

enum LogLevel
{
    LogCommon         = 0x0001,
    LogScreen         = 0x0002,
    LogDamageProcess  = 0x0004,
    LogDamageSkill    = 0x0008,
    LogSkillBase      = 0x0010,
    LogSkillDispel    = 0x0020,
    LogSkillRetal     = 0x0040,
    LogCharAttack     = 0x0080,
    LogUnknownSkill   = 0x0100,
    LogCombatMgr      = 0x0200,
    LogStackTrace     = 0x1000,
    LogDamageAccum    = 0x2000
};

#ifndef _RELEASE
#define LOGF(format, ...) Logger::Logf(format, ##__VA_ARGS__);
#define DLOG(x, format, ...) Logger::LevelLog(x, format, ##__VA_ARGS__);
#else
#define LOGF(format, ...)
#define DLOG(x, format, ...)
#endif

//#define SHOWDBG_LOG
#ifdef SHOWDBG_LOG
#define DBGLOG(format, ...) Logger::Logf(format, ##__VA_ARGS__);
#else
#define DBGLOG(format, ...)
#endif

//#define SHOWSKILL_DBG
#ifdef SHOWSKILL_DBG
#define SKILLLOG(format, ...) Logger::Logf(format, ##__VA_ARGS__);
#else
#define SKILLLOG(format, ...)
#endif

class Logger
{
public:
    static void SetLogLevel(unsigned level);
    static void LevelLog(unsigned level, const char *format, ...);
    static void Logf(const char *format, ...);

};

