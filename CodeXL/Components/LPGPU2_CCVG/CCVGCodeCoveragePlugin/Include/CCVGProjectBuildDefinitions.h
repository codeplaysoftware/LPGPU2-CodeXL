// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVG build preprocessor definitions for all CCVG's code base. File will 
/// also be included in CodeXL's standard code base where code has to be 
/// compiled in or out depending of these definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
 
#ifndef LPGPU2_CCVG_PROJECTBUILDPREPROCESSORDEFIITIONS_H_INCLUDE
#define LPGPU2_CCVG_PROJECTBUILDPREPROCESSORDEFIITIONS_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Visual Studio generated file, see AMDTApplicationFramework project
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// Local:
#include "CCVGProjectBuildDefinitions_Generated.h"
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// CCVG project code base include with work with other CodeXL execution modes.
// 1 = yes compile in, 0 = compile out
// It can also be defined when building the project
#ifndef LPGPU2_CCVG_COMPILE_IN
#define LPGPU2_CCVG_COMPILE_IN 0
#else
#define LPGPU2_CCVG_COMPILE_IN 1
#endif // LPGPU2_CCVG_COMPILE_IN

// Only compile the the required CodeXL code base to allow CCVG project code
// to cpmpiler and operate. Exclude the other CodeXL execution modes. Note
// other CodeXL projects for other execution modes may need building sperately
// so their DLLs exist.
// 1 = yes compile out, 0 = compile in
#define LPGPU2_CCVG_COMPILE_OUT_CODECOVERAGE_ONLY_EXE_MODE 0

#endif // LPGPU2_CCVG_PROJECTBUILDPREPROCESSORDEFIITIONS_H_INCLUDE
