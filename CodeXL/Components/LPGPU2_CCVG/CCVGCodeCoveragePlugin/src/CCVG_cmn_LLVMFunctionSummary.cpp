// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMFunctionSummary implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
 
// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionSummary.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. No work done here.
CCVGFWLLVMFunctionSummary::CCVGFWLLVMFunctionSummary(gtString vName, gtUInt32 vStartLine, gtUInt32 vCounterCount, gtUInt32 vNonZeroCounterCount)
: m_name(vName)
, m_startLine(vStartLine)
, m_counterCount(vCounterCount)
, m_nonZeroCounterCount(vNonZeroCounterCount)
{
}

/// @brief Class destructor. No work done here.
CCVGFWLLVMFunctionSummary::~CCVGFWLLVMFunctionSummary() = default;

/// @brief Get the hash of the function's name.
/// @return gtUInt32  The retrieved hash value as calculated during construction.
gtUInt32 CCVGFWLLVMFunctionSummary::GetStartLine() const
{
  return m_startLine;
}

/// @brief Get the function's name.
/// @return  The retrieved name of the function represented by this result.
const gtString& CCVGFWLLVMFunctionSummary::GetName() const
{
  return m_name;
}

/// @brief Get the number of counters in this function
/// @return gtUInt32 The number of counters.
gtUInt32 CCVGFWLLVMFunctionSummary::GetCounterCount() const
{
  return m_counterCount;
}

/// @brief Get the number of counter regions in this function that were executed
///        at least once.
/// @return gtUInt32  The number of executed counters
gtUInt32 CCVGFWLLVMFunctionSummary::GetNonZeroCounterCount() const
{
  return m_nonZeroCounterCount;
}

} // namespace fw
} // namespace ccvg
