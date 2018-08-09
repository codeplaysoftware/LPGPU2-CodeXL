// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL information collector queries the system for available SYCL
///        devices and sends it to the specified pipe.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2SyclInfo/LPGPU2_sycl_DLLBuild.h>

// STL:
#include <vector>

// Declarations
class gtString;
using SYCLDeviceInfoList = std::vector<std::vector<gtString>>;

// Define the function pointer
using PFNLPGPU2SYCLGETDEVICESINFO = bool(*)(SYCLDeviceInfoList&);

//typedef bool(*PFNLPGPU2SYCLGETDEVICESINFO)(SYCLDeviceInfoList&);

// This function name needs to match the actual declaration because it is meant to be used for dynamic loading the function.
static const char *g_pFnLPGPU2SyclGetDevicesInfoFunctionName = "LPGPU2_SYCL_GetDevicesInfo";

// Functions
LPGPU2_SYCL_API bool LPGPU2_SYCL_GetDevicesInfo(SYCLDeviceInfoList &vSyclDevicesInfo);