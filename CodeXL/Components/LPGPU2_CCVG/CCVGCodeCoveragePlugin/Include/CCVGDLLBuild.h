// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_DLLBUILD_H_INCLUDE
#define LPGPU2_CCVG_DLLBUILD_H_INCLUDE

/// @brief  Windows DLL import export __declspec definition.
///
/// Under Win32 builds - define: LPGPU2_CCVG_API to be:
///  - When building LPGPU2_CCVGCodeCoverage.dll: __declspec(dllexport).
///  - When building other projects:              __declspec(dllimport).
#if defined(_WIN32)
  #if defined(LPGPU2_CCVG_EXPORTS)
      #define LPGPU2_CCVG_API __declspec(dllexport)
  #else
      #define LPGPU2_CCVG_API __declspec(dllimport)
  #endif // defined(LPGPU2_CCVG_EXPORTS)
#else
    #define LPGPU2_CCVG_API
#endif // defined(_WIN32)

#endif  // LPGPU2_CCVG_DLLBUILD_H_INCLUDE
