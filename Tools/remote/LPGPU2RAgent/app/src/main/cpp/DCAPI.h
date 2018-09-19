/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef __DCAPI_DEFINE_H
#define __DCAPI_DEFINE_H

#define DCAPI_VERSION_MAJOR      2
#define DCAPI_VERSION_MINOR      0

#define DCAPI_MAX_COUNTER_NAME   32
#define DCAPI_MAX_COUNTER_DESC   128

#define DCAPI_DCT_HEADER    0xF000
#define DCAPI_DCT_DATA      0xF001
#define DCAPI_GATOR_HEADER  0xF002
#define DCAPI_GATOR_DATA    0xF003
#define DCAPI_TEST_DATA     0xF004

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DCAPI_EXPORT __attribute__ ((dllexport))
    #else
      #define DCAPI_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DCAPI_EXPORT __attribute__ ((dllimport))
    #else
      #define DCAPI_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DCAPI_LOCAL
#else
  #if __GNUC__ >= 4
    #define DCAPI_EXPORT __attribute__ ((visibility ("default")))
    #define DCAPI_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DCAPI_EXPORT
    #define DCAPI_LOCAL
  #endif
#endif

typedef enum
{
  rRunning,
  rPaused,
  rStopped
} eRun;

typedef enum
{
  cHigh,
  cSoftware,
  cDriver,
  cCounter,
  cMaxCategory
} eCategory;

typedef enum
{
  tInteger,
  tFloat,
  tPercentage
} eType;

typedef enum
{
  uPowerWatts,
  uPowerKJ,
  uCurrent,
  uFrequency,
  uLoad,
  uTempC,
  umAmps,
  uFPS,
} eUnit;

// CollectionMode can be used as a mask, so new modes must use a new bit.
typedef enum
{
  mNone     = 0x0 << 0,  // No mode set
  mTime     = 0x1 << 0,  // Time base Sampling
  mEvent    = 0x1 << 1,  // Event based sampling
  mCustom   = 0x1 << 2,  // Implementation specific
  mLast     = mCustom    // The last valid mode.
} eCollectionMode;

typedef enum
{
  fInternalBuffers = 0,
  fUserBuffers = 1,
} eFlags;

typedef enum
{
  DCAPI_UNINITIALIZED = -15,
  DCAPI_SHIM_UNINITIALIZED = -14,
  DCAPI_COUNTERSET_UNINITIALIZED = -13,
  DCAPI_INVALID_CONFIGURATION = -12,
  DCAPI_INVALID_SIZE = -11,
  DCAPI_INVALID_RATE = -10,
  DCAPI_INVALID_MODE = -9,
  DCAPI_INITIALIZATION_FAILURE = -8,
  DCAPI_COLLECTION_NOT_ACTIVE = -7,
  DCAPI_COLLECTION_ALREADY_ACTIVE = -6,
  DCAPI_INVALID_PTR_ARGUMENT = -5,
  DCAPI_BAD_COUNTERSET_ID = -4,
  DCAPI_BAD_COUNTER_INDEX = -3,
  DCAPI_COLLECTION_INACTIVE = -2,
  DCAPI_NO_COUNTERS_DEFINED = -1,
  DCAPI_SUCCESS = 0,
} eError;

typedef enum
{
  DCAPI_ENABLE = 1,
  DCAPI_DISABLE = 2,
} eEnable;

typedef struct _tag_CounterDefinition
{
  uint32_t     ID;
  uint8_t      Name[DCAPI_MAX_COUNTER_NAME];
  uint8_t      Description[DCAPI_MAX_COUNTER_DESC];
  uint8_t      Category;
  uint8_t      Type;
  uint8_t      Unit;
  uint8_t      Enabled;
} CounterDefinition;


typedef struct _tag_CounterSetDefinition
{
  uint32_t        ID;
  uint8_t         Name[DCAPI_MAX_COUNTER_NAME];
  uint8_t         Description[DCAPI_MAX_COUNTER_DESC];
  uint32_t        NumCounters;
  uint8_t         Category;
  uint64_t        MinimumSamplingInterval; // in ms
  uint64_t        CurrentSamplingInterval; // in ms
  eCollectionMode SupportedModes;
  eCollectionMode CurrentMode;
  uint8_t         Enabled;
} CounterSetDefinition;

typedef struct _tag_CharacteristicsDefinition
{
  uint32_t        MajorVersion;
  uint32_t        MinorVersion;
  uint32_t        HardwareID;
  uint32_t        NumCounterSets;
  uint32_t        BlobSize;
  bool            UserBufferSupported; // The user can supply the output buffers.
} CharacteristicsDefinition;

#ifdef __cplusplus
extern "C"
{
#endif

/*  \brief Initialize the library.
 *  \param[in] flags
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_Initialize(uint32_t flags);

/*  \brief Terminate the library.
 *  \param[in] flags
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_Terminate(uint32_t flags);

/*  \brief Get the API Characteristics
 *
 *  \param[out] definition CharacteristicsDefinition
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_GetCharacteristics(CharacteristicsDefinition *definition);

/*  \brief Get the CounterSet definition.
 *
 *  \param[in] index CounterSet index 0...GetNumCounterSets()
 *  \param[out] definition CounterSetDefinition
 *  \return eError
 */

extern DCAPI_EXPORT int32_t DCAPI_GetCounterSetDefinition(uint32_t index, CounterSetDefinition *definition);

/*  \brief Get the Counter definition.
 *
 *  \param[in] csid CounterSet ID (see CounterSetDefinition)
 *  \param[in] idx Counter Index
 *  \param[out] definition CounterDefinition
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_GetCounterSetCounterDefinition(uint32_t csid, uint32_t idx, CounterDefinition *definition);

/*  \brief Get the sizes needed for user allocated buffers
 *
 *  \param[out] bufferSizes Array of sizes
 *  \param[in] numSizes Should match the number of enabled CounterSets
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_GetUserBufferSizes(uint32_t *bufferSizes, uint32_t numSizes);

/*  \brief Set the CounterSet state
 *
 *  Enable/Disable all counters in a set, and the set itself.
 *
 *  \param[in] csid CounterSet ID (See CounterSetDefinition)
 *  \param[in] enable_disable eEnable
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_SetCounterSetState(uint32_t csid, uint32_t enable_disable);

/*  \brief Set the CounterSet mode and sampling interval
 *
 *  \param[in] csid CounterSet ID (See CounterSetDefinition)
 *  \param[in] mode (See eCollectionMode)
 *  \param[in] sampleInterval in ms
 *  \return eError
 */
extern DCAPI_EXPORT int32_t DCAPI_SetCounterSetModeAndInterval(uint32_t csid, eCollectionMode mode, uint32_t sampleInterval);

/*  \brief Set the counter enable state by ID, for a particular CounterSet.
 *
 *  \param[in] csid CounterSet ID (See CounterSetDefinition)
 *  \param[in] id The counter ID (See CounterDefinition)
 *  \param[in] enable_disable eEnable
 *  \return eError.
 */
extern DCAPI_EXPORT int32_t DCAPI_SetCounterState(uint32_t csid, uint32_t id, uint32_t enable_disable);

/*  \brief Set the user allocated buffers.
 *
 *  \param[in] buffers Array of buffer pointers
 *  \param[in] numBuffers Should match the number of enabled CounterSets
 *  \return eError.
 */
extern DCAPI_EXPORT int32_t DCAPI_SetUserBuffers(uint8_t **buffers, uint32_t numBuffers);
/*  \brief Start the data collection.
 *
 *  \param[in] flags
 *  \return eError.
 */
extern DCAPI_EXPORT int32_t DCAPI_StartCollection(uint32_t /*flags*/);

/*  \brief Pause the data collection.
 *
 *  \param[in] flags
 *  \return eError.
 */
extern DCAPI_EXPORT int32_t  DCAPI_PauseCollection(uint32_t flags);

/*  \brief Stop the data collection.
 *
 *  \param[in] flags
 *  \return eError.
 */
extern DCAPI_EXPORT int32_t  DCAPI_StopCollection(uint32_t flags);


typedef uint32_t (*DCAPI_DATA_READY_CALLBACK)(uint8_t *memoryBlock, uint32_t blockSize, uint32_t flags, void *userData);

/*  \brief Set the callback for data collection
 *
 *  \param[in] pfnCallback The callback function
 *  \param[in] flags
 *  \param[in] userData Pointer to user data (returned in the callback)
 *  \return eError.
 */
extern DCAPI_EXPORT int32_t  DCAPI_SetDataReadyCallback(DCAPI_DATA_READY_CALLBACK pfnCallback, uint32_t flags, void *userData);

#ifdef __cplusplus
}
#endif

#endif // header
