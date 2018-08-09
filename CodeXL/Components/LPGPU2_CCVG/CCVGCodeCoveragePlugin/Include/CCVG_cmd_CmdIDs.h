// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// cmd::CmdID class numeration.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMD_CMDIDS_H_INCLUDE
#define LPGPU2_CCVG_CMD_CMDIDS_H_INCLUDE

namespace ccvg {
namespace cmd {

/// @brief    CCVG command unique IDs. One ID for each and every CCVG command
///           in existance. Note the command IDs are not the same as menu 
///           action IDs (though there is normally a one to one relationship).
/// @warning  None.
/// @see      cmd::Invoker, fw::CommandIDs.
/// @date     18/08/2016.
/// @author   Illya Rudkin.
// clang-format off
enum class CmdID 
{
  kInvalid = 0,
  kProjectNew,
  kProjectClose,
  kProjectSettingDlgOpen,
  kSwitchCCVGMode,
  kFilesReadAll,
  kFilesReadResults,
  kFilesReadDrivers,
  kFilesReadSource,
  kFilesReadExecuteable,
  kProjDataSetNoDriverFilesReq,
  kWndSrcOpen,
  kFileRead,
  kCmdCount     // Always the last!
};
// clang-format on

} // namespace cmd
} //  namespace ccvg

#endif // LPGPU2_CCVG_CMD_CMDIDS_H_INCLUDE