/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef GPGPU2_API_H
#define GPGPU2_API_H

#include <string>

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

#define LPGPU2_ANNOTATION_TYPE_USER       0
#define LPGPU2_ANNOTATION_TYPE_KHR_DEBUG  1
#define LPGPU2_ANNOTATION_TYPE_SHIM       2
#define LPGPU2_ANNOTATION_TYPE_FEEDBACK   10 // Feedback will use from 10 onwards.


#define LPGPU2_ANNOTATION_BEGIN           0
#define LPGPU2_ANNOTATION_END             1

#define LPGPU2_API_SUCCESS                0
#define LPGPU2_API_INIT_ERROR             1
#define LPGPU2_API_OOM_ERROR              2
#define LPGPU2_API_COLLECTION_ERROR       3
#define LPGPU2_API_FEATURE_DISABLED       4
#define LPGPU2_API_ILLEGAL_PARAMETER      5
#define LPGPU2_API_CALL_FAILED            6

// defines for lpgpu2_TrackResource
#define LPGPU2_RESOURCE_TYPE_BUFFER       0
#define LPGPU2_RESOURCE_TYPE_PROGRAM      1
#define LPGPU2_RESOURCE_TYPE_SHADER       2
#define LPGPU2_RESOURCE_TYPE_REFERENCE    3
#define LPGPU2_RESOURCE_TYPE_MEMORY       4
#define LPGPU2_RESOURCE_TYPE_CACHE        5
#define LPGPU2_RESOURCE_TYPE_USER_START   6
#define LPGPU2_RESOURCE_TYPE_USER_END     22


#define LPGPU2_RESOURCE_FLAG_ALLOCATE     0
#define LPGPU2_RESOURCE_FLAG_FREE         1
#define LPGPU2_RESOURCE_FLAG_MOVE         2
#define LPGPU2_RESOURCE_FLAG_RESIZE       3
#define LPGPU2_RESOURCE_FLAG_INCREMENT    4
#define LPGPU2_RESOURCE_FLAG_DECREMENT    5
#define LPGPU2_RESOURCE_FLAG_COMPILE      6
#define LPGPU2_RESOURCE_FLAG_LINK         7
#define LPGPU2_RESOURCE_FLAG_USER_START   8
#define LPGPU2_RESOURCE_FLAG_USER_END     24

#define LPGPU2_SHADER_TYPE_UNKNOWN        0
#define LPGPU2_SHADER_TYPE_KERNEL         1
#define LPGPU2_SHADER_TYPE_VERTEX         2
#define LPGPU2_SHADER_TYPE_GEOMETRY       3
#define LPGPU2_SHADER_TYPE_FRAGMENT       4
#define LPGPU2_SHADER_TYPE_COMPUTE        5

#define LPGPU2_SHADER_META_TYPE_SOURCE    0
#define LPGPU2_SHADER_META_TYPE_ASM       1
#define LPGPU2_SHADER_META_TYPE_BINARY    2
#define LPGPU2_SHADER_META_TYPE_DEBUG     3

#define LPGPU2_USER_COUNTER_AFFINITY      0x3000
#define LPGPU2_NUM_USER_COUNTERS          6
#define LPGPU2_USER_COUNTER_0             0x3001
#define LPGPU2_USER_COUNTER_1             0x3002
#define LPGPU2_USER_COUNTER_2             0x3003
#define LPGPU2_USER_COUNTER_3             0x3004
#define LPGPU2_USER_COUNTER_4             0x3005
#define LPGPU2_USER_COUNTER_5             0x3006

struct LPGPU2_CAPTURE
{
  uint32_t dwFlags;
};

struct LPGPU2_ANNOTATION
{
  uint32_t dwType;
  char *dwString;
};

struct LPGPU2_RESOURCE_TYPE
{
  uint32_t dwFlags;
};

struct LPGPU2_RESOURCE_FLAGS
{
  uint32_t dwFlags;
};

struct LPGPU2_TRACK_TYPE
{
  uint32_t dwFlags;
};

extern int (*ShimStartCapture)(LPGPU2_CAPTURE *);
extern int (*ShimStopCapture)(LPGPU2_CAPTURE *);
extern int (*ShimTerminateFrame)(void);
extern int (*ShimAnnotateCapture)(LPGPU2_ANNOTATION *);
extern int (*ShimTrackResource)(LPGPU2_RESOURCE_TYPE *, LPGPU2_RESOURCE_FLAGS *, uint64_t);
extern int (*ShimGetVersion)(float *);
extern int (*ShimStoreShaderMetaData)(uint, uint64_t, uint ,uint ,int ,char *);
extern int (*ShimStoreAffinity)(uint, uint);
extern int (*ShimStoreUserCounter)(uint, uint32_t);
extern "C" {
int lpgpu2_StartCapture(LPGPU2_CAPTURE *pCapture);
int lpgpu2_StopCapture(LPGPU2_CAPTURE *pCapture);
int lpgpu2_TerminateFrame(void);
int lpgpu2_AnnotateCapture(LPGPU2_ANNOTATION *pAnnotation);
int lpgpu2_TrackResource(LPGPU2_RESOURCE_TYPE *, LPGPU2_RESOURCE_FLAGS *, uint64_t);
int lpgpu2_GetVersion(float *version);
int lpgpu2_AssociateShaderMetaData(uint api, uint64_t shaderID, uint shader_type, uint MetaDataType, uint length, void *pBlob);
int lpgpu2_LogAffinityCounter(void);
int lpgpu2_LogUserCounter(uint Id, uint32_t value);
int lpgpu2_GetThreadAffinityMask(uint *mask);
int lpgpu2_SetThreadAffinityMask(uint mask);

}
#endif // header
