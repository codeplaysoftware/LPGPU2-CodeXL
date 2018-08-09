// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMMapping implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMMapping.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. No work done here.
/// @param[in] vSpan         The line/col span that this mapping covers
/// @param[in] vCounter      The index of the counter that this region maps to (default = 0)
/// @param[in] vExprLHS      The expression type (if any) represented by this mapping.
///                          (default  = CCVGFWLLVMRegionExprType::kNotExpression)
/// @param[in] vIsExpansion  Whether this mapping region is a macro region (default = false)
/// @param[in] vExprLHS      If an expression region, the index of the region that the left
///                          hand side refers to.
/// @param[in] vExprLHS      If an expression region, the index of the region that the right
///                          hand side refers to.
CCVGFWLLVMMapping::CCVGFWLLVMMapping(CCVGLLVMMappingSpan vSpan, gtUInt32 vCounter, 
  CCVGFWLLVMRegionExprType vExpr, // = CCVGFWLLVMRegionExprType::kNotExpression 
  bool vIsExpansion,  // = false 
  gtUInt32 vExprLHS, // = 0
  gtUInt32 vExprRHS) // = 0
: m_span(vSpan)
, m_counter(vCounter)
, m_bIsExpansion(vIsExpansion)
, m_eExpressionType(vExpr)
, m_exprLHS(vExprLHS)
, m_exprRHS(vExprRHS)
{
}

/// @brief Class destructor.
CCVGFWLLVMMapping::~CCVGFWLLVMMapping() = default;

/// @brief Retrieve the span of this region (the start and end lines and columns)
/// @return CCVGLLVMMappingSpan This region's span
CCVGLLVMMappingSpan CCVGFWLLVMMapping::GetSpan() const
{
  return m_span;
}

/// @brief Retrieve the counter value of this region.
/// @return gtUInt32 This region's counter value data.
gtUInt32 CCVGFWLLVMMapping::GetCounter() const
{
  return m_counter;
}

/// @brief Retrieve the expression type of this region.
/// @return CCVGFWLLVMRegionExprType Expression type enum value.
CCVGFWLLVMRegionExprType CCVGFWLLVMMapping::GetExpressionType() const
{
  return m_eExpressionType;
}

/// @brief Retrieve if this mapping region is an expansion region
/// @return bool true = region is expansion region, 
///              false = region is normal region.
bool CCVGFWLLVMMapping::IsExpansion() const
{
  return m_bIsExpansion;
}

/// @brief Retrieve the left hand side of the region's expression
/// @return gtUInt32 The LHS of the region's expression, or 0 if the mapping region is
///         not an expression region.
gtUInt32 CCVGFWLLVMMapping::GetExprLHS() const
{
  return m_exprLHS;
}

/// @brief Retrieve the right hand side of the region's expression
/// @return gtUInt32 The RHS of the region's expression, or 0 if the mapping region is
///         not an expression region.
gtUInt32 CCVGFWLLVMMapping::GetExprRHS() const
{
  return m_exprRHS;
}
} // namespace fw
} // namespace ccvg
