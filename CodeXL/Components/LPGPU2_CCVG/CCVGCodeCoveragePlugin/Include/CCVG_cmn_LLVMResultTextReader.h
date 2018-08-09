// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMResultTextReader class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMRESULTTEXTREADER_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMRESULTTEXTREADER_H_INCLUDE

// STL:
#include <map>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionResult.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWLLVMResultTextReader is a class used to extract function
///           counter values from a text-formatted LLVM result file (.profdata)
/// @see      CCVGFWCCVGItemDataResults, CCVGFWLLVMFunctionResult
/// @date     06/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMResultTextReader final
: public CCVGClassCmnBase
{
// Typedefs:
using MapFuncHashToLLVMResult_t   = std::map<gtUInt64, CCVGFWLLVMFunctionResult>;
using PairFuncHashAndLLVMResult_t = std::pair<gtUInt64, CCVGFWLLVMFunctionResult>;
  
// Methods:
public:
  CCVGFWLLVMResultTextReader();
  status                            ReadData(const char *vpData, gtSize_t vDataSize);
  const MapFuncHashToLLVMResult_t&  GetMapFuncResults() const;

// Overridden:
public:
  virtual ~CCVGFWLLVMResultTextReader() override; // From CCVGClassCmnBase

// Constants:
public:
  static constexpr char     ms_constProfDataComment = '#';
  static constexpr CCVGuint ms_constProfDataMinLinesPerFunction = 4;

// Attributes:
protected:
  MapFuncHashToLLVMResult_t m_mapFuncResults;
    
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMRESULTTEXTREADER_H_INCLUDE