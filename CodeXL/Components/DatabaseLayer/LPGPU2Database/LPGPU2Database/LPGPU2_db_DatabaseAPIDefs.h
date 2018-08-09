// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customization of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// Symbol export/import definitions
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DB_DATABASE_API_DEFS_H_INCLUDE
#define LPGPU2_DB_DATABASE_API_DEFS_H_INCLUDE

#if defined(_WIN32)
    #if defined(LPGPU2_DATABASE_EXPORTS)
        #define LPGPU2_DATABASE_API __declspec(dllexport)
    #else
        #define LPGPU2_DATABASE_API __declspec(dllimport)
    #endif
#else
    #define LPGPU2_DATABASE_API
#endif // if defined(_WIN32)

#endif // LPGPU2_DB_DATABASE_API_DEFS_H_INCLUDE