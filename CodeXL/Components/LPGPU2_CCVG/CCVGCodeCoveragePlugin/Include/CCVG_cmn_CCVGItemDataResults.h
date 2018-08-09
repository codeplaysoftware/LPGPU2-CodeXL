// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataResults interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_CCVGITEMDATARESULTS_H_INCLUDE
#define LPGPU2_CCVG_FW_CCVGITEMDATARESULTS_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMResultTextReader.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWCCVGItemDataResults is derived class. The class represents 
///           data in this case read from either a code coverage results, the 
///           source to a kernel file or the source code from a driver source 
///           code file. It may holder other information to associated with 
///           that file or content.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems, 
///           CCVGFWTheCurrentStateProjectDataCcvg
/// @date     16/02/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWCCVGItemDataResults final
: public CCVGFWCCVGItemDataBase
{
// Methods:
public:
  CCVGFWCCVGItemDataResults(CCVGFWTheCurrentStateProjectDataCcvg &vpParent);
  CCVGFWCCVGItemDataResults(const CCVGFWCCVGItemDataResults &vRhs);
  CCVGFWCCVGItemDataResults(CCVGFWCCVGItemDataResults &&vRhs);
  CCVGFWCCVGItemDataResults& operator= (const CCVGFWCCVGItemDataResults &vRhs);
  CCVGFWCCVGItemDataResults& operator= (CCVGFWCCVGItemDataResults &&vRhs);
  bool                       operator!= (const CCVGFWCCVGItemDataResults &vRhs) const;
  bool                       operator== (const CCVGFWCCVGItemDataResults &vRhs) const;
  //
  status                            ReReadFileData();
  status                            ParseResults();
  const CCVGFWLLVMResultTextReader& GetResultReader() const;

// Overridden:
public:
  // From CCVGClassCmnBase
  virtual ~CCVGFWCCVGItemDataResults();
  // From CCVGFWCCVGItemDataBase
  virtual void Clear() override;
  
// Attributes:
protected:
  CCVGFWLLVMResultTextReader m_resultReader;

// Methods:
private:
  void Copy(const CCVGFWCCVGItemDataResults &vRhs);
  void Move(CCVGFWCCVGItemDataResults &&vRhs);
  
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_CCVGITEMDATARESULTS_H_INCLUDE