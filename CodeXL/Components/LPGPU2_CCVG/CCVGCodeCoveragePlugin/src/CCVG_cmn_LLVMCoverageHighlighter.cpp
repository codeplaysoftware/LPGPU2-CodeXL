// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMCoverageHighlighter implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMCoverageHighlighter.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. No work done here.
CCVGFWLLVMCoverageHighlighter::CCVGFWLLVMCoverageHighlighter()
{
}

/// @brief Class destructor. Explicitly erases the contents of containers in *this object.
CCVGFWLLVMCoverageHighlighter::~CCVGFWLLVMCoverageHighlighter()
{
  m_vecHighlights.clear();
  m_mapLineVecSummaries.clear();
}

/// @brief Retrieve the vector of Highlight objects generated.
/// @returns CCVGFWLLVMCoverageHighlighter::VecCCVGHighlight_t The vector of highlight objects.
const CCVGFWLLVMCoverageHighlighter::VecCCVGHighlight_t& CCVGFWLLVMCoverageHighlighter::GetVecHighlights() const
{
  return m_vecHighlights;
}

/// @brief Retrieve the map of line numbers to vector of Summary objects generated.
/// @returns CCVGFWLLVMCoverageHighlighter::MapLineToVecSummaries_t The map of line numbers to vector of Summary objects.
const CCVGFWLLVMCoverageHighlighter::MapLineToVecSummaries_t& CCVGFWLLVMCoverageHighlighter::GetMapLineVecSummaries() const
{
  return m_mapLineVecSummaries;
}

/// @brief  Read the coverage mappings and counter results to populate this
///         LLVMCoverageHighlighter's vector of highlights.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWLLVMCoverageHighlighter::ProcessResults(const CCVGFWLLVMCovMapReader &vrCovMapReader, const CCVGFWLLVMResultTextReader &vrResultReader,
                                                     std::vector<gtSize_t> &vrLineStarts)
{
  const auto &rMapCovMap = vrCovMapReader.GetMapFuncHashToCoverageMap();
  const auto &rMapResult = vrResultReader.GetMapFuncResults();
    
  for (std::pair<gtUInt64, CCVGFWLLVMFunctionCovMap> mapCovMapPair : rMapCovMap)
  {
    // Ensure that a matching hash exists in the results
    const gtUInt64 funcHash = mapCovMapPair.first;
    const auto funcCovMap = mapCovMapPair.second;
    const auto funcResultEntry = rMapResult.find(funcHash);
    if (funcResultEntry == rMapResult.end())
    {
      // No result available for function (indicates mismatched files)
      const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCoverageHighlighter_err_MissingResults);
      gtString errMsg;
      errMsg.appendFormattedString(pErrMsg, funcHash);
      return ErrorSet(errMsg);
    }
    const auto funcResult = funcResultEntry->second;
    
    // Generate a highlight from every mapping region in the function, and 
    // collect information for the function summary.
    gtUInt32 executedRegionCount = 0;
    gtInt32  functionStartLine = -1;
    const gtSize_t mapCount = funcCovMap.GetMappingCount();
    for (const CCVGFWLLVMMapping &mappingRegion : funcCovMap.GetVecLLVMMappings())
    {
      const CCVGLLVMMappingSpan span = mappingRegion.GetSpan();
      const CCVGuint startPos = vrLineStarts.at(span.m_startLine - 1) + span.m_startCol;
      const CCVGuint endPos   = vrLineStarts.at(span.m_endLine - 1) + span.m_endCol;
      // Function start line is the start line of the first region:
      if (functionStartLine == -1)
      {
        functionStartLine = span.m_startLine;
      }
      
      gtUInt32 counterValue = 0;
      if (mappingRegion.GetExpressionType() == CCVGFWLLVMRegionExprType::kSubExpression)
      {
        counterValue = (funcResult.GetCounterValue(mappingRegion.GetExprLHS())) - (funcResult.GetCounterValue(mappingRegion.GetExprRHS()));
      }
      else if (mappingRegion.GetExpressionType() == CCVGFWLLVMRegionExprType::kAddExpression)
      {
        counterValue = (funcResult.GetCounterValue(mappingRegion.GetExprLHS())) + (funcResult.GetCounterValue(mappingRegion.GetExprRHS()));
      }
      else
      {
        counterValue = funcResult.GetCounterValue(mappingRegion.GetCounter());      
      }
      const auto highlightType = counterValue > 0 ? CCVGFWCCVGHighlightType::kCodeExecuted : CCVGFWCCVGHighlightType::kCodeNotExecuted;
      try
      {
        m_vecHighlights.emplace_back(startPos, endPos - startPos, highlightType);
      }
      catch (...)
      {
        return failure;
      }
      
      if (counterValue > 0)
      {
        executedRegionCount++;
      }
    }
    // Created function summary information
    try
    {
      m_mapLineVecSummaries[functionStartLine].emplace_back(funcResult.GetName(), functionStartLine, mapCount, executedRegionCount);
    }
    catch (...)
    {
      return failure;
    }
    
  }
  
  return success;
}

} // namespace fw
} // namespace ccvg
