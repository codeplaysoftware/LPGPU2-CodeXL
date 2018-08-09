// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMFunctionResult class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMFUNCTIONRESULT_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMFUNCTIONRESULT_H_INCLUDE

// STL:
#include <vector>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWLLVMFunctionResult is a class that represents the counter
///           result values for a given function, extracted from a .profdata file.
/// @see      CCVGFWLLVMResultTextReader
/// @date     06/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMFunctionResult final
: public CCVGClassCmnBase
{
  // Typedefs:
  using VecCounterValue_t = std::vector<gtUInt32>;

// Methods:
public:
  CCVGFWLLVMFunctionResult(gtString vName, gtUInt64 vStructHash);

  status    Initialise();

  status    AddCounterValue(gtUInt32 vValue);
  gtUInt32  GetCounterValue(gtUInt32 vIndex) const;
  gtUInt64  GetHash() const;
  gtString  GetName() const;
  gtUInt32  GetCounterCount() const;
  gtUInt32  GetNonZeroCounterCount() const;

// Overridden:
public:
  virtual ~CCVGFWLLVMFunctionResult() override; // From CCVGClassCmnBase
  
// Constants:
public:
    static const CCVGuint ms_constHashLowerBytes = 8;

// Attributes:
protected:
  gtString          m_name;
  gtUInt64          m_hash;
  gtUInt64          m_structHash;
  VecCounterValue_t m_vecCounterValues;
  gtUInt32          m_counterCount;
  gtUInt32          m_nonZeroCounterCount;
    
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMFUNCTIONRESULT_H_INCLUDE