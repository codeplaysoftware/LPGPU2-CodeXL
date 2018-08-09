// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppXMLParserCommon.h
///
/// @brief File with class definitions used by all the XML parsers
///        developed for the LPGPU2 project.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2PPXMLPARSEPPFnStatusOMMON_H
#define LPGPU2PPXMLPARSEPPFnStatusOMMON_H

// Lib
#include <tinyxml.h>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

namespace lpgpu2 {

/// @brief    Abstract base class representing an XML element in the
///           XML file.
/// @date     04/10/2017.
/// @author   Alberto Taiuti.
// clang-format off
class AMDTPOWERPROFILINGMIDTIER_API TElement
{
// Methods
public:
  TElement(TiXmlElement *pElement, TElement *pParent = nullptr);
  PPFnStatus init();
  PPFnStatus shutdown();

// Defaults
public:
  TElement(const TElement &) = default;
  TElement &operator=(const TElement &) = default;
  TElement() = default;
  virtual ~TElement() = default;
  TElement(TElement &&) = default;
  TElement &operator=(TElement &&) = default;

// Deleted
public:

// Accessors
public:
  const gtString &GetText() const { return m_textStr; }

// Methods
protected:
  bool GetAttribute(const char *attrName, gtString &value) const;
  bool GetAttribute(const char *attrName, gtUInt32 &value) const;

  /// @brief Gets all of the children of this element which match the
  ///        specified element name.
  /// @param[in]  elementName The element name of the children to search for.
  /// @param[out] children    The vector which will contain the results.
  /// @return PPFnStatus::Success if at least one child found, PPFnStatus::Failure if
  //          no children found
  template <typename T>
  bool GetChildren(const char *elementName, gtVector<T> &children)
  {
    GT_ASSERT_EX(elementName, L"Can't be NULL!");

    TiXmlElement* pChild = m_pElement->FirstChildElement(elementName);
    bool found = pChild != nullptr;
    while (pChild != nullptr)
    {
      children.emplace_back(pChild, this);
      auto rc = children.back().init();
      if (rc != PPFnStatus::success)
      {
        children.clear();
        return false;
      }

      pChild = pChild->NextSiblingElement(elementName);
    }

    return found;
  }

// Methods
private:
  virtual PPFnStatus initImpl() = 0;
  virtual PPFnStatus shutdownImpl();

// Attributes
protected:
//  There are raw ptrs here because this class simply references them, it does
//  not manage them.
  TiXmlElement* m_pElement = nullptr;
  TElement* m_pParent = nullptr;
  gtString m_textStr;

}; // class TElement

// Utility functions
const TiXmlElement *GetValueAndIterFromParent(const TiXmlElement *pParent,
    const char *childElemName,
    gtString &value);
const TiXmlElement *GetValueAndIterFromParent(const TiXmlElement *pParent,
    const char *childElemName,
    gtUInt32 &value);
const TiXmlElement *GetValueAndIter(const TiXmlElement *pCurr,
    const char *elemName,
    gtString &value);
const TiXmlElement *GetValueAndIter(const TiXmlElement *pCurr,
    const char *elemName,
    gtUInt32 &value);

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2PPXMLPARSEPRCOMMON_H
