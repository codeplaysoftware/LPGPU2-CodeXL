/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "lpgpu2_api.h"
#include "logger.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>



extern "C" {

extern DLL_PUBLIC int lpgpu2_StartCapture(LPGPU2_CAPTURE *pCapture)
{
  return ShimStartCapture(pCapture);
}

extern DLL_PUBLIC int lpgpu2_StopCapture(LPGPU2_CAPTURE *pCapture)
{
  return ShimStopCapture(pCapture);
}

extern DLL_PUBLIC int lpgpu2_TerminateFrame(void)
{
  return ShimTerminateFrame();
}

extern DLL_PUBLIC int lpgpu2_AnnotateCapture(LPGPU2_ANNOTATION *pAnnotation)
{
  return ShimAnnotateCapture(pAnnotation);
}

extern DLL_PUBLIC int lpgpu2_TrackResource(LPGPU2_RESOURCE_TYPE *pType, LPGPU2_RESOURCE_FLAGS *pFlags, uint64_t resource)
{
  return ShimTrackResource(pType, pFlags, resource);
}

extern DLL_PUBLIC int lpgpu2_GetVersion(float *version)
{
  return ShimGetVersion(version);
}

extern DLL_PUBLIC int lpgpu2_AssociateShaderMetaData(uint api, uint64_t shaderID, uint shader_type, uint MetaDataType, uint length, void *pBlob)
{
  return ShimStoreShaderMetaData(api, shaderID, shader_type, MetaDataType, length, (char *)pBlob);
}

extern DLL_PUBLIC int lpgpu2_LogAffinityCounter(void)
{
  unsigned cpu;

  pid_t tid;

  tid = syscall(SYS_gettid);

  if (syscall(__NR_getcpu, &cpu, NULL, NULL) < 0)
  {
     return LPGPU2_API_INIT_ERROR;
  }
  else
  {
    return ShimStoreAffinity(LPGPU2_USER_COUNTER_AFFINITY, cpu);
  }
}

extern DLL_PUBLIC int lpgpu2_LogUserCounter(uint Id, uint32_t value)
{
  if ((Id < LPGPU2_USER_COUNTER_0) || (Id > LPGPU2_USER_COUNTER_5))
  {
    return LPGPU2_API_ILLEGAL_PARAMETER;
  }
  else
  {
    return ShimStoreUserCounter(Id, value);
  }
}

extern DLL_PUBLIC int lpgpu2_SetThreadAffinityMask(uint mask)
{
    int res;
    pid_t pid = gettid();
    res = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
    if (res)
    {
      return LPGPU2_API_CALL_FAILED;
    }
    return LPGPU2_API_SUCCESS;
}

extern DLL_PUBLIC int lpgpu2_GetThreadAffinityMask(uint *mask)
{
    int res;
    pid_t pid = gettid();
    res = syscall(__NR_sched_getaffinity, pid, sizeof(uint), mask);
    if (res)
    {
      return LPGPU2_API_CALL_FAILED;
    }
    return LPGPU2_API_SUCCESS;
}

} // extern "C"

int ShimStartCaptureNoOp(LPGPU2_CAPTURE *)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimStartCaptureNoOp() - Set ExplicitControl in CollectionDefinition file.\n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int ShimStopCaptureNoOp(LPGPU2_CAPTURE *)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimStopCaptureNoOp() - Set ExplicitControl in CollectionDefinition file.\n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int ShimTerminateFrameNoOp(void)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimTerminateFrameNoOp() - Set ExplicitTerminate in CollectionDefinition file.\n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int ShimAnnotateCaptureNoOp(LPGPU2_ANNOTATION *)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimAnnotateCaptureNoOp().\n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int ShimTrackResourceNoOp(LPGPU2_RESOURCE_TYPE *, LPGPU2_RESOURCE_FLAGS *, uint64_t)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimTrackResourceNoOp(). \n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int ShimGetVersionNoOp(float *version)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimGetVersionNoOp()\n");
    firsttime = false;
  }
  *version = 0;
  return LPGPU2_API_INIT_ERROR;
}

int ShimStoreAffinityNoOp(uint, uint)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimStoreAffinityNoOp()\n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int ShimStoreUserCounterNoOp(uint, uint32_t)
{
  static bool firsttime = true;
  if (firsttime)
  {
    Log(LogError, "Error: Calling ShimStoreUserCounterNoOp()\n");
    firsttime = false;
  }
  return LPGPU2_API_INIT_ERROR;
}

int (*ShimStartCapture)(LPGPU2_CAPTURE *) = ShimStartCaptureNoOp;
int (*ShimStopCapture)(LPGPU2_CAPTURE *) = ShimStopCaptureNoOp;
int (*ShimTerminateFrame)(void) = ShimTerminateFrameNoOp;
int (*ShimAnnotateCapture)(LPGPU2_ANNOTATION *) = ShimAnnotateCaptureNoOp;
int (*ShimTrackResource)(LPGPU2_RESOURCE_TYPE *pType, LPGPU2_RESOURCE_FLAGS *pFlags, uint64_t resource) = ShimTrackResourceNoOp;
int (*ShimGetVersion)(float *) = ShimGetVersionNoOp;
int (*ShimStoreAffinity)(uint, uint) = ShimStoreAffinityNoOp;
int (*ShimStoreUserCounter)(uint, uint32_t) = ShimStoreUserCounterNoOp;

