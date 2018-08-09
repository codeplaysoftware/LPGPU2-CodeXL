// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGMdiViewId interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MIDVIEWID_H_INCLUDE
#define LPGPU2_CCVG_FW_MIDVIEWID_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>

using namespace ccvg::fnstatus;

namespace ccvg {
namespace fw {

/// @brief    CCVG MDI View ID class. Holds the CCVGItemId and the sub-item 
///           type. The ID is formatted as "<CVGItemId>,<File type>".
/// @warning  None.
/// @see      CCVGFWMDIViewMgr, 
///           CCVGFWMDIViewCreator,
///           CCVGFWMDIViewBase
/// @date     15/03/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGMDIViewId final
{
// Methods:
public:
  CCVGMDIViewId(const gtString &vCCVGItemId, const gtString &vSubItemType);
  CCVGMDIViewId(const gtString &viewId);
  ~CCVGMDIViewId();
  //
  const gtString& GetCCVGItemId() const;
  const gtString& GetCCVGItemDataSubItemType() const;
  const gtString  GetId() const;
  //
  const cmd::CCVGCmdFileRead::FileDataType GetDataTypeEnum() const;

// Methods:
private:
  void SplitViewId(const gtString &viewId);

// Attributes:
private:
  gtString m_strCCVGItemId;
  gtString m_strCCVGItemDataSubItemType;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MIDVIEWID_H_INCLUDE