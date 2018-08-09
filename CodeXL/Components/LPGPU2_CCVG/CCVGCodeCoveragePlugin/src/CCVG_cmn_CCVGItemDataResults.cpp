// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataResults implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataResults.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

// @brief Class constructor. No work done here.
CCVGFWCCVGItemDataResults::CCVGFWCCVGItemDataResults(CCVGFWTheCurrentStateProjectDataCcvg &vpParent)
: CCVGFWCCVGItemDataBase(vpParent)
{
  m_constStrObjId = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailResults);
  m_eConstThisCCVGItemDataSubObjType = EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataResults;
}

/// @brief Overrideable. Class destructor. Tidy up release resources used 
///        by *this instance.
CCVGFWCCVGItemDataResults::~CCVGFWCCVGItemDataResults()
{
  // Tidy up
  Clear();
}

/// @brief Class copy constructor. No work done here.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataResults::CCVGFWCCVGItemDataResults(const CCVGFWCCVGItemDataResults &vRhs)
: CCVGFWCCVGItemDataBase(vRhs)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataResults::CCVGFWCCVGItemDataResults(CCVGFWCCVGItemDataResults &&vRhs)
: CCVGFWCCVGItemDataBase(std::move(vRhs))
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataResults& CCVGFWCCVGItemDataResults::operator= (const CCVGFWCCVGItemDataResults &vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (vRhs);
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataResults& CCVGFWCCVGItemDataResults::operator= (CCVGFWCCVGItemDataResults &&vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (std::move(vRhs));
  Move(std::move(vRhs));
  return *this;
}

/// @brief Overridden. Helper function to copy *this object's members to 
///        itself.
void CCVGFWCCVGItemDataResults::Copy(const CCVGFWCCVGItemDataResults &vRhs)
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
void CCVGFWCCVGItemDataResults::Move(CCVGFWCCVGItemDataResults &&vRhs)
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
bool CCVGFWCCVGItemDataResults::operator!= (const CCVGFWCCVGItemDataResults &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return !bSame;
}

/// @brief Equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = equal, false = not equal.
bool CCVGFWCCVGItemDataResults::operator== (const CCVGFWCCVGItemDataResults &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return bSame;
}

/// @brief Overridden. Clear, release or reset *this objects' attributes to 
///        default.
void CCVGFWCCVGItemDataResults::Clear()
{
  // ToDo: put attributes here

}

/// @brief Return the LLVM result reader object associated with this result file.
const CCVGFWLLVMResultTextReader& CCVGFWCCVGItemDataResults::GetResultReader() const
{
  return m_resultReader;
}

/// @brief *this object represent a phyiscal file. Re-read the file and check
///        if the data with has changed.
/// @return status Success = all ok, failure = see error description in 
///         CCVGFWTheCurrentStateProjectDataCcvg object.
status CCVGFWCCVGItemDataResults::ReReadFileData()
{
  const bool bStoreDataInTempBuffer = true;
  return CCVGFWCCVGItemDataBase::ReReadFileData(cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Results, bStoreDataInTempBuffer);
}

/// @brief Parse the contents of the result file.
/// @return status Success = all ok, failure = see error description
status CCVGFWCCVGItemDataResults::ParseResults()
{
  return m_resultReader.ReadData(m_pFileContentBuffer, m_fileContentBufferSizeBytes);
}

} // namespace fw
} // namespace ccvg
