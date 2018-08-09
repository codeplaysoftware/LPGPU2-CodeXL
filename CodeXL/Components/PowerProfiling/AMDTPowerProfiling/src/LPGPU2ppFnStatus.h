// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Define return status for functions within the Power Profiling project
///        which hve been implemented by Codeplay.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef PP_LPGPU2_FNSTATUS_H_INCLUDE
#define PP_LPGPU2_FNSTATUS_H_INCLUDE

/// @brief  Used to idenfity the return status of a function.
/// @author  Alberto Taiuti

namespace lpgpu2 {

enum class PPFnStatus {
  success,
  failure
}; // enum class PPFnStatus

} // namespace lpgpu2

#endif // PP_LPGPU2_FNSTATUS_H_INCLUDE
