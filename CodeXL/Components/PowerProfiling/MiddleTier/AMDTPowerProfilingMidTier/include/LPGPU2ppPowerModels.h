// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppPowerModels.h
///
/// @brief Read the XML data from the PowerModels XML file and stores
///        it in a
///        data structure which mimics the file's XML hierarchy.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2PPPOWERMODELS_H
#define LPGPU2PPPOWERMODELS_H

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

/// @brief    Concrete class representing a Counter XML element in
///           the Power Models XML file.
/// @date     10/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API PPMCounterElement final : public TElement
{
// Methods
public:
  PPMCounterElement(TiXmlElement *pElem, TElement *pParent = nullptr);
  PPMCounterElement(PPMCounterElement &&);

// Defaults
public:
  ~PPMCounterElement() = default;

// Deleted
public:
  PPMCounterElement() = delete;
  PPMCounterElement(const PPMCounterElement &) = delete;
  PPMCounterElement &operator=(const PPMCounterElement &) = delete;
  PPMCounterElement &operator=(PPMCounterElement &&) = delete;

// Accessors
public:
  gtUInt32 GetId() const { return m_id; }
  const gtString &GetName() const { return m_name; }
  const gtString &GetDescription() const { return m_description; }
  const gtString &GetModelMappedName() const { return m_model_mapped_name; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtUInt32 m_id;
  gtString m_name;
  gtString m_description;
  gtString m_model_mapped_name;

}; // class PPMCounterElement

/// @brief    Concrete class representing a Device XML element in the
///           Power Models XML file.
/// @date     10/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API DeviceElement final : public TElement
{
// Type aliases
public:
  using CounterElemsVec = gtVector<PPMCounterElement>;

// Methods
public:
  DeviceElement(TiXmlElement *pElem, TElement *pParent = nullptr);
  DeviceElement(DeviceElement &&);

// Defaults
public:
  ~DeviceElement() = default;

// Deleted
public:
  DeviceElement() = delete;
  DeviceElement(const DeviceElement &) = delete;
  DeviceElement &operator=(const DeviceElement &) = delete;
  DeviceElement &operator=(DeviceElement &&) = delete;

// Accessors
public:
  gtUInt32 GetId() const { return m_id; }
  const gtString &GetName() const { return m_name; }
  const gtString &GetDescription() const { return m_description; }
  const CounterElemsVec &GetCounterElemsVec() const { return m_counters; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtUInt32 m_id;
  gtString m_name;
  gtString m_description;
  CounterElemsVec m_counters;

}; // class DeviceElement

/// @brief    Concrete class representing a Parameter XML element in
///           the Power Models XML file.
/// @date     29/11/2017.
/// @author   Sohan Lal.
class AMDTPOWERPROFILINGMIDTIER_API PPMParameterElement final : public TElement
{
// Methods
public:
  PPMParameterElement(TiXmlElement *pElem, TElement *pParent = nullptr);
  PPMParameterElement(PPMParameterElement &&);

// Defaults
public:
  ~PPMParameterElement() = default;

// Deleted
public:
  PPMParameterElement() = delete;
  PPMParameterElement(const PPMParameterElement &) = delete;
  PPMParameterElement &operator=(const PPMParameterElement &) = delete;
  PPMParameterElement &operator=(PPMParameterElement &&) = delete;

// Accessors
public:
  const gtString &GetName() const { return m_name; }
  const gtString &GetType() const { return m_type; }
  const gtString &GetValue() const { return m_value; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtString m_name;
  gtString m_type;
  gtString m_value;

}; // class PPMParameterElement


/// @brief    Concrete class representing a per-power-model Device XML
///           element in the Power Models XML file.
/// @date     10/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API PPMDeviceElement final : public TElement
{
// Type aliases
public:
  using CounterIdsVec = gtVector<gtUInt32>;
  using ParameterElemsVec = gtVector<PPMParameterElement>;

// Methods
public:
  PPMDeviceElement(TiXmlElement *pCounter, TElement *pParent = nullptr);
  PPMDeviceElement(PPMDeviceElement &&);

// Defaults
public:
  ~PPMDeviceElement() = default;

// Deleted
public:
  PPMDeviceElement() = delete;
  PPMDeviceElement(const PPMDeviceElement &) = delete;
  PPMDeviceElement &operator=(const PPMDeviceElement &) = delete;
  PPMDeviceElement &operator=(PPMDeviceElement &&) = delete;

// Accessors
public:
  gtUInt32 GetId() const { return m_id; }
  const CounterIdsVec &GetRequiredCounters() const {
    return m_requiredCounters;
  }

  const ParameterElemsVec &GetParameterElemsVec() const { return m_parameters; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtUInt32 m_id;
  CounterIdsVec m_requiredCounters;
  ParameterElemsVec m_parameters;

}; // class PPMDeviceElement


/// @brief    Concrete class representing a Power Model XML element in the
///           Power Models XML file.
/// @date     10/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API PwrModelElement final : public TElement
{
// Type aliases
public:
  using DevicesElemsVec = gtVector<PPMDeviceElement>;

// Methods
public:
  PwrModelElement(TiXmlElement *pTarget, TElement *pParent = nullptr);
  PwrModelElement(PwrModelElement &&);

// Default
public:
  ~PwrModelElement() = default;

// Deleted
public:
  PwrModelElement() = delete;
  PwrModelElement(const PwrModelElement &) = delete;
  PwrModelElement &operator=(const PwrModelElement &) = delete;
  PwrModelElement &operator=(PwrModelElement &&) = delete;

// Accessors
public:
  const gtString GetUUID() const { return m_uuid; }
  const gtString &GetName() const { return m_name; }
  const gtString &GetDescription() const { return m_description; }
  const gtString &GetLongDescription() const { return m_longDescription; }
  const DevicesElemsVec &GetDevices() const { return m_devices; }

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Attributes
private:
  gtString m_uuid;
  gtString m_name;
  gtString m_description;
  gtString m_longDescription;
  DevicesElemsVec m_devices;

}; // class PwrModelElement

/// @brief    Class representing the Supported Power Models XML element.
/// @date     10/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API SupportedPwrModelsElement final
: public TElement
{
// Type aliases
public:
  using PowerModelsVec = gtVector<PwrModelElement>;
  using DeviceElemsVec = gtVector<DeviceElement>;

// Methods
public:
  SupportedPwrModelsElement(TiXmlElement *pTarget, TElement *pParent = nullptr);
  SupportedPwrModelsElement(SupportedPwrModelsElement &&);

// Default
public:
  ~SupportedPwrModelsElement() = default;

// Deleted
public:
  SupportedPwrModelsElement() = delete;
  SupportedPwrModelsElement(const SupportedPwrModelsElement &) = delete;
  SupportedPwrModelsElement &operator=(const SupportedPwrModelsElement &)
    = delete;
  SupportedPwrModelsElement &operator=(SupportedPwrModelsElement &&) = delete;

// Overridden
private:
  PPFnStatus initImpl() override;
  PPFnStatus shutdownImpl() override { return PPFnStatus::success; }

// Accessors
public:
  const PowerModelsVec &GetPwrModels() const { return m_pwrModels; }
  const DeviceElemsVec &GetDevices() const { return m_devices; }

// Attributes
private:
  PowerModelsVec m_pwrModels;
  DeviceElemsVec m_devices;

}; // SupportedPwrModelsElement

/// @brief    Class representing the Supported Power Models XML file after
///           parsing.
///           Aggregates the various elements forming the XML file.
/// @date     10/10/2017.
/// @author   Alberto Taiuti.
class AMDTPOWERPROFILINGMIDTIER_API PowerModels
{
// Methods
public:
  PPFnStatus init(const gtString &xmlFilePath);
  PPFnStatus init(TiXmlElement &startElem);
  PPFnStatus shutdown() { return PPFnStatus::success; }

// Defaults
public:
  PowerModels() = default;
  ~PowerModels() = default;
  PowerModels(const PowerModels &) = default;
  PowerModels &operator=(const PowerModels &) = default;

// Deleted
public:
  PowerModels(PowerModels &&) = delete;
  PowerModels &operator=(PowerModels &&) = delete;

// Accessors
public:
  const SupportedPwrModelsElement &GetElements() const {
    return *m_pwrModels.get();
  }

// Type aliases
private:
  using PMElmSP = std::shared_ptr<SupportedPwrModelsElement>;

// Attributes
private:
  PMElmSP m_pwrModels = nullptr;

}; // PowerModels

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2PPPOWERMODELS_H
