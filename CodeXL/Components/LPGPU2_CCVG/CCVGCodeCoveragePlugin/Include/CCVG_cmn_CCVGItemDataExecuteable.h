// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGItemDataExecuteable interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_CCVGITEMDATAEXECUTABLE_H_INCLUDE
#define LPGPU2_CCVG_FW_CCVGITEMDATAEXECUTABLE_H_INCLUDE

// Standard:
#include <memory> // For unique_ptr for creating an AMDTExecutableFormat

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_CCVGItemDataBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMCovMapReader.h>

// Framework:
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWCCVGItemDataExecuteable is derived class. The class 
///           represents  data in this case read from either a code 
///           coverage executable. It may holder other information to 
///           be associated with that file or content.
/// @see      CCVGFWTheCurrentStateProjectData, CCVGItems, 
///           CCVGFWTheCurrentStateProjectDataCcvg
/// @date     22/03/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWCCVGItemDataExecuteable final
: public CCVGFWCCVGItemDataBase
{
// Methods:
public:
  CCVGFWCCVGItemDataExecuteable(CCVGFWTheCurrentStateProjectDataCcvg &vpParent);
  CCVGFWCCVGItemDataExecuteable(const CCVGFWCCVGItemDataExecuteable &vRhs);
  CCVGFWCCVGItemDataExecuteable(CCVGFWCCVGItemDataExecuteable &&vRhs);
  CCVGFWCCVGItemDataExecuteable& operator= (const CCVGFWCCVGItemDataExecuteable &vRhs);
  CCVGFWCCVGItemDataExecuteable& operator= (CCVGFWCCVGItemDataExecuteable &&vRhs);
  bool                      operator!= (const CCVGFWCCVGItemDataExecuteable &vRhs) const;
  bool                      operator== (const CCVGFWCCVGItemDataExecuteable &vRhs) const;
  //
  status                        ReReadFileData();
  status                        CreateExecutable(const gtString &vrFileName, const gtString &vrFileDirPath);
  const CCVGFWLLVMCovMapReader& GetCovMapReader() const;
  
// Overridden:
public:
  // From CCVGClassCmnBase
  virtual ~CCVGFWCCVGItemDataExecuteable() override;
  // From CCVGFWCCVGItemDataBase
  virtual void Clear() override;
  
// Attributes
protected:
  std::unique_ptr<ExecutableFile> m_executableFile;
  const gtUByte                  *m_pCovMapBytes;
  CCVGFWLLVMCovMapReader          m_covMapReader;
  
// Methods:
private:
  void Copy(const CCVGFWCCVGItemDataExecuteable &vRhs);
  void Move(CCVGFWCCVGItemDataExecuteable &&vRhs);  
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_CCVGITEMDATAEXECUTABLE_H_INCLUDE