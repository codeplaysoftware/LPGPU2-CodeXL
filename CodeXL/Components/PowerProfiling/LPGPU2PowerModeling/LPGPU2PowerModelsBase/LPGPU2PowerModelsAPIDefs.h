// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief
///
/// Symbol export/import definitions
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_POWER_MODELING_API_DEFS_H_INCLUDE
#define LPGPU2_POWER_MODELING_API_DEFS_H_INCLUDE

#if defined(_WIN32)
#if defined(LPGPU2_POWER_MODELING_EXPORTS)
#define LPGPU2_POWER_MODELING_API __declspec(dllexport)
#else
#define LPGPU2_POWER_MODELING_API __declspec(dllimport)
#endif
#else
#define LPGPU2_POWER_MODELING_API
#endif // if defined(_WIN32)

#endif // LPGPU2_POWER_MODELING_API_DEFS_H_INCLUDE