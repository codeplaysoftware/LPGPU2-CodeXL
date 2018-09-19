/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#pragma once

#if defined(__linux__)

#include <limits>
#include <android/log.h>
#include "time.h"

inline void LogcatOut(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  __android_log_vprint(ANDROID_LOG_INFO, "lpgpu2-shim", format, args);
  va_end(args);
}

inline uint64_t GetTimeMicroSeconds()
{
  timespec t;
  static int firsttime = 1;
  static timespec begin_t;
  static uint64_t begin;
  uint64_t time;
  clock_gettime(CLOCK_MONOTONIC, &t);
  time = (1e3 * t.tv_sec) + (t.tv_nsec / 1e6);
  if (firsttime)
  {
    begin_t = t;
    begin = time;
    firsttime = 0;
  }

  if (time < begin)
  {
    LogcatOut("GetTimeMicroSeconds Error - time hasn't incremented.  New time = %lld  (%d sec, %d nsec), Original time = %lld (%d sec, %d nsec)",
        time, t.tv_sec, t.tv_nsec, begin, begin_t.tv_sec, begin_t.tv_nsec);
  }
  return time;
}

inline uint64_t GetTimeMilliSeconds()
{
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (1e6 * t.tv_sec) + (t.tv_nsec / 1e3);
}

inline uint64_t GetTimeNanoSeconds()
{
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (1e9 * t.tv_sec) + (t.tv_nsec);
}

#elif defined(WIN32)

inline uint64_t GetTimeMicroSeconds()
{

# error add windows QueryPerformanceCounter

}

#else

# error unsupported

#endif
