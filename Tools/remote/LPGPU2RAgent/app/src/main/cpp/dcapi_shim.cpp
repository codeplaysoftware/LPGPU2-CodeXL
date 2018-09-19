/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <dlfcn.h>
#include <android/log.h>
#include "dcapi_shim.h"


// #define SYMBOL_LOADED(name) (_##name = (Type_DcApi_##name)dlsym(library, #name))

#define LOAD_SYMBOL_OR_BREAK(name) {                                 \
    if(NULL == (_##name = (Type_DcApi_##name)dlsym(library, "DCAPI_" #name))) \
    {                                                                \
      __android_log_print(ANDROID_LOG_ERROR, "RAGENT: Socket", "Cannot load function'" #name "'\n");            \
      break;                                                         \
    }                                                                \
  }

DcApiShim::DcApiShim()
{
  _Initialize = NULL;
  _Terminate = NULL;
  _GetCharacteristics = NULL;
  _GetCounterSetDefinition = NULL;
  _GetCounterSetCounterDefinition = NULL;
  _GetUserBufferSizes = NULL;
  _SetCounterSetState = NULL;
  _SetCounterSetModeAndInterval = NULL;
  _SetCounterState = NULL;
  _SetUserBuffers = NULL;
  _StartCollection = NULL;
  _PauseCollection = NULL;
  _StopCollection = NULL;
  _SetDataReadyCallback = NULL;
}

bool DcApiShim::InitializeFunctionPointers(const std::string& libraryPath)
{
  bool allLoaded = false;

  void* library = dlopen(libraryPath.c_str(), RTLD_LOCAL | RTLD_LAZY);

  if(library)
  {
    __android_log_print(ANDROID_LOG_INFO, "RAGENT: Socket", "          DCAPI: %s\n", libraryPath.c_str());
  }
  else
  {
    __android_log_print(ANDROID_LOG_ERROR, "RAGENT: Socket", "          DCAPI: Cannot open: %s\n", libraryPath.c_str());
    return false;
  }

  do {
    LOAD_SYMBOL_OR_BREAK(Initialize);

    LOAD_SYMBOL_OR_BREAK(Terminate);

    LOAD_SYMBOL_OR_BREAK(GetCharacteristics);

    LOAD_SYMBOL_OR_BREAK(GetCounterSetDefinition);

    LOAD_SYMBOL_OR_BREAK(GetCounterSetCounterDefinition);

    LOAD_SYMBOL_OR_BREAK(GetUserBufferSizes);

    LOAD_SYMBOL_OR_BREAK(SetCounterSetState);

    LOAD_SYMBOL_OR_BREAK(SetCounterSetModeAndInterval);

    LOAD_SYMBOL_OR_BREAK(SetCounterState);

    LOAD_SYMBOL_OR_BREAK(SetUserBuffers);

    LOAD_SYMBOL_OR_BREAK(StartCollection);

    LOAD_SYMBOL_OR_BREAK(PauseCollection);

    LOAD_SYMBOL_OR_BREAK(StopCollection);

    LOAD_SYMBOL_OR_BREAK(SetDataReadyCallback);

    allLoaded = true;
  } while(false);

  return allLoaded;
}

int32_t DcApiShim::Initialize(uint32_t flags)
{
  if(_Initialize)
    return _Initialize(flags);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::Terminate(uint32_t flags)
{
  if(_Terminate)
    return _Terminate(flags);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}
int32_t DcApiShim::GetCharacteristics(CharacteristicsDefinition *definition)
{
  if(_GetCharacteristics)
    return _GetCharacteristics(definition);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::GetCounterSetDefinition(uint32_t index, CounterSetDefinition *pDefinition)
{
  if(_GetCounterSetDefinition)
    return _GetCounterSetDefinition(index, pDefinition);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::GetCounterSetCounterDefinition(uint32_t csid, uint32_t id, CounterDefinition *pDefinition)
{
  if(_GetCounterSetCounterDefinition)
    return _GetCounterSetCounterDefinition(csid, id, pDefinition);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::SetCounterSetState(uint32_t csid, uint32_t enable_disable)
{
  if(_SetCounterSetState)
    return _SetCounterSetState(csid, enable_disable);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::GetUserBufferSizes(uint32_t *bufferSizes, uint32_t numSizes)
{
  if(_GetUserBufferSizes)
    return _GetUserBufferSizes(bufferSizes, numSizes);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::SetCounterSetModeAndInterval(uint32_t csid, eCollectionMode mode, uint32_t sampleInterval)
{
  if(_SetCounterSetModeAndInterval)
    return _SetCounterSetModeAndInterval(csid, mode, sampleInterval);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::SetCounterState(uint32_t csid, uint32_t id, uint32_t enable_disable)
{
  if(_SetCounterState)
    return _SetCounterState(csid, id, enable_disable);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::SetUserBuffers(uint8_t **buffers, uint32_t numBuffers)
{
  if(_SetUserBuffers)
    return _SetUserBuffers(buffers, numBuffers);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::StartCollection(uint32_t flags)
{
  if(_StartCollection)
    return _StartCollection(flags);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::PauseCollection(uint32_t flags)
{
  if(_PauseCollection)
    return _PauseCollection(flags);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::StopCollection(uint32_t flags)
{
  if(_StopCollection)
    return _StopCollection(flags);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}

int32_t DcApiShim::SetDataReadyCallback(DCAPI_DATA_READY_CALLBACK pfnCallback, uint32_t flags, void* userData)
{
  if(_SetDataReadyCallback)
    return _SetDataReadyCallback(pfnCallback, flags, userData);
  else
    return DCAPI_SHIM_UNINITIALIZED;
}
