/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>
#include <unistd.h>
#include "logger.h"

class Lock
{
public:
  void init()
  {
    pthread_mutex_init(&_lock, &mutexattr);
  }
  void destroy()
  {
    pthread_mutex_destroy(&_lock);
  }
  void lock()
  {
    static bool sleeping  = false;
    while (pthread_mutex_trylock(&_lock) != 0)
    {
      if (!sleeping)
      {
        Log(LogVerbose, "Locked. Sleeping \n");
        sleeping = true;
      }
      usleep(100);
    }
    sleeping = false;
  }
  void unlock()
  {
    pthread_mutex_unlock(&_lock);
  }
private:
  pthread_mutexattr_t mutexattr = PTHREAD_MUTEX_ERRORCHECK;
  pthread_mutex_t _lock;
};

#endif // header
