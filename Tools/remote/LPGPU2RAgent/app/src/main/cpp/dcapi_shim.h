/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef DCAPI_SHIM_H
#define DCAPI_SHIM_H

#include <stdint.h>
#include <string>

#include "DCAPI.h"

typedef int32_t (*Type_DcApi_Initialize)(uint32_t flags);
typedef int32_t (*Type_DcApi_Terminate)(uint32_t flags);
typedef int32_t (*Type_DcApi_GetCharacteristics)(CharacteristicsDefinition *definition);
typedef int32_t (*Type_DcApi_GetCounterSetDefinition)(uint32_t index, CounterSetDefinition *pDefinition);
typedef int32_t (*Type_DcApi_GetCounterSetCounterDefinition)(uint32_t csid, uint32_t id, CounterDefinition *pDefinition);
typedef int32_t (*Type_DcApi_GetUserBufferSizes)(uint32_t *bufferSizes, uint32_t numSizes);
typedef int32_t (*Type_DcApi_SetCounterSetState)(uint32_t csid, uint32_t enable_disable);
typedef int32_t (*Type_DcApi_SetCounterSetModeAndInterval)(uint32_t csid, eCollectionMode mode, uint32_t sampleInterval);
typedef int32_t (*Type_DcApi_SetCounterState)(uint32_t csid, uint32_t id, uint32_t enable_disable);
typedef int32_t (*Type_DcApi_SetUserBuffers)(uint8_t **buffers, uint32_t numBuffers);
typedef int32_t (*Type_DcApi_StartCollection)(uint32_t flags);
typedef int32_t (*Type_DcApi_PauseCollection)(uint32_t flags);
typedef int32_t (*Type_DcApi_StopCollection)(uint32_t flags);
typedef int32_t (*Type_DcApi_SetDataReadyCallback)(DCAPI_DATA_READY_CALLBACK pfnCallback, uint32_t flags, void* userData);

class DcApiShim {
public:
  DcApiShim();

  bool InitializeFunctionPointers(const std::string& libraryPath);

  int32_t  Initialize(uint32_t flags);
  int32_t  Terminate(uint32_t flags);
  int32_t  GetCharacteristics(CharacteristicsDefinition *definition);
  int32_t  GetCounterSetDefinition(uint32_t index, CounterSetDefinition *pDefinition);
  int32_t  GetCounterSetCounterDefinition(uint32_t csid, uint32_t id, CounterDefinition *pDefinition);
  int32_t  GetUserBufferSizes(uint32_t *bufferSizes, uint32_t numSizes);
  int32_t  SetCounterSetState(uint32_t csid, uint32_t enable_disable);
  int32_t  SetCounterSetModeAndInterval(uint32_t csid, eCollectionMode mode, uint32_t sampleInterval);
  int32_t  SetCounterState(uint32_t csid, uint32_t id, uint32_t enable_disable);
  int32_t  SetUserBuffers(uint8_t **buffers, uint32_t numBuffers);
  int32_t  StartCollection(uint32_t flags);
  int32_t  PauseCollection(uint32_t flags);
  int32_t  StopCollection(uint32_t flags);
  int32_t  SetDataReadyCallback(DCAPI_DATA_READY_CALLBACK pfnCallback, uint32_t flags, void* userData);

private:
  Type_DcApi_Initialize _Initialize;
  Type_DcApi_Terminate _Terminate;
  Type_DcApi_GetCharacteristics _GetCharacteristics;
  Type_DcApi_GetCounterSetDefinition _GetCounterSetDefinition;
  Type_DcApi_GetCounterSetCounterDefinition _GetCounterSetCounterDefinition;
  Type_DcApi_GetUserBufferSizes _GetUserBufferSizes;
  Type_DcApi_SetCounterSetState _SetCounterSetState;
  Type_DcApi_SetCounterSetModeAndInterval _SetCounterSetModeAndInterval;
  Type_DcApi_SetCounterState _SetCounterState;
  Type_DcApi_SetUserBuffers _SetUserBuffers;
  Type_DcApi_StartCollection _StartCollection;
  Type_DcApi_PauseCollection _PauseCollection;
  Type_DcApi_StopCollection _StopCollection;
  Type_DcApi_SetDataReadyCallback _SetDataReadyCallback;

};


#endif // header
