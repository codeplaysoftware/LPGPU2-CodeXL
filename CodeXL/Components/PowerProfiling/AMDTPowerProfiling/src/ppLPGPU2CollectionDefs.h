// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2CollectionDefs.h
///
/// @brief Representation of the collection definitions XML document.
///        CollectionDefs is generated via its Builder, which can be
///        used to configure the XML output stored by CollectionDefs.
///
/// CollectionDefs interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __PPLPGPU2COLLECTIONDEFS_H
#define __PPLPGPU2COLLECTIONDEFS_H

#include <tinyxml.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/ppLPGPU2TargetCharacteristics.h>

#include <vector>
#include <unordered_map>
#include <array>
#include <memory>

/// @brief Class representing a Command entry in the CollectionDefinition.xml
struct APICommand
{
// Methods
  APICommand() = default;
  APICommand(gtUInt32 id, const gtString &name, const gtString &overrides);

// Attributes
  gtUInt32 id = 0;
  gtString name;
  gtString overrides;
}; // struct APICommand

// Container for counter attributes
struct CollectionDefCounter
{
    bool enabled;
    gtString id;
};

// Container for counter set attributes
struct CollectionDefCounterSet
{
    gtUInt32 samplingInterval;
    bool enabled;
    gtString id;
    gtList<CollectionDefCounter> counters;
};

struct APITypeStruct
{
  enum APITypesEnum
  {
   VK = 0, GL, GLES, CL, END_OF_ENUM
  };
};

typedef APITypeStruct::APITypesEnum APIType;

using APIsToTrace = std::array<bool, APIType::END_OF_ENUM>;

class CollectionDefs
{
public:
    enum CommandNamesOverride { none, recordParameters };
    enum GpuTimingMode { DisjointFrame, PerDraw };
    enum LogLevel { Default };

    class Builder;

    TiXmlDocument& GetXML();

    const gtList<CollectionDefCounterSet> &GetCounterSets() const {
        return m_counterSets;
    }
    gtUInt32 GetIntervalHz() const { return m_dcApiIntervalHz; }
    const gtString &GetFileName() const { return m_fileName; }
    const gtString &GetProcessName() const { return m_processName; }
    bool IsTimelineEnabled() const { return m_isTimelineEnabled; }
    bool IsParamCaptureEnabled() const;
    bool IsScreenshotCaptureEnabled() const;
    bool IsGPUTimingEnabled() const;
    gtString GetGPUTimingMode() const;
    bool IsCallStackDumpEnabled() const;
    bool IsKHRDebugEnabled() const;
    bool IsShaderSourceCaptureEnabled() const;
    gtInt32 GetStartDraw() const { return m_startDraw; }
    gtInt32 GetStartFrame() const { return m_startFrame; }
    gtInt32 GetStopDraw() const { return m_stopDraw; }
    gtInt32 GetStopFrame() const { return m_stopFrame; }
    gtUInt32 GetFrameTerminator() const { return m_frameTerminatorsBitfield; }
    bool IsExplicitFrameTermEnabled() const { return m_explicitFrameTerm; }
    bool IsAPIBeingTraced(APIType type) const { return m_apisToTrace[type]; }
    bool IsExplicitControlEnabled() const { return m_explicitControlEnable ; }

private:
    TiXmlDocument m_xmlDoc;

    // Added these because we need to retrieve this sort of data from the
    // LPGPU2_AndroidBackend to give it to CXL
    gtList<CollectionDefCounterSet> m_counterSets;
    gtUInt32 m_dcApiIntervalHz = 1000;
    gtString m_fileName;
    gtString m_processName;
    gtUInt32 m_frameTerminatorsBitfield =
      FrameTerminatorApiCalls::FT_CL_FINISH |
      FrameTerminatorApiCalls::FT_EGL_SWAPBUFFERS |
      FrameTerminatorApiCalls::FT_VK_QUEUEPRESENTKHR;
    bool m_isTimelineEnabled = true;
    bool m_compressed = false;
    bool m_frameCaptureEnabled = true;
    bool m_callStackDumpEnabled = false;
    bool m_kHRDebugEnabled = false;
    bool m_gpuTimingEnabled = true;
    bool m_captureShaderSourcesEnabled = true;
    bool m_explicitFrameTerm = false;
    bool m_explicitControlEnable = false;
    GpuTimingMode m_gpuTimingMode = DisjointFrame;
    CommandNamesOverride m_commandsDefaultOverride = recordParameters;
    gtUInt32 m_numberedBufferSize = 10000;
    gtInt32 m_statsWriteThreshold = -1;
    gtInt32 m_startDraw = -1, m_startFrame = -1, m_stopDraw = -1,
            m_stopFrame = -1;
    APIsToTrace m_apisToTrace { {true, true, true, true} };

};

class CollectionDefs::Builder
{
public:
    Builder& SetCounterSets(gtList<CollectionDefCounterSet> counterSets);
    Builder& SetTimelineEnabled(bool isEnabled);
    Builder& SetCompressed(bool isCompressed);
    Builder& SetFrameCaptureEnabled(bool isEnabled);
    Builder& SetCallStackDumpEnabled(bool isEnabled);
    Builder& SetGpuTimingMode(bool isEnabled, GpuTimingMode mode = DisjointFrame);
    Builder& SetGpuTimingMode(bool isEnabled, const gtString &mode);
    Builder& SetKHRDebugEnabled(bool isEnabled, const std::unordered_map<gtUInt64, ApiElement>&apisLUT);
    Builder& SetCaptureShaderSourcesEnabled(bool isEnabled, const std::unordered_map<gtUInt64, ApiElement>&apisLUT);
    Builder& SetCaptureDebugGroupEnabled(bool isEnabled, const std::unordered_map<gtUInt64, ApiElement>&apisLUT);
    Builder& SetCommandNamesOverride(CommandNamesOverride defaultOverride);
    Builder& SetCommandNamesOverride(const gtString &defaultOverride);
    Builder& SetCommandName(gtUInt32 commandId, const gtString &originalCommandName, const gtString &newCommandName);
    Builder& SetCommandName(gtUInt32 apiId,
        const gtString &originalCommandName, const gtString &newCommandName, const std::unordered_map<gtUInt64, ApiElement>&apisLUT);
    Builder& SetNumberedBufferSize(gtUInt32 size);
    Builder& SetDcApiIntervalHz(gtUInt32 intervalHz);
    Builder& SetDataDirectory(const gtString& dataDirectory);
    Builder& SetLogLevel(LogLevel level);
    Builder& SetLogLevel(const gtString &level);
    Builder& SetStatsWriteThreshold(gtInt32 threshold);
    Builder& SetProcessName(const gtString& name);
    Builder& SetCollectionRange(gtInt32 startDraw, gtInt32 startFrame, gtInt32 stopDraw, gtInt32 stopFrame);
    Builder& SetFileName(gtString fileName);
    Builder& SetFrameTerminator(gtUInt32 bitfield);
    Builder& SetExplicitFrameTerminator(bool isEnabled);
    Builder& SetAPITrace(APIType apiType, bool isEnabled);
    Builder& SetExplicitControl(bool isEnabled);

    CollectionDefs Build();

    friend class CollectionDefs;

// Type aliases
private:
  using APICommandMap = std::unordered_map<gtUInt32, APICommand>;

private:
    // Default values for collection definitions
    gtList<CollectionDefCounterSet> m_counterSets;
    bool m_timelineEnabled = false;
    bool m_compressed = false;
    bool m_frameCaptureEnabled = true;
    bool m_callStackDumpEnabled = true;
    bool m_kHRDebugEnabled = true;
    bool m_gpuTimingEnabled = true;
    bool m_captureShaderSourcesEnabled = true;
    bool m_explicitFrameTerm = false;
    bool m_explicitControlEnable = false;
    GpuTimingMode m_gpuTimingMode = DisjointFrame;
    CommandNamesOverride m_commandsDefaultOverride = recordParameters;
    gtUInt32 m_numberedBufferSize = 10000;
    gtUInt32 m_dcApiIntervalHz = 1000;
    gtUInt32 m_frameTerminatorsBitfield =
      FrameTerminatorApiCalls::FT_CL_FINISH |
      FrameTerminatorApiCalls::FT_EGL_SWAPBUFFERS |
      FrameTerminatorApiCalls::FT_VK_QUEUEPRESENTKHR;
    gtString m_dataDirectory;
    LogLevel m_logLevel = Default;
    gtInt32 m_statsWriteThreshold = -1;
    gtString m_processName;
    gtInt32 m_startDraw = -1, m_startFrame = -1, m_stopDraw = -1, m_stopFrame = -1;
    gtString m_fileName;
    GpuTimingMode ToGpuTimingMode(const gtString &mode);
    LogLevel ToLogLevel(const gtString &level);
    CommandNamesOverride ToCommandNamesOverride(const gtString &defaultOverride);
    APIsToTrace m_apisToTrace { {true, true, true, true} };

// Attributes
private:
  APICommandMap m_commands;
};

#endif
