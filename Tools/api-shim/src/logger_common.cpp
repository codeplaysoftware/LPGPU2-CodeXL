/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <stdio.h>
#include "logger.h"
#include "string_bitfield.h"

namespace {

typedef std::map<std::string, LogLevel> LogLevelLut;
LogLevelLut logLevelLut;

} // anonymous namespace

LogLevel logLevel = LogDefault;

LogLevel LogLevelFromString(const char *s)
{
  LogLevel ret =  GetStringBitField(s, logLevelLut);
  return ret;
}

void InitializeLogger(const std::string& level)
{
  logLevelLut["None"] = LogNone;
  logLevelLut["Error"] = LogError;
  logLevelLut["Info"] = LogInfo;
  logLevelLut["Debug"] = LogDebug;
  logLevelLut["Verbose"] = LogVerbose;
  logLevelLut["Default"] = LogDefault;
  logLevelLut["All"] = LogAll;

  if(level.size())
  {
    logLevel = LogLevelFromString(level.c_str());
  }
}

std::string LogLevelToString(LogLevel level)
{
  std::string ret;

  LogLevelLut::iterator iter = logLevelLut.begin();
  LogLevelLut::iterator end = logLevelLut.end();
  for(; iter != end; iter++)
  {
    if(LogAll == iter->second || LogDefault == iter->second)
    {
      continue;
    }
    else
    {
      if(iter->second & level)
      {
        ret += std::string(iter->first) + std::string(" ");
      }
    }
  }
  return ret;
}
