//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file siAMDTSystemInformationHelper.cpp
///
//==================================================================================

/// AMDTSystemInformationHelper.cpp : Defines the entry point for the console application.
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

#include <stdio.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <tchar.h>
#endif

#include "../inc/siOpenCLInformationCollector.h"
#include "../inc/LPGPU2_sycl_InformationCollector.h"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    int _tmain(int argc, _TCHAR* argv[])
#else
    int main(int argc, char* argv[])
#endif
{
    int exitCode = 0;

    gtString pipeName;
    gtString api = AP_STR_OpenCL;

    if (argc > 1)
    {
        // getting pipe name
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        pipeName = gtString(argv[1]);
#else
        pipeName.fromASCIIString(argv[1]);
#endif
      
        //++TLRS: LPGPU2: Getting the API from the command line
        if (argc > 2)
        {
            #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                api = gtString{ argv[2] };
            #else
                api.fromASCIIString(argv[2]);
            #endif            
        }        
        //--TLRS: LPGPU2: Getting the API from the command line
    }

    if (!pipeName.isEmpty())
    {
        if (api == gtString{ AP_STR_OpenCL })
        {
            siOpenCLInformationCollector sysInfo;
            bool rcGetInfo = sysInfo.GenerateAndSendOpenCLDevicesInformation(pipeName);
            exitCode = rcGetInfo ? 0 : 1;
        }
        else if (api == gtString{ AP_STR_SYCL })
        {
            lpgpu2::sycl::InformationCollector sysInfo;
            const auto bRetGetInfo = sysInfo.GenerateAndSendSYCLDevicesInformation(pipeName);
            exitCode = bRetGetInfo ? 0 : 1;
        }
        else
        {
            exitCode = 0;
        }
    }

    return exitCode;
}
