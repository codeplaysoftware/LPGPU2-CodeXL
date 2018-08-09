// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_DATATYPES_H_INCLUDE
#define LPGPU2_CCVG_DATATYPES_H_INCLUDE

// Portability issues:
#ifdef _WIN64
typedef unsigned __int64 size_t;
typedef __int64 CCVGint;
typedef unsigned __int64 CCVGuint;
#else
#ifdef _WIN32
typedef unsigned int size_t;
typedef int CCVGint;
typedef unsigned int CCVGuint;
#else
typedef int CCVGint;
typedef unsigned int CCVGuint;
#endif // _WIN32
#endif // _WIN64

#endif // LPGPU2_CCVG_DATATYPES_H_INCLUDE
