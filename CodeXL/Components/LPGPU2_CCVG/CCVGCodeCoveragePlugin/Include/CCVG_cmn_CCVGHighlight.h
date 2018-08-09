// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGHighlight class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_CCVGHIGLIGHT_H_INCLUDE
#define LPGPU2_CCVG_CMN_CCVGHIGLIGHT_H_INCLUDE


// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGHighlightType.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesColor.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWCCVGHighlight is a class that represents a single source
///           file annotation generated from a code coverage report. It contains
///           the start position in the file, the length of the annotation and 
///           the type of the annotation (e.g. Code not executed)
/// @see      CCVGFWMDIViewSourceCodeNonEdit
/// @date     23/08/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWCCVGHighlight final
: public CCVGClassCmnBase
{
// Methods:
public:
  CCVGFWCCVGHighlight(CCVGuint vStartPos, CCVGuint vLength, CCVGFWCCVGHighlightType veHighlightType);
  CCVGuint                GetStartPos() const;
  CCVGuint                GetLength() const;
  CCVGFWCCVGHighlightType GetHighlightType() const;
  QColor                  GetHighlightColor() const;

// Overridden:
public:
  virtual ~CCVGFWCCVGHighlight() override; // From CCVGClassCmnBase

// Attributes:
protected:
  CCVGuint                              m_startPos;
  CCVGuint                              m_length;      
  CCVGFWCCVGHighlightType               m_eHighlightType;     
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_CCVGHIGLIGHT_H_INCLUDE