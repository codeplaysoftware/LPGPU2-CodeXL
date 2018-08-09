// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL information collector queries the system for available SYCL
///        devices and sends it to the specified pipe.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#if defined(_WIN32)
    #if defined(LPGPU2SYCLAPI_EXPORTS)
        #define LPGPU2_SYCL_API extern "C" __declspec(dllexport)
    #else
        #define LPGPU2_SYCL_API extern "C" __declspec(dllimport)
    #endif
#else
    #define LPGPU2_SYCL_API extern "C"
#endif
