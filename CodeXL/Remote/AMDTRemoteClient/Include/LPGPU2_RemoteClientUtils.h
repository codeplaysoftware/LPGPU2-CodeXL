// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_RemoteClientUtils.h
///
/// @brief Utility functions
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2REMOTECLIENTUTILS_H
#define LPGPU2REMOTECLIENTUTILS_H

#include <AMDTRemoteClient/Include/AMDTRemoteClientBuild.h>

// Introduce:
// std ints
#include <cstdint>

namespace lpgpu2 {

uint64_t AMDT_REMOTE_CLIENT_API swapEndianess(uint64_t x);
uint32_t AMDT_REMOTE_CLIENT_API swapEndianess(uint32_t x);

} // namespace lpgpu2

#endif
