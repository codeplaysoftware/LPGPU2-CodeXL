// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataDriver implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataDriver.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectDataCcvg.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

// @brief Class constructor. No work done here.
CCVGFWCCVGItemDataDriver::CCVGFWCCVGItemDataDriver(CCVGFWTheCurrentStateProjectDataCcvg &vpParent)
: CCVGFWCCVGItemDataBase(vpParent)
{
  m_constStrObjId = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGItemData_err_reReadFileDataForCmpFailDrivers);
  m_eConstThisCCVGItemDataSubObjType = EnumCCVGItemDataSubObjType::kCCVGItemDataSubObjType_fileDataDrivers;
}

/// @brief Overrideable. Class destructor. Tidy up release resources used by 
///        *this instance.
CCVGFWCCVGItemDataDriver::~CCVGFWCCVGItemDataDriver()
{
  // Tidy up
  Clear();
}

/// @brief Class copy constructor. No work done here.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataDriver::CCVGFWCCVGItemDataDriver(const CCVGFWCCVGItemDataDriver &vRhs)
: CCVGFWCCVGItemDataBase(vRhs)
{
  Copy(vRhs);
}

/// @brief Class move constructor. No work done here.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataDriver::CCVGFWCCVGItemDataDriver(CCVGFWCCVGItemDataDriver &&vRhs)
: CCVGFWCCVGItemDataBase(std::move(vRhs))
{
  Move(std::move(vRhs));
}

/// @brief Class assignment operator copy.
/// @param[in] vRhs The object to copy from.
CCVGFWCCVGItemDataDriver& CCVGFWCCVGItemDataDriver::operator= (const CCVGFWCCVGItemDataDriver &vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (vRhs);
  Copy(vRhs);
  return *this;
}

/// @brief Class assignment operator move.
/// @param[in] vRhs The object to copy or move from.
CCVGFWCCVGItemDataDriver& CCVGFWCCVGItemDataDriver::operator= (CCVGFWCCVGItemDataDriver &&vRhs)
{
  CCVGFWCCVGItemDataBase::operator= (std::move(vRhs));
  Move(std::move(vRhs));
  return *this;
}

/// @brief Overridden. Helper function to copy *this object's members to 
///        itself.
void CCVGFWCCVGItemDataDriver::Copy(const CCVGFWCCVGItemDataDriver &vRhs)
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
void CCVGFWCCVGItemDataDriver::Move(CCVGFWCCVGItemDataDriver &&vRhs)
{
  if (this == &vRhs)
  {
    return;
  }
  
  // ToDo: put attributes here
  // m_dummyDriver = std::move(vRhs.m_dummyDriver);
}

/// @brief Not equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = not equal, false = equal.
bool CCVGFWCCVGItemDataDriver::operator!= (const CCVGFWCCVGItemDataDriver &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return !bSame;
}

/// @brief Equals operator. The comparison is for all *this object's
///        attributes and those of the base class.
/// @param[in] vRhs The other object.
/// @return bool True = equal, false = not equal.
bool CCVGFWCCVGItemDataDriver::operator== (const CCVGFWCCVGItemDataDriver &vRhs) const
{
  bool bSame = CCVGFWCCVGItemDataBase::operator== (vRhs);
  //bSame = bSame && (m_dummyDriver == vRhs.m_dummyDriver);
  return bSame;
}

/// @brief Overridden. Clear, release or reset *this objects' attributes to 
///        default.
void CCVGFWCCVGItemDataDriver::Clear()
{
  // ToDo: put attributes here
  // m_dummyDriver = 0;
}

/// @brief *this object represent a phyiscal file. Re-read the file and check
///        if the data with has changed.
/// @return status Success = all ok, failure = see error description in 
///         CCVGFWTheCurrentStateProjectDataCcvg object.
status CCVGFWCCVGItemDataDriver::ReReadFileData()
{
  const bool bStoreDataInTempBuffer = true;
  return CCVGFWCCVGItemDataBase::ReReadFileData(cmd::CCVGCmdFileRead::FileDataType::kFileDataType_Driver, bStoreDataInTempBuffer);
}

} // namespace fw
} // namespace ccvg
