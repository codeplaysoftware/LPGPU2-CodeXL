// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateProjectDataCcvg implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

// @brief Class constructor. No work done here.
// @param[in] vrIdCCVGItemFileName CCVGItem's ID.
CCVGFWTheCurrentStateProjectDataCcvg::CCVGFWTheCurrentStateProjectDataCcvg(const gtString &vrIdCCVGItemFileName)
: m_idCCVGItemFileName(vrIdCCVGItemFileName)
, m_ccvgItemDataResults(*this)
, m_ccvgItemDataSource(*this)
, m_ccvgItemDataDrivers(*this)
, m_ccvgItemDataExecuteable(*this)
{
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateProjectDataCcvg::~CCVGFWTheCurrentStateProjectDataCcvg()
{
}

/// @brief Retrieve the CCVGItem ID. The ID is the common filename between all
///        3 files that make up the CCVGItem. The CCVGItem class ID is the same
///        ID used by *this class. 
/// @return gtString& Text ID.
const gtString& CCVGFWTheCurrentStateProjectDataCcvg::GetIdCCVGItemFileName() const
{
  return m_idCCVGItemFileName;
}

/// @brief Retrieve the data object representing CCVG Results file content. It 
///        may holder other information to associated with that file or content.
CCVGFWCCVGItemDataResults& CCVGFWTheCurrentStateProjectDataCcvg::GetDataResults()
{
  return m_ccvgItemDataResults;
}

/// @brief Retrieve the data object representing CCVG Source file content. It 
///        may holder other information to associated with that file or content.
CCVGFWCCVGItemDataSource& CCVGFWTheCurrentStateProjectDataCcvg::GetDataSource()
{
  return m_ccvgItemDataSource;
}

/// @brief Retrieve the data object representing CCVG Drivers file content. It 
///        may holder other information to associated with that file or content.
CCVGFWCCVGItemDataDriver& CCVGFWTheCurrentStateProjectDataCcvg::GetDataDrivers()
{
  return m_ccvgItemDataDrivers;
}

/// @brief Retrieve the data object representing CCVG Executeable file content. It 
///        may holder other information to associated with that file or content.
CCVGFWCCVGItemDataExecuteable& CCVGFWTheCurrentStateProjectDataCcvg::GetDataExecuteable()
{
  return m_ccvgItemDataExecuteable;
}

/// @brief Check if the file data has changed. This may be because the user has
///         asked for CCVG refresh action. All 3 file types are checked for 
///         changes.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectDataCcvg::ChkDataChanged()
{
  const status statusResults = m_ccvgItemDataResults.ReReadFileData();
  const status statusSource = m_ccvgItemDataSource.ReReadFileData();
  const status statusDrivers = m_ccvgItemDataDrivers.ReReadFileData();
  const status statusExecuteable = m_ccvgItemDataExecuteable.ReReadFileData();

  bool bHaveErr = false;
  gtString errMsgSubObjs;
  CCVGuint commaCnt = 0;
  if (statusResults == failure)
  {
    bHaveErr = true;
    commaCnt++;
    errMsgSubObjs = m_ccvgItemDataResults.ErrorGetDescription();
  }
  if (statusSource == failure)
  {
    bHaveErr = true;
    if (commaCnt > 0)
    {
      errMsgSubObjs.appendFormattedString(L", %ls", m_ccvgItemDataSource.ErrorGetDescription().asCharArray());
    }
    else
    {
      errMsgSubObjs = m_ccvgItemDataSource.ErrorGetDescription();
    }
    commaCnt++;
  }
  if (statusDrivers == failure)
  {
    bHaveErr = true;
    if (commaCnt > 0)
    {
      errMsgSubObjs.appendFormattedString(L", %ls", m_ccvgItemDataDrivers.ErrorGetDescription().asCharArray());
    }
    else
    {
      errMsgSubObjs = m_ccvgItemDataDrivers.ErrorGetDescription();
    }
    commaCnt++;
  }
  if (statusExecuteable == failure)
  {
    bHaveErr = true;
    if (commaCnt > 0)
    {
      errMsgSubObjs.appendFormattedString(L", %ls", m_ccvgItemDataExecuteable.ErrorGetDescription().asCharArray());
    }
    else
    {
      errMsgSubObjs = m_ccvgItemDataExecuteable.ErrorGetDescription();
    }
    commaCnt++;
  }


  if (!bHaveErr)
  {
    return success;
  }

  gtString errMsg;
  errMsg.appendFormattedString((CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFail), errMsgSubObjs.asCharArray()));
  return ErrorSet(errMsg);
}

/// @brief Determine is any part of the CCVGItem's data has changed since the
///        data content being added to CCVGItem or CCVGItem data being 
///        refreshed.
/// @return bool True = yes change, false = no change.
bool CCVGFWTheCurrentStateProjectDataCcvg::HasDataChanged() const
{
  const bool bChangeResults = m_ccvgItemDataResults.HasFileContentChanged();
  const bool bChangeSource = m_ccvgItemDataSource.HasFileContentChanged();
  const bool bChangeDrivers = m_ccvgItemDataDrivers.HasFileContentChanged();

  return bChangeResults || bChangeSource || bChangeDrivers;
}

/// @brief Reset all the CCVGItem's sub components (objects) data has
///        changed flags.
void CCVGFWTheCurrentStateProjectDataCcvg::ClrDataChangedFlags()
{
  m_ccvgItemDataResults.ClrFileContentChangedFlag();
  m_ccvgItemDataSource.ClrFileContentChangedFlag();
  m_ccvgItemDataDrivers.ClrFileContentChangedFlag();
}

/// @brief Empty the data from *this CCVGItemData object sub item. It may be the
///        CCVG Item CodeXL CCVG view for specific type file has been closed by
///        the user.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectDataCcvg::ClrData(EnumCCVGItemDataSubObjType veCCVGItemDataSubObj)
{
  switch (veCCVGItemDataSubObj)
  {
    case ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataResults:
      m_ccvgItemDataResults.ClrFileContent();
      m_ccvgItemDataResults.SetInUse(false);
      return success;
    case ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataSource:
      m_ccvgItemDataSource.ClrFileContent();
      m_ccvgItemDataSource.SetInUse(false);
      return success;
    case ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataDrivers:
      m_ccvgItemDataDrivers.ClrFileContent();
      m_ccvgItemDataDrivers.SetInUse(false);
      return success;
    default:
      // Should not reach here
      const wchar_t *pErrTxt = CCVGRSRC(kIDS_CCVG_STR_code_err_defaultReached);
      gtString errMsg;
      errMsg.appendFormattedString(pErrTxt, L"CCVGFWTheCurrentStateProjectDataCcvg::ClrData");
      return ErrorSet(errMsg);
  }
}

/// @brief Determine if at least one of *this object's sub-items or data
///        objects of type X is currently being used in the rest of CCVG.
///        If not in use *this object can be deleted.
bool CCVGFWTheCurrentStateProjectDataCcvg::GetInUseSubItemAny() const
{
  const bool bInUseResults = m_ccvgItemDataResults.GetInUse();
  const bool bInUseSource = m_ccvgItemDataSource.GetInUse();
  const bool bInUseDrivers = m_ccvgItemDataResults.GetInUse();
  const bool bAtLeastOneInUse = bInUseResults || bInUseSource || bInUseDrivers;
  return bAtLeastOneInUse;
}

/// @brief Check a data type belonging to *this CCVGItemData object is in use.
/// @param[out] vwrbInUse True = Yes data being used, false = not be used.
/// @return status Success = all ok, failure = error occurred check error
///          description.
status CCVGFWTheCurrentStateProjectDataCcvg::GetInUseSubItem(EnumCCVGItemDataSubObjType veType, bool &vwrbInUse)
{
  switch (veType)
  {
  case ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataResults:
    vwrbInUse = m_ccvgItemDataResults.GetInUse();
    return success;
  case ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataSource:
    vwrbInUse = m_ccvgItemDataSource.GetInUse();
    return success;
  case ccvg::fw::EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataDrivers:
    vwrbInUse = m_ccvgItemDataDrivers.GetInUse();
    return success;
  default:
    // Should not reach here
    const wchar_t *pErrTxt = CCVGRSRC(kIDS_CCVG_STR_code_err_defaultReached);
    gtString errMsg;
    errMsg.appendFormattedString(pErrTxt, L"CCVGFWTheCurrentStateProjectDataCcvg::GetInUseSubItem");
    return ErrorSet(errMsg);
  }
}

} // namespace fw
} // namespace ccvg
