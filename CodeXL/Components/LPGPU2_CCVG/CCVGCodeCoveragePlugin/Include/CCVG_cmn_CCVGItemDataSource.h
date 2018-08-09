// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataSource interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_CCVGITEMDATASOURCE_H_INCLUDE
#define LPGPU2_CCVG_FW_CCVGITEMDATASOURCE_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataExecuteable.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataResults.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWCCVGItemDataSource is derived class. The class represents 
///           data in this case read from either a code coverage results, the 
///           source to a kernel file or the source code from a driver source 
///           code file. It may holder other information to associated with 
///           that file or content.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems, 
///           CCVGFWTheCurrentStateProjectDataCcvg
/// @date     16/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWCCVGItemDataSource final
: public CCVGFWCCVGItemDataBase
{
// Methods:
public:
  CCVGFWCCVGItemDataSource(CCVGFWTheCurrentStateProjectDataCcvg &vpParent);
  CCVGFWCCVGItemDataSource(const CCVGFWCCVGItemDataSource &vRhs);
  CCVGFWCCVGItemDataSource(CCVGFWCCVGItemDataSource &&vRhs);
  CCVGFWCCVGItemDataSource& operator= (const CCVGFWCCVGItemDataSource &vRhs);
  CCVGFWCCVGItemDataSource& operator= (CCVGFWCCVGItemDataSource &&vRhs);
  bool                      operator!= (const CCVGFWCCVGItemDataSource &vRhs) const;
  bool                      operator== (const CCVGFWCCVGItemDataSource &vRhs) const;
  //
  status ReReadFileData();
  status GenerateHighlights(CCVGFWCCVGItemDataExecuteable &rDataExecutable, CCVGFWCCVGItemDataResults &rDataResults);

  // Overridden:
public:
  // From CCVGClassCmnBase
  virtual ~CCVGFWCCVGItemDataSource() override;
  // From CCVGFWCCVGItemDataBase
  virtual void Clear() override;

  // Methods:
private:
  void Copy(const CCVGFWCCVGItemDataSource &vRhs);
  void Move(CCVGFWCCVGItemDataSource &&vRhs);
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_CCVGITEMDATASOURCE_H_INCLUDE