// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMFunctionSummary class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMFUNCTIONSUMMARY_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMFUNCTIONSUMMARY_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWLLVMFunctionSummary is a class that represents the information
///           to be displayed in an inline annotation at the top of each function.
///           Currently this consists on the number of regions executed. It also
///           contains the line number where the function begins.
/// @see      CCVGFWLLVMResultTextReader
/// @date     19/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMFunctionSummary final
: public CCVGClassCmnBase
{
// Methods:
public:
  CCVGFWLLVMFunctionSummary(gtString vName, gtUInt32 vStartLine, gtUInt32 vCounterCount, gtUInt32 vNonZeroCounterCount);
  gtUInt32        GetStartLine() const;
  const gtString& GetName() const;
  gtUInt32        GetCounterCount() const;
  gtUInt32        GetNonZeroCounterCount() const;

// Overridden
public:
  // From CCVGClassCmnBase
  virtual ~CCVGFWLLVMFunctionSummary() override; 

// Attributes:
protected:
  gtString  m_name;
  gtUInt32  m_startLine;
  gtUInt32  m_counterCount;
  gtUInt32  m_nonZeroCounterCount;
    
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMFUNCTIONSUMMARY_H_INCLUDE