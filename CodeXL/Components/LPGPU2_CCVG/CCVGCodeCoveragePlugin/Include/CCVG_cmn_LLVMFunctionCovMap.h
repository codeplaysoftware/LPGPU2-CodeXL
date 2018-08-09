// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWCCVGFunctionCovMap class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMFUNCTIONCOVMAP_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMFUNCTIONCOVMAP_H_INCLUDE

// Std:
#include <vector>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMMapping.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWLLVMFunctionCovMap is a class which represents the extracted
///           information of a particular function from LLVM Coverage Mapping
///           data, including the individual mapping regions.
/// @see      CCVGFWLLVMCovMapReader
/// @date     07/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMFunctionCovMap final
: public CCVGClassCmnBase
{
// Typedefs:
public:
  using VecFilenameID_t   = std::vector<gtUInt32>;
  using VecLLVMMapping_t  = std::vector<CCVGFWLLVMMapping>;

// Methods:
public:
  CCVGFWLLVMFunctionCovMap(gtUInt64 vFuncHash, gtUInt64 vStructHash);
  gtUInt64                GetFuncHash() const;
  gtUInt64                GetStructHash() const;
  gtSize_t                GetFileNameIDCount() const;
  gtSize_t                GetMappingCount() const;
  const VecFilenameID_t&  GetVecFileNameIDs() const;
  const VecLLVMMapping_t& GetVecLLVMMappings() const;
  status                  AddFileNameID(gtUInt32);
  status                  AddMapping(const CCVGFWLLVMMapping &vMapping);

// Overridden:
public:
  virtual ~CCVGFWLLVMFunctionCovMap() override; // From CCVGClassCmnBase

// Attributes:
protected:
  gtUInt64          m_funcHash;
  gtUInt64          m_structHash;
  VecFilenameID_t   m_vecFileNameIDs;
  VecLLVMMapping_t  m_vecLLVMMappings;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMFUNCTIONCOVMAP_H_INCLUDE