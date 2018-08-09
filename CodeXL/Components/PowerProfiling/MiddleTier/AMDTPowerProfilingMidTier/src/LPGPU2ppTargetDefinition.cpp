// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppTargetDefinition.cpp
///
/// @brief Read the XML from the TargetCharacteristics.xml file and stores
///        it in a
///        data structure which mimics the file's XML hierarchy.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppTargetDefinition.h>

#include <AMDTOSWrappers/Include/osFile.h>

// STL
// Introduce:
// std::move
#include <utility>

namespace lpgpu2 {

/// @brief Ctor; no work is done here
/// @param[in]  pElement    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
CounterElement::CounterElement(TiXmlElement *pCounter, TElement *pParent)
: TElement{pCounter, pParent},
  m_category{},
  m_description{},
  m_name{},
  m_type{},
  m_unit{},
  m_id{0U}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
CounterElement::CounterElement(CounterElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_category{std::move(rhs.m_category)},
  m_description{std::move(rhs.m_description)},
  m_name{std::move(rhs.m_name)},
  m_type{std::move(rhs.m_type)},
  m_unit{std::move(rhs.m_unit)},
  m_id{rhs.m_id}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
PPFnStatus CounterElement::initImpl()
{
  auto isOk = GetAttribute("Category", m_category);
  isOk &= GetAttribute("Description", m_description);
  isOk &= GetAttribute("Name", m_name);
  isOk &= GetAttribute("Type", m_type);
  isOk &= GetAttribute("Unit", m_unit);
  gtString idStr;
  isOk &= GetAttribute("Id", idStr);
  isOk &= idStr.toUnsignedIntNumber(m_id);
  GT_ASSERT_EX(isOk, L"Error when reading CounterElement.");

  return isOk ? PPFnStatus::success : PPFnStatus::failure;
}

/// @brief Ctor; no work is done here
/// @param[in]  pElement    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
CounterSetElement::CounterSetElement(TiXmlElement *pCounterSet,
    TElement *pParent)
: TElement{pCounterSet, pParent},
  m_category{},
  m_description{},
  m_id{0U},
  m_name{},
  m_counters{},
  m_numCounters{0U},
  m_minimumSamplingInterval{1U},
  m_supportedModes{0U}
{
}

/// @brief Move ctor; no work is done here
/// @param[in] rhs The object to move from.
CounterSetElement::CounterSetElement(CounterSetElement &&rhs)
: TElement{rhs.m_pElement, rhs.m_pParent},
  m_category{std::move(rhs.m_category)},
  m_description{std::move(rhs.m_description)},
  m_id{rhs.m_id},
  m_name{std::move(rhs.m_name)},
  m_counters{std::move(rhs.m_counters)},
  m_numCounters{rhs.m_numCounters},
  m_minimumSamplingInterval{rhs.m_minimumSamplingInterval},
  m_supportedModes{rhs.m_supportedModes}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
PPFnStatus CounterSetElement::initImpl()
{
  auto isOk = GetAttribute("Category", m_category);
  isOk &= GetAttribute("Description", m_description);
  gtString conversionStr;
  isOk &= GetAttribute("Id", conversionStr);
  isOk &= conversionStr.toUnsignedIntNumber(m_id);
  isOk &= GetAttribute("Name", m_name);
  isOk &= GetAttribute("NumCounters", m_numCounters);
  isOk &= GetAttribute("MinimumSamplingInterval", conversionStr);
  isOk &= conversionStr.toUnsignedIntNumber(m_minimumSamplingInterval);
  isOk &= GetAttribute("SupportedModes", conversionStr);
  isOk &= conversionStr.toUnsignedIntNumber(m_supportedModes);
  isOk &= GetChildren("Counter", m_counters);
  GT_ASSERT_EX(isOk, L"Error when reading CounterSet.");

  return isOk ? PPFnStatus::success : PPFnStatus::failure;
}

/// @brief Ctor; no work is done here
/// @param[in]  pElement    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
TargetElement::TargetElement(TiXmlElement *pTarget, TElement *pParent)
: TElement{pTarget, pParent},
  m_counterSetsVec{},
  m_platform{},
  m_hardware{},
  m_DCAPIVersion{},
  m_RAgentVersion{},
  m_blobSize{0U},
  m_pTargetElement{pTarget}
{
}

/// @brief Impl of the init method. Read the attributes of the element
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
PPFnStatus TargetElement::initImpl()
{
  auto isOk = GetChildren("CounterSet", m_counterSetsVec);
  isOk &= GetAttribute("Platform", m_platform);
  isOk &= GetAttribute("Hardware", m_hardware);
  isOk &= GetAttribute("DCAPI", m_DCAPIVersion);
  isOk &= GetAttribute("RAgent", m_RAgentVersion);
  isOk &= GetAttribute("HardwareID", m_hardwareID);
  gtString conversionStr;
  isOk &= GetAttribute("BlobSize", conversionStr);
  isOk &= conversionStr.toUnsignedIntNumber(m_blobSize);

  GT_ASSERT_EX(isOk, L"Error when reading Target.");

  return isOk ? PPFnStatus::success : PPFnStatus::failure;
}

/// @brief Init method. Read the file in and parse it
/// @param[in] xmlFilePath Path to the XML file to parse
/// @return PPFnStatus::success if init succeeded, PPFnStatus::failure if not
PPFnStatus TargetDefinition::init(const gtString &xmlFilePath)
{
  TiXmlDocument doc(xmlFilePath.asUTF8CharArray());
  bool loadOk = doc.LoadFile();
  GT_IF_WITH_ASSERT_EX(loadOk, L"Loading TargetCharacteristics file failed.")
  {
    auto target = TargetElement{doc.FirstChildElement("Target")};
    m_target = target;
    m_target.init();
  }

  return loadOk ? PPFnStatus::success : PPFnStatus::failure;
}

/// @brief Init method. Parse the already loaded XML doc starting from
///        startElem
/// @param[in] startElem The base element to use start from when reading
///            the XML
/// @return Always returns PPFnStatus::success
PPFnStatus TargetDefinition::init(TiXmlElement &startElem)
{
  auto target = TargetElement{&startElem};
  m_target = target;
  m_target.init();

  return PPFnStatus::success;
}

PPFnStatus TargetElement::writeToFile(const gtString &xmlFilePath)
{
  // Create the collection definitions XML string to write to a file
  TiXmlPrinter printer;
  printer.SetIndent("\t");
  m_pTargetElement->Accept(&printer);
  gtString fileContent;
  fileContent.fromASCIIString(printer.CStr());

  // Create and/or write to the file
  osFile file(xmlFilePath);
  bool isOk = file.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
  GT_ASSERT_EX(isOk, L"Failed to open XML file.");
  if (!isOk)
  {
    return PPFnStatus::failure;
  }
  isOk = file.write(fileContent.asASCIICharArray(), fileContent.length());
  if (!isOk)
  {
    return PPFnStatus::failure;
  }
  GT_ASSERT_EX(isOk, L"Failed to write XML file.");
  file.close();

  return PPFnStatus::success;
}

PPFnStatus TargetDefinition::writeToFile(const gtString &xmlFilePath)
{ 
  return m_target.writeToFile(xmlFilePath);
}

} // namespace lpgpu2
