// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGClassCmnBase interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CLASSCMNBASE_H_INCLUDE
#define LPGPU2_CCVG_CLASSCMNBASE_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>

using namespace ccvg::fnstatus;

/// @brief  CCVG base class to most other CCVG classes. Contains utility
///         functionality or attributes used by derived classes. Error
///         handling between object case in point.
/// @warning  None.
/// @see      None.
/// @date     05/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGClassCmnBase
{
// Methods:
public:
  CCVGClassCmnBase();
  CCVGClassCmnBase(const CCVGClassCmnBase &vRhs);
  CCVGClassCmnBase(CCVGClassCmnBase &&vRhs);
  CCVGClassCmnBase& operator= (const CCVGClassCmnBase &vRhs);
  CCVGClassCmnBase& operator= (CCVGClassCmnBase &&vRhs);
  bool              operator!= (const CCVGClassCmnBase &vRhs) const;
  bool              operator== (const CCVGClassCmnBase &vRhs) const;
  //
  void SetDoNotDeleteThisFlag(bool vbYes);
  bool GetDoNotDeleteThisFlag() const;
  //
  const gtString& ErrorGetDescription() const;
  const wchar_t*  ErrorGetDescriptionW() const;
  const char*     ErrorGetDescriptionA() const;
  bool            ErrorGetHaveFlag() const;
  void            ErrorClear();
  status          ErrorSet(const gtString &vrMsg);
  status          ErrorSet(const wchar_t *vpMsg);
  status          ErrorSet(const char *vpMsg);

// Overrideable:
public:
  virtual ~CCVGClassCmnBase();

// Methods:
protected:
  void Copy(const CCVGClassCmnBase &vRhs);
  void Move(CCVGClassCmnBase &&vRhs);


// Attributes:
protected:
  gtString m_strErrorDescription;
  bool     m_bHaveErrorCondition; // True = *this object has a fault, false = all ok
  bool     m_bDoNotDeleteThisObj; // True = Qt or other owns this and deletes, false = do delete this object
  //
  static const wchar_t *ms_pStrErrorMsgInvalidMsg;
  static const wchar_t *ms_pStrErrorMsgNoErrorMsg;
  static const wchar_t *ms_pConstStrDfltErrMsg;
};

// clang-format on

#endif // LPGPU2_CCVG_APPWRAPPER_H_INCLUDE