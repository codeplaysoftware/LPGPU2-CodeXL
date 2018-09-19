/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <string>

enum LogLevel
{
    LogNone,
    LogError      = (1<<1),
    LogInfo       = (1<<2),
    LogDebug      = (1<<3),
    LogVerbose    = (1<<4),
    LogDefault    = LogDebug | LogInfo | LogError,
    LogAll        = LogDebug | LogError | LogInfo | LogVerbose
};

extern LogLevel logLevel;

extern void InitializeLogger(const std::string& level);
extern void Log(LogLevel level, const char *format, ...);
extern void LogPrint(const char *format, ...);
extern LogLevel LogLevelFromString(const char *s);
extern std::string LogLevelToString(LogLevel level);

#ifdef __ANDROID__
#define BREAKPOINT
#else
#define BREAKPOINT { asm("int $3"); }
#endif

#endif // header
