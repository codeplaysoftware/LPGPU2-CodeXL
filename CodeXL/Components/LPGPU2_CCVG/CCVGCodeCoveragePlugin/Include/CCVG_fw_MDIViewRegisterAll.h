// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWREGISTERALL_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWREGISTERALL_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGFnStatus.h>

namespace ccvg {
namespace fw {

  /// @brief CCVG Views are instantiated and registered automatically with the
  ///        MDI View Factory.
  fnstatus::status MDIViewRegisterAll();

} // namespace cmd
}//  namespace ccvg

#endif // LPGPU2_CCVG_FW_MDIVIEWREGISTERALL_H_INCLUDE