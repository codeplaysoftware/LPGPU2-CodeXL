// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateCurrentPCUser interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_THECURRENTSTATECURRENTPCUSER_H_INCLUDE
#define LPGPU2_CCVG_FW_THECURRENTSTATECURRENTPCUSER_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWTheCurrentStateCurrentPCUser provides determines and 
///           provides a signature on the current PC being used and the
///           current user. This is information is used to determine if 
///           CodeXL project has been copied or moved to a PC with a
///           different file drive configuration to that stored with the
///           project. If different then the CCVG plugin will disabled
///           itself for the current project letting the user reconfigure
///           to the seting to point the folder file directories to valid
///           file paths.
/// @warning  None.
/// @see      CCVGFWTheCurrentState.
/// @date     29/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTheCurrentStateCurrentPCUser final
: public CCVGClassCmnBase
{
  

// Methods:
public:
  CCVGFWTheCurrentStateCurrentPCUser();
  ~CCVGFWTheCurrentStateCurrentPCUser();
  //
  gtString GetComputerIDCurrent() const;
  gtString GetUserIDCurrent() const;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_THECURRENTSTATECURRENTPCUSER_H_INCLUDE