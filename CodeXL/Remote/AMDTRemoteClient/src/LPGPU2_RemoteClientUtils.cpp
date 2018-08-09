// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_RemoteClientUtils.cpp
///
/// @brief Utility functions
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTRemoteClient/Include/LPGPU2_RemoteClientUtils.h>

namespace lpgpu2 {

/// @brief Change endinaness of a u64
/// @param x The u64 of which to change the endinaness
/// @return The original u64 with changed endinaness
uint64_t  swapEndianess(uint64_t x)
{
  x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
  x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
  x = (x & 0x00FF00FF00FF00FF) << 8  | (x & 0xFF00FF00FF00FF00) >> 8;
  return x;
}

/// @brief Change endinaness of a u32
/// @param x The u32 of which to change the endinaness
/// @return The original u32 with changed endinaness
uint32_t swapEndianess(uint32_t x)
{
  x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;
  x = (x & 0x00FF00FF) << 8  | (x & 0xFF00FF00) >> 8;
  return x;
}

} // namespace lpgpu2
