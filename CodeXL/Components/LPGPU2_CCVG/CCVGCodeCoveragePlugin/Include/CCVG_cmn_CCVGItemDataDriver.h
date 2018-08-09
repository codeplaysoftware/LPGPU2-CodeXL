// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// /// CCVGFWCCVGItemDataDriver interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_CCVGITEMDATADRIVER_H_INCLUDE
#define LPGPU2_CCVG_FW_CCVGITEMDATADRIVER_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataBase.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWCCVGItemDataDriver is derived class. The class represents 
///           data in this case read from either a code coverage results, the 
///           source to a kernel file or the source code from a driver source 
///           code file. It may holder other information to associated with 
///           that file or content.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems, 
///           CCVGFWTheCurrentStateProjectDataCcvg
/// @date     16/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWCCVGItemDataDriver final
: public CCVGFWCCVGItemDataBase
{
// Methods:
public:
  CCVGFWCCVGItemDataDriver(CCVGFWTheCurrentStateProjectDataCcvg &vpParent);
  CCVGFWCCVGItemDataDriver(const CCVGFWCCVGItemDataDriver &vRhs);
  CCVGFWCCVGItemDataDriver(CCVGFWCCVGItemDataDriver &&vRhs);
  CCVGFWCCVGItemDataDriver& operator= (const CCVGFWCCVGItemDataDriver &vRhs);
  CCVGFWCCVGItemDataDriver& operator= (CCVGFWCCVGItemDataDriver &&vRhs);
  bool                      operator!= (const CCVGFWCCVGItemDataDriver &vRhs) const;
  bool                      operator== (const CCVGFWCCVGItemDataDriver &vRhs) const;
  //
  status ReReadFileData();

// Overridden:
public:
  // From CCVGClassCmnBase
  virtual ~CCVGFWCCVGItemDataDriver() override;
  // From CCVGFWCCVGItemDataBase
  virtual void Clear() override;
   
// Methods:
private:
  void Copy(const CCVGFWCCVGItemDataDriver &vRhs);
  void Move(CCVGFWCCVGItemDataDriver &&vRhs);
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_CCVGITEMDATADRIVER_H_INCLUDE