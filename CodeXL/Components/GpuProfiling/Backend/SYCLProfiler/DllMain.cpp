// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief SYCLTraceAgent is the project that defines a Dynamic Linked Library
///        meant to be injected into the target SYCL application process. This
///        will register a callback function to capture the profiled events.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include "SYCLTraceAgent.h"

#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, PVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            SYCLProfiler::SYCLProfilerInit();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            SYCLProfiler::SYCLProfilerDeinit();
            break;
    }
    return TRUE;
}

