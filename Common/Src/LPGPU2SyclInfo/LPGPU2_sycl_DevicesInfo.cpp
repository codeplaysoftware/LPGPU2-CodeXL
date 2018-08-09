// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL information collector queries the system for available SYCL
///        devices and sends it to the specified pipe.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2SyclInfo/LPGPU2_sycl_DevicesInfo.h>

// SYCL:
#include <CL/sycl.hpp>
#include <SYCL/device.h>

// STL:
#include <iostream>
#include <sstream>

// Infra:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTBaseTools/Include/gtString.h>

namespace lpgpu2 {
namespace sycl {

/// @brief Defines the conversion policies that can be used with
///        the ToString functions
enum class EConversionPolicy
{
    kNoConversion,          ///< No conversion is performed.
    kConvertToMemory,       ///< The value is treated as a memory size in bytes and gets converted to MB, 
                            ///  if it is bigger than 1024.
                            kConvertToFrequency     ///< Adds the suffix MHz to the converted string.
};

/// @brief                        Converts a value a formatted string depending on 
///                               the specified conversion policy.
/// @param[in] vValue             The value to be converted.
/// @param[in] vConversionPolicy  The conversion policy to use when building the string.
/// @returns                      The converted string.
/// @see                          EConversionPolicy.
template<typename T>
gtString ToString(T vValue, EConversionPolicy vConversionPolicy = EConversionPolicy::kNoConversion)
{
    switch (vConversionPolicy)
    {
    case EConversionPolicy::kConvertToMemory:
        return gtString{}.fromMemorySize(vValue);

    case EConversionPolicy::kConvertToFrequency:
        return gtString{}.appendFormattedString(L"%d MHz", vValue);

    case EConversionPolicy::kNoConversion:
    default:
        return gtString{ std::to_wstring(vValue).data() };
    }
}

/// @brief                       Converts a cl::sycl::string_class to a gtString.
/// @param[in] vValue            The sycl string to be converted.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
/// @note                        This needs to be a template specialisation otherwise
///                              the compiler will match the string_class with the generic
///                              version of the function.
template<>
gtString ToString(cl::sycl::string_class vValue, EConversionPolicy)
{
    gtString retValue;
    retValue << vValue.data();
    return retValue;
}

/// @brief                       Converts a bool value to a gtString. Will return "Yes" or "No".
/// @param[in] vValue            The bool value to be converted.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
/// @note                        This needs to be a template specialisation otherwise
///                              the compiler will match the string_class with the generic
///                              version of the function.
template<>
gtString ToString(bool vValue, EConversionPolicy)
{
    return vValue ? AF_STR_Yes : AF_STR_No;
}

/// @brief                       Converts a cl::sycl::device_type to string.
/// @param[in] vDeviceType       The device_type to convert.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
gtString ToString(cl::sycl::info::device_type vDeviceType, EConversionPolicy)
{
    switch (vDeviceType)
    {
    case cl::sycl::info::device_type::cpu:
        return AP_STR_CL_DEVICE_TYPE_CPU_AS_STR;

    case cl::sycl::info::device_type::gpu:
        return AP_STR_CL_DEVICE_TYPE_GPU_AS_STR;

    case cl::sycl::info::device_type::accelerator:
        return AP_STR_CL_DEVICE_TYPE_ACCELERATOR_AS_STR;

    case cl::sycl::info::device_type::custom:
        return AP_STR_CL_DEVICE_TYPE_CUSTOM_AS_STR;

    case cl::sycl::info::device_type::automatic:
        return AP_STR_SYCL_DEVICE_TYPE_AUTOMATIC;

    case cl::sycl::info::device_type::host:
        return AP_STR_SYCL_DEVICE_TYPE_HOST;

    case cl::sycl::info::device_type::all:
        return AP_STR_SYCL_DEVICE_TYPE_ALL;

    default:
        return AP_STR_CL_DEVICE_TYPE_UNKNOWN_AS_STR;
    };
}

/// @brief                       Converts a cl::sycl::info::fp_config to string.
/// @param[in] vFPConfig         The fp_config to convert.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
gtString ToString(cl::sycl::info::fp_config vFPConfig, EConversionPolicy)
{
    switch (vFPConfig)
    {
    case cl::sycl::info::fp_config::denorm:
        return AP_STR_CL_FP_DENORM_AS_STR;

    case cl::sycl::info::fp_config::inf_nan:
        return AP_STR_CL_FP_INF_NAN_AS_STR;

    case cl::sycl::info::fp_config::round_to_nearest:
        return AP_STR_CL_FP_ROUND_TO_NEAREST_AS_STR;

    case cl::sycl::info::fp_config::round_to_zero:
        return AP_STR_CL_FP_ROUND_TO_ZERO_AS_STR;

    case cl::sycl::info::fp_config::round_to_inf:
        return AP_STR_CL_FP_ROUND_TO_INF_AS_STR;

    case cl::sycl::info::fp_config::fma:
        return AP_STR_CL_FP_FMA_AS_STR;

    case cl::sycl::info::fp_config::correctly_rounded_divide_sqrt:
        return AP_STR_CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT;

    case cl::sycl::info::fp_config::soft_float:
        return AP_STR_CL_FP_SOFT_FLOAT_STR;

    default:
        return AP_STR_CL_FP_CONFIG_UNKNOWN_AS_STR;
    }
}

/// @brief                       Converts a cl::sycl::info::global_mem_cache_type to string.
/// @param[in] vCacheType        The global_mem_cache_type to convert.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
gtString ToString(cl::sycl::info::global_mem_cache_type vCacheType, EConversionPolicy)
{
    switch (vCacheType)
    {
    case cl::sycl::info::global_mem_cache_type::none:
        return AP_STR_CL_MEM_CACHE_NONE_AS_STR;

    case cl::sycl::info::global_mem_cache_type::read_only:
        return AP_STR_CL_MEM_READ_ONLY_CACHE_AS_STR;

    case cl::sycl::info::global_mem_cache_type::write_only:
        return AP_STR_CL_MEM_READ_WRITE_CACHE_AS_STR;

    default:
        return AP_STR_CL_MEM_CACHE_UNKNOWN_AS_STR;
    }
}

/// @brief                       Converts a cl::sycl::info::local_mem_type to string.
/// @param[in] vLocalMemType     The local_mem_type to convert.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
gtString ToString(cl::sycl::info::local_mem_type vLocalMemType, EConversionPolicy)
{
    switch (vLocalMemType)
    {
    case cl::sycl::info::local_mem_type::none:
        return AP_STR_CL_LOCAL_MEM_TYPE_UNKNOWN_AS_STR;

    case cl::sycl::info::local_mem_type::local:
        return AP_STR_CL_MEM_LOCAL_AS_STR;

    case cl::sycl::info::local_mem_type::global:
        return AP_STR_CL_MEM_GLOBAL_AS_STR;

    default:
        return AP_STR_CL_LOCAL_MEM_TYPE_UNKNOWN_AS_STR;
    }
}

/// @brief                       Converts a cl::sycl::info::execution_capability to string.
/// @param[in] vExecCap          The execution_capacility to convert.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
gtString ToString(cl::sycl::info::execution_capability vExecCap, EConversionPolicy)
{
    switch (vExecCap)
    {
    case cl::sycl::info::execution_capability::exec_kernel:
        return AP_STR_CL_EXEC_KERNEL_AS_STR;

    case cl::sycl::info::execution_capability::exec_native_kernel:
        return AP_STR_CL_EXEC_NATIVE_KERNEL_AS_STR;

    default:
        return AP_STR_CL_EXEC_CAPABILITIES_UNKNOWN_AS_STR;
    }
}

/// @brief                       Converts a cl::sycl::info::partition_property to string.
/// @param[in] vPartitionProp    The partition_property to convert.
/// @param[in] EConversionPolicy Unused in this context but needs to conform to the API.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
gtString ToString(cl::sycl::info::partition_property vPartitionProp, EConversionPolicy)
{
    switch (vPartitionProp)
    {
    case cl::sycl::info::partition_property::no_partition:
        return AP_STR_SYCL_PARTITION_PROP_NO_PARTITION;

    case cl::sycl::info::partition_property::partition_equally:
        return AP_STR_SYCL_PARTITION_PROP_PARTITION_EQUALLY;

    case cl::sycl::info::partition_property::partition_by_counts:
        return AP_STR_SYCL_PARTITION_PROP_PARTITION_BY_COUNTS;

    case cl::sycl::info::partition_property::partition_by_affinity_domain:
        return AP_STR_SYCL_PARTITION_PROP_PARTITION_BY_AFFINITY_DOMAINS;

    default:
        return AP_STR_SYCL_PARTITION_PROP_UNKNOWN;
    }
}

/// @brief                              Converts a cl::sycl::info::partition_affinity_domain to string.
/// @param     vPartitionAffinityDomain The partition_affinity_domain to convert.
/// @param[in] EConversionPolicy        Unused in this context but needs to conform to the API.
/// @returns   gtString                 The converted string.
/// @see                                EConversionPolicy.
gtString ToString(cl::sycl::info::partition_affinity_domain vPartitionAffinityDomain, EConversionPolicy)
{
    switch (vPartitionAffinityDomain)
    {
    case cl::sycl::info::partition_affinity_domain::unsupported:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_UNSUPPORTED;

    case cl::sycl::info::partition_affinity_domain::numa:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_NUMA;

    case cl::sycl::info::partition_affinity_domain::L4_cache:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_L4CACHE;

    case cl::sycl::info::partition_affinity_domain::L3_cache:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_L3CACHE;

    case cl::sycl::info::partition_affinity_domain::L2_cache:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_L2CACHE;

    case cl::sycl::info::partition_affinity_domain::L1_cache:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_L1CACHE;

    case cl::sycl::info::partition_affinity_domain::next_partitionable:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_NEXT_PARTITIONABLE;

    case cl::sycl::info::partition_affinity_domain::not_applicable:
    default:
        return AP_STR_SYCL_PARTITION_AFFINITY_DOMAIN_NOT_APPLICABLE;
    }
}

/// @brief                        Converts a cl::sycl::id to string.
/// @param[in] vSyclId            The cl::sycl::id<T> to convert.
/// @param[in] EConversionPolicy  Unused in this context but needs to conform to the API.
/// @returns   gtString           The converted string.
/// @see                          EConversionPolicy.
template<int dimensions>
gtString ToString(cl::sycl::id<dimensions> vSyclId, EConversionPolicy)
{
    gtString retValue;
    for (int idx = 0; idx < dimensions; ++idx)
    {
        const auto syclIdValueAsString = ToString(vSyclId[idx], EConversionPolicy::kNoConversion);

        if (idx < dimensions - 1)
        {
            retValue << syclIdValueAsString << L", ";
        }
        else
        {
            retValue << syclIdValueAsString;
        }
    }

    return retValue;
}

/// @brief                       Convert a vector of values to a single string. If the vector contains
///                              more than 4 elements, each string will be separated by a line break,
///                              otherwise the values will be separated by a comma.
/// @param[in] vVectorOfValues   A vector with values to be converted to string.
/// @param[in] EConversionPolicy The conversion policy to be used for each vector value.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
template<typename T>
gtString ToString(const std::vector<T> &vVectorOfValues, EConversionPolicy vConversionPolicy = EConversionPolicy::kNoConversion)
{
    if (vVectorOfValues.empty())
    {
        return AP_STR_NotAvailable;
    }
    else
    {
        gtString retValue;
        for (size_t idx = 0; idx < vVectorOfValues.size(); ++idx)
        {
            const auto valueAsString = ToString(vVectorOfValues[idx], vConversionPolicy);

            if (idx < vVectorOfValues.size() - 1)
            {
                if (vVectorOfValues.size() <= 4)
                {
                    retValue << valueAsString << L", ";
                }
                else
                {
                    retValue << valueAsString << L"\n";
                }
            }
            else
            {
                retValue << valueAsString;
            }
        }

        return retValue;
    }
}

/// @brief                       Convenience function to convert a value returned by
///                              cl::sycl::device::get_info function into a string. This
///                              function acts a proxy to a call to the previously defined
///                              ToString functions.
/// @param[in] vDevice           The device to query information from.
/// @param[in] EConversionPolicy The conversion policy to be used for each vector value.
/// @returns   gtString          The converted string.
/// @see                         EConversionPolicy.
template<cl::sycl::info::device T>
gtString ToString(cl::sycl::device vDevice, EConversionPolicy vConversionPolicy = EConversionPolicy::kNoConversion)
{
    return ToString(vDevice.get_info<T>(), vConversionPolicy);
}

/// @brief                 Convenience function to convert a value returned by
///                        cl::sycl::platform::get_info function into a string. This
///                        function acts a proxy to a call to the previously defined
///                        ToString functions.
/// @param[in]  vPlatform  The platform to query information from.
/// @return     gtString   The platform information as a gtString.
template<cl::sycl::info::platform T>
gtString ToString(cl::sycl::platform vPlatform)
{
    return ToString(vPlatform.get_info<T>());
}

gtString GetComputeCppVersionString()
{
    return gtString{}.fromASCIIString(__COMPUTECPP__);
}

/// @brief                       Collects information from the SYCL runtime and
///                              returns it as a table of strings ready to be displayed
///                              in CodeXL GUI.
/// @param[out] vSyclDevicesInfo A list, in the form of a table with 
///                              information on the available SYCL devices.
/// @return     bool             true = At least one device was found,
///                              false = no devices were found in the current system.
/// @warning    vSyclDevicesInfo will be cleared out.
bool GetDevicesInfo(SYCLDeviceInfoList &vSyclDevicesInfo)
{
    auto bReturn = false;

    try
    {
        // Acquires the list of devices from the SYCL runtime
        auto devices = cl::sycl::device::get_devices();

        // Some declarations used locally
        using SYCLDeviceInfoPair = std::pair<gtString, gtString>;
        using SYCLDeviceInfoPairList = std::vector<SYCLDeviceInfoPair>;
        using SYCLDeviceInfoInternal = std::vector<SYCLDeviceInfoPairList>;

        SYCLDeviceInfoInternal devicesInfo;

        // Collects the information for each device.
        for (auto device : devices)
        {
            const SYCLDeviceInfoPairList deviceInfo =
            {
                { AF_STR_Platform, ToString<cl::sycl::info::platform::name>(device.get_platform()) },
                { AF_STR_SystemInformationComputeCppVersion, GetComputeCppVersionString() },
                { AF_STR_SystemInformationCommandDeviceName, ToString<cl::sycl::info::device::name>(device) },
                { AF_STR_SystemInformationCommandVendor, ToString<cl::sycl::info::device::vendor>(device) },
                { AF_STR_SystemInformationSYCLDriverVersion, ToString<cl::sycl::info::device::driver_version>(device) },
                { AF_STR_SystemInformationCommandProfile, ToString<cl::sycl::info::device::profile>(device) },
                { AF_STR_SystemInformationCommandVersion, ToString<cl::sycl::info::device::version>(device) },
                { AF_STR_SystemInformationCommandDeviceType, ToString<cl::sycl::info::device::device_type>(device) },
                { AF_STR_SystemInformationCommandVendorID, ToString<cl::sycl::info::device::vendor_id>(device) },
                { AF_STR_SystemInformationCommandMaxComputeUnits, ToString<cl::sycl::info::device::max_compute_units>(device) },
                { AF_STR_SystemInformationCommandMaxWorkItemDimensions, ToString<cl::sycl::info::device::max_work_item_dimensions>(device) },
                { AF_STR_SystemInformationCommandMaxWorkItemSizes, ToString<cl::sycl::info::device::max_work_item_sizes>(device) },
                { AF_STR_SystemInformationCommandMaxWorkgroupSize, ToString<cl::sycl::info::device::max_work_group_size>(device) },
                { AF_STR_SystemInformationCommandPreferredCharVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_char>(device) },
                { AF_STR_SystemInformationCommandPreferredShortVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_short>(device) },
                { AF_STR_SystemInformationCommandPreferredIntVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_int>(device) },
                { AF_STR_SystemInformationCommandPreferredLongVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_long>(device) },
                { AF_STR_SystemInformationCommandPreferredFloatVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_float>(device) },
                { AF_STR_SystemInformationCommandPreferredDoubleVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_double>(device) },
                { AF_STR_SystemInformationCommandPreferredHalfVectorWidth, ToString<cl::sycl::info::device::preferred_vector_width_half>(device) },
                { AF_STR_SystemInformationCommandNativeCharVectorWidth, ToString<cl::sycl::info::device::native_vector_width_char>(device) },
                { AF_STR_SystemInformationCommandNativeShortVectorWidth, ToString<cl::sycl::info::device::native_vector_width_short>(device) },
                { AF_STR_SystemInformationCommandNativeIntVectorWidth, ToString<cl::sycl::info::device::native_vector_width_int>(device) },
                { AF_STR_SystemInformationCommandNativeLongVectorWidth, ToString<cl::sycl::info::device::native_vector_width_long>(device) },
                { AF_STR_SystemInformationCommandNativeFloatVectorWidth, ToString<cl::sycl::info::device::native_vector_width_float>(device) },
                { AF_STR_SystemInformationCommandNativeDoubleVectorWidth, ToString<cl::sycl::info::device::native_vector_width_double>(device) },
                { AF_STR_SystemInformationCommandNativeHalfVectorWidth, ToString<cl::sycl::info::device::native_vector_width_half>(device) },
                { AF_STR_SystemInformationCommandMaxClockFrequency, ToString<cl::sycl::info::device::max_clock_frequency>(device, EConversionPolicy::kConvertToFrequency) },
                { AF_STR_SystemInformationCommandAddressBits, ToString<cl::sycl::info::device::address_bits>(device) },
                { AF_STR_SystemInformationCommandMaxMemoryAllocationSize, ToString<cl::sycl::info::device::max_mem_alloc_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandAreImageSupported, ToString<cl::sycl::info::device::image_support>(device) },
                { AF_STR_SystemInformationCommandMaxReadWriteImageArguments, ToString<cl::sycl::info::device::max_read_image_args>(device) },
                { AF_STR_SystemInformationCommandMaxWriteImageArguments, ToString<cl::sycl::info::device::max_write_image_args>(device) },
                { AF_STR_SystemInformationCommandImage2DMaxHeight, ToString<cl::sycl::info::device::image2d_max_height>(device) },
                { AF_STR_SystemInformationCommandImage2DMaxWidth, ToString<cl::sycl::info::device::image2d_max_width>(device) },
                { AF_STR_SystemInformationCommandImage3DMaxHeight, ToString<cl::sycl::info::device::image3d_max_height>(device) },
                { AF_STR_SystemInformationCommandImage3DMaxWidth, ToString<cl::sycl::info::device::image3d_max_width>(device) },
                { AF_STR_SystemInformationCommandImage3DMaxDepth, ToString<cl::sycl::info::device::image3d_max_depth>(device) },
                { AF_STR_SystemInformationCommandImageMaxBufferSize, ToString<cl::sycl::info::device::image_max_buffer_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandImageMaxArraySize, ToString<cl::sycl::info::device::image_max_array_size>(device) },
                { AF_STR_SystemInformationCommandMaxSamplers, ToString<cl::sycl::info::device::max_samplers>(device) },
                { AF_STR_SystemInformationCommandMaxParameterSize, ToString<cl::sycl::info::device::max_parameter_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandMemoryBaseAddressAlignment, ToString<cl::sycl::info::device::mem_base_addr_align>(device) },
                { AF_STR_SystemInformationCommandHalfFPConfig, ToString<cl::sycl::info::device::half_fp_config>(device) },
                { AF_STR_SystemInformationCommandSingleFPConfig, ToString<cl::sycl::info::device::single_fp_config>(device) },
                { AF_STR_SystemInformationCommandDoubleFPConfig, ToString<cl::sycl::info::device::double_fp_config>(device) },
                { AF_STR_SystemInformationCommandGlobalMemoryCacheType, ToString<cl::sycl::info::device::global_mem_cache_type>(device) },
                { AF_STR_SystemInformationCommandGlobalMemoryCacheLineSize, ToString<cl::sycl::info::device::global_mem_cache_line_size>(device) },
                { AF_STR_SystemInformationCommandGlobalMemoryCacheSize, ToString<cl::sycl::info::device::global_mem_cache_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandGlobalMemorySize, ToString<cl::sycl::info::device::global_mem_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandMaxConstantBufferSize, ToString<cl::sycl::info::device::max_constant_buffer_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandMaxConstantArguments, ToString<cl::sycl::info::device::max_constant_args>(device) },
                { AF_STR_SystemInformationCommandLocalMemoryType, ToString<cl::sycl::info::device::local_mem_type>(device) },
                { AF_STR_SystemInformationCommandLocalMemorySize, ToString<cl::sycl::info::device::local_mem_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandErrorCorrectionSupport, ToString<cl::sycl::info::device::error_correction_support>(device) },
                { AF_STR_SystemInformationCommandHostUnifiedMemory, ToString<cl::sycl::info::device::host_unified_memory>(device) },
                { AF_STR_SystemInformationCommandProfilingTimerResolution, ToString<cl::sycl::info::device::profiling_timer_resolution>(device) },
                { AF_STR_SystemInformationCommandIsLittleEndian, ToString<cl::sycl::info::device::is_endian_little>(device) },
                { AF_STR_SystemInformationCommandIsAvailable, ToString<cl::sycl::info::device::is_available>(device) },
                { AF_STR_SystemInformationCommandIsCompilerAvailable, ToString<cl::sycl::info::device::is_compiler_available>(device) },
                { AF_STR_SystemInformationCommandIsLinkerAvailable, ToString<cl::sycl::info::device::is_linker_available>(device) },
                { AF_STR_SystemInformationCommandExecutionCapabilities, ToString<cl::sycl::info::device::execution_capabilities>(device) },
                { AF_STR_SystemInformationCommandQueueProfiling, ToString<cl::sycl::info::device::queue_profiling>(device) },
                { AF_STR_SystemInformationCommandBuiltInKernels, ToString<cl::sycl::info::device::built_in_kernels>(device) },
                { AF_STR_SystemInformationCommandOpenCLCVersion, ToString<cl::sycl::info::device::opencl_c_version>(device) },
                { AF_STR_SystemInformationCommandExtensions, ToString<cl::sycl::info::device::extensions>(device) },
                { AF_STR_SystemInformationCommandPrintfBufferSize, ToString<cl::sycl::info::device::printf_buffer_size>(device, EConversionPolicy::kConvertToMemory) },
                { AF_STR_SystemInformationCommandPreferredInteropUserSync, ToString<cl::sycl::info::device::preferred_interop_user_sync>(device) },
                { AF_STR_SystemInformationCommandParentDevice, ToString<cl::sycl::info::device::name>(device.get_info<cl::sycl::info::device::parent_device>()) },
                { AF_STR_SystemInformationCommandPartitionMaxSubDevices, ToString<cl::sycl::info::device::partition_max_sub_devices>(device) },
                { AF_STR_SystemInformationCommandPartitionProperties, ToString<cl::sycl::info::device::partition_properties>(device) },
                { AF_STR_SystemInformationCommandPartitionAffinityDomains, ToString<cl::sycl::info::device::partition_affinity_domains>(device) },
                { AF_STR_SystemInformationCommandPartitionTypeProperties, ToString<cl::sycl::info::device::partition_type_property>(device) },
                { AF_STR_SystemInformationCommandPartitionTypeAffinityDomains, ToString<cl::sycl::info::device::partition_type_affinity_domain>(device) },
                { AF_STR_SystemInformationCommandReferenceCount, ToString<cl::sycl::info::device::reference_count>(device) },
                { AF_STR_SystemInformationCommandSPIRSupport, ToString(device.supports_backend(cl::sycl::detail::device_backend::SPIR)) },
                { AF_STR_SystemInformationCommandSPIRVSupport, ToString(device.supports_backend(cl::sycl::detail::device_backend::SPIRV)) },
                { AF_STR_SystemInformationCommandPTXSupport, ToString(device.supports_backend(cl::sycl::detail::device_backend::PTX)) },
            };

            devicesInfo.push_back(deviceInfo);
        }

        // The output table will be in the form
        //
        //  ----------------------------------------------
        // | Parameter   Device1   Device2   ...  DeviceN |
        // | ----------------------------------------------
        // | Name        Name1     Name2     ...  NameN   |
        // | Driver      Driver1   Driver2   ...  DriverN |
        // |  .                                           |
        // |  .                                           |
        // |  .                                           |
        //  ----------------------------------------------

        // This piece of code will turn the vectors of pairs collected before
        // into this output table.
        // CodeXL requires the "Parameter" header.

        if (!devicesInfo.empty())
        {
            // We have at least one device

            // Pre allocate the output vectors
            vSyclDevicesInfo.clear();
            vSyclDevicesInfo.resize(devicesInfo.front().size() + 1);

            auto &header = vSyclDevicesInfo.front();
            header.emplace_back(AF_STR_SystemInformationCommandParameter);

            for (size_t deviceIndex = 0; deviceIndex < devicesInfo.size(); ++deviceIndex)
            {
                const auto &deviceInfo = devicesInfo[deviceIndex];

                gtString wsStream;
                wsStream << AF_STR_Device << L" " << ToString(deviceIndex);

                header.push_back(wsStream);

                for (size_t infoIndex = 0; infoIndex < deviceInfo.size(); ++infoIndex)
                {
                    // For the first device, extract the info name and add to the table.
                    if (deviceIndex == 0)
                    {
                        vSyclDevicesInfo[infoIndex + 1].emplace_back(deviceInfo[infoIndex].first);
                    }

                    // Add the information to the output table.
                    vSyclDevicesInfo[infoIndex + 1].emplace_back(deviceInfo[infoIndex].second);
                }
            }

            bReturn = true;
        }
    }
    catch (const cl::sycl::exception& exp)
    {
        // In case some exception is thrown by the SYCL
        // runtime, catch the exception here, but there
        // is really nothing to do except to print the
        // reason for debugging purposes.
        std::cerr << exp.what() << std::endl;
    }

    return bReturn;
}

} // namespace sycl
} // namespace lpgpu2

/// @brief                       Collects information from the SYCL runtime and
///                              returns it as a table of strings ready to be displayed
///                              in CodeXL GUI.
/// @param[out] vSyclDevicesInfo A list, in the form of a table with 
///                              information on the available SYCL devices.
/// @return     bool             true = At least one device was found,
///                              false = no devices were found in the current system.
/// @warning    vSyclDevicesInfo will be cleared out.
bool LPGPU2_SYCL_GetDevicesInfo(SYCLDeviceInfoList &vSyclDevicesInfo)
{
    return lpgpu2::sycl::GetDevicesInfo(vSyclDevicesInfo);
}
