// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWEnumCCVGMDIViewEnumType interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_MDIVIEWNUMTYPES_H_INCLUDE
#define LPGPU2_CCVG_FW_MDIVIEWNUMTYPES_H_INCLUDE

namespace ccvg {
namespace fw {

/// @brief Enumeration of the CCVG's MDI view types.
/// @see      CCVGMDIViewCreator.
/// @date     15/03/2017.
/// @author   Illya Rudkin.
// clang-format off
enum EnumCCVGMDIViewType
{
  kCCVGMDIViewType_invalid = 0,
  kCCVGMDIViewTypeType_SourceCodeNonEdit,
  kCCVGMDIViewTypeType_count              // Always the last one
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_MDIVIEWNUMTYPES_H_INCLUDE