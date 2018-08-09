// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_REGISTERALL_H_INCLUDE
#define LPGPU2_CCVG_CMD_REGISTERALL_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>

namespace ccvg {
namespace cmd {

/// @brief CCVG commands are instantiated and registered automatically with the
///        Command Factory.
fnstatus::status RegisterAll();

} // namespace cmd
}//  namespace ccvg

#endif // LPGPU2_CCVG_CMD_REGISTERALL_H_INCLUDE