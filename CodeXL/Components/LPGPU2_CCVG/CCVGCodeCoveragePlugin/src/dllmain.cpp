// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Third party includes:
#include <Windows.h>

// AMDT includes:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

/// @brief DLL main.
///
/// The Code Coverage functions does not used the DLL functionality at this 
/// time.
/// An optional entry point into a dynamic-link library (DLL). When the system
/// starts or terminates a process or thread, it calls the entry-point function
/// for each loaded DLL using the first thread of the process. The system also
/// calls the entry-point function for a DLL when it is loaded or unloaded 
/// using the LoadLibrary and FreeLibrary functions.
/// @see        Microsoft knowledge base for greater detail.
/// @param[in]  hModule A handle to the DLL module. The value is the base 
///             address of the DLL. The HINSTANCE of a DLL is the same as the 
///             HMODULE of the DLL, so hinstDLL can be used in calls to 
///             functions that require a module handle.
/// @param[in]  fdwReason The reason code that indicates why the DLL 
///             entry-point function is being called.
/// @param[in]  lpReserved If fdwReason is DLL_PROCESS_ATTACH, lpvReserved is
///             NULL for dynamic loads and non-NULL for static loads. If 
///             fdwReason is DLL_PROCESS_DETACH, lpvReserved is NULL if 
///             FreeLibrary has been called or the DLL load failed and non - 
///             NULL if the process is terminating.
/// @return     BOOL When the system calls the DllMain function with the 
///             DLL_PROCESS_ATTACH value, the function returns TRUE if it 
///             succeeds or FALSE if initialization fails. If the return value
///             is FALSE when DllMain is called because the process uses the 
///             LoadLibrary function, LoadLibrary returns NULL. (The system 
///             immediately calls your entry-point function with 
///             DLL_PROCESS_DETACH and unloads the DLL.) If the return value is
///             FALSE when DllMain is called during process initialization, the 
///             process terminates with an error. To get extended error 
///             information, call GetLastError. When the system calls the 
///             DllMain function with any value other than DLL_PROCESS_ATTACH, 
///             the return value is ignored.
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
