///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGClassCmnBase implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

// Instantiations:
const wchar_t* CCVGClassCmnBase::ms_pStrErrorMsgInvalidMsg = L"Error: Message not valid";
const wchar_t* CCVGClassCmnBase::ms_pStrErrorMsgNoErrorMsg = L"No error";
const wchar_t* CCVGClassCmnBase::ms_pConstStrDfltErrMsg = L"No error";

/// @brief  Class constructor. No work is done here.
CCVGClassCmnBase::CCVGClassCmnBase()
: m_bHaveErrorCondition( false )
, m_strErrorDescription(ms_pConstStrDfltErrMsg)
, m_bDoNotDeleteThisObj( false )
{
}

/// @brief  Class destructorr. No work is done here.
CCVGClassCmnBase::~CCVGClassCmnBase()
{
}

/// @brief Class copy constructor. No work done here.
/// @param[in] vRhs The object to copy from.
CCVGClassCmnBase::CCVGClassCmnBase(const CCVGClassCmnBase &vRhs)
: m_bHaveErrorCondition(false)
, m_strErrorDescription(ms_pConstStrDfltErrMsg)
, m_bDoNotDeleteThisObj(false)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here.
/// @param[in] vRhs The object to copy or move from.
CCVGClassCmnBase::CCVGClassCmnBase(CCVGClassCmnBase &&vRhs)
: m_bHaveErrorCondition(false)
, m_strErrorDescription(ms_pConstStrDfltErrMsg)
, m_bDoNotDeleteThisObj(false)
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy.
/// @param[in] vRhs The object to copy from.
CCVGClassCmnBase& CCVGClassCmnBase::operator= (const CCVGClassCmnBase &vRhs)
{
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move.
/// @param[in] vRhs The object to copy or move from.
CCVGClassCmnBase& CCVGClassCmnBase::operator= (CCVGClassCmnBase &&vRhs)
{
  Move(std::move(vRhs));
  return *this;
}

/// @brief Helper function to copy *this object's members to itself.
void CCVGClassCmnBase::Copy(const CCVGClassCmnBase &vRhs)
{
  if (this == &vRhs)
  {
    return;
  }

  m_bHaveErrorCondition = vRhs.m_bHaveErrorCondition;
  m_strErrorDescription = vRhs.m_strErrorDescription;
  m_bDoNotDeleteThisObj = vRhs.m_bDoNotDeleteThisObj;
}

/// @brief Helper function to move *this object's members to itself.
void CCVGClassCmnBase::Move(CCVGClassCmnBase &&vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  
  // Return vRhs to default condition, not empty
  m_bHaveErrorCondition = vRhs.m_bHaveErrorCondition;
  vRhs.m_bHaveErrorCondition = false;
  m_strErrorDescription = vRhs.m_strErrorDescription; 
  vRhs.m_strErrorDescription = ms_pConstStrDfltErrMsg;
  m_bDoNotDeleteThisObj = vRhs.m_bDoNotDeleteThisObj;
  vRhs.m_bDoNotDeleteThisObj = false;
}

/// @brief Not equals operator.
/// @param[in] vRhs The other object.
/// @return bool True = not equal, false = equal.
bool CCVGClassCmnBase::operator!= (const CCVGClassCmnBase &vRhs) const
{
  bool bSame = (m_bHaveErrorCondition == vRhs.m_bHaveErrorCondition);
  bSame = bSame && (m_strErrorDescription == vRhs.m_strErrorDescription);
  bSame = bSame && (m_bDoNotDeleteThisObj == vRhs.m_bDoNotDeleteThisObj);
  return !bSame;
}

/// @brief Equals operator.
/// @param[in] vRhs The other object.
/// @return bool True = Equal, false = not equal.
bool CCVGClassCmnBase::operator== (const CCVGClassCmnBase &vRhs) const
{
  bool bSame = (m_bHaveErrorCondition == vRhs.m_bHaveErrorCondition);
  bSame = bSame && (m_strErrorDescription == vRhs.m_strErrorDescription);
  bSame = bSame && (m_bDoNotDeleteThisObj == vRhs.m_bDoNotDeleteThisObj);
  return bSame;
}

/// @brief Some parts of the CodeXL frame do delete objects, owns them, after
///        registration. Other parts do not (which should!) so we need to
///        tidy up ourselves. Qt may also delete objects derived from QWidget.
/// @param[in] vbYes True = Do not delete, false = do delete this object.
/// @return gtString& Natural language text.
void CCVGClassCmnBase::SetDoNotDeleteThisFlag(bool vbYes)
{
  m_bDoNotDeleteThisObj = vbYes;
}

/// @brief Some parts of the CodeXL frame do delete objects, owns them, after
///        registration. Other parts do not (which should!) so we need to
///        tidy up ourselves. Qt may also delete objects derived from QWidget.
/// @return bool True = Do not delete, false = do delete this object.
bool CCVGClassCmnBase::GetDoNotDeleteThisFlag() const
{
  return m_bDoNotDeleteThisObj;
}

/// @brief Retrieve the error description currently set for *this object.
/// @return gtString& Natural language text.
const gtString& CCVGClassCmnBase::ErrorGetDescription() const
{
  return m_strErrorDescription;
}

/// @brief Retrieve the error description currently set for *this object.
/// @return wchar_t* Natural language text.
const wchar_t* CCVGClassCmnBase::ErrorGetDescriptionW() const
{
  return ErrorGetDescription().asCharArray();
}

/// @brief Retrieve the error description currently set for *this object.
/// @return char* Natural language text.
const char* CCVGClassCmnBase::ErrorGetDescriptionA() const
{
  return ErrorGetDescription().asUTF8CharArray();
}

/// @brief Retrieve the error flag currently set for *this object.
/// @return bool True = currently have an error condition, false = no error.
bool CCVGClassCmnBase::ErrorGetHaveFlag() const
{
  return m_bHaveErrorCondition;
}

/// @brief Clear *this object's current error flag and description, reset to 
///        ok.
void CCVGClassCmnBase::ErrorClear()
{
  m_bHaveErrorCondition = false;
  m_strErrorDescription = ms_pStrErrorMsgNoErrorMsg;
}

/// @brief Set *this object's error flag and description.
/// @param[in] vrMsg The error description text.
/// @return    status Always set to failure.
status CCVGClassCmnBase::ErrorSet(const gtString &vrMsg)
{
  m_bHaveErrorCondition = true;
  m_strErrorDescription = vrMsg;
  return failure;
}

/// @brief Set *this object's error flag and description.
/// @param[in] vpMsg The error description text.
/// @return    status Always set to failure.
status CCVGClassCmnBase::ErrorSet(const wchar_t *vpMsg)
{
  m_bHaveErrorCondition = true;
  if ((vpMsg != nullptr) && (::wcslen(vpMsg) > 0))
  {
    m_strErrorDescription = vpMsg;
  }
  else
  {
    m_strErrorDescription = ms_pStrErrorMsgInvalidMsg;
  }
  return failure;
}

/// @brief Set *this object's error flag and description.
/// @param[in] vpMsg The error description text.
/// @return    status Always set to failure.
status CCVGClassCmnBase::ErrorSet(const char *vpMsg)
{
  m_bHaveErrorCondition = true;
  if ((vpMsg != nullptr) && (::strlen(vpMsg) > 0))
  {
    m_strErrorDescription.fromASCIIString(vpMsg);
  }
  else
  {
    m_strErrorDescription = ms_pStrErrorMsgInvalidMsg;
  }
  return failure;
}
