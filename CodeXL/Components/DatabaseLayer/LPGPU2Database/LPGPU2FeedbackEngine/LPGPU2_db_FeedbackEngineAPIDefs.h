// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Feedback engine is an add on to LPGPU2Database Layer that
///         allows execution of custom operations on the database regarding
///         crunching of Android Profiling data.
///
/// Symbol export/import definitions
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
#ifndef LPGPU2_DB_FEEDBACKENGINE_API_DEFS_H_INCLUDE
#define LPGPU2_DB_FEEDBACKENGINE_API_DEFS_H_INCLUDE

#if defined(_WIN32)
    #if defined(LPGPU2FEEDBACKENGINE_EXPORTS)
        #define LPGPU2_FEEDBACKENGINE_API __declspec(dllexport)
    #else
        #define LPGPU2_FEEDBACKENGINE_API __declspec(dllimport)
    #endif
#else
    #define LPGPU2_FEEDBACKENGINE_API
#endif // if defined(_WIN32)

#endif // LPGPU2_DB_FEEDBACKENGINE_API_DEFS_H_INCLUDE