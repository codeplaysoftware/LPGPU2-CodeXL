// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMCoverageHighlighter class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMCOVERAGEHIGHLIGHTER_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMCOVERAGEHIGHLIGHTER_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMCovMapReader.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMResultTextReader.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGHighlight.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionSummary.h>

namespace ccvg {
namespace fw {


/// @brief    CCVGFWLLVMCoverageHighlighter is a class that takes processed
///           coverage mapping records and coverage result result records
///           and generates highlights to be applied to the source view.
/// @see      CCVGFWLLVMResultTextReader, CCVGFWLLVMCovMapReader
/// @date     11/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMCoverageHighlighter final
: public CCVGClassCmnBase
{
// Typedefs:
using VecSummary_t                = std::vector<CCVGFWLLVMFunctionSummary>;
using VecCCVGHighlight_t          = std::vector<CCVGFWCCVGHighlight>;
using MapLineToVecSummaries_t     = std::map<gtUInt32, VecSummary_t>;
using PairLineNumAndVecSummary_t  = std::pair<gtUInt32, VecSummary_t>;

// Methods:
public:
  CCVGFWLLVMCoverageHighlighter();
  status ProcessResults(const CCVGFWLLVMCovMapReader &vrCovMapReader, const CCVGFWLLVMResultTextReader &vrResultReader, std::vector<gtSize_t> &vrLineStarts);
  const VecCCVGHighlight_t&       GetVecHighlights() const;
  const MapLineToVecSummaries_t&  GetMapLineVecSummaries() const;

// Overridden:
public:
  virtual ~CCVGFWLLVMCoverageHighlighter(); // From CCVGClassCmnBase
  
// Attributes:
protected:
  VecCCVGHighlight_t      m_vecHighlights;
  MapLineToVecSummaries_t m_mapLineVecSummaries;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMCOVERAGEHIGHLIGHTER_H_INCLUDE