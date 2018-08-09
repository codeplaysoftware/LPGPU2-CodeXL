// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppTargetDefinition.h
///
/// @brief Read the XML from the TargetCharacteristics.xml file and stores
///        it in a
///        data structure which mimics the file's XML hierarchy.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2PPTARGETDEFINITION_H
#define LPGPU2PPTARGETDEFINITION_H

// Lib
#include <tinyxml.h>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppXMLParserCommon.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// STL
// Introduce:
// std::shared_ptr
#include <memory>

namespace lpgpu2 {

/// @brief    Concrete class representing a Counter XML element in the
///           TargetDefinition file.
/// @date     04/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API CounterElement final : public TElement
{
// Methods
public:
  CounterElement(TiXmlElement *pCounter, TElement *pParent = nullptr);
  CounterElement(CounterElement &&);

// Defaults
public:
  CounterElement() = default;
  ~CounterElement() = default;
  CounterElement(const CounterElement &) = default;
  CounterElement &operator=(const CounterElement &) = default;

// Deleted
public:
  CounterElement &operator=(CounterElement &&) = delete;

// Accessors
public:
  const gtString &GetCategory() const { return m_category; }
  const gtString &GetDescription() const { return m_description; }
  const gtString &GetName() const { return m_name; }
  const gtString &GetType() const { return m_type; }
  const gtString &GetUnit() const { return m_unit; }
  gtUInt32 GetId() const { return m_id; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtString m_category;
  gtString m_description;
  gtString m_name;
  gtString m_type;
  gtString m_unit;
  gtUInt32 m_id = 0U;

}; // class CounterElement

/// @brief    Concrete class representing a CounterSet XML element in the
///           TargetDefinition file.
/// @date     04/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API CounterSetElement final : public TElement
{
// Type aliases
public:
  using CounterElemsVec = gtVector<CounterElement>;

// Methods
public:
  CounterSetElement(TiXmlElement *pCounterSet, TElement *pParent = nullptr);
  CounterSetElement(CounterSetElement &&);

// Defaults
public:
  CounterSetElement() = default;
  ~CounterSetElement() = default;
  CounterSetElement(const CounterSetElement &) = default;
  CounterSetElement &operator=(const CounterSetElement &) = default;

// Deleted
public:
  CounterSetElement &operator=(CounterSetElement &&) = delete;

// Accessors
public:
  const gtString &GetCategory() const { return m_category; }
  const gtString &GetDescription() const { return m_description; }
  gtUInt32 GetId() const { return m_id; }
  const gtString &GetName() const { return m_name; }
  const CounterElemsVec &GetCounterElemsVec() const { return m_counters; }
  gtUInt32 GetNumCounters() const { return m_numCounters; }
  gtUInt32 GetMinSamplingInterval() const { return m_minimumSamplingInterval; }
  gtUInt32 GetSupportedModes() const { return m_supportedModes; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtString m_category;
  gtString m_description;
  gtUInt32 m_id = 0U;
  gtString m_name;
  CounterElemsVec m_counters;
  gtUInt32 m_numCounters = 0U;
  gtUInt32 m_minimumSamplingInterval = 0U;
  gtUInt32 m_supportedModes = 0U;

}; // class CounterSetElement

/// @brief    Concrete class representing a Target XML element in the
///           TargetDefinition file.
/// @date     04/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API TargetElement final : public TElement
{
// Type aliases
public:
  using CounterSetElemsVec = gtVector<CounterSetElement>;

// Methods
public:
  TargetElement(TiXmlElement* pTarget, TElement* pParent = nullptr);

// Default
public:
  TargetElement() = default;
  TargetElement(const TargetElement &) = default;
  TargetElement &operator=(const TargetElement &) = default;
  ~TargetElement() = default;
  TargetElement(TargetElement &&) = default;
  TargetElement &operator=(TargetElement &&) = default;

// Accessors
public:
  const CounterSetElemsVec &GetCounterElemsVec() const {
    return m_counterSetsVec;
  }
  const gtString &GetPlatform() const { return m_platform; }
  const gtString &GetHardware() const { return m_hardware; }
  const gtString &GetDCAPIVersion() const { return m_DCAPIVersion; }
  const gtString &GetRAgentVersion() const { return m_RAgentVersion; }
  const gtString &GetHardwareID() const { return m_hardwareID; }
  gtUInt32 GetBlobSize() const { return m_blobSize; }
  PPFnStatus writeToFile(const gtString &xmlFilePath);

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  CounterSetElemsVec m_counterSetsVec;
  gtString m_platform;
  gtString m_hardware;
  gtString m_DCAPIVersion;
  gtString m_RAgentVersion;
  gtString m_hardwareID;
  gtUInt32 m_blobSize = 0U;
  TiXmlElement *m_pTargetElement = nullptr;

}; // class TargetElement

/// @brief    Class representing the TargetDefition file after parsing.
///           Aggregates the various elements forming the TargetDefinition
///           file.
/// @date     04/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API TargetDefinition
{
// Methods
public:
  PPFnStatus init(const gtString &xmlFilePath);
  PPFnStatus init(TiXmlElement &startElem);
  PPFnStatus shutdown() { return PPFnStatus::success; }
  PPFnStatus writeToFile(const gtString &xmlFilePath);

// Defaults
public:
  TargetDefinition() = default;
  ~TargetDefinition() = default;
  TargetDefinition(const TargetDefinition &) = default;
  TargetDefinition &operator=(const TargetDefinition &) = default;
  TargetDefinition(TargetDefinition &&) = default;
  TargetDefinition &operator=(TargetDefinition &&) = default;

// Deleted
public:

// Accessors
public:
  const TargetElement &GetTargetElement() const { return m_target; }

// Attributes
private:
  TargetElement m_target;

}; // TargetDefinition

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2PPTARGETDEFINITION_H
