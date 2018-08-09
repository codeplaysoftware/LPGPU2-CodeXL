// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Android remote protocol build definitions file which setup whether
///        the Android profiling addition is compiled in or not.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_ANDROIDREMOTEPROTOCOLBUILDDEFINITIONS_H_INCLUDE
#define LPGPU2_ANDROIDREMOTEPROTOCOLBUILDDEFINITIONS_H_INCLUDE

// Determines whether the Android remote protcol code is to be compiled in or
// not.
// 1 = yes compile in, 0 = compile out
// It can also be defined when building the project
// Default = Compile IN
#ifndef LPGPU2_ANDROID_REMOTE_PROTOCOL_COMPILE_OUT
#define LPGPU2_ANDROID_REMOTE_PROTOCOL_COMPILE_IN 1
#else
#define LPGPU2_ANDROID_REMOTE_PROTOCOL_COMPILE_IN 0
#endif // LPGPU2_ANDROID_REMOTE_PROTOCOL_COMPILE_IN

#endif // LPGPU2_ANDROIDREMOTEPROTOCOLBUILDDEFINITIONS_H_INCLUDE
