// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Specify the duration mode for power profiling
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DURATIONMODE_H_INCLUDE
#define LPGPU2_DURATIONMODE_H_INCLUDE

namespace lpgpu2 {

enum class DurationMode
{
  Manual,
  Timer,
  DeviceDuration,
  ExplicitControl,
  ENUM_COUNT
}; // enum class DurationMode

} // namespace lpgpu2

#endif
