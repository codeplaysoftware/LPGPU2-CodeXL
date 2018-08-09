// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2TargetCharacteristics.h
///
/// @brief Reads XML from the TargetCharacteristics.xml file and stores it in a
///        data structure which mimics the file's XML hierarchy.
///
/// TargetCharacteristics interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __PPLPGPU2TARGETCHARACTERISTICS_H
#define __PPLPGPU2TARGETCHARACTERISTICS_H

#include <tinyxml.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>
//++AT:LPGPU2
// Import:
// smart ptrs
// unordered_map
#include <memory>
#include <string>
#include <unordered_map>
//--AT:LPGPU2

// ============================================================================
// Base class for all TargetCharacteristics XML elements
// ============================================================================

class AMDTPOWERPROFILINGMIDTIER_API TCElement
{
public:
    TCElement(TiXmlElement* pElement, TCElement* pParent = 0);

    template <typename T>
    bool GetChildren(const char* elementName, gtVector<T>& children);

    gtString GetText() const;

//++AT:LPGPU2
    TCElement() = default;
    virtual ~TCElement() = default;
//--AT:LPGPU2

protected:
    bool GetAttribute(const char* attrName, gtString* value);
    bool GetAttribute(const char* attrName, gtInt32* value);
    bool GetAttribute(const char *attrName, gtUInt32 *value);
    bool GetAttribute(const char *attrName, gtUInt64 *value);

private:
//++AT:LPGPU2
//  There are raw ptrs here because this class simply references them, it does
//  not manage them.
//--AT:LPGPU2
    TiXmlElement* m_pElement = nullptr;
    TCElement* m_pParent = nullptr;
    gtString m_textStr;
};

// ============================================================================
// Machine XML element & children
// ============================================================================

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API MachinePathElement final : public TCElement
//--AT:LPGPU2
{
public:
//++AT:LPGPU2
    MachinePathElement() = default;
//--AT:LPGPU2
    MachinePathElement(TiXmlElement* pElement, TCElement* pParent = 0);
    gtString library;
    gtString libraryOpenError;
    gtString name;
    gtString readError;
    gtString readFile;
    gtString readable;
    gtString writeError;
    gtString writable;
};

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API MachinePropertiesElement final : public TCElement
//--AT:LPGPU2
{
public:
    MachinePropertiesElement() = default;
    MachinePropertiesElement(TiXmlElement* pElement, TCElement* pParent = 0);
};

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API MachinePropertyElement final : public TCElement
//--AT:LPGPU2
{
public:
    MachinePropertyElement() = default;
    MachinePropertyElement(TiXmlElement* pElement, TCElement* pParent = 0);
    gtString name;
    gtString value;
};

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API MachineElement final : public TCElement
//--AT:LPGPU2
{
public:
    MachineElement() = default;
    MachineElement(TiXmlElement* pElement, TCElement* pParent = 0);
    gtInt32 endian;
    gtInt32 intSize;
    gtInt32 longSize;
    gtString name;
    gtVector<MachinePathElement> paths;
    gtVector<MachinePropertiesElement> propertiesVec;
    gtVector<MachinePropertyElement> propertyVec;
};

// ============================================================================
// Process XML element & children
// ============================================================================

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API ProcessCommandLineElement final : public TCElement
//--AT:LPGPU2
{
public:
    ProcessCommandLineElement() = default;
    ProcessCommandLineElement(TiXmlElement* pCmdLine, TCElement* pParent = 0);
    gtString name;
};

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API ProcessEnvElement final : public TCElement
//--AT:LPGPU2
{
public:
    ProcessEnvElement() = default;
    ProcessEnvElement(TiXmlElement* pEnv, TCElement* pParent = 0);
    gtString name;
    gtString value;
};

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API ProcessElement final : public TCElement
//--AT:LPGPU2
{
public:
    ProcessElement() = default;
    ProcessElement(TiXmlElement* pProcess, TCElement* pParent = 0);
    gtInt32 pid;
    gtVector<ProcessEnvElement> envs;
    gtVector<ProcessCommandLineElement> commandLines;
};

// ============================================================================
// CounterSet XML element & children
// ============================================================================

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API CounterElement final : public TCElement
//--AT:LPGPU2
{
public:
    CounterElement() = default;
    CounterElement(TiXmlElement* pCounter, TCElement* pParent = 0);
    gtString category;
    gtString description;
    gtString id;
    gtString name;
    gtString type;
    gtString unit;
};

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API CounterSetElement final : public TCElement
//--AT:LPGPU2
{
public:
    CounterSetElement(TiXmlElement* pCounterSet, TCElement* pParent = 0);
    gtString category;
    gtString description;
    gtString id;
    gtString name;
    gtInt32 numCounters;
    gtVector<CounterElement> counters;
};

// ============================================================================
// Dcapi XML element & children
// ============================================================================

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API DcApiElement final : public TCElement
//--AT:LPGPU2
{
public:
    DcApiElement() = default;
    DcApiElement(TiXmlElement* pDcApi, TCElement* pParent = 0);
    gtInt32 maxSampleFrequency;
};

// ============================================================================
// Commands XML element & children
// ============================================================================

//++AT:LPGPU2
class AMDTPOWERPROFILINGMIDTIER_API CommandParamElement final : public TCElement
//--AT:LPGPU2
{
public:
  CommandParamElement() = default;
  CommandParamElement(TiXmlElement* pParam, TCElement* pParent = 0);
  gtString name;
  gtUInt32 size;
  gtString type;
};

class AMDTPOWERPROFILINGMIDTIER_API CommandElement final : public TCElement
{
public:
  CommandElement() = default;
  CommandElement(TiXmlElement* pCommand, TCElement* pParent = 0);
  gtInt32 id;
  gtString name;
  gtInt32 returnSize;
  gtString returnType;
  gtVector<CommandParamElement> params;
  gtString typeInfo;
};

class AMDTPOWERPROFILINGMIDTIER_API ApiElement final : public TCElement
{
// Type aliases
public:
  using CmdID = gtUInt64;
  using CmdLUT = std::unordered_map<CmdID, CommandElement>;
  using CmdElementsVec = gtVector<CommandElement>;
  using CmdName = std::string;
  using CmdNamesLUT = std::unordered_map<CmdName, CmdID>;
public:
  ApiElement() = default;
  ApiElement(TiXmlElement* pApi, TCElement* pParent = 0);
  CmdID id;
  CmdElementsVec commands;

  CmdLUT      cmdsLUT;
  CmdNamesLUT cmdNamesLUT;
};

class AMDTPOWERPROFILINGMIDTIER_API CommandsElement final : public TCElement
{
// Type aliases
public:
  using ApiID = gtUInt64;
  using ApiLUT = std::unordered_map<ApiID, ApiElement>;
  using ApisVec = gtVector<ApiElement>;

public:
  CommandsElement() = default;
  CommandsElement(TiXmlElement* pCommands, TCElement* pParent = 0);
  ApisVec apis;
  ApiLUT apisLUT;

};

// ============================================================================
// Target characteristics
// ============================================================================

class AMDTPOWERPROFILINGMIDTIER_API TargetElement
{
public:
  TargetElement(TiXmlElement* pTarget, TCElement* pParent = 0);
  TargetElement() = default;
  TargetElement(const TargetElement &other) = default;
  TargetElement &operator=(const TargetElement &rhs) = default;
  void writeToFile(const gtString &xmlFilePath);

// Type aliases
public:
  using MachineElements = gtVector<MachineElement>;
  using ProcessElements = gtVector<ProcessElement>;
  using CounterSetElements = gtVector<CounterSetElement>;
  using DcApiElements = gtVector<DcApiElement>;
  using CommandsElements = gtVector<CommandsElement>;

  MachineElements machineVec;
  ProcessElements processVec;
  CounterSetElements counterSetsVec;
  DcApiElements dcApiVec;
  CommandsElements commandsVec;

private:
  std::shared_ptr<TCElement> m_tcElem;
  TiXmlElement *m_pTargetElement = nullptr;

};

class AMDTPOWERPROFILINGMIDTIER_API TargetCharacteristics
{
public:
  TargetCharacteristics(gtString xmlFilePath);
  TargetCharacteristics(TiXmlElement &startElem);
  TargetCharacteristics() = default;
  TargetCharacteristics(const TargetCharacteristics &other) = default;
  TargetCharacteristics &operator=(const TargetCharacteristics &rhs) = default;
  ~TargetCharacteristics() = default;

  void writeToFile(const gtString &xmlFilePath);

  const TargetElement::MachineElements &GetMachine() const
  {
    return target.machineVec;
  }
  const TargetElement::ProcessElements &GetProcess() const
  {
    return target.processVec;
  }
  const TargetElement::CounterSetElements &GetCounterSets() const
  {
    return target.counterSetsVec;
  }
  const TargetElement::DcApiElements &GetDcApi() const
  {
    return target.dcApiVec;
  }
  const TargetElement::CommandsElements &GetCommands() const
  {
    return target.commandsVec;
  }

private:
  TargetElement target;
};

#endif
