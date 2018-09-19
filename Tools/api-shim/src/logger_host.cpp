/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <stdarg.h>
#include <stdio.h>
#include "logger.h"


void LogPrint(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  fflush(stdout);
}

void Log(LogLevel level, const char *format, ...)
{
  if(level & logLevel)
  {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
  }
}
