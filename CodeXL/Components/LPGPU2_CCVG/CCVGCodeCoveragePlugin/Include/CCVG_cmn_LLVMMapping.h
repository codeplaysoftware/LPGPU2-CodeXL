// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMMapping class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMMAPPING_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMMAPPING_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMRegionExprType.h>

namespace ccvg {
namespace fw {
  
/// @brief CCVGLLVMMappingSpan is a struct which contains the range of the source
///        code occupied by a particular LLVM mapping region.
struct CCVGLLVMMappingSpan
{
  CCVGLLVMMappingSpan(gtUInt32 vStartLine, gtUInt32 vStartCol, gtUInt32 vEndLine, gtUInt32 vEndCol)
  : m_startLine(vStartLine)
  , m_startCol(vStartCol)
  , m_endLine(vEndLine)
  , m_endCol(vEndCol)
  {
  }
  gtUInt32  m_startLine;
  gtUInt32  m_startCol;
  gtUInt32  m_endLine;
  gtUInt32  m_endCol;
};

/// @brief    CCVGFWLLVMMapping is a class which represents an individual
///           mapping from a region in a source file to an LLVM counter. 
///           Unlike in the encoded format, positions should be absolute.
/// @see      CCVGFWLLVMFunctionCovMap
/// @date     07/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMMapping final
: public CCVGClassCmnBase
{
// Methods:
public:
  CCVGFWLLVMMapping(CCVGLLVMMappingSpan span, gtUInt32 vCounter, CCVGFWLLVMRegionExprType veExpr = CCVGFWLLVMRegionExprType::kNotExpression, 
    bool vIsExpansion = false, gtUInt32 vExprLHS = 0, gtUInt32 vExprRHS = 0);
  CCVGLLVMMappingSpan       GetSpan() const;
  bool                      IsExpansion() const;
  gtUInt32                  GetCounter() const;
  gtUInt32                  GetExprLHS() const;
  gtUInt32                  GetExprRHS() const;
  CCVGFWLLVMRegionExprType  GetExpressionType() const;

// Overridden
public:
  virtual ~CCVGFWLLVMMapping() override; // From CCVGClassCmnBase

// Attributes:
protected:
  CCVGLLVMMappingSpan       m_span;
  gtUInt32                  m_counter;
  bool                      m_bIsExpansion;
  CCVGFWLLVMRegionExprType  m_eExpressionType;
  gtUInt32                  m_exprLHS;
  gtUInt32                  m_exprRHS;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMMAPPING_H_INCLUDE