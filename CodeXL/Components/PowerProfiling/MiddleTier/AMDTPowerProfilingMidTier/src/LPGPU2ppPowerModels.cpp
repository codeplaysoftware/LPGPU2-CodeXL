// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppPowerModels.cpp
///
/// @brief Read the XML data from the PowerModels XML file and stores
///        it in a
///        data structure which mimics the file's XML hierarchy.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppPowerModels.h>

// STL
// Introduce:
// std::move
#include <utility>

namespace lpgpu2 {

/// @brief Ctor; no work is done here
/// @param[in]  pElem    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
PPMCounterElement::PPMCounterElement(TiXmlElement *pElem, TElement *pParent)
: TElement{pElem, pParent},
  m_id{0U},
  m_name{},
  m_description{},
  m_model_mapped_name{}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
PPMCounterElement::PPMCounterElement(PPMCounterElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_id{rhs.m_id},
  m_name{std::move(rhs.m_name)},
  m_description{std::move(rhs.m_description)},
  m_model_mapped_name{std::move(rhs.m_model_mapped_name)}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
///         For now, it always returns PPFnStatus::success
PPFnStatus PPMCounterElement::initImpl()
{
  GetValueAndIterFromParent(m_pElement, "id", m_id);
  GetValueAndIterFromParent(m_pElement, "name", m_name);
  GetValueAndIterFromParent(m_pElement, "description", m_description);
  GetValueAndIterFromParent(m_pElement, "model_mapped_name", m_model_mapped_name);

  return PPFnStatus::success;
}

/// @brief Ctor; no work is done here
/// @param[in]  pElem    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
DeviceElement::DeviceElement(TiXmlElement *pElem, TElement *pParent)
: TElement{pElem, pParent},
  m_id{0U},
  m_name{},
  m_description{},
  m_counters{}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
DeviceElement::DeviceElement(DeviceElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_id{rhs.m_id},
  m_name{std::move(rhs.m_name)},
  m_description{std::move(rhs.m_description)},
  m_counters{std::move(rhs.m_counters)}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
///         For now, it always returns PPFnStatus::success
PPFnStatus DeviceElement::initImpl()
{
  GetValueAndIterFromParent(m_pElement, "id", m_id);
  GetValueAndIterFromParent(m_pElement, "name", m_name);
  GetValueAndIterFromParent(m_pElement, "description", m_description);

  auto *firstChild = m_pElement->FirstChildElement("available_counters");
  auto *oldPElem = m_pElement;
  if (firstChild)
  {
    m_pElement = firstChild;
    GetChildren("counter", m_counters);
  }

  m_pElement = oldPElem;

  return PPFnStatus::success;
}

/// @brief Ctor; no work is done here
/// @param[in]  pElem    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
PPMParameterElement::PPMParameterElement(TiXmlElement *pElem, TElement *pParent)
: TElement{pElem, pParent},
  m_name{},
  m_type{},
  m_value{}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
PPMParameterElement::PPMParameterElement(PPMParameterElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_name{std::move(rhs.m_name)},
  m_type{std::move(rhs.m_type)},
  m_value{std::move(rhs.m_value)}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return RC::Success if init succeeded, RC::Failure if not
///         For now, it always returns RC::Success
PPFnStatus PPMParameterElement::initImpl()
{
  GetValueAndIterFromParent(m_pElement, "name", m_name);
  GetValueAndIterFromParent(m_pElement, "type", m_type);
  GetValueAndIterFromParent(m_pElement, "value", m_value);

  return PPFnStatus::success;
}

/// @brief Ctor; no work is done here
/// @param[in]  pElem    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
PPMDeviceElement::PPMDeviceElement(TiXmlElement *pElem, TElement *pParent)
: TElement{pElem, pParent},
  m_id{0U},
  m_requiredCounters{},
  m_parameters{}  
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
PPMDeviceElement::PPMDeviceElement(PPMDeviceElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_id{rhs.m_id},
  m_requiredCounters{std::move(rhs.m_requiredCounters)},
  m_parameters{std::move(rhs.m_parameters)}  
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
///         For now, it always returns PPFnStatus::success
PPFnStatus PPMDeviceElement::initImpl()
{
  GetValueAndIterFromParent(m_pElement, "id", m_id);

  const auto *pReqCounters = m_pElement->FirstChildElement("required_counters");
  if (!pReqCounters)
  {
    return PPFnStatus::failure;
  }

  gtUInt32 val = 0U;

  // Get the first counterId element from required_counters
  const auto *pReqCount = pReqCounters->FirstChildElement("counterId");

  while(pReqCount) {
    pReqCount = GetValueAndIter(pReqCount, "counterId", val);
    m_requiredCounters.push_back(val);
  }
  // ++ Sohan:LPGPU2 Parse power model spefic parameters
  auto *firstChild = m_pElement->FirstChildElement("parameters");
  auto *oldPElem = m_pElement;
  if (firstChild)
  {
    m_pElement = firstChild;
    GetChildren("parameter", m_parameters);
  }
  m_pElement = oldPElem;  
  // --Sohan: Parse power model spefic parameters

  return PPFnStatus::success;
}


/// @brief Ctor; no work is done here
/// @param[in]  pElem    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
PwrModelElement::PwrModelElement(TiXmlElement *pElem, TElement *pParent)
: TElement{pElem, pParent},
  m_uuid{},
  m_name{},
  m_description{},
  m_longDescription{},
  m_devices{}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
PwrModelElement::PwrModelElement(PwrModelElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_uuid{rhs.m_uuid},
  m_name{std::move(rhs.m_name)},
  m_description{std::move(rhs.m_description)},
  m_longDescription{std::move(rhs.m_longDescription)},
  m_devices{std::move(rhs.m_devices)}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
///         For now, it always returns PPFnStatus::success
PPFnStatus PwrModelElement::initImpl()
{
  GetValueAndIterFromParent(m_pElement, "uuid", m_uuid);
  GetValueAndIterFromParent(m_pElement, "name", m_name);
  GetValueAndIterFromParent(m_pElement, "description", m_description);
  GetValueAndIterFromParent(m_pElement, "longDescription", m_longDescription);

  auto *firstChild = m_pElement->FirstChildElement("supportedDevices");
  auto *oldPElem = m_pElement;
  if (firstChild)
  {
    m_pElement = firstChild;
    GetChildren("device", m_devices);
  }

  m_pElement = oldPElem;

  return PPFnStatus::success;
}

/// @brief Ctor; no work is done here
/// @param[in]  pElem    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
SupportedPwrModelsElement::SupportedPwrModelsElement(TiXmlElement *pElem,
    TElement *pParent)
: TElement{pElem, pParent},
  m_pwrModels{},
  m_devices{}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
SupportedPwrModelsElement::SupportedPwrModelsElement(
    SupportedPwrModelsElement&&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_pwrModels{std::move(rhs.m_pwrModels)},
  m_devices{std::move(rhs.m_devices)}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
///         For now, it always returns PPFnStatus::success
PPFnStatus SupportedPwrModelsElement::initImpl()
{
  auto *firstChild = m_pElement->FirstChildElement("devices");
  auto *oldPElem = m_pElement;
  if (firstChild)
  {
    m_pElement = firstChild;
    GetChildren("device", m_devices);
  }

  m_pElement = oldPElem;

  firstChild = m_pElement->FirstChildElement("power_models");
  oldPElem = m_pElement;
  if (firstChild)
  {
    m_pElement = firstChild;
    GetChildren("power_model", m_pwrModels);
  }

  m_pElement = oldPElem;

  return PPFnStatus::success;
}

/// @brief Init method. Read the file in and parse it
/// @param[in] xmlFilePath Path to the XML file to parse
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
PPFnStatus PowerModels::init(const gtString &xmlFilePath)
{
  TiXmlDocument doc(xmlFilePath.asUTF8CharArray());
  bool loadOk = doc.LoadFile();
  auto initOk = PPFnStatus::failure;

  GT_IF_WITH_ASSERT_EX(loadOk, L"Loading file failed.")
  {
    m_pwrModels.reset(new SupportedPwrModelsElement{
        doc.FirstChildElement("supported_devices_power_models")});
    initOk = m_pwrModels->init();
  }

  return initOk;
}

/// @brief Init method. Parse the already loaded XML doc starting from
///        startElem
/// @param[in] startElem The base element to use start from when reading
///            the XML
/// @return Always returns PPFnStatus::success
PPFnStatus PowerModels::init(TiXmlElement &startElem)
{
  m_pwrModels.reset(new SupportedPwrModelsElement{&startElem});
  return m_pwrModels->init();
}

} // namespace lpgpu2
