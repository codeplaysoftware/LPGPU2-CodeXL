// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataSource implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataSource.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMCoverageHighlighter.h>

namespace ccvg {
namespace fw {

// @brief Class constructor. No work done here.
CCVGFWCCVGItemDataSource::CCVGFWCCVGItemDataSource(CCVGFWTheCurrentStateProjectDataCcvg &vpParent)
: CCVGFWCCVGItemDataBase(vpParent)
{
  m_constStrObjId = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailSource);
  m_eConstThisCCVGItemDataSubObjType = EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataSource;
}

/// @brief Overrideable. Class destructor. Tidy up release resources used 
///        by *this instance.
CCVGFWCCVGItemDataSource::~CCVGFWCCVGItemDataSource()
{
  // Tidy up
  Clear();
}

/// @brief Class copy constructor. No work done here.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataSource::CCVGFWCCVGItemDataSource(const CCVGFWCCVGItemDataSource &vRhs)
: CCVGFWCCVGItemDataBase(vRhs)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataSource::CCVGFWCCVGItemDataSource(CCVGFWCCVGItemDataSource &&vRhs)
: CCVGFWCCVGItemDataBase(std::move(vRhs))
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataSource& CCVGFWCCVGItemDataSource::operator= (const CCVGFWCCVGItemDataSource &vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (vRhs);
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataSource& CCVGFWCCVGItemDataSource::operator= (CCVGFWCCVGItemDataSource &&vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (std::move(vRhs));
  Move(std::move(vRhs));
  return *this;
}

/// @brief Overridden. Helper function to copy *this object's members to 
///        itself.
void CCVGFWCCVGItemDataSource::Copy(const CCVGFWCCVGItemDataSource &vRhs)
{
  if (this == &vRhs)
  {
    return;
  }

  // ToDo: put attributes here
  //m_dummyDriver = vRhs.m_dummyDriver;
}

/// @brief Overridden. Helper function to move *this object's members to 
///        itself.
void CCVGFWCCVGItemDataSource::Move(CCVGFWCCVGItemDataSource &&vRhs)
{
  if (this == &vRhs)
  {
    return;
  }

  // ToDo: put attributes here
  //m_dummyDriver = std::move(vRhs.m_dummyDriver);
}

/// @brief Not equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = not equal, false = equal.
bool CCVGFWCCVGItemDataSource::operator!= (const CCVGFWCCVGItemDataSource &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return !bSame;
}

/// @brief Equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = equal, false = not equal.
bool CCVGFWCCVGItemDataSource::operator== (const CCVGFWCCVGItemDataSource &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return bSame;
}

/// @brief Overridden. Clear, release or reset *this objects' attributes to 
///        default.
void CCVGFWCCVGItemDataSource::Clear()
{
  // ToDo: put attributes here
  //m_dummyDriver = 0;
}

/// @brief *this object represent a phyiscal file. Re-read the file and check
///        if the data with has changed.
/// @return status Success = all ok, failure = see error description in 
///         CCVGFWTheCurrentStateProjectDataCcvg object.
status CCVGFWCCVGItemDataSource::ReReadFileData()
{
  const bool bStoreDataInTempBuffer = true;
  return CCVGFWCCVGItemDataBase::ReReadFileData(cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Source, bStoreDataInTempBuffer);
}

/// @brief Using the item's associated executable and results file, populate the
///        vector of highlights to annotate the source code with CCVG information.
///        The source and executable files should already be processed when this i
///        called to ensure that the correct data is available.
/// @return status Success = all ok, failure = see error description
status CCVGFWCCVGItemDataSource::GenerateHighlights(CCVGFWCCVGItemDataExecuteable &rDataExecutable, CCVGFWCCVGItemDataResults &rDataResults)
{
  status ok = success;
  CCVGFWLLVMCoverageHighlighter highlighter;
  
  // To map line and column numbers to number of characters into the text, generate 
  // a vector of the cumulative number of characters at the start of each line.
  std::vector<gtSize_t> lineCharCounts({0});
  gtSize_t currentChar = 0;
  while (currentChar < m_fileContentBufferSizeBytes)
  {
    // Scan through to next line break or EOF
    while ((m_pFileContentBuffer[currentChar++] != '\n') && (currentChar < m_fileContentBufferSizeBytes));
    
    try
    {
      lineCharCounts.push_back(currentChar);
    }
    catch (...)
    {
      ok = failure;
    }    
  }

  if (ok == success)
  {
    // Process the results and coverage maps, and populate the vector of highlights
    const CCVGFWLLVMCovMapReader &covMapReader = rDataExecutable.GetCovMapReader();
    const CCVGFWLLVMResultTextReader &resultTextReader = rDataResults.GetResultReader();
    ok = highlighter.ProcessResults(covMapReader, resultTextReader, lineCharCounts);

    if (ok)
    {
      for (auto h : highlighter.GetVecHighlights())
      {
        try
        {
          m_vecHighlights.push_back(h);
        }
        catch (...)
        {
          ok = failure;
        }
      }

      for (auto lineVecPair : highlighter.GetMapLineVecSummaries())
      {
        try
        {
          m_mapLineToVecSummaries[lineVecPair.first] = lineVecPair.second;
        }
        catch (...)
        {
          ok = failure;
        }        
      }
    }
  }

  return ok;
}

} // namespace fw
} // namespace ccvg
