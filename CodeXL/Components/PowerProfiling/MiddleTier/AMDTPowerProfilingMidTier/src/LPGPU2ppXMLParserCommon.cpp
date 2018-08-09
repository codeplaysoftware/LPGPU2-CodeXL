// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppXMLParserCommon.cpp
///
/// @brief File with class definitions used by all the XML parsers
///        developed for the LPGPU2 project.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppXMLParserCommon.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

namespace lpgpu2 {

/// @brief Ctor; it sets the internal textStr from an ASCII string
/// @param[in]  pElement    The corresponding tinyXML element for this object.
/// @param[in]  pParent     The parent TElement object.
TElement::TElement(TiXmlElement *pElement, TElement *pParent)
: m_pElement{pElement}, m_pParent{pParent}, m_textStr{}
{
  m_textStr.fromASCIIString(pElement->GetText());
}

/// @brief Initialise the object. Uses the NVI idiom to call into the
///        private virtual implementation
/// @return PPFnStatus::success if initialisation succeeded, PPFnStatus::failure if not.
PPFnStatus TElement::init()
{
  return initImpl();
}

/// @brief Clear up the object. Uses the NVI idiom to call into the
///        private virtual implementation
/// @return PPFnStatus::success if deinitialisation succeeded, PPFnStatus::failure if not.
PPFnStatus TElement::shutdown()
{
  return shutdownImpl();
}

/// @brief Base implementation of the shutdown method. No work is done here.
/// @return Always returns PPFnStatus::success.
PPFnStatus TElement::shutdownImpl()
{
  return PPFnStatus::success;
}

/// @brief Gets the string value of the attribute with the specified name
///        within this XML element.
/// @param[in]  attrName    The name of the attribute.
/// @param[out] value       The string value of the value.
/// @return true if found, false if not.
bool TElement::GetAttribute(const char *attrName, gtString &value) const
{
  GT_ASSERT_EX(attrName, L"Can't be NULL!");

  const auto *ret = m_pElement->Attribute(attrName);
  value.fromASCIIString(ret);
  return (ret != nullptr) ? true : false;
}

/// @brief Gets the integer value of the attribute with the specified name
///        within this XML element.
/// @param[in]  attrName    The name of the attribute.
/// @param[out] value       The integer value of the attribute.
/// @return true if found, false if not.
bool TElement::GetAttribute(const char *attrName, gtUInt32 &value) const
{
  GT_ASSERT_EX(attrName, L"Can't be NULL!");

  gtInt32 signedVal = 0U;
  const auto *ret = m_pElement->Attribute(attrName, &signedVal);
  if (!ret)
  {
    return false;
  }

  value = static_cast<gtUInt32>(signedVal);
  return true;
}

/// @brief Gets the string value of the value of an element and return the
///        next sibling with the same name, if any
/// @param pCurr Ptr to elem to read the value from and which will be advanced.
/// @param elemName Name of the elements from which to read the values.
/// @param[out] value       The string value of the value.
/// @return  Ptr to a TiXmlElement if next element exists, nullptr if next
///          pPrev was last node with name attrName
const TiXmlElement *GetValueAndIter(const TiXmlElement *pCurr,
    const char *elemName,
    gtString &value)
{
  GT_ASSERT_EX(pCurr, L"Can't be NULL!");
  GT_ASSERT_EX(elemName, L"Can't be NULL!");

  const auto *val = pCurr->GetText();
  if (val)
  {
    value.fromASCIIString(val);
  }

  // Advance to next sibling, if any
  return pCurr->NextSiblingElement(elemName);
}

/// @brief Gets the string value of the value of an element which is the
///        child of the parent element and return the
///        next sibling of the child with the same name, if any
/// @param pCurr Ptr to the parent elem to read the child values from
/// @param elemName Name of the child elements from which to read the values.
/// @param[out] value       The string value of the value.
/// @return  Ptr to a TiXmlElement if next element exists, nullptr if next
///          pPrev was last node with name attrName
const TiXmlElement *GetValueAndIterFromParent(const TiXmlElement *pParent,
    const char *childElemName,
    gtString &value)
{
  GT_ASSERT_EX(pParent, L"Can't be NULL!");
  GT_ASSERT_EX(childElemName, L"Can't be NULL!");

  auto *pFirstChild = pParent->FirstChildElement(childElemName);
  if (!pFirstChild)
  {
    return nullptr;
  }

  return GetValueAndIter(pFirstChild, childElemName, value);
}

/// @brief Gets the unsigned integer value of the value of an element which is
///        the child of the parent element and return the
///        next sibling of the child with the same name, if any
/// @note Calls into the string version of GetValueAndIterFromParent
/// @param pCurr Ptr to the parent elem to read the child values from
/// @param elemName Name of the child elements from which to read the values.
/// @param[out] value       The unsigned integer value of the value.
/// @return  Ptr to a TiXmlElement if next element exists, nullptr if next
///          pPrev was last node with name attrName
const TiXmlElement *GetValueAndIterFromParent(const TiXmlElement *pParent,
    const char *childElemName,
    gtUInt32 &value)
{
  gtString stringVal;
  auto *nextEl = GetValueAndIterFromParent(pParent, childElemName, stringVal);
  GT_ASSERT(stringVal.toUnsignedIntNumber(value));

  return nextEl;
}

/// @brief Gets the unsigned integer value of the value of an element and
///        return the
///        next sibling with the same name, if any
/// @note Calls into the string version of GetValueAndIter
/// @param pCurr Ptr to elem to read the value from and which will be advanced.
/// @param elemName Name of the elements from which to read the values.
/// @param[out] value       The unsigned integer value of the attribute.
/// @return  Ptr to a TiXmlElement if next element exists, nullptr if next
///          pPrev was last node with name attrName
const TiXmlElement *GetValueAndIter(
    const TiXmlElement *pCurr,
    const char *elemName,
    gtUInt32 &value)
{
  gtString stringVal;
  auto *nextEl = GetValueAndIter(pCurr, elemName, stringVal);
  GT_ASSERT(stringVal.toUnsignedIntNumber(value));

  return nextEl;
}

} // namespace lpgpu2
