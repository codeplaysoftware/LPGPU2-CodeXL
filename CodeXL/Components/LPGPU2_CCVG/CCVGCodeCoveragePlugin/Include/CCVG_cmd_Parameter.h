// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdParameter interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_PARAMETER_H_INCLUDE
#define LPGPU2_CCVG_CMD_PARAMETER_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_VariantType.h>

namespace ccvg {
namespace cmd {

/// @brief    CCVG command parameter. A command may need information from 
///           outside in order for it to carry out it's task. Parameters
///           allow the passing of information to the commmand. *this 
///           parameter is passed to the client inside a parameters container
///           cmd::Params.
/// @warning  None.
/// @see      cmd::Params.
/// @date     19/08/2016.
/// @author   Illya Rudkin.
// clang-format off
struct CCVGCmdParameter final
{
// Attributes:
public:
  CCVGuint             parameterID;
  bool                 bManadatory; // True = yes mandatory, false = optional
  const wchar_t* const pTypeName;
  const wchar_t* const pParamName;
  const wchar_t* const pParamDescription;
  CCVGUtilVariantType  param;

// Methods:
public:
  CCVGCmdParameter()
  : parameterID(0)
  , bManadatory(true)
  , pTypeName(nullptr)
  , pParamName(nullptr)
  , pParamDescription(nullptr)
  {}
  CCVGCmdParameter(CCVGuint vID, bool vbM, const wchar_t * const vpTn, const wchar_t * const vpPn, const wchar_t * const vpPd)
  : parameterID(vID)
  , bManadatory(vbM)
  , pTypeName(vpTn)
  , pParamName(vpPn)
  , pParamDescription(vpPd)
  {}
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_PARAMETER_H_INCLUDE