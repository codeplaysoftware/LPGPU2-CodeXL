// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customisation of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// Data types related to database operations
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DB_DATABASE_DATA_UTILS_H_INCLUDE
#define LPGPU2_DB_DATABASE_DATA_UTILS_H_INCLUDE

// Infrastructure:
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>
#include <AMDTProfilerDAL/include/AMDTProfileDALDataTypes.h>

// STL
#include<utility>

namespace lpgpu2 {
namespace db {

/// @brief Represents a line in the profiling database's LPGPU2_traces table.
struct ProfileTraceInfo
{
    gtUInt64 m_traceId;
    gtString m_callName;
    gtUInt64 m_cpuStart;
    gtUInt64 m_cpuEnd;
    gtUInt64 m_frameNum;
    gtUInt64 m_drawNum;
    gtUInt64 m_apiId;
    gtUInt64 m_parameterOffset;
    gtUInt64 m_parameterLength;
};

/// @brief Represents a line in the profiling database's LPGPU2_estimatedPower table.
struct EstimatedPowerSample
{
    gtUInt64    m_sampleTime;
    gtFloat32   m_estimatedPower;
    gtUInt64    m_modelId;
    gtUInt64    m_componentId;
    gtString    m_unit;
};

/// @brief Represents a line in the profiling database's LPGPU2_estimatedPowerComponents table.
struct EstimatedPowerComponent
{
    gtUInt64    m_deviceId;
    gtUInt64    m_componentId;
    gtString    m_componentName;
};

/// @brief Represents a line in the profiling database's LPGPU2_stackTraces table.
struct StackTraceInfo
{
    gtUInt64    m_frameNum;
    gtUInt64    m_drawNum;
    gtUInt64    m_baseAddr;
    gtString    m_fileName;
    gtUInt64    m_symbolAddr;
    gtString    m_symbolName;
};

/// @brief Represents a line in the profiling database's LPGPU2_annotation table.
struct Annotation
{
    gtUInt64    m_annotationId;
    gtUInt64    m_frameNum;
    gtUInt64    m_drawNum;
    gtUInt64    m_annotationType;
    gtUInt64    m_cpuStartTime;
    gtUInt64    m_cpuEndTime;
    gtString    m_text;
};

/// @brief Represents a line in the profiling database's LPGPU2_gpuTimers table.
struct GPUTimer
{
    gtUInt64 m_type;
    gtUInt64 m_frameNum;
    gtUInt64 m_drawNum;
    gtUInt64 m_time;
};


enum class EShaderTraceAPIId
{
    kGLES2  = 0x0100,
    kGLES3  = 0x0200,
    kEGL    = 0x0400,
    kGL     = 0x0800,
    kCL     = 0x1000,
    kVULKAN = 0x2000,
    kNEMA   = 0x4000,
    kALL    = 0x3F00
};

enum class EShaderMetadataType
{
    kSource = 0,
    kAsm    = 1,
    kBinary = 2,
    kDebug  = 3,
    kSourceFromMainShaderDataPacket = 4,
    count
};

/// @brief Represents a line in the profiling database's LPGPU2_shader table.
struct ShaderTrace
{
    gtUInt64          m_shaderId;
    EShaderTraceAPIId m_apiId;
    gtUInt64          m_frameNum;
    gtUInt64          m_drawNum;    
    gtUInt64          m_shaderType;
    gtUInt64          m_cpuTime;
    gtString          m_srcCode;
    gtString          m_asmCode;
    gtVector<gtUByte> m_debug;
    gtVector<gtUByte> m_binary;
};

/// @brief  Represents a line in the LPGPU2_shaderStats table.
struct ShaderAsmStat
{
    gtUInt64          m_shaderId;
    EShaderTraceAPIId m_apiId;
    gtUInt64          m_asmLine;
    gtFloat32         m_percentage;
};

/// @brief  Represents an association between shader source code and ASM code.
struct ShaderToAsm
{
    gtUInt64          m_shaderId;
    EShaderTraceAPIId m_apiId;
    gtUInt64          m_shaderLine;
    gtUInt64          m_asmStartLine;
    gtUInt64          m_asmEndLine;
};

/// @brief Represents a line in the profiling database's LPGPU2_traceParameters table.
struct StackTraceParameter
{
    gtUInt64 m_frameNum;
    gtUInt64 m_drawNum;
    gtUInt64 m_threadId;
    gtString m_typeDefDescription;
    gtVector<gtUByte> m_binaryParams;
    gtString m_paramsSize;
    gtVector<gtUByte> m_binaryReturn;
    gtInt32 m_returnSize;
    gtInt32 m_error;
};

/// @brief Represents a line in the profiling database's LPGPU2_counterWeight table.
struct CounterWeight
{
    gtString    m_counterName;
    gtFloat32   m_weightValue;
};

/// @brief Represents a line in the profiling database's LPGPU2_regionsOfInterest table.
struct RegionOfInterest
{
    gtUInt64    m_regionId;
    gtUInt64    m_frameStart;
    gtUInt64    m_frameEnd;
    gtFloat32   m_degreeOfInterest;
};

/// @brief Represents a line in the profiling database's LPGPU2_callSummary table.
struct CallSummary
{
    gtUInt64    m_regionId;
    gtString    m_callName;
    gtUInt64    m_timesCalled;
    gtUInt64    m_totalTime;
};

/// @brief Represents a line in the profiling database's LPGPU2_callsPerType table.
struct CallsPerType
{
    gtUInt64    m_regionId;
    gtFloat32   m_drawPercentage;
    gtFloat32   m_bindPercentage;
    gtFloat32   m_setupPercentage;
    gtFloat32   m_waitPercentage;
    gtFloat32   m_errorPercentage;
    gtFloat32   m_delimiterPercentage;
    gtFloat32   m_otherPercentage;
};

/// @brief Information about a category of a function call.
struct CallCategory
{
    gtString m_callName;
    gtString m_category;
    gtString m_apiName;
};

struct InsertCounterCategoryInfo
{
    gtString m_counterName;
    gtString m_category;
};

/// @brief Information about counter category. Used to group things.
struct CounterCategory
{
    gtUInt64 m_counterId;
    gtString m_category;
};

/// @brief Information about a device.
struct Device
{
    gtUInt32 m_deviceId;
    gtUInt32 m_deviceTypeId;
    gtString m_deviceName;
    gtString m_deviceDescription;
};

/// Syntactic sugar to declare vectors of blobs
using CounterSampleBlobsVec = gtVector<PPSampleData::BlobVec>;

} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_DATABASE_DATA_UTILS_H_INCLUDE
