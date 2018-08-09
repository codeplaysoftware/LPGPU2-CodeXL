// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL information collector queries the system for available SYCL
///        devices and sends it to the specified pipe.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTSystemInformationHelper/inc/LPGPU2_sycl_InformationCollector.h>

// Sycl Info
#include <LPGPU2SyclInfo/LPGPU2_sycl_DevicesInfo.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>

// STL: for std::unique_ptr
#include <memory>

namespace lpgpu2 {
namespace sycl {

/// @brief                Query the system for SYCL devices and sends the data through the specified pipe.
/// @param[in] vPipeName  The pipe to where information will be send.
/// @returns   bool       true = the information was collected and sent to the calling process,
///                       false = something went wrong while collecting or sending the information.
bool InformationCollector::GenerateAndSendSYCLDevicesInformation(const gtString &vPipeName)
{
    auto bReturn = false;

    gtList<gtList<gtString>> infoData;
    const auto bRetCollectInfo = CollectSYCLDevicesInformation(infoData);

    auto pClient = std::unique_ptr<osPipeSocketClient>(new osPipeSocketClient{ vPipeName, L"System Information Connection Socket" });
    pClient->setWriteOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);
    const auto bRetOpenClient = pClient->open();

    GT_IF_WITH_ASSERT(bRetCollectInfo)
    {       
        GT_IF_WITH_ASSERT(bRetOpenClient)
        {
            gtString size;
            size.appendUnsignedIntNumber(static_cast<unsigned int>(infoData.size()));
            *pClient << size;

            while (infoData.size() > 0)
            {
                auto curList = infoData.front();
                size = L"";
                size.appendUnsignedIntNumber(static_cast<unsigned int>(curList.size()));
                *pClient << size;

                while (curList.size() > 0)
                {
                    auto curStr = curList.front();
                    *pClient << curStr;
                    curList.pop_front();
                }

                infoData.pop_front();
            }

            bReturn = true;
        }
    }

    pClient->close();

    return bReturn;
}

/// @brief                Collects information about SYCL devices available in the system.
///                       Loads the LPGPU2SyclInfo-x64 library that will in turn query the system
///                       using Codeplay's ComputeCpp Runtime.
/// @param[out] vInfoData A table with the SYCL devices information.
/// @return     bool      true = The collection was successful,
///                       false = Something went wrong during the collection.
bool InformationCollector::CollectSYCLDevicesInformation(gtList<gtList<gtString>> &vInfoData)
{
    auto bReturn = false;

    // Unfortunately we have to use macros here because we do not
    // access to the AMDTApplicationFramework definitions
    #if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        #define MODULE_PREFIX L""
        #define MODULE_EXTENSION L"dll"
        #if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            #define MODULE_SUFFIX L"-x64-d"
        #else
            #define MODULE_SUFFIX L"-x64"
        #endif
    #elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
        #define MODULE_PREFIX L"lib"
        #define MODULE_SUFFIX L""
        #define MODULE_EXTENSION L"so"
    #else
        #error Unsupported OS
    #endif

    osFilePath lpgpu2SyclInfoModulePath{ osFilePath::OS_CODEXL_BINARIES_PATH, MODULE_PREFIX L"LPGPU2SyclInfo" MODULE_SUFFIX, MODULE_EXTENSION };    
    if (lpgpu2SyclInfoModulePath.exists())
    {
        auto bRetSYCLModuleLoaded = false;
        osModuleHandle lpgpu2SyclInfoModuleHandle = OS_NO_MODULE_HANDLE;
        
        gtString err;
        const auto bAssertOnFail = false;
        bRetSYCLModuleLoaded = osLoadModule(lpgpu2SyclInfoModulePath, lpgpu2SyclInfoModuleHandle, &err, bAssertOnFail);

        GT_IF_WITH_ASSERT_EX(bRetSYCLModuleLoaded, err.asCharArray())
        {
            osProcedureAddress pFunctionHandler = nullptr;
            auto bRetGetProcAddress = osGetProcedureAddress(lpgpu2SyclInfoModuleHandle, g_pFnLPGPU2SyclGetDevicesInfoFunctionName, pFunctionHandler);

            GT_IF_WITH_ASSERT(bRetGetProcAddress)
            {
                auto lpgpu2GetSyclInfoDataFunc = reinterpret_cast<PFNLPGPU2SYCLGETDEVICESINFO>(pFunctionHandler);

                // Get SYCL info data
                SYCLDeviceInfoList syclDevicesInfoData;
                auto bGetSyclInfoRet = lpgpu2GetSyclInfoDataFunc(syclDevicesInfoData);

                GT_IF_WITH_ASSERT(bGetSyclInfoRet)
                {
                    // Transform the data into gtLists<gtString>
                    for (const auto &deviceInfo : syclDevicesInfoData)
                    {
                        gtList<gtString> deviceInfoAsGTList;
                        for (const auto &info : deviceInfo)
                        {
                            // Constructs a gtString in place
                            deviceInfoAsGTList.emplace_back(info);
                        }
                        vInfoData.push_back(deviceInfoAsGTList);
                    }

                    bReturn = true;
                }                
            }

            GT_ASSERT(osReleaseModule(lpgpu2SyclInfoModuleHandle));
        }
    }

    return bReturn;
}

} // namespace sycl
} // namespace lpgpu2

