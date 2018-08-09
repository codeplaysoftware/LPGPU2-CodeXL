// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMRegionExprType class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMREGIONEXPRTYPE_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMREGIONEXPRTYPE_H_INCLUDE

namespace ccvg {
namespace fw {

/// @brief    CCVGFWLLVMRegionExprType is an enum that represents whether a
///           particular mapping region is an expression region (a region whose
///           value is expressed in terms of other regions), and if so what type
///           it is.
/// @see      CCVGFWLLVMCovMapReader
/// @date     12/09/2017.
/// @author   Callum Fare.
// clang-format off
enum class CCVGFWLLVMRegionExprType
{
  kNotExpression,
  kAddExpression,
  kSubExpression
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMREGIONEXPRTYPE_H_INCLUDE