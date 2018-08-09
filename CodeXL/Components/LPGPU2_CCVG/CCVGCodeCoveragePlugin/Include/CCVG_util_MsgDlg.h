// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// Global utility or helper functions.
/// CCVGUtilMsgDlg interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_UTIL_MSGDLG_H_INCLUDE
#define LPGPU2_CCVG_UTIL_MSGDLG_H_INCLUDE

/// @brief    .
/// @warning  None.
/// @see      None.
/// @date     06/01/2017.
/// @author   Illya Rudkin.
// clang-format off
class CCVGUtilMsgDlg final
{
// Statics:
public:
  static void Show(const gtString &vMsg);

// Attributes:
private:
  static constexpr int ms_msgBxWidth = 600;
  static constexpr int ms_msgBxHeight = 400;
};
// clang-format on

#endif // LPGPU2_CCVG_UTIL_MSGDLG_H_INCLUDE