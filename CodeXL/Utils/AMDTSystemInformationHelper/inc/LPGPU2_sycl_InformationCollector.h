// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL information collector queries the system for available SYCL
///        devices and sends it to the specified pipe.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_SI_SYCL_SYSTEM_INFO_COLLECTOR_H_INCLUDE
#define LPGPU2_SI_SYCL_SYSTEM_INFO_COLLECTOR_H_INCLUDE

#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

namespace lpgpu2 {
namespace sycl {

/// @brief    The SYCL information collector queries the system for available SYCL
///           devices and sends it to the specified pipe.
/// @date     02/01/2018
/// @warning  Calling GenerateAndSendSYCLDevicesInformation will cause data to be sent
///           using osPipeSocketClient.
/// @author   Thales Sabino.
// clang-format off
class InformationCollector final
{
// Methods
public:
    InformationCollector() = default;
    ~InformationCollector() = default;    
    
    bool GenerateAndSendSYCLDevicesInformation(const gtString &vPipeName);

// Methods
private:
    bool CollectSYCLDevicesInformation(gtList<gtList<gtString>> &vInfoData);    
};
// clang-format on

} // namespace sycl
} // namespace lpgpu2

#endif // LPGPU2_SI_SYCL_SYSTEM_INFO_COLLECTOR_H_INCLUDE