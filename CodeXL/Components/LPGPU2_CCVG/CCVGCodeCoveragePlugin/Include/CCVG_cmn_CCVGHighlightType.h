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

#ifndef LPGPU2_CCVG_CMN_CCVGHIGLIGHTTYPE_H_INCLUDE
#define LPGPU2_CCVG_CMN_CCVGHIGLIGHTTYPE_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWCCVGHighlight is a class that represents a single source
///           file annotation generated from a code coverage report. It contains
///           the start position in the file, the length of the annotation and 
///           the type of the annotation (e.g. Code not executed)
///           The underlying values are used to map to a QScintilla indicator,
///           which are numbered from 0 to INDIC_MAX(32).
/// @see      CCVGFWCCVGHighlight
/// @date     23/08/2017.
/// @author   Callum Fare.
// clang-format off
enum class CCVGFWCCVGHighlightType
{
  kCodeExecuted = 0,
  kCodeNotExecuted = 1,
  kBothBranchesTaken = 2,
  kOneBranchTaken = 3,
  kNoBranchTaken = 4
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_CCVGHIGLIGHTTYPE_H_INCLUDE