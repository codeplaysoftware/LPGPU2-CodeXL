// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2TargetCharacteristics.cpp
///
/// @brief Reads XML from the TargetCharacteristics.xml file and stores it in a
///        data structure which mimics the file's XML hierarchy.
///
/// TargetCharacteristics implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfilingMidTier/include/ppLPGPU2TargetCharacteristics.h>
#include <AMDTOSWrappers/Include/osFile.h>

#include <string.h>
// ============================================================================
// Base class for all TargetCharacteristics XML elements
// ============================================================================

/// @brief Constructor
/// @param[in]  pElement    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TCElement object.
TCElement::TCElement(TiXmlElement* pElement, TCElement* pParent)
: m_pElement{pElement}
, m_pParent{pParent}
, m_textStr{}
{
    m_textStr.fromASCIIString(pElement->GetText());
}

/// @brief Gets all of the children of this element which match the specified
///        element name.
/// @param[in]  elementName The element name of the children to search for.
/// @param[out] children    The vector which will contain the results.
/// @return     bool        True if found, false if not.
template <typename T>
bool TCElement::GetChildren(const char* elementName, gtVector<T>& children)
{
    TiXmlElement* pChild = m_pElement->FirstChildElement(elementName);
    bool found = pChild != nullptr;
    while (pChild != nullptr)
    {
        children.emplace_back(pChild, this);
        pChild = pChild->NextSiblingElement(elementName);
    }
    return found;
}

/// @brief Gets this XML element's text.
/// @return     gtString    XML element's text.
gtString TCElement::GetText() const
{
    return m_textStr;
}

/// @brief Gets the string value of the attribute with the specified name
///        within this XML element.
/// @param[in]  attrName    The name of the attribute.
/// @param[out] value       The string value of the attribute.
/// @return     bool        True if found, false if not.
bool TCElement::GetAttribute(const char* attrName, gtString* value)
{
    const char* ret = m_pElement->Attribute(attrName);
    value->fromASCIIString(ret);
    return value != NULL;
}

/// @brief Gets the integer value of the attribute with the specified name
///        within this XML element.
/// @param[in]  attrName    The name of the attribute.
/// @param[out] value       The integer value of the attribute.
/// @return     bool        True if found, false if not.
bool TCElement::GetAttribute(const char* attrName, gtInt32* value)
{
    int signedValue = 0;
    const char* ret = m_pElement->Attribute(attrName, &signedValue);
    if (ret != nullptr)
    {
        *value = static_cast<gtInt32>(signedValue);
    }
    return ret != nullptr;
}

/// @brief Gets the integer value of the attribute with the specified name
///        within this XML element.
/// @param[in]  attrName    The name of the attribute.
/// @param[out] value       The unsigned integer value of the attribute.
/// @return     bool        True if found, false if not.
/// @note We use a ptr to pass value to conform with the existing API
bool TCElement::GetAttribute(const char *attrName, gtUInt32 *value)
{
    int signedValue = 0;
    const char* ret = m_pElement->Attribute(attrName, &signedValue);
    if (ret != nullptr)
    {
      *value = static_cast<gtUInt32>(signedValue);
    }
    return ret != nullptr;
}

/// @brief Gets the integer value of the attribute with the specified name
///        within this XML element.
/// @param[in] attrName The name of the attribute.
/// @param[out] value   The gtUInt64 value of the attribute.
/// @return     bool    true if found, false if not.
/// @note We use a ptr to pass value to conform with the existing API
bool TCElement::GetAttribute(const char *attrName, gtUInt64 *value)
{
    int signedValue = 0;
    const char* ret = m_pElement->Attribute(attrName, &signedValue);
    if (ret != nullptr)
    {
        *value = static_cast<gtUInt64>(signedValue);
    }
    return ret != nullptr;
}

// ============================================================================
// Machine XML element & children
// ============================================================================

/// @brief Reads the 'Path' XML element
/// @see TCElement
MachinePathElement::MachinePathElement(TiXmlElement* pElement, TCElement* pParent) : TCElement(pElement, pParent)
{
    bool isOk = GetAttribute("Library", &library);
    isOk &= GetAttribute("LibraryOpenError", &libraryOpenError);
    isOk &= GetAttribute("Name", &name);
    isOk &= GetAttribute("ReadError", &readError);
    isOk &= GetAttribute("ReadFile", &readFile);
    isOk &= GetAttribute("Readable", &readable);
    isOk &= GetAttribute("WriteError", &writeError);
    isOk &= GetAttribute("Writable", &writable);
    GT_ASSERT_EX(isOk, L"Error when reading Path.");
}

/// @brief Reads the 'Properties' XML element
/// @see TCElement
MachinePropertiesElement::MachinePropertiesElement(TiXmlElement* pElement, TCElement* pParent) : TCElement(pElement, pParent)
{

}

/// @brief Reads the 'Property' XML element
/// @see TCElement
MachinePropertyElement::MachinePropertyElement(TiXmlElement* pElement, TCElement* pParent) : TCElement(pElement, pParent)
{
    bool isOk = GetAttribute("Name", &name);
    isOk &= GetAttribute("Value", &value);
    GT_ASSERT_EX(isOk, L"Error when reading Property.");
}

/// @brief Reads the 'Machine' XML element
/// @see TCElement
MachineElement::MachineElement(TiXmlElement* pElement, TCElement* pParent) : TCElement(pElement, pParent)
{
// TODO Uncomment these once SAMS changes the TargetCharacteristics.xml file
// produced by the device back to how it was.
// "endian", "int" and "long" all were uppercase before. The commented out
// attributes and children were uncommented.
    bool isOk = GetAttribute("endian", &endian);
    isOk &= GetAttribute("int", &intSize);
    isOk &= GetAttribute("long", &longSize);
    //isOk &= GetAttribute("Name", &name);
    isOk &= GetChildren("Path", paths);
    //isOk &= GetChildren("Properties", propertiesVec);
    //isOk &= GetChildren("Property", propertyVec);
    GT_ASSERT_EX(isOk, L"Error when reading Machine.");
}

// ============================================================================
// Process XML element & children
// ============================================================================

/// @brief Reads the 'CommandLine' XML element
/// @see TCElement
ProcessCommandLineElement::ProcessCommandLineElement(TiXmlElement* pCmdLine, TCElement* pParent) : TCElement(pCmdLine, pParent)
{
    bool isOk = GetAttribute("Name", &name);
    GT_ASSERT_EX(isOk, L"Error when reading CommandLine.");
}

/// @brief Reads the 'Env' XML element
/// @see TCElement
ProcessEnvElement::ProcessEnvElement(TiXmlElement* pEnv, TCElement* pParent) : TCElement(pEnv, pParent)
{
    bool isOk = GetAttribute("Name", &name);
    isOk &= GetAttribute("Value", &value);
    GT_ASSERT_EX(isOk, L"Error when reading Env.");
}

/// @brief Reads the 'Process' XML element
/// @see TCElement
ProcessElement::ProcessElement(TiXmlElement* pProcess, TCElement* pParent) : TCElement(pProcess, pParent)
{
    bool isOk = GetAttribute("pid", &pid);
    isOk &= GetChildren("Env", envs);
// TODO Uncomment this once SAMS changes the TargetCharacteristics.xml file
// produced by the device back to how it was.
    //isOk &= GetChildren("CommandLine", commandLines);
    GT_ASSERT_EX(isOk, L"Error when reading CommandLine.");
}

// ============================================================================
// CounterSet XML element & children
// ============================================================================

/// @brief Reads the 'Counter' XML element
/// @see TCElement
CounterElement::CounterElement(TiXmlElement* pCounter, TCElement* pParent) : TCElement(pCounter, pParent)
{
    bool isOk = GetAttribute("Category", &category);
    isOk &= GetAttribute("Description", &description);
    isOk &= GetAttribute("Id", &id);
    isOk &= GetAttribute("Name", &name);
    isOk &= GetAttribute("Type", &type);
    isOk &= GetAttribute("Unit", &unit);
    GT_ASSERT_EX(isOk, L"Error when reading Counter.");
}

/// @brief Reads the 'CounterSet' XML element
/// @see TCElement
CounterSetElement::CounterSetElement(TiXmlElement* pCounterSet, TCElement* pParent) : TCElement(pCounterSet, pParent)
{
    bool isOk = GetAttribute("Category", &category);
    isOk &= GetAttribute("Description", &description);
    isOk &= GetAttribute("Id", &id);
    isOk &= GetAttribute("Name", &name);
    isOk &= GetAttribute("NumCounters", &numCounters);
    isOk &= GetChildren("Counter", counters);
    GT_ASSERT_EX(isOk, L"Error when reading CounterSet.");
}

// ============================================================================
// Dcapi XML element & children
// ============================================================================

/// @brief Reads the 'Dcapi' XML element
/// @see TCElement
DcApiElement::DcApiElement(TiXmlElement* pDcApi, TCElement* pParent) : TCElement(pDcApi, pParent)
{
    bool isOk = GetAttribute("MaxSampleFrequency", &maxSampleFrequency);
    GT_ASSERT_EX(isOk, L"Error when reading Dcapi.");
}

// ============================================================================
// Commands XML element & children
// ============================================================================

/// @brief Reads the 'Parameter' XML element
/// @see TCElement
CommandParamElement::CommandParamElement(TiXmlElement* pParam, TCElement* pParent) : TCElement(pParam, pParent)
{
    bool isOk = GetAttribute("Name", &name);
    isOk &= GetAttribute("Size", &size);
    isOk &= GetAttribute("Type", &type);
    GT_ASSERT_EX(isOk, L"Error when reading Parameter.");
}

/// @brief Reads the 'Command' XML element
/// @see TCElement
CommandElement::CommandElement(TiXmlElement* pCommand, TCElement* pParent) : TCElement(pCommand, pParent)
{
    bool isOk = GetAttribute("Id", &id);
    isOk &= GetAttribute("Name", &name);
    isOk &= GetAttribute("ReturnSize", &returnSize);
    isOk &= GetAttribute("ReturnType", &returnType);
    isOk |= GetChildren("Parameter", params);
    GT_ASSERT_EX(isOk, L"Error when reading Command.");

//++AT:LPGPU2
    // Create the comma-separated type information
    typeInfo.assign(returnType);
    for (const auto &param : params)
    {
      typeInfo.append(L",");
      typeInfo.append(param.type);
    }
//--AT:LPGPU2
}

/// @brief Reads the 'Api' XML element
/// @see TCElement
ApiElement::ApiElement(TiXmlElement* pApi, TCElement* pParent) : TCElement(pApi, pParent)
{
    bool isOk = GetAttribute("Id", &id);
    isOk &= GetChildren("Command", commands);
    GT_ASSERT_EX(isOk, L"Error when reading Api.");
    std::string name;

    // Build LUT
    for (const auto &cmd : commands)
    {
      cmdsLUT[cmd.id] = cmd;
      name.assign(cmd.name.asASCIICharArray());
      cmdNamesLUT[name] = cmd.id;
    }
}

/// @brief Reads the 'Commands' XML element
/// @see TCElement
CommandsElement::CommandsElement(TiXmlElement* pCommands, TCElement* pParent) : TCElement(pCommands, pParent)
{
    bool isOk = GetChildren("Api", apis);
    GT_ASSERT_EX(isOk, L"Error when reading Commands.");

    // Build LUT
    for (const auto &api : apis)
    {
      apisLUT[api.id] = api;
    }
}

// ============================================================================
// Target characteristics
// ============================================================================

/// @brief Reads the 'Target' XML element
/// @see TCElement
TargetElement::TargetElement(TiXmlElement* pTarget, TCElement* pParent)
  : m_pTargetElement{pTarget}
{
  m_tcElem = std::make_shared<TCElement>(pTarget, pParent);
  bool isOk = m_tcElem->GetChildren("Machine", machineVec);
  isOk &= m_tcElem->GetChildren("Process", processVec);
  isOk &= m_tcElem->GetChildren("CounterSet", counterSetsVec);
  //isOk &= m_tcElem->GetChildren("Dcapi", dcApiVec);
  isOk &= m_tcElem->GetChildren("Commands", commandsVec);
  GT_ASSERT_EX(isOk, L"Error when reading Target."); }

/// @brief Constructor
/// @param[in]  xmlFilePath The path to the XML file to read.
TargetCharacteristics::TargetCharacteristics(gtString xmlFilePath)
{
  TiXmlDocument doc(xmlFilePath.asUTF8CharArray());
  bool loadOk = doc.LoadFile();
  GT_IF_WITH_ASSERT_EX(loadOk, L"Loading TargetCharacteristics file failed.")
  {
//++AT:LPGPU2
    target = TargetElement{doc.FirstChildElement("Target")};
//--AT:LPGPU2
  }
}

//++AT:LPGPU2
/// @brief Constructor
/// @param[in] startElem The base element to use start from when reading the XML 
TargetCharacteristics::TargetCharacteristics(TiXmlElement &startElem)
{
  target = TargetElement{&startElem};
}
  
void TargetCharacteristics::writeToFile(const gtString &xmlFilePath)
{
  target.writeToFile(xmlFilePath);
}
  
void TargetElement::writeToFile(const gtString &xmlFilePath)
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
  isOk = file.write(fileContent.asASCIICharArray(), fileContent.length());
  GT_ASSERT_EX(isOk, L"Failed to write XML file.");
  file.close();
  
}
//--AT:LPGPU2
