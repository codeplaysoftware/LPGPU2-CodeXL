// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGHighlight implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGHighlight.h>


namespace ccvg {
namespace fw {

// @brief Class constructor. No work done here.
CCVGFWCCVGHighlight::CCVGFWCCVGHighlight(CCVGuint vStartPos, 
                                         CCVGuint vLength, 
                                         CCVGFWCCVGHighlightType veHighlightType)
: m_startPos(vStartPos)
, m_length(vLength)
, m_eHighlightType(veHighlightType)
{
}

// @brief Class destructor.
CCVGFWCCVGHighlight::~CCVGFWCCVGHighlight()
{  
}

/// @brief Retrieve the start position of the highlight
/// @return CCVGuint Start position
CCVGuint CCVGFWCCVGHighlight::GetStartPos() const
{
  return m_startPos;
}

/// @brief Retrieve the length of the highlight
/// @return CCVGuint length
CCVGuint CCVGFWCCVGHighlight::GetLength() const
{
  return m_length;
}

/// @brief Retrieve the type of coverage represented by the highlight
/// @return CCVGFWCCVGHighlightType highlight type
CCVGFWCCVGHighlightType CCVGFWCCVGHighlight::GetHighlightType() const
{
  return m_eHighlightType;
}

/// @brief Retrieve the color that corresponds to the type of the highlight
/// @return QColor Color that represents the highlight type.
QColor CCVGFWCCVGHighlight::GetHighlightColor() const
{
  switch (m_eHighlightType)
  {
  case CCVGFWCCVGHighlightType::kCodeExecuted:
    return CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_Covered);
  case CCVGFWCCVGHighlightType::kCodeNotExecuted:
    return CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_Uncovered);
  case CCVGFWCCVGHighlightType::kNoBranchTaken:
    return CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_BranchTakenNone);
  case CCVGFWCCVGHighlightType::kOneBranchTaken:
    return CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_BranchTakenOne);
  case CCVGFWCCVGHighlightType::kBothBranchesTaken:
  default:
    return CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_BranchTakenBoth);
  }
}

} // namespace fw
} // namespace ccvg
