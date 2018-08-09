// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateProjectDataCcvg interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTDATACCVG_H_INCLUDE
#define LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTDATACCVG_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataResults.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataSource.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataDriver.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataExecuteable.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWTheCurrentStateProjectDataCcvg handles CCVGItems' data
///           such as the file data. The data held in *this class is not to be
///           stored between sessions like the CCVGItem structure data. Data is
///           to be refresh on each project open or refresh and cleaned out on
///           project close - cache behaviour. Data held here is used to 
///           populate appropriate CCVG views and a like. The objects 
///           DataResults, DataSource, DataDriver are know as sub-items (obj) of 
///           *this object (there may be more in the future) identified by the
///           enumeration EnumCCVGItemDataSubObjType.
///           CCVGFWTheCurrentStateProjectData holds a *this class objects.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems.
/// @date     15/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTheCurrentStateProjectDataCcvg final
: public CCVGClassCmnBase
{
// Methods:
public:
  CCVGFWTheCurrentStateProjectDataCcvg(const gtString &vrIdCCVGItemFileName);
  ~CCVGFWTheCurrentStateProjectDataCcvg();
  const gtString&                 GetIdCCVGItemFileName() const;
  CCVGFWCCVGItemDataResults&      GetDataResults();
  CCVGFWCCVGItemDataSource&       GetDataSource();
  CCVGFWCCVGItemDataDriver&       GetDataDrivers();
  CCVGFWCCVGItemDataExecuteable&  GetDataExecuteable();
  bool                            GetInUseSubItemAny() const;
  status                          GetInUseSubItem(EnumCCVGItemDataSubObjType veType, bool &vwrbInUse);
  status                          ChkDataChanged();
  bool                            HasDataChanged() const;
  void                            ClrDataChangedFlags();
  status                          ClrData(EnumCCVGItemDataSubObjType veCCVGItemDataSubObj);

// Attributes:
private:
  CCVGFWCCVGItemDataResults      m_ccvgItemDataResults;
  CCVGFWCCVGItemDataSource       m_ccvgItemDataSource;
  CCVGFWCCVGItemDataDriver       m_ccvgItemDataDrivers;
  CCVGFWCCVGItemDataExecuteable  m_ccvgItemDataExecuteable;
  gtString                       m_idCCVGItemFileName;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTDATACCVG_H_INCLUDE