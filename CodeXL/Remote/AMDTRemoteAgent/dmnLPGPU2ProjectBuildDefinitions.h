// Copyright (C) 2002-2017 Codeplay Software Limited. All Rights Reserved.

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
/// Copyright (C) 2002-2017 Codeplay Software Limited. All Rights Reserved.
 
#ifndef LPGPU2_LCXL_REMOTEAGENT_PROJECTBUILDPREPROCESSORDEFIITIONS_H_INCLUDE
#define LPGPU2_LCXL_REMOTEAGENT_PROJECTBUILDPREPROCESSORDEFIITIONS_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#define LGPGPU2_LCXL_REMOTEAGENT_ARCH_ARM     0
#define LGPGPU2_LCXL_REMOTEAGENT_ARCH_X86     1
#define LGPGPU2_LCXL_REMOTEAGENT_ARCH_X86_64  2
#define LGPGPU2_LCXL_REMOTEAGENT_ARCH_AARCH64 3

// Associate different architectures with numbers so that we can use them
// to compile in or our certain code parts depending on the architecture in use.
// See below for an explanation on why we need to do this.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
  #if defined(__arm__)
    #define LPGPU2_LCXL_REMOTEAGENT_ARCH LGPGPU2_LCXL_REMOTEAGENT_ARCH_ARM
  #elif defined(__i386__)
    #define LPGPU2_LCXL_REMOTEAGENT_ARCH LGPGPU2_LCXL_REMOTEAGENT_ARCH_X86
  #elif defined(__x86_64__)
    #define LPGPU2_LCXL_REMOTEAGENT_ARCH LGPGPU2_LCXL_REMOTEAGENT_ARCH_X86_64
  #elif defined(__aarch64__)
    #define LPGPU2_LCXL_REMOTEAGENT_ARCH LGPGPU2_LCXL_REMOTEAGENT_ARCH_AARCH64
  #endif
#endif

// The RemoteAgent module needs to be compiled in different modes depending on
// the type of target architecture. For example, if we are compiling for ARMv7
// we need to exclude the code which references the Power Profiling module
// (for now, as it is not YET supported on that platform).
// It can also be defined when building the project
//
//  If the macro is already defined, it could be that it has been set in the
//  build and so we should not change it.
#define LGPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_YES 0
#define LGPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_NO 1

#ifndef LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN
  #if LGPGPU2_LCXL_REMOTEAGENT_ARCH == LGPGPU2_LCXL_REMOTEAGENT_ARCH_ARM
    #define LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN LGPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_NO
  #elif LGPGPU2_LCXL_REMOTEAGENT_ARCH == LGPGPU2_LCXL_REMOTEAGENT_ARCH_AARCH64
    #define LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN LGPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_NO
  #elif (LGPGPU2_LCXL_REMOTEAGENT_ARCH == LGPGPU2_LCXL_REMOTEAGENT_ARCH_X86) || (LGPGPU2_LCXL_REMOTEAGENT_ARCH == LGPGPU2_LCXL_REMOTEAGENT_ARCH_X86_64)
    #define LPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN LGPGPU2_LCXL_REMOTEAGENT_POWERPROFILING_COMPILE_IN_YES
  #endif
#endif

#endif // LPGPU2_LCLX_REMOTEAGENT_PROJECTBUILDPREPROCESSORDEFIITIONS_H_INCLUDE
