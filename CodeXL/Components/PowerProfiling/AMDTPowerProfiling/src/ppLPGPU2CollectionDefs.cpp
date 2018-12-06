// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2CollectionDefs.cpp
///
/// @brief Representation of the collection definitions XML document.
///        CollectionDefs is generated via its Builder, which can be
///        used to configure the XML output stored by CollectionDefs.
///
/// CollectionDefs implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include "ppLPGPU2CollectionDefs.h"
// STL
// Used for std::move()
#include <utility>

// Infra
// Included for GT_ASSERT
#include <AMDTBaseTools/Include/gtAssert.h> 
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

#include <QString>

// The size of buffer to use when converting int -> char*
#define CONVERSION_BUFFER_SIZE 20

/// @brief Convert a gtString containing an unsigned integer to a QString
///        containing the same number in hexadecimal base, prefixed by "0x"
/// @param str The string to convert containing the number
/// @return A QString containing the hex form of the number in str
QString gtStrDecimalToQStrHexWithPrefix(const gtString &str)
{
  unsigned int asUInt = 0U;
  GT_ASSERT(str.toUnsignedIntNumber(asUInt));
  auto asHexQStr = QString::number(asUInt, 16);
  return asHexQStr.prepend("0x");
}

/// @brief                  Utility method. Converts a 32-bit signed integer
///                         into its string representation.
/// @param[in]      i       The integer to convert.
/// @param[in/out]  buffer  The buffer to use in the conversion.
/// @return         char*   The same buffer passed as a parameter.
static char* intToChar(gtInt32 i, char* buffer)
{
    sprintf(buffer, "%d", i);
    return buffer;
}

/// @brief                  Utility method. Converts a 32-bit unsigned integer
///                         into its string representation.
/// @param[in]      i       The integer to convert.
/// @param[in/out]  buffer  The buffer to use in the conversion.
/// @return         char*   The same buffer passed as a parameter.
static char* intToChar(gtUInt32 i, char* buffer)
{
    sprintf(buffer, "%u", i);
    return buffer;
}

/// @brief                  Utility method. Converts GpuTimingMode enum values
///                         into their respective string representations.
/// @param[in]  mode        GpuTimingMode value to convert.
/// @return     const char* String representation of the GpuTimingMode enum value.
const char* ToString(CollectionDefs::GpuTimingMode mode)
{
    switch (mode)
    {
    case CollectionDefs::DisjointFrame: return "DisjointFrame";
    case CollectionDefs::PerDraw: return "PerDraw";
    default: return "DisjointFrame";
    }
}

/// @brief                  Utility method. Converts LogLevel enum values
///                         into their respective string representations.
/// @param[in]  level       LogLevel value to convert.
/// @return     const char* String representation of the LogLevel enum value.
const char* ToString(CollectionDefs::LogLevel level)
{
    switch (level)
    {
    case CollectionDefs::Default: return "Default";
    default: return "Default";
    }
}

/// @brief                      Utility method. Converts CommandNamesOverride enum values
///                             into their respective string representations.
/// @param[in]  defaultOverride CommandNamesOverride value to convert.
/// @return     const char*     String representation of the CommandNamesOverride enum value.
const char* ToString(CollectionDefs::CommandNamesOverride defaultOverride)
{
    switch (defaultOverride)
    {
    case CollectionDefs::recordParameters: return "recordParameters";
    default: return "";
    }
}

/// @brief                  Gets the XML representation of the collection
///                         definitions document.
/// @return TiXmlDocument&  Tinyxml representation of collection definitions.
TiXmlDocument& CollectionDefs::GetXML()
{
    return m_xmlDoc;
}

/// @brief                  Builder method. Sets the list of 'counter sets' to be
///                         generated in the XML document.
/// @param[in]  counterSets The list of counter sets.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetCounterSets(const gtList<CollectionDefCounterSet> counterSets)
{
    m_counterSets = counterSets;
    return *this;
}

/// @brief                  Builder method. Enables or disables the timeline
///                         generated in the XML document.
/// @param[in]  isEnabled   Whether the timeline should be enabled/disabled.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetTimelineEnabled(bool isEnabled)
{
    m_timelineEnabled = isEnabled;
    return *this;
}

/// @brief                      Builder method. Enables or disables the compression
///                             element generated in the XML document.
/// @param[in]  isCompressed    Whether compression should be enabled/disabled.
/// @return     Builder&        A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetCompressed(bool isCompressed)
{
    m_compressed = isCompressed;
    return *this;
}

/// @brief                  Builder method. Enables or disables the frame capture
///                         element generated in the XML document.
/// @param[in]  isEnabled   Whether frame capture should be enabled/disabled.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetFrameCaptureEnabled(bool isEnabled)
{
    m_frameCaptureEnabled = isEnabled;
    return *this;
}

/// @brief                  Builder method. Enables or disables the callstack dump
///                         element generated in the XML document.
/// @param[in]  isEnabled   Whether callstack dump should be enabled/disabled.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetCallStackDumpEnabled(bool isEnabled)
{
    m_callStackDumpEnabled = isEnabled;
    return *this;
}

/// @brief                  Builder method. Enables or disables the GPU timing
///                         element generated in the XML document, as well as
///                         the mode which it should use.
/// @param[in]  isEnabled   Whether GPU timing should be enabled/disabled.
/// @param[in]  mode        The mode to use when GPU timing is enabled.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetGpuTimingMode(bool isEnabled, CollectionDefs::GpuTimingMode mode)
{
    m_gpuTimingEnabled = isEnabled;
    m_gpuTimingMode = mode;
    return *this;
}

/// @brief                  Builder method. Enables or disables the GPU timing
///                         element generated in the XML document, as well as
///                         the mode which it should use.
/// @param[in]  isEnabled   Whether GPU timing should be enabled/disabled.
/// @param[in]  mode        The mode to use when GPU timing is enabled.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetGpuTimingMode(bool isEnabled, const gtString &mode)
{
    m_gpuTimingEnabled = isEnabled;
    m_gpuTimingMode = ToGpuTimingMode(mode);
    return *this;
}

/// @brief                  Builder method. Enables or disables KHR Debug
///                         element generated in the XML document.
/// @param[in]  isEnabled   Whether KHR debug should be enabled/disabled.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetKHRDebugEnabled(bool isEnabled,
    const std::unordered_map<gtUInt64, ApiElement>&apisLUT)
{

    m_kHRDebugEnabled = isEnabled;

    if (m_kHRDebugEnabled)
    {
      if (m_commandsDefaultOverride == recordParameters)
      {
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kEGL,
            L"eglCreateContext", L"khr_debug_params_eglCreateContext", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2,
            L"glDebugMessageCallbackKHR", L"khr_debug_params_glDebugMessageCallbackKHR", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kEGL,
            L"eglMakeCurrent", L"khr_debug_params_eglMakeCurrent", apisLUT);
      }
      else
      {
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kEGL,
            L"eglCreateContext", L"khr_debug_eglCreateContext", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2,
            L"glDebugMessageCallbackKHR", L"khr_debug_glDebugMessageCallbackKHR", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kEGL,
            L"eglMakeCurrent", L"khr_debug_eglMakeCurrent", apisLUT);
      }
    }

    return *this;
}

/// @brief                  Builder method. Enables or disables the shader capture
///                         element generated in the XML document.
/// @param[in]  isEnabled   Whether shader capture should be enabled/disabled.
/// @return     Builder&    A reference to this builder.

CollectionDefs::Builder& CollectionDefs::Builder::SetCaptureShaderSourcesEnabled(bool isEnabled,
    const std::unordered_map<gtUInt64, ApiElement>&apisLUT)
{
    m_captureShaderSourcesEnabled = isEnabled;

    if (m_captureShaderSourcesEnabled)
    {
      if (m_commandsDefaultOverride == recordParameters)
      {
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glCreateShader", L"shader_params_glCreateShader", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glShaderSource", L"shader_params_glShaderSource", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kCL, L"clCreateProgramWithSource",
            L"shader_params_clCreateProgramWithSource", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kVULKAN, L"vkCreateShaderModule",
            L"shader_params_vkCreateShaderModule", apisLUT);
      }
      else
      {
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glCreateShader", L"shader_glCreateShader", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glShaderSource", L"shader_glShaderSource", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kCL, L"clCreateProgramWithSource",
            L"shader_clCreateProgramWithSource", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kVULKAN, L"vkCreateShaderModule",
            L"shader_vkCreateShaderModule", apisLUT);
      }
    }

    return *this;
}

/// @brief                  Builder method. Enables or disables the DebugGroup capture
///                         element generated in the XML document.
/// @param[in]  isEnabled   Whether DebugGroup capture should be enabled/disabled.
/// @return     Builder&    A reference to this builder.

CollectionDefs::Builder& CollectionDefs::Builder::SetCaptureDebugGroupEnabled(bool isEnabled, const std::unordered_map<gtUInt64, ApiElement>&apisLUT)
{

    if (isEnabled)
    {
      if (m_commandsDefaultOverride == recordParameters)
      {
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glPushDebugGroup", L"debuggroup_params_glPushDebugGroup", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glPopDebugGroup", L"debuggroup_params_glPopDebugGroup", apisLUT);
      }
      else
      {
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glPushDebugGroup", L"debuggroup_glPushDebugGroup", apisLUT);
        SetCommandName((gtUInt32)lpgpu2::db::EShaderTraceAPIId::kGLES2, L"glPopDebugGroup", L"debuggroup_glPopDebugGroup", apisLUT);
      }
    }

    return *this;
}

/// @brief                          Builder method. Sets the default override for the
///                                 command names element in the XML document.
/// @param[in]  defaultOverride     The command names default override to use.
/// @return     Builder&            A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetCommandNamesOverride(CollectionDefs::CommandNamesOverride defaultOverride)
{
    m_commandsDefaultOverride = defaultOverride;

#if 0 // Unnecessary
    if (m_commandsDefaultOverride == recordParameters)
    {
      SetCommandName(300, L"glCreateShader", L"shader_params_glCreateShader");
      SetCommandName(826, L"glShaderSource", L"shader_params_glShaderSource");
      SetCommandName(10, L"clCreateProgramWithSource",
          L"shader_params_clCreateProgramWithSource");
      SetCommandName(1063, L"vkCreateShaderModule",
          L"shader_params_vkCreateShaderModule");
    }
    else
    {
      SetCommandName(300, L"glCreateShader", L"shader_glCreateShader");
      SetCommandName(826, L"glShaderSource", L"shader_glShaderSource");
      SetCommandName(10, L"clCreateProgramWithSource",
          L"shader_clCreateProgramWithSource");
      SetCommandName(1063, L"vkCreateShaderModule",
          L"shader_vkCreateShaderModule");
    }
#endif
    return *this;
}

/// @brief                          Builder method. Sets the default override for the
///                                 command names element in the XML document.
/// @param[in]  defaultOverride     The command names default override to use.
/// @return     Builder&            A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetCommandNamesOverride(const gtString &defaultOverride)
{
    m_commandsDefaultOverride = ToCommandNamesOverride(defaultOverride);
    return *this;
}

/// @brief                  Builder method. Sets the size of the numbered buffer
///                         size element generated in the XML document.
/// @param[in]  size        The size of the numbered buffer.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetNumberedBufferSize(gtUInt32 size)
{
    m_numberedBufferSize = size;
    return *this;
}

/// @brief                  Builder method. Sets the sampling frequency of the
///                         DCAPI interval element in the XML document.
/// @param[in]  intervalHz  The size of the numbered buffer.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetDcApiIntervalHz(gtUInt32 intervalHz)
{
    m_dcApiIntervalHz = intervalHz;
    return *this;
}

/// @brief                  Builder method. Sets the path of the data directory
///                         element in the XML document.
/// @param[in]  directory   The path to the data directory on the device.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetDataDirectory(const gtString& directory)
{
    m_dataDirectory = directory;
    return *this;
}

/// @brief                  Builder method. Sets the log level of the log level
///                         element in the XML document.
/// @param[in]  level       The log level.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
    return *this;
}

/// @brief                  Builder method. Sets the log level of the log level
///                         element in the XML document.
/// @param[in]  level       The log level.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetLogLevel(const gtString &level)
{
    m_logLevel = ToLogLevel(level);
    return *this;
}

/// @brief                  Builder method. Sets the threshold value for the
///                         stats write threshold element in the XML document.
/// @param[in]  threshold   The threshold limit at which to write stats.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetStatsWriteThreshold(gtInt32 threshold)
{
    m_statsWriteThreshold = threshold;
    return *this;
}

/// @brief                  Builder method. Sets the name of the process element
///                         in the XML document.
/// @param[in]  name        The process name to profile on the device.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetProcessName(const gtString& name)
{
    m_processName = name;
    return *this;
}

/// @brief                  Builder method. Sets the range of the collection range
///                         element in the XML document.
/// @param[in]  startDraw   Draw number at which to start collecting counter values.
/// @param[in]  startFrame  Frame number at which to start collecting counter values.
/// @param[in]  stopDraw    Draw number at which to stop collecting counter values.
/// @param[in]  stopFrame   Frame number at which to stop collecting counter values.
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetCollectionRange(gtInt32 startDraw, gtInt32 startFrame, gtInt32 stopDraw, gtInt32 stopFrame)
{
    m_startDraw = startDraw;
    m_startFrame = startFrame;
    m_stopDraw = stopDraw;
    m_stopFrame = stopFrame;
    return *this;
}

CollectionDefs::Builder&
CollectionDefs::Builder::SetExplicitFrameTerminator(bool isEnabled)
{
  m_explicitFrameTerm = isEnabled;
  return *this;
}

CollectionDefs::Builder& CollectionDefs::Builder::SetFrameTerminator(
    gtUInt32 bitfield)
{
  m_frameTerminatorsBitfield = bitfield;
  return *this;
}

CollectionDefs::Builder& CollectionDefs::Builder::SetAPITrace(
    APIType apiType, bool isEnabled)
{
  m_apisToTrace[apiType] = isEnabled;
  return *this;
}

CollectionDefs::Builder& CollectionDefs::Builder::SetExplicitControl(
    bool isEnabled)
{
  m_explicitControlEnable = isEnabled;
  return *this;
}

/// @brief                  Uses the configuration set via the builder methods
///                         to generate a representative collection definitions
///                         XML document.
/// @return CollectionDefs  The collection definitions, which has the tinyxml representation.
CollectionDefs CollectionDefs::Builder::Build()
{
    char valueBuffer[CONVERSION_BUFFER_SIZE];

    TiXmlDeclaration* pDecl = new TiXmlDeclaration("1.0", "utf8", "");
    TiXmlElement* pCollection = new TiXmlElement("Collection");

    // Timeline
    TiXmlElement* pTimeline = new TiXmlElement("Timeline");
    pTimeline->SetAttribute("Enable", (m_timelineEnabled ? "1" : "0"));
    pCollection->LinkEndChild(pTimeline);

    // CounterSet & Counter
    for (const auto &counterSet : m_counterSets)
    {
        // CounterSet
        TiXmlElement* pCounterSet = new TiXmlElement("CounterSet");
        pCounterSet->SetAttribute("Enable", (counterSet.enabled ? "1" : "0"));
        // Convert to hexadecimal
        auto setIdAsHexStr = gtStrDecimalToQStrHexWithPrefix(counterSet.id);
        pCounterSet->SetAttribute("Id", setIdAsHexStr.toLocal8Bit().data());
        // Convert to string
        gtString samplingIntervalAsStr;
        samplingIntervalAsStr.appendUnsignedIntNumber(
            counterSet.samplingInterval);
        pCounterSet->SetAttribute("SamplingInterval",
            samplingIntervalAsStr.asUTF8CharArray());
        for (const CollectionDefCounter& counter : counterSet.counters)
        {
            // Counter
            TiXmlElement* pCounter = new TiXmlElement("Counter");
            pCounter->SetAttribute("Enable", (counter.enabled ? "1" : "0"));
            // Convert to hexadecimal
            auto idAsHexStr = gtStrDecimalToQStrHexWithPrefix(counter.id);
            pCounter->SetAttribute("Id", idAsHexStr.toLocal8Bit().data());
            pCounterSet->LinkEndChild(pCounter);
        }
        pCollection->LinkEndChild(pCounterSet);
    }

    // Compressed
    TiXmlElement* pCompressed = new TiXmlElement("Compressed");
    pCompressed->SetAttribute("Enable", (m_compressed ? "1" : "0"));
    pCollection->LinkEndChild(pCompressed);

    // FrameCapture
    TiXmlElement* pFrameCapture = new TiXmlElement("FrameCapture");
    pFrameCapture->SetAttribute("Enable", (m_frameCaptureEnabled ? "1" : "0"));
    pCollection->LinkEndChild(pFrameCapture);

    // ExplicitFrameTerminator
    auto *pExplicitFrameTerm = new TiXmlElement("ExplicitFrameTerminator");
    pExplicitFrameTerm->SetAttribute("Enable",
        (m_explicitFrameTerm ? "1" : "0"));
    pCollection->LinkEndChild(pExplicitFrameTerm);

    // ExplicitControl
    auto *pExplicitControl = new TiXmlElement("ExplicitControl");
    pExplicitControl->SetAttribute("Enable",
        (m_explicitControlEnable ? "1" : "0"));
    pCollection->LinkEndChild(pExplicitControl);

    auto *pFrameTerminators = new TiXmlElement("FrameTerminators");
    auto bitfieldAsString = std::to_string(m_frameTerminatorsBitfield);
    pFrameTerminators->SetAttribute("Flags",
        bitfieldAsString.c_str());
    pCollection->LinkEndChild(pFrameTerminators);

    // APIs to trace
    gtString names;
    if (m_apisToTrace[APIType::GL])
    {
      names.append(L"egl ");
    }
    if (m_apisToTrace[APIType::CL])
    {
      names.append(L"cl ");
    }
    if (m_apisToTrace[APIType::GLES])
    {
      names.append(L"gles2 ");
    }
    if (m_apisToTrace[APIType::VK])
    {
      names.append(L"vulkan ");
    }
    
    {
      auto* pAPIsName = new TiXmlElement("API");
      pAPIsName->SetAttribute("Name", names.asASCIICharArray());
      pCollection->LinkEndChild(pAPIsName);
    }

    // DumpCallstack
    TiXmlElement* pCallStackDump = new TiXmlElement("DumpCallstack");
    pCallStackDump->SetAttribute("Enable", (m_callStackDumpEnabled ? "1" : "0"));
    pCollection->LinkEndChild(pCallStackDump);

    // GpuTimingMode
    // DumpCallstack
    auto *pGPUTimingEnabled = new TiXmlElement("GPUTiming");
    pGPUTimingEnabled->SetAttribute("Enable", (m_gpuTimingEnabled? "1" : "0"));
    pCollection->LinkEndChild(pGPUTimingEnabled);
    if (m_gpuTimingEnabled) // TODO: Find out if this also uses an "Enable" attribute instead of removing completely
    {
        TiXmlElement* pGpuTimingMode = new TiXmlElement("GpuTimingMode");
        pGpuTimingMode->SetAttribute("Mode", ToString(m_gpuTimingMode));
        pCollection->LinkEndChild(pGpuTimingMode);
    }

    // KHR Debug
    TiXmlElement* pKHRDebug = new TiXmlElement("KHRDebug");
    pKHRDebug->SetAttribute("Enable", (m_kHRDebugEnabled ? "1" : "0"));
    pCollection->LinkEndChild(pKHRDebug);

    // CaptureShaders
    TiXmlElement* pCaptureShaders = new TiXmlElement("CaptureShaders");
    pCaptureShaders->SetAttribute("Enable",
        (m_captureShaderSourcesEnabled ? "1" : "0"));
    pCollection->LinkEndChild(pCaptureShaders);


// TIZEN FIX set override even when m_commands is empty
#define FORCE_DEFAULT_OVERRIDE 1
#if FORCE_DEFAULT_OVERRIDE // new code

    // Command Names
    TiXmlElement* pCommandNames = new TiXmlElement("CommandNames");
    if (m_commandsDefaultOverride != none)
    {
      pCommandNames->SetAttribute("DefaultOverride",
        ToString(m_commandsDefaultOverride));
    }
#endif

    // Commands
    if (m_commands.size() > 0)
    {
#if !FORCE_DEFAULT_OVERRIDE // original code
      // Command Names
      TiXmlElement* pCommandNames = new TiXmlElement("CommandNames");
      if (m_commandsDefaultOverride != none)
      {
        pCommandNames->SetAttribute("DefaultOverride",
          ToString(m_commandsDefaultOverride));
      }
#endif
      for (const auto &item : m_commands)
      {
        const auto &cmnd = item.second;
        TiXmlElement* pCommand = new TiXmlElement("Command");
        gtString idAsString;
        idAsString.appendUnsignedIntNumber(cmnd.id);
        pCommand->SetAttribute("Id", idAsString.asASCIICharArray());
        pCommand->SetAttribute("Name", cmnd.name.asASCIICharArray());
        pCommand->SetAttribute("Override", cmnd.overrides.asASCIICharArray());
        pCommandNames->LinkEndChild(pCommand);
      }

#if !FORCE_DEFAULT_OVERRIDE // original code
      pCollection->LinkEndChild(pCommandNames);
#endif
    }

#if FORCE_DEFAULT_OVERRIDE // new code
    pCollection->LinkEndChild(pCommandNames);
#endif

    // Config (NumberedBufferSize)
    TiXmlElement* pNumberedBufferSize = new TiXmlElement("Config");
    pNumberedBufferSize->SetAttribute("Name", "NumberedBufferSize");
    pNumberedBufferSize->SetAttribute("Value", intToChar(m_numberedBufferSize, valueBuffer));
    pCollection->LinkEndChild(pNumberedBufferSize);

    // Config (DcapiIntervalHz)
    TiXmlElement* pDcApiIntervalHz = new TiXmlElement("Config");
    pDcApiIntervalHz->SetAttribute("Name", "DcapiIntervalHz");
    pDcApiIntervalHz->SetAttribute("Value", intToChar(m_dcApiIntervalHz, valueBuffer));
    pCollection->LinkEndChild(pDcApiIntervalHz);

    // Config (LogLevel)
    TiXmlElement* pLogLevel = new TiXmlElement("Config");
    pLogLevel->SetAttribute("Name", "LogLevel");
    pLogLevel->SetAttribute("Value", ToString(m_logLevel));
    pCollection->LinkEndChild(pLogLevel);

    // Config (DataDirectory)
    TiXmlElement* pDataDirectory = new TiXmlElement("Config");
    pDataDirectory->SetAttribute("Name", "DataDirectory");
    pDataDirectory->SetAttribute("Value", m_dataDirectory.asASCIICharArray());
    pCollection->LinkEndChild(pDataDirectory);

    // Config (StatsWriteThreshold)
    TiXmlElement* pThreshold = new TiXmlElement("Config");
    pThreshold->SetAttribute("Name", "StatsWriteThreshold");
    pThreshold->SetAttribute("Value", intToChar(m_statsWriteThreshold, valueBuffer));
    pCollection->LinkEndChild(pThreshold);

    // Process
    TiXmlElement* pProcess = new TiXmlElement("Process");
    pProcess->SetAttribute("Name", m_processName.asASCIICharArray());
    pCollection->LinkEndChild(pProcess);

    // CollectionRange
    TiXmlElement* pCollectionRange = new TiXmlElement("CollectionRange");
    pCollectionRange->SetAttribute("StartDraw", intToChar(m_startDraw, valueBuffer));
    pCollectionRange->SetAttribute("StartFrame", intToChar(m_startFrame, valueBuffer));
    pCollectionRange->SetAttribute("StopDraw", intToChar(m_stopDraw, valueBuffer));
    pCollectionRange->SetAttribute("StopFrame", intToChar(m_stopFrame, valueBuffer));
    pCollection->LinkEndChild(pCollectionRange);

    // Create the CollectionDefs object and return it
    CollectionDefs defs;
    defs.m_xmlDoc.LinkEndChild(pDecl);
    defs.m_xmlDoc.LinkEndChild(pCollection);
    defs.m_counterSets = m_counterSets;
    defs.m_dcApiIntervalHz = m_dcApiIntervalHz;
    defs.m_fileName = std::move(m_fileName);
    defs.m_processName = std::move(m_processName);
    defs.m_isTimelineEnabled = m_timelineEnabled;
    defs.m_compressed = m_compressed;
    defs.m_frameCaptureEnabled = m_frameCaptureEnabled;
    defs.m_callStackDumpEnabled = m_callStackDumpEnabled;
    defs.m_kHRDebugEnabled = m_kHRDebugEnabled;
    defs.m_gpuTimingEnabled = m_gpuTimingEnabled;
    defs.m_captureShaderSourcesEnabled = m_captureShaderSourcesEnabled;
    defs.m_gpuTimingMode = m_gpuTimingMode;
    defs.m_commandsDefaultOverride = m_commandsDefaultOverride;
    defs.m_numberedBufferSize = m_numberedBufferSize;
    defs.m_startDraw = m_startDraw;
    defs.m_startFrame = m_startFrame;
    defs.m_stopDraw = m_stopDraw;
    defs.m_stopFrame = m_stopFrame;
    defs.m_frameTerminatorsBitfield = m_frameTerminatorsBitfield;
    defs.m_apisToTrace = m_apisToTrace;
    defs.m_explicitFrameTerm = m_explicitFrameTerm;
    defs.m_explicitControlEnable = m_explicitControlEnable;

    return defs;
}

/// @brief                  Utility method. Converts string to GpuTimingMode enum values
///                         into their respective string representations.
/// @param[in]  mode        GpuTimingMode value to convert.
/// @return     GpuTimingMode Enum version of the GpuTimingMode
CollectionDefs::GpuTimingMode CollectionDefs::Builder::ToGpuTimingMode(const gtString &mode)
{
  if (mode == L"PerDraw")
  {
    return GpuTimingMode::PerDraw;
  }
  else
  {
    return GpuTimingMode::DisjointFrame;
  }
}

/// @brief                  Utility method. Converts LogLevel enum values
///                         into their respective string representations.
/// @param[in]  level       LogLevel value to convert.
/// @return     LogLevel Enum version of the LogLevel
CollectionDefs::LogLevel CollectionDefs::Builder::ToLogLevel(const gtString &level)
{
  if (level == L"Default")
  {
    return LogLevel::Default;
  }
  // The if-else, even if they return the same thing, is introduced for conformancy with the
  // pre-existing (at the time of writing this function) ToString method which uses a switch
  // statment even if it only returns one variant of the enum
  else
  {
    return LogLevel::Default;
  }
}

/// @brief                      Utility method. Converts CommandNamesOverride enum values
///                             into their respective string representations.
/// @param[in]  defaultOverride CommandNamesOverride value to convert.
/// @return     CommandNamesOverride Enum version of the CommandNamesOverride
CollectionDefs::CommandNamesOverride CollectionDefs::Builder::ToCommandNamesOverride(const gtString &defaultOverride)
{
  if (defaultOverride == L"recordParameters")
  {
    return CollectionDefs::recordParameters;
  }
  else
  {
    return CollectionDefs::none;
  }
}

 
/// @brief                  Utility method. Set the file name of the file which was
///                         created from the object representation of this class
/// @param[in]  filename    The name of the file to set
/// @return     Builder&    A reference to this builder.
CollectionDefs::Builder& CollectionDefs::Builder::SetFileName(gtString fileName)
{
  m_fileName = std::move(fileName);
  return *this;
}

/// @brief Ctor; no work is done here
/// @param Id The ID of the API to which the command belongs
/// @param Name The name of the API command to replace
/// @param Overrides The name of the new command which replaces the original one
APICommand::APICommand(gtUInt32 Id, const gtString &Name,
  const gtString &Overrides)
: id{Id}, name{Name}, overrides{Overrides}
{
}

CollectionDefs::Builder&
CollectionDefs::Builder::SetCommandName(gtUInt32 commandId,
    const gtString &originalCommandName,
    const gtString &newCommandName)
{
  m_commands.insert ( std::pair<gtUInt32, APICommand>(commandId,APICommand{ commandId, originalCommandName,
      newCommandName }) );
  //m_commands[commandId] = APICommand{ commandId, originalCommandName,
  //    newCommandName };

  return *this;
}

CollectionDefs::Builder&
CollectionDefs::Builder::SetCommandName(gtUInt32 apiId,
    const gtString &originalCommandName,
    const gtString &newCommandName, const std::unordered_map<gtUInt64, ApiElement>&apisLUT)
{

  std::string cmdName;
  printf("SetCommandName: Looking for Api %u, command %s \n", apiId, originalCommandName.asASCIICharArray());
  auto apiInfo = apisLUT.find(apiId);
  // assert(apiInfo != apisLUT.end());

  
  // assert(cmdInfoPtr != apiInfo->second.cmdNamesLUT.end());

  if (apiInfo != apisLUT.end())
  {
      // TIZEN FIX set cmdName before first use
      cmdName.assign(originalCommandName.asASCIICharArray());
      auto cmdInfoPtr = apiInfo->second.cmdNamesLUT.find(cmdName);
      if (cmdInfoPtr != apiInfo->second.cmdNamesLUT.end())
      {
          gtUInt32 commandId = (gtUInt32)cmdInfoPtr->second;

          m_commands.insert(std::pair<gtUInt32, APICommand>(commandId, APICommand{ commandId, originalCommandName,
              newCommandName }));
      }
      
  } 

  return *this;
}


bool CollectionDefs::IsParamCaptureEnabled() const
{
  if (m_commandsDefaultOverride == recordParameters)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/// @brief Whether the screenshot capture is enabled or not
/// @return Always false for now as this feature is not supported
bool CollectionDefs::IsScreenshotCaptureEnabled() const
{
  return false;
}

/// @brief Whether the GPU timing is enabled or not
/// @return True if enabled, false if not
bool CollectionDefs::IsGPUTimingEnabled() const
{
  return m_gpuTimingEnabled;
}

gtString CollectionDefs::GetGPUTimingMode() const
{
  gtString returnVal;
  returnVal.fromASCIIString(ToString(m_gpuTimingMode));
  return returnVal;
}

bool CollectionDefs::IsCallStackDumpEnabled() const
{
  return m_callStackDumpEnabled;
}

bool CollectionDefs::IsKHRDebugEnabled() const
{
  return m_kHRDebugEnabled;
}

bool CollectionDefs::IsShaderSourceCaptureEnabled() const
{
  return m_captureShaderSourcesEnabled;
}
