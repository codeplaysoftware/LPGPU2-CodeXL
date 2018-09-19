/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
// TestDCAPI.cpp : Defines the entry point for the console application.
//
#include <stdint.h>
#include <stdlib.h>
#include "DCAPI.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <android/log.h>
#include <vector>

#include "get_time.h"
#include "SimCounter.h"

//
// dummy api implementation
//

#define HARDWARE_ID 0x1234

#define NUM_COUNTERSETS         4
#define NUM_COUNTERS_PER_SET    4

#define BLOB_SIZE       0


DCAPI_DATA_READY_CALLBACK pfnDataReadyCallback = NULL;

const float frequency[NUM_COUNTERSETS] =
{
    1,
    0.5,
    0.1
};

const float step = 0.1; // will be overridden once the sampleInterval is known.

std::vector<SimCounter<>> simCounters[NUM_COUNTERSETS]
{
  {
    RndCounter<>(step,1,200),
    SqWaveCounter<>(step,1,200),
    SinCounter<>(step,0,200,M_PI/1),
    ExpCounter<>(step,0,400)
  },
  {
    RndCounter<>(step,1,300),
    SqWaveCounter<>(step,1,300),
    SinCounter<>(step, 0,300,M_PI/2),
    ExpCounter<>(step,0,600)
  },
  {
    RndCounter<>(step,1,400),
    SqWaveCounter<>(step,1,400),
    SinCounter<>(step, 0,400,M_PI/3),
    ExpCounter<>(step,0,800)
  },
};

CounterSetDefinition aCSDef[NUM_COUNTERSETS] = {
    //                                                                          MinSampleInterval (ns)
    //                                                                                  |
    //                                                                                  V
    { 0x1001, "CounterSet 0", "Waves at 1Hz",   NUM_COUNTERS_PER_SET, cHigh, (uint64_t)1e8,  0, mTime, mNone, 0 },
    { 0x1002, "CounterSet 1", "Waves at 0.5Hz",  NUM_COUNTERS_PER_SET, cHigh, (uint64_t)1e8, 0, mTime, mNone, 0 },
    { 0x1003, "CounterSet 2", "Waves at 0.1Hz", NUM_COUNTERS_PER_SET, cHigh, (uint64_t)1e8, 0, mTime, mNone, 0 },
    { 0x1004, "User Counters", "User Counters", NUM_COUNTERS_PER_SET, cHigh, (uint64_t)1e8, 0, mTime, mNone, 0 }
};


CounterDefinition someCounters[NUM_COUNTERSETS][NUM_COUNTERS_PER_SET] = {
  {
    { 0x2001, "RndCounter", "RndCounter",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x2002, "SqWaveCounter", "SqWaveCounter", cCounter, tInteger, uPowerWatts, 0 },
    { 0x2003, "SinCounter", "SinCounter",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x2004, "ExpCounter", "ExpCounter",    cCounter, tInteger, uPowerWatts, 0 }
  },
  {
    { 0x2006, "RndCounter", "RndCounter",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x2007, "SqWaveCounter", "SqWaveCounter", cCounter, tInteger, uPowerWatts, 0 },
    { 0x2008, "SinCounter", "SinCounter",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x2009, "ExpCounter", "ExpCounter",    cCounter, tInteger, uPowerWatts, 0 }
  },
  {
    { 0x200b, "RndCounter", "RndCounter",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x200c, "SqWaveCounter", "SqWaveCounter", cCounter, tInteger, uPowerWatts, 0 },
    { 0x200d, "SinCounter", "SinCounter",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x200e, "ExpCounter", "ExpCounter",    cCounter, tInteger, uPowerWatts, 0 }
  },
  {
    { 0x3000, "User Affinity", "User Affinity",    cCounter, tInteger, uPowerWatts, 0 },
    { 0x3001, "User Counter 0", "User Counter 0",  cCounter, tInteger, uPowerWatts, 0 },
    { 0x3002, "User Counter 1", "User Counter 1",  cCounter, tInteger, uPowerWatts, 0 },
    { 0x3003, "User Counter 2", "User Counter 2",  cCounter, tInteger, uPowerWatts, 0 }
  }
};

void LogPrint(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  __android_log_vprint(ANDROID_LOG_INFO, "dcapi-test", format, args);
  va_end(args);
}


uint32_t numCounterSetsEnabled = 0;

bool userBuffersSet = false;
bool usingUserBuffers = false;
bool initialized = false;

int state = rStopped;

pthread_t callbackThread[NUM_COUNTERSETS] = {0, 0};

typedef struct _tag_threadParams {
  DCAPI_DATA_READY_CALLBACK      pfnDataReadyCallback;
  void                           *userData;
  uint8_t                        *pDataBuffer;
  uint32_t                       nBufferSize;
  uint32_t                       samplingInterval;
  uint32_t                       nCounterSet;
  CounterDefinition              *pCounters;
  uint32_t                       nNumCounters;
  int32_t                        nMode;
  std::vector<SimCounter<>>      counters;
} DCAPIThreadParams;

static DCAPIThreadParams dcapiThreadParams[NUM_COUNTERSETS] =
{
    { NULL, NULL, NULL, 0, 200000, 0, &someCounters[0][0], NUM_COUNTERS_PER_SET, 0, simCounters[0]},
    { NULL, NULL, NULL, 0, 1000000, 0, &someCounters[1][0], NUM_COUNTERS_PER_SET, 0, simCounters[1]},
    { NULL, NULL, NULL, 0, 1000000, 0, &someCounters[2][0], NUM_COUNTERS_PER_SET, 0, simCounters[2]},
    { NULL, NULL, NULL, 0, 1000000, 0, &someCounters[3][0], NUM_COUNTERS_PER_SET, 0, simCounters[3]}
};

void *dcapiWorkerThread(void *param);
int32_t GetDataBufferSizeRequired(uint32_t nCounter);

namespace
{
  class Lock
  {
  public:
    void init()
    {
      pthread_mutex_init(&_lock, NULL);
    }
    void destroy()
    {
      pthread_mutex_destroy(&_lock);
    }
    void lock()
    {
      pthread_mutex_lock(&_lock);
    }
    void unlock()
    {
      pthread_mutex_unlock(&_lock);
    }
  private:
    pthread_mutex_t _lock;
  };
  Lock lock;
}

//
// dummy api implementation - end
//


DCAPI_EXPORT int32_t DCAPI_Initialize(uint32_t /*flags*/)
{
  int32_t rv = DCAPI_SUCCESS;

  lock.init();
  initialized = true;

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_Terminate(uint32_t /*flags*/)
{
  int32_t rv = DCAPI_SUCCESS;

  lock.destroy();
  initialized = false;

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_GetCharacteristics(CharacteristicsDefinition *pCharacteristics)
{
  int32_t rv = DCAPI_SUCCESS;

  LogPrint("DCAPI_GetCharacteristics \n");

  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (pCharacteristics)
  {
    pCharacteristics->MajorVersion        = DCAPI_VERSION_MAJOR;
    pCharacteristics->MinorVersion        = DCAPI_VERSION_MINOR;
    pCharacteristics->HardwareID          = HARDWARE_ID;
    pCharacteristics->NumCounterSets      = NUM_COUNTERSETS;
    pCharacteristics->BlobSize            = BLOB_SIZE;
    pCharacteristics->UserBufferSupported = true;
  }
  else
  {
    rv = DCAPI_INVALID_PTR_ARGUMENT;
  }

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_GetCounterSetDefinition(uint32_t index, CounterSetDefinition *pDefinition)
{
  int32_t rv = DCAPI_SUCCESS;

  LogPrint("DCAPI_GetCounterSetDefinition \n");

  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (pDefinition)
  {
    if (NUM_COUNTERSETS > index)
    {
      *pDefinition = aCSDef[index];
    }
    else
    {
      rv = DCAPI_BAD_COUNTERSET_ID;
    }
  }
  else {
    rv = DCAPI_INVALID_PTR_ARGUMENT;
  }

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_GetCounterSetCounterDefinition(uint32_t csid, uint32_t idx, CounterDefinition *pDefinition)
{
  int32_t rv = DCAPI_BAD_COUNTERSET_ID;

  LogPrint("DCAPI_GetCounterSetCounterDefinition(csid: %d, idx: %d) \n", csid, idx);
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (pDefinition)
  {

    for (int cs = 0; cs < NUM_COUNTERSETS; cs++ )
    {
      if (csid == aCSDef[cs].ID)
      {
        if (idx < aCSDef[cs].NumCounters)
        {
          *pDefinition = someCounters[cs][idx];
          rv = DCAPI_SUCCESS;
        }
        else
        {
          rv = DCAPI_BAD_COUNTER_INDEX;
        }
      }
    }
  }
  else
  {
    rv = DCAPI_INVALID_PTR_ARGUMENT;
  }

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_GetUserBufferSizes(uint32_t *bufferSizes, uint32_t numSizes)
{
  int numCountersEnabled = 0;
  int index = 0;

  LogPrint("DCAPI_GetUserBufferSizes \n");
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (bufferSizes == NULL)
  {
    return DCAPI_INVALID_PTR_ARGUMENT;
  }
  if (numSizes != numCounterSetsEnabled)
  {
    return DCAPI_INVALID_SIZE;
  }

  for ( unsigned int cs = 0; cs < NUM_COUNTERSETS; cs ++ )
  {
    if (aCSDef[cs].Enabled)
    {
      numCountersEnabled = 0;
      for ( unsigned int c = 0 ; c < aCSDef[cs].NumCounters; c ++ )
      {
        if ( someCounters[cs][c].Enabled )
        {
          numCountersEnabled++;
        }
      }
      // Timestamp + numCounters + ( numCountersEnabled * (id + data + BLOB_SIZE ))
      bufferSizes[index++] = GetDataBufferSizeRequired(numCountersEnabled);//8 + 4 + (numCountersEnabled * (4 + 4 + BLOB_SIZE) );
    }
  }

  return DCAPI_SUCCESS;
}

DCAPI_EXPORT int32_t DCAPI_SetCounterSetState(uint32_t csid, uint32_t enable_disable)
{
  LogPrint("DCAPI_SetCounterSetState \n");
  int32_t rv = DCAPI_BAD_COUNTERSET_ID;
  userBuffersSet = false;
  numCounterSetsEnabled = 0;
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  for (int cs = 0; cs < NUM_COUNTERSETS - 1; cs++ ) // Ignore the user counterSet, which is last.
  {
      if (csid == aCSDef[cs].ID)
      {
        aCSDef[cs].Enabled = enable_disable;
        for(uint c = 0; c < aCSDef[cs].NumCounters; c++)
        {
          someCounters[cs][c].Enabled = enable_disable;
        }
        rv = DCAPI_SUCCESS;
      }
      if (aCSDef[cs].Enabled)
      {
        numCounterSetsEnabled++;
      }
  }

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_SetCounterSetModeAndInterval(uint32_t csid, eCollectionMode mode, uint32_t sampleInterval)
{
  LogPrint("DCAPI_SetCounterSetModeAndInterval \n");

  int32_t rv = DCAPI_BAD_COUNTERSET_ID;
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  for (int cs = 0; cs < NUM_COUNTERSETS -1 ; cs++ ) // Ignore the user counterSet, which is last.
  {
    if (csid == aCSDef[cs].ID)
    {
      if ((aCSDef[cs].SupportedModes & mode) == 0)
      {
        rv = DCAPI_INVALID_MODE;
      }
      else if (sampleInterval < aCSDef[cs].MinimumSamplingInterval)
      {
        rv = DCAPI_INVALID_RATE;
      }
      else
      {
        aCSDef[cs].CurrentMode = mode;
        aCSDef[cs].CurrentSamplingInterval = sampleInterval;
        // Update the step for the counters
        for (int c = 0; c < NUM_COUNTERS_PER_SET; c++)
        {
          simCounters[cs][c].updateDelta(frequency[cs], sampleInterval);
        }
        rv = DCAPI_SUCCESS;
      }
    }
  }
  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_SetCounterState(uint32_t csid, uint32_t id, uint32_t enable_disable)
{
  int32_t rv = DCAPI_BAD_COUNTERSET_ID;

  LogPrint("DCAPI_SetCounterState \n");

  bool found = false;
  userBuffersSet = false;
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  for (int cs = 0; cs < NUM_COUNTERSETS - 1; cs++ ) // Ignore the user counterSet, which is last.
  {
      if (csid == aCSDef[cs].ID)
      {
        for(uint c = 0; c < aCSDef[cs].NumCounters; c++)
        {
          if(someCounters[cs][c].ID == id)
          {
            someCounters[cs][c].Enabled = enable_disable;
            found = true;
          }
        }
        rv = DCAPI_SUCCESS;
      }
  }

  if(!found && (rv == DCAPI_SUCCESS))
  {
    rv = DCAPI_BAD_COUNTER_INDEX;
  }

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_SetUserBuffers(uint8_t **buffers, uint32_t numBuffers)
{
  int index = 0;
  LogPrint("DCAPI_SetUserBuffers \n");
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (buffers == NULL)
  {
    return DCAPI_INVALID_PTR_ARGUMENT;
  }
  if (numBuffers != numCounterSetsEnabled)
  {
    return DCAPI_INVALID_SIZE;
  }

  for ( unsigned int cs = 0; cs < NUM_COUNTERSETS; cs ++ )
  {
    if (aCSDef[cs].Enabled)
    {
      if (buffers[index] == NULL)
      {
        return DCAPI_INVALID_PTR_ARGUMENT;
      }
      dcapiThreadParams[cs].pDataBuffer = buffers[index++];
    }
  }

  userBuffersSet = true;
  return DCAPI_SUCCESS;
}

DCAPI_EXPORT int32_t DCAPI_StartCollection(uint32_t flags)
{
  int32_t rv = DCAPI_SUCCESS;

  LogPrint("DCAPI_StartCollection \n");
  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  for (int cs = 0 ; cs < NUM_COUNTERSETS; cs++)
  {
    if (aCSDef[cs].Enabled && !aCSDef[cs].CurrentMode)
    {
      return DCAPI_COUNTERSET_UNINITIALIZED;
    }
  }
  if ( (flags == fUserBuffers) && !userBuffersSet )
  {
    return DCAPI_INVALID_CONFIGURATION;
  }
  else
  {
    usingUserBuffers = (flags == fUserBuffers);
  }

  if (state != rStopped)
  {
    rv = DCAPI_COLLECTION_ALREADY_ACTIVE;
  }
  else
  {
    for (int cs = 0 ; cs < NUM_COUNTERSETS - 1; cs++) // Ignore the user counterSet, which is last.
    {
      if (aCSDef[cs].Enabled)
      {
        LogPrint("DCAPI_StartCollection Set %d\n", cs);
        lock.lock();
        dcapiThreadParams[cs].nBufferSize = GetDataBufferSizeRequired(aCSDef[cs].NumCounters) * 2;
        if (!usingUserBuffers)
        {
          dcapiThreadParams[cs].pDataBuffer = (uint8_t*)calloc(1, dcapiThreadParams[cs].nBufferSize);
        }
        dcapiThreadParams[cs].samplingInterval = aCSDef[cs].CurrentSamplingInterval;
        dcapiThreadParams[cs].nMode = 0;
        dcapiThreadParams[cs].counters = simCounters[cs];
        lock.unlock();
        if ( 0 == pthread_create(&callbackThread[cs], NULL, dcapiWorkerThread, &dcapiThreadParams[cs]) )
        {
          state = rRunning;
        }
      }
    }
  }
  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_PauseCollection(uint32_t /*flags*/)
{
  int32_t rv = DCAPI_SUCCESS;

  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (state == rStopped)
  {
    rv = DCAPI_COLLECTION_NOT_ACTIVE;
  }
  else if (state == rRunning)
  {
    state = rPaused;
  }
  else if (state == rPaused)
  {
    state = rRunning;
  }

  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_StopCollection(uint32_t /*flags*/)
{
  int32_t rv = DCAPI_SUCCESS;

  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  if (state == rStopped)
  {
    rv = DCAPI_COLLECTION_NOT_ACTIVE;
  }
  else
  {

    lock.lock();
    for (int cs = 0; cs < NUM_COUNTERSETS; cs++)
    {
      dcapiThreadParams[cs].nMode = -1;
    }
    state = rStopped;
    lock.unlock();

    for (int cs = 0 ; cs < NUM_COUNTERSETS; cs++)
    {
      if (aCSDef[cs].Enabled &&  (0 == pthread_join(callbackThread[cs], NULL)) )
      {
        // all good
        if (!usingUserBuffers)
        {
          free(dcapiThreadParams[cs].pDataBuffer);
        }
      }
    }
  }
  return (rv);
}

DCAPI_EXPORT int32_t DCAPI_SetDataReadyCallback(DCAPI_DATA_READY_CALLBACK pfnCallback, uint32_t /*flags*/, void* userData)
{
  int32_t rv = DCAPI_SUCCESS;

  LogPrint("DCAPI_SetDataReadyCallback \n");

  if (!initialized)
  {
    return DCAPI_UNINITIALIZED;
  }

  lock.lock();
  for (int cs = 0; cs < NUM_COUNTERSETS; cs++)
  {
    dcapiThreadParams[cs].userData = userData;
    dcapiThreadParams[cs].pfnDataReadyCallback = pfnCallback;
  }
  lock.unlock();

  return (rv);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Internal functionality
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DCAPI_HEADER_SIZE (8 + 4)// Timestamp + numCounters
#define DCAPI_COUNTER_SIZE (4 + 4 + BLOB_SIZE) // Id + Value + BLOB_SIZE

int32_t GetDataBufferSizeRequired(uint32_t nCounter)
{
  return ( DCAPI_HEADER_SIZE + ( nCounter * DCAPI_COUNTER_SIZE ) );
}

static uint64_t (* const GetTime)(void) = GetTimeNanoSeconds;

void thread_exit_handler(int /*sig*/)
{
  pthread_exit(0);
}

// very simplistic
// there is no attempt to buffer, so callback frequency is higher than it would be otherwise

void *dcapiWorkerThread(void *param)
{
  DCAPIThreadParams *tParams = (DCAPIThreadParams *)param;
  uint32_t done = 0;
  uint32_t interval = 0;

  lock.lock();
  // sleep for our sampling period (in nanoseconds)
  interval = tParams->samplingInterval;
  lock.unlock();


  // until thread is terminated
  while ( done == 0)
  {
    // sleep for our sampling period (convert to microseconds)
    usleep(interval / 1000);
    lock.lock();
    // first check and see if we want to exit
    if ( tParams->nMode == -1 )
    {
      // all done
      done = 1;
    }
    else if (state == rRunning)
    {
      // now notify caller (if there is one)
      if ( tParams->pfnDataReadyCallback ){

        // generate our data
        uint32_t *pDataPtr = (uint32_t*)tParams->pDataBuffer;

        // first time stamp
        uint64_t t = GetTime();
        *((uint64_t*)pDataPtr) = t;
        pDataPtr += sizeof(uint64_t)/sizeof(uint32_t);

        int numCountersEnabled = 0;
        for ( unsigned int i = 0 ; i < tParams->nNumCounters ; i ++ )
        {
          if (tParams->pCounters[i].Enabled)
          {
            numCountersEnabled++;
          }
        }
        // second number of counters to read
        *pDataPtr++ = numCountersEnabled;

        // then for each counter
        for ( unsigned int i = 0 ; i < tParams->nNumCounters ; i ++ ){
          // is it enabled
          if ( tParams->pCounters[i].Enabled  )
          {
            // Counter ID
            *pDataPtr++ = tParams->pCounters[i].ID;
            // Counter Data
            *pDataPtr++ = tParams->counters[i]();
            // Binary Blob. This API has no binary blob data, but if it did:
            /*
            *((BinaryBlobStruct*)pDataPtr) = binaryblobdata;
            pDataPtr += sizeof(BinaryBlobStruct)/sizeof(pDataPtr);
            */
          }
        }
        tParams->pfnDataReadyCallback(tParams->pDataBuffer,
                                      (uint8_t*)pDataPtr - tParams->pDataBuffer,
                                      DCAPI_DCT_DATA,
                                      tParams->userData);
      }
    }
    lock.unlock();
  }

  return ( NULL );
}
