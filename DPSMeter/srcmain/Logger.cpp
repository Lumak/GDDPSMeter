#include <windows.h>
#include <string>
#include <sstream>
#include <debugapi.h>

#include "Logger.h"


static unsigned sLevel = 0;

void Logger::SetLogLevel(unsigned level)
{
    sLevel = level;
}

void Logger::LevelLog(unsigned level, const char *format, ...)
{
    char buf[2048];

    if (level & sLevel)
    {
        va_list args;
        va_start(args, format);
        vsprintf_s(buf, format, args);
        va_end(args);

        OutputDebugString(buf);
    }
}

void Logger::Logf(const char *format, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, format);
    vsprintf_s(buf, format, args);
    va_end(args);

    OutputDebugString(buf);
}

std::string number_fmt(unsigned long long n, char sep = ',') 
{
  std::stringstream fmt;
  fmt << n;
  std::string s = fmt.str();
  s.reserve(s.length() + s.length() / 3);

  // loop until the end of the string and use j to keep track of every
  // third loop starting taking into account the leading x digits (this probably
  // can be rewritten in terms of just i, but it seems more clear when you use
  // a seperate variable)
  for (unsigned int i = 0, j = 3 - s.length() % 3; i < s.length(); ++i, ++j)
    if (i != 0 && j % 3 == 0)
      s.insert(i++, 1, sep);

  return s;
}
