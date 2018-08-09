// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customization of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// LPGPU2DatabaseAdapter implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAccessor.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

// Infra:
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtString.h>

// STL:
#include <algorithm>

// String constants
#define LPGPU2_STR_SESSION_KEY_W L"LPGPU2 Session"
#define LPGPU2_STR_IS_LPGPU2_SESSION_W L"1"
#define LPGPU2_STR_IS_NOT_LPGPU2_SESSION_W L"0"
static const gtString kLPGPU2PlatformSessionKey{ L"LPGPU2_Platform" };
static const gtString kLPGPU2HardwareSessionKey{ L"LPGPU2_Hardware" };
static const gtString kLPGPU2DCAPISessionKey{ L"LPGPU2_DCAPI" };
static const gtString kLPGPU2RagentSessionKey{ L"LPGPU2_RAgent" };
static const gtString kLPGPU2HardwareIDSessionKey{ L"LPGPU2_HardwareID" };
static const gtString kLPGPU2BlobSizeSessionKey{ L"LPGPU2_BlobSize" };

namespace lpgpu2 {
namespace db {

/// @brief  Default constructor. No work is done here.
LPGPU2DatabaseAdapter::LPGPU2DatabaseAdapter() = default;

/// @brief  Initialises the accessor with LPGPU2DatabaseAccessor in case
///         it is not initialised yet.
/// @return bool    true = The accessor was initialised,
///                 false = An error has occurred during initialisation
/// @warning    This returns bool because of parent class method signature.
bool LPGPU2DatabaseAdapter::Initialise()
{
    auto bReturn = failure;

    if (m_pDbAccessor != nullptr)
    {
        bReturn = true;
    }
    else
    {
        m_pDbAccessor = new (std::nothrow) LPGPU2DatabaseAccessor{};
        GT_IF_WITH_ASSERT_EX(m_pDbAccessor != nullptr, L"Cannot allocate LPGPU2DatabaseAccessor")
        {
            bReturn = true;
        }

    }

    return bReturn;
}

/// @brief   Gets the current error message, if any
/// @returns The error message of any unsuccessful database operation
gtString LPGPU2DatabaseAdapter::GetErrorMessage() const
{
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        return pDbAccessor->GetErrorMessage();
    }

    return gtString{};
}

/// @brief                        Get all ProfileTraceInfo entries.
/// @param[out] vProfileTraceInfo A list of ProfileTraceInfo stored in the database .
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfo(gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfo(vProfileTraceInfo);
    }

    return bReturn;
}

/// @brief                        Gets all ProfileTraceInfo objects. 
///                               with the given call name.
/// @param[in]  vCallName         Name of the function to filter.
/// @param[out] vProfileTraceInfo A list with ProfileTraceInfo objects representing a trace.
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfoByCallName(const gtString &vCallName, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoByCallName(vCallName, vProfileTraceInfo);
    }

    return bReturn;
}

/// @brief                         Gets the ProfileTraceInfo with the maximum value for cpuEnd. 
///                                This will return a ProfileTraceInfo object that matches the search criteria.
/// @param[in]  vFrameNum          The frameNum to look for.
/// @param[out] vProfileTraceInfo  The ProfileTraceInfo with the max cpuEnd time.
/// @returns    status             success = the information was retrieved,
///                                failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfoMaxCpuEndTimeByFrameNumber(gtUInt64 vFrameNum, ProfileTraceInfo &vProfileTraceInfo) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoMaxCpuEndTimeByFrameNumber(vFrameNum, vProfileTraceInfo);
    }

    return bReturn;
}

/// @brief
status LPGPU2DatabaseAdapter::GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(const gtString &vCallCategory, gtUInt64 vThreshold, gtVector<ProfileTraceInfo>& vProfileTraceInfo) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(vCallCategory, vThreshold, vProfileTraceInfo);
    }

    return bReturn;
}

status LPGPU2DatabaseAdapter::GetFrameTimeRange(const gtUInt64 vFrameNum, gtUInt64& vStartTime, gtUInt64& vEndTime) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetFrameTimeRange(vFrameNum, vStartTime, vEndTime);
    }

    return bReturn;
}

    /// @brief                          Gets a list of unique frame numbers from the LPGPU2_traces table.
/// @param[out] vUniqueFrameNumbers A list of unique frame numbers from the LPGPU2_traces table.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.          
status LPGPU2DatabaseAdapter::GetUniqueProfileTraceFrameNumbers(gtVector<gtUInt64> &vUniqueFrameNumbers) const
{
    // Gets all profile trace info from the traces table
    gtVector<ProfileTraceInfo> profileTraceInfo;    
    status bReturn = GetProfileTraceInfo(profileTraceInfo);

    if (bReturn)
    {
        // Get all unique frameNums
        try
        {
            // Sort the frame numbers
            std::sort(profileTraceInfo.begin(), profileTraceInfo.end(), [](const ProfileTraceInfo &lhs, const ProfileTraceInfo &rhs)
            {
                return lhs.m_frameNum < rhs.m_frameNum;
            });

            // Move all duplicated frameNums to the back of the vector
            auto lastUniqueTrace = std::unique(profileTraceInfo.begin(), profileTraceInfo.end(), [](const ProfileTraceInfo &lhs, const ProfileTraceInfo &rhs)
            {
                return lhs.m_frameNum == rhs.m_frameNum;
            });

            // Remove all the objects from the original vector
            profileTraceInfo.erase(lastUniqueTrace, profileTraceInfo.end());

            // Inserts the data into the output vector.
            for (const auto& profileTrace : profileTraceInfo)
            {
                vUniqueFrameNumbers.push_back(profileTrace.m_frameNum);

                // If at least a single element was inserted consider a success
                bReturn = success;
            }
        }
        catch (...)
        {
            bReturn = failure;            
        }
    }

    return bReturn;
}

/// @brief                         Get the first timestamp available from the recorded traces.
/// @param[out] vCpuStartTimestamp The first timestamp available.
/// @returns    bool               true = the information was retrieved,
///                                false = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetFirstTraceCallTimestamp(gtUInt64 &vCpuStartTimestamp) const
{
    bool bReturn = false;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetFirstTraceCallTimestamp(vCpuStartTimestamp);
    }

    return bReturn;
}

/// @brief                        Retrieves a list of ProfileTraceInfo where cpuStart >= vRangeStart
///                               and cpuEnd <= vRangeEnd.
/// @param[in]  vRangeStart       The start of the range to query for profile traces.
/// @param[in]  vRangeEnd         The end of the range to query for profile traces.
/// @param[out] vProfileTraceInfo The list of ProfileTraceInfo resulted from the query.
/// @returns    status            success = the information was retrieved,
///                               failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfoInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoInRange(vRangeStart, vRangeEnd, vProfileTraceInfo);
    }

    return bReturn;
}

/// @brief                   Gets all the Annotation's from the database.
/// @param[out] vAnnotations A list of Annotation objects.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAnnotations(gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetAnnotations(vAnnotations);
    }

    return bReturn;
}

/// @brief                    Gets all the Annotation's from the database that
///                           matches the given ID
/// @param[in]  vAnnotationId Annotation ID to filter the results
/// @param[out] vAnnotation   A list of Annotation objects
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAnnotationById(gtUInt64 vAnnotationId, Annotation &vAnnotation) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetAnnotationById(vAnnotationId, vAnnotation);
    }

    return bReturn;
}

/// @brief      Get the list of Annotation objects in a given range.
/// @param[in]  vRangeStart   The start of the time range.
/// @param[in]  vRangeEnd     The end of the time range.
/// @param[out] vAnnotations  A list of Annotation objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetAnnotationsInRange(vRangeStart, vRangeEnd, vAnnotations);
    }

    return bReturn;
}

/// @brief      Get the list of Annotation objects in a given frame range.
/// @param[in]  vRangeStart   The start of the frame range.
/// @param[in]  vRangeEnd     The end of the frame range (inclusive).
/// @param[out] vAnnotations  A list of Annotation objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetAnnotationsInFrameRange(vFrameStart, vFrameEnd, vAnnotations);
    }

    return bReturn;
}

/// @brief                   Gets all the global annotations from the database.
/// @param[out] vAnnotations A list of Annotation objects.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be
///                          inconsistent.
status LPGPU2DatabaseAdapter::GetGlobalAnnotations(
  gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetGlobalAnnotations(vAnnotations);
    }

    return bReturn;
}

/// @brief                   Gets all the user annotations from the database.
/// @param[out] vAnnotations A list of Annotation objects.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be
///                          inconsistent.
status LPGPU2DatabaseAdapter::GetUserAnnotations(
  gtVector<Annotation> &vAnnotations) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetUserAnnotations(vAnnotations);
    }

    return bReturn;
}

/// @brief                  Gets all the EstimatedPowerSample's from the database
/// @param[out] vPwrSamples A list of EstimatedPowerSample's
/// @returns    status      success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetEstimatedPowerSamples(gtVector<EstimatedPowerSample> &vPwrSamples) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetEstimatedPowerSamples(vPwrSamples);
    }

    return bReturn;
}

/// @brief                             Gets a map of power components per device.
/// @param[out] vPwrComponentsByDevice A map with component per device. The key is the device id.
///                                    for each device there is a list of power components
/// @returns    status                 success = the information was retrieved,
///                                    failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetEstimatedPowerComponents(gtMap<gtUInt64, gtVector<EstimatedPowerComponent>> &vPwrComponentsByDevice) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetEstimatedPowerComponents(vPwrComponentsByDevice);
    }

    return bReturn;
}

/// @brief                     Gets the list of ShaderTrace objects.
/// @param[out] vShaderTraces  A list of ShaderTrace objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetShaderTraces(gtVector<ShaderTrace> &vShaderTraces) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetShaderTraces(vShaderTraces);
    }

    return bReturn;
}

/// @brief    Get the list of ShaderTrace objects in a given range.
/// @param[in]   vRangeStart   The start of the time range.
/// @param[in]   vRangeEnd     The end of the time range.
/// @param[out] vShaderTraces  A list of ShaderTrace objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ShaderTrace>& vShaderTraces) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetShaderTracesInRange(vRangeStart, vRangeEnd, vShaderTraces);
    }

    return bReturn;
}

/// @brief    Get the list of ShaderTrace objects in a given range of frames.
/// @param[in]   vRangeStart   The start of the frame range.
/// @param[in]   vRangeEnd     The end of the frame range (inclusive).
/// @param[out] vShaderTraces  A list of ShaderTrace objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<ShaderTrace>& vShaderTraces) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetShaderTracesInFrameRange(vFrameStart, vFrameEnd, vShaderTraces);
    }

    return bReturn;
}

/// @brief                     Gets a list of ShaderTrace objects with the specified API ID.
/// @param[in]  vApiId         The API id to look for shader traces.
/// @param[out] vShaderTraces  A list of ShaderTrace objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetShaderTracesByAPIID(EShaderTraceAPIId vApiId, gtVector<ShaderTrace> &vShaderTraces) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();
    
    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetShaderTracesByAPIId(vApiId,  vShaderTraces);
    }

    return bReturn;
}

/// @brief                     Gets a list of ShaderTrace objects with the specified API Name.
/// @param[in]  vApiName       The name of the API to query for shader traces.
/// @param[out] vShaderTraces  A list of ShaderTrace objects.
/// @returns    status         success = the information was retrieved,
///                            failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetShaderTracesByAPIName(const gtString &vApiName, gtVector<ShaderTrace> &vShaderTraces) const
{
    static const std::map<gtString, EShaderTraceAPIId> apiNameToIdMap = 
    {
        { L"OpenGLES", EShaderTraceAPIId::kGLES2 },
        { L"OpenGL" , EShaderTraceAPIId::kGL },
        { L"Vulkan", EShaderTraceAPIId::kVULKAN },
        { L"OpenCL", EShaderTraceAPIId::kCL }
    };

    auto status = failure;

    auto it = apiNameToIdMap.find(vApiName);
    if (it != apiNameToIdMap.end())
    {
        status = GetShaderTracesByAPIID(it->second, vShaderTraces);
    }

    return status;
}

/// @brief                   Get the shader stats from the specified shaderId.
/// @param[in]  vFrameNum    The frameNum that identifies the shader.
/// @param[in]  vDrawNum     The drawNum that identifies the shader.
/// @param[out] vShaderStats The stats for the specified shaderId.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetShaderAsmStatsByShaderId(gtUInt64 vShaderId, gtVector<ShaderAsmStat> &vShaderStats) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetShaderAsmStatsByShaderId(vShaderId, vShaderStats);
    }

    return bReturn;    
}

status LPGPU2DatabaseAdapter::GetShaderToAsmByShaderId(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetShaderToAsmByShaderId(vShaderId, vShaderToAsm);
    }

    return bReturn;
}

/// @brief                  Gets the list of GPUTimer objects from the database.
/// @param[out] vGpuTimers  A list of GPUTimer objects.
/// @returns    status      success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetGPUTimers(gtVector<GPUTimer> &vGpuTimers) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetGPUTimers(vGpuTimers);
    }

    return bReturn;
}

/// @brief                  Gets the list of GPUTimer objects from the database where the frameNum
///                         is between the informed range.
/// @param[in]  vRangeStart The start of the frameNum range to query.
/// @param[in]  vRangeEnd   The end of the frameNum range to query.
/// @param[out] vGpuTimers  A list of GPUTimer objects.
/// @returns    status      success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetGPUTimersInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<GPUTimer> &vGpuTimers) const
{
    status bReturn = false;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetGPUTimersInRange(vRangeStart, vRangeEnd, vGpuTimers);
    }

    return bReturn;
}

/// @brief                      Gets the list of CounterWeight objects from the database.
/// @param[out] vCounterWeights A list of CounterWeight objects.
/// @returns    status          success = the information was retrieved,
///                             failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetCounterWeights(gtVector<CounterWeight> &vCounterWeights) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetCounterWeights(vCounterWeights);
    }

    return bReturn;
}

/// @brief                  Get the CounterWeight of a counter looking by name.
/// @param   vCounterName   The name of the counter to look for.
/// @param   vCounterWeight The CounterWeight object for the corresponding counter name.
/// @returns status         success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetCounterWeightForCounterByName(const gtString &vCounterName, CounterWeight &vCounterWeight) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetCounterWeightForCounterByName(vCounterName, vCounterWeight);
    }

    return bReturn;
}

/// @brief                    Return the weighted sum for a counter with samples in the specified range
///                           using the information from the table LPGPU2_counterWeights.
/// @param[in]  vCounterName  The name of the counter.
/// @param[in]  vRangeStart   The start of range to look for samples.
/// @param[in]  vRangeEnd     The end of range to look for samples.
/// @param[out] vWeightedSum  The weighted sum the samples.
/// @returns   status         success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetWeightedSumForCounterByNameInRange(const gtString &vCounterName, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtFloat32 &vWeightedSum) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetWeightedSumForCounterByNameInRange(vCounterName, vRangeStart, vRangeEnd, vWeightedSum);
    }

    return bReturn;
}

/// @brief                  Get a list of all the samples in the samples table.
/// @param   vSamples       The retrieved collection of samples.
/// @returns status         success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAllSamples(gtVector<PPSampleData> &vSamples) const
{
  auto bReturn = failure;
  auto pDbAccessor = GetDatabaseAccessor();

  GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
  {
    bReturn = pDbAccessor->GetAllSamples(vSamples);
  }

  return bReturn;
}

/// @brief                  Get a list of PPSampleData for the counter where the sample values are
///                         weighted by the corresponding counter weight.
/// @param   vCounterName   The name of the counter to look for samples.
/// @param   vSampleValues  The list of Power Profiling Sample Data for the counter name.
/// @returns status         success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetWeightedSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetWeightedSamplesForCounterByName(vCounterName, vSampleValues);
    }

    return bReturn;
}

/// @brief                   Gets the list of StackTraceInfo objects from the database.
/// @param[out] vStackTraces A list of StackTraceInfo objects.
/// @returns    status       success = the information was retrieved,
///                          failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetStackTraces(gtVector<StackTraceInfo> &vStackTraces) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetStackTraces(vStackTraces);
    }

    return bReturn;
}

/// @brief                       Gets the list of StackTraceParameter objects from the database.
/// @param[out] vTraceParameters A list of StackTraceInfo objects.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetStackTraceParameters(gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetStackTraceParameters(vTraceParameters);
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the vApiName and vCallCategory.
/// @param[in]  vCallCategory    The call category to look for in LPGPU2_callCategory.
/// @param[in]  vApiName         The apiName to look for in LPGUP2_callCategory.
/// @param[out] vTraceParameters The result StackTraceParameter vector.
/// @returns   status            success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetStackTraceParametersByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetStackTraceParametersByCallCategoryAndAPIName(vCallCategory, vApiName, vTraceParameters);
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the vCallCategory.
/// @param[in]  vCallCategory    The call category to look for in LPGPU2_callCategory.
/// @param[out] vTraceParameters The result StackTraceParameter vector.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetStackTraceParametersByCallCategory(const gtString & vCallCategory, gtVector<StackTraceParameter>& vTraceParameters) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetStackTraceParametersByCallCategory(vCallCategory, vTraceParameters);
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the vCallCategory.
/// @param[in]  vApiName         The apiName to look for in LPGUP2_callCategory.
/// @param[out] vTraceParameters The result StackTraceParameter vector.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetStackTraceParametersByAPIName(const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetStackTraceParametersByAPIName(vApiName, vTraceParameters);
    }

    return bReturn;
}

/// @brief                  Gets the number of unique threadIds from the stack trace
///                         parameters record.
/// @param[out] vNumThreads The number of unique threadIds.
/// @returns    status      success = the information was retrieved,
///                         failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetNumThreadsFromTraceParameters(gtUInt64 &vNumThreads) const
{
    status bReturn = false;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetNumThreadsFromTraceParameters(vNumThreads);
    }

    return bReturn;
}

status LPGPU2DatabaseAdapter::GetStackTraceParametersByFrameAndDrawNumber(gtUInt64 vFrameNum, gtUInt64 vDrawNum, gtVector<StackTraceParameter> &vTraceParameters) const
{
    status bReturn = false;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetStackTraceParametersByFrameAndDrawNumber(vFrameNum, vDrawNum, vTraceParameters);
    }

    return bReturn;
}

/// @brief                         Gets the list of RegionOfInterest objects from the database.
/// @param[out] vRegionsOfInterest A list of RegionOfInterest objects.
/// @returns    status             success = the information was retrieved,
///                                failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetRegionsOfInterest(gtVector<RegionOfInterest> &vRegionsOfInterest) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetRegionsOfInterest(vRegionsOfInterest);
    }

    return bReturn;
}

/// @brief                      Get a RegionOfInterest with the given id.
/// @param   vRegionId          The region id to look for.
/// @param   vRegionOfInterest  The RegionOfInterest with the requested id.
/// @returns status             success = the information was retrieved,
///                             failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetRegionOfInterestByRegionId(gtUInt64 vRegionId, RegionOfInterest &vRegionOfInterest) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetRegionOfInterestByRegionId(vRegionId, vRegionOfInterest);
    }

    return bReturn;
}

/// @brief                    Get the accumulated time from all the calls from the Calls
///                           summary data.
/// @param   vAccumulatedTime The accumulated time from the calls summary.
/// @returns status           success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetAccumulatedCallSummaryTime(vAccumulatedTime);
    }

    return bReturn;
}

/// @brief                       Gets a list of CallSummary objects that match vRegionId.
/// @param[in]  vRegionId        RegionId to search for call summaries.
/// @param[out] vCallSummaryList A list of call summaries that match the region id specified.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetCallSummaryForRegionById(gtUInt64 vRegionId, gtVector<CallSummary> &vCallSummaryList) const
{
    status bReturn = false;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetCallSummaryForRegionById(vRegionId, vCallSummaryList);
    }

    return bReturn;
}

/// @brief                    Gets a list of CallsPerType objects that match vRegionId.
/// @param[in]  vRegionId     RegionId to search for CallsPerType entries.     
/// @param[out] vCallsPerType A CallsPerType that match the region id specified.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetCallsPerTypeForRegionById(const gtUInt64 vRegionId, CallsPerType &vCallsPerType) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetCallsPerTypeForRegionById(vRegionId, vCallsPerType);
    }

    return bReturn;
}

/// @brief                    Get a list of unique quantized times from the recorded samples.
/// @param[out] vUniqueTimes  The list of unique times from the samples recorded.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetUniqueQuantizedTimesFromSamples(gtVector<gtUInt64> &vUniqueTimes) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetUniqueQuantizedTimesFromSamples(vUniqueTimes);
    }

    return bReturn;
}

/// @brief                    Get a list of unique quantized times from the recorded samples
///                           stored in the samples table but filter the results by the informed range.
/// @param[in]  vRangeStart   The start of the range to look for quantized times.
/// @param[in]  vRangeEnd     The end of the range to look for quantized times.
/// @param[out] vUniqueTimes  The list of unique times from the samples recorded.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetUniqueQuantizedTimesFromSamplesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetUniqueQuantizedTimesFromSamplesInRange(vRangeStart, vRangeEnd, vUniqueTimes);
    }

    return bReturn;
}

/// @brief                       Get a list of unique quantized times from the recorded samples
///                              that match the counter category and are inside the requested range.
/// @param[in]  vCounterCategory The counter category to look for samples.
/// @param[in]  vRangeStart      The start of the range to look for quantized times.
/// @param[in]  vRangeEnd        The end of the range to look for quantized times.
/// @param[out] vUniqueTimes     The list of unique times from the samples recorded.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetUniqueQuantizedTimesFromSamplesByCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetUniqueQuantizedTimesFromSamplesByCategoryInRange(vCounterCategory, vRangeStart, vRangeEnd, vUniqueTimes);
    }

    return bReturn;
}

/// @brief                      Return a list of CallCategory entries that match vCategory.
/// @param[in]  vCategory       The category to search entries for.
/// @param[out] vCallCategories The list of CallCategory objects that match vCategory.
/// @returns    status          success = the information was retrieved,
///                             failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetCallNamesByCategory(const gtString &vCategory, gtVector<CallCategory> &vCallCategories) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetCallNamesByCategory(vCategory, vCallCategories);
    }

    return bReturn;
}

/// @brief                            Returns a list of ProfileTraceInfo with callNames that belongs to vCategory.
/// @param[in]  vCallCategory         The category used to look for trace information.
/// @param[out] vProfileTraceInfoList The list of ProfileTraceInfo where callName is from category vCallCategory.
/// @returns    status                success = the information was retrieved,
///                                   failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfoByCallCategory(const gtString &vCallCategory, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoByCallCategory(vCallCategory, vProfileTraceInfoList);
    }

    return bReturn;
}

/// @brief                          Returns a list of ProfileTraceInfo with callNames that belongs to vApiName.
/// @param   vApiName               The apiName to look for trace information.
/// @param   vProfileTraceInfoList  The list of ProfileTraceInfo where callNAme is from API vApiName.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfoByAPIName(const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoByAPIName(vApiName, vProfileTraceInfoList);
    }

    return bReturn;
}

/// @brief                       Get a list of StackTraceParameters for calls that match the frameNum and drawNum
/// @param[in]  vFrameNum        The frameNum to look for in LPGPU2_traceParameters.
/// @param[in]  vDrawNum         The drawNum to look for in LPGPU2_traceParameters.
/// @param[out] vTraceParameters The result StackTraceParameter vector that match the search criteria.
/// @returns    status           success = the information was retrieved,
///                              failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetProfileTraceInfoByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetProfileTraceInfoByCallCategoryAndAPIName(vCallCategory, vApiName, vProfileTraceInfoList);
    }

    return bReturn;
}

/// @brief                    Get the samples for a counter with name.
/// @param[in]  vCounterName  The counter name to look for samples.
/// @param[out] vSampleValues The samples for the counter.
/// @returns    status        success = the information was retrieved,
///                           failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetSamplesForCounterByName(vCounterName, vSampleValues);
    }

    return bReturn;
}

/// @brief                          Gets the average sample values for counters that belong to the
///                                 category vCounterCategory and the quantized times lies between
///                                 vRangeStart and vRangeEnd.
/// @param[in]   vCounterCategory   The counter category used to average sample values.
/// @param[in]   vRangeStart        The start of the quantized time range.
/// @param[in]   vRangeEnd          The end of quantized time range.
/// @param[out]  vSampleValues      The list of sample values but the m_sampleValue field will be the
///                                 average of values for the corresponding counter id.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAverageSamplesByCounterCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetAverageSamplesByCounterCategoryInRange(vCounterCategory, vRangeStart, vRangeEnd, vSampleValues);
    }

    return bReturn;
}

/// @brief                          Gets the maximum sample values for counters that belong to the
///                                 category vCounterCategory.
/// @param[in]   vCounterCategory   The counter category used to get the maximum of sample values.
/// @param[out]  vSampleValues      The list of sample values but the m_sampleValue field will be the
///                                 maximum of values for the corresponding counter id grouped by the
///                                 quantizedTimeMs field.
/// @returns    status              success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetMaxSampleValuesByCounterCategory(const gtString &vCounterCategory, gtVector<PPSampleData> &vSampleValues) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->GetMaxSampleValuesByCounterCategory(vCounterCategory, vSampleValues);
    }

    return bReturn;
}

/// @brief                          Gets the nearest sample for a counter that occurs before a specified time.
/// @param[in]   vTime              The time that the sample must occur before.
/// @param[in]   vCounter           The ID of the counter the sample must belong to.
/// @param[out]  vrNearestSample    The retrieved sample.
/// @returns     status             success = the information was retrieved,
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetNearestSampleValueBeforeTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue &vrNearestSample)
{
  status bReturn = failure;
  auto pDbAccessor = GetDatabaseAccessor();

  GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
  {
      bReturn = pDbAccessor->GetNearestSampleValueBeforeTime(vTime, vCounter, vrNearestSample);
  }

  return bReturn;
}

/// @brief                          Gets the nearest sample for a counter that occurs after a specified time.
/// @param[in]   vTime              The time that the sample must occur after.
/// @param[in]   vCounter           The ID of the counter the sample must belong to.
/// @param[out]  vrNearestSample    The retrieved sample.
/// @returns     status             success = the information was retrieved
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetNearestSampleValueAfterTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue &vrNearestSample)
{
  status bReturn = failure;
  auto pDbAccessor = GetDatabaseAccessor();

  GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
  {
      bReturn = pDbAccessor->GetNearestSampleValueAfterTime(vTime, vCounter, vrNearestSample);
  }

  return bReturn;
}

/// @brief                          Gets all the devices in the DB
/// @param[out]  devices            The vector of devices
/// @returns     status             success = the information was retrieved
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetAllDevicesAsVector(
    gtVector<Device> &devices) const
{
  status bReturn = failure;
  auto pDbAccessor = GetDatabaseAccessor();

  GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
  {
      bReturn = pDbAccessor->GetAllDevicesAsVector(devices);
  }

  return bReturn;
}

/// @brief                          Estimates the sampling frequency of a counter.
/// @param[in]   vCounterId         The ID of counter to estimate
/// @param[out]  vFreq              The estimated sampling frequency.
/// @returns     status             success = the information was retrieved
///                                 failure = an error has occurred and result may be inconsistent.
status LPGPU2DatabaseAdapter::GetEstimatedCounterFrequency(
    gtInt32 vCounterId, double& vFreq) const
{
  status bReturn = failure;
  auto pDbAccessor = GetDatabaseAccessor();

  GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
  {
      bReturn = pDbAccessor->GetEstimatedCounterFrequency(vCounterId, vFreq);
  }

  return bReturn;
}

/// @brief                       Write a Counter Sample Blob into the database.
/// @param[in] blobs             Binary blobs to write into the database.
/// @returns   bool              true = the information was inserted into the database,
///                              false = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertLPGPU2CounterSampleBlobs(
    const CounterSampleBlobsVec &blobs)
{
  auto bReturn = failure;
  auto pDbAccessor = GetDatabaseAccessor();

  GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
  {
      bReturn = pDbAccessor->InsertLPGPU2CounterSampleBlobs(blobs);
  }

  return bReturn;
}

/// @brief                       Write a ProfileTraceInfo into the database.
/// @param[in] vProfileTraceInfo ProfileTraceInfo to write into the database.
/// @returns   bool              true = the information was inserted into the database,
///                              false = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertProfileTraceInfo(const ProfileTraceInfo &vProfileTraceInfo)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertProfileTraceInfo(vProfileTraceInfo);
    }

    return bReturn;
}

/// @brief                         Write an EstimatedPowerSample into the database.
/// @param[in] vEstimatedPwrSample ProfileTraceInfo to write into the database.
/// @returns   status              success = the information was inserted into the database,
///                                failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertEstimatedPowerSample(const EstimatedPowerSample &vEstimatedPwrSample)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertEstimatedPowerSample(vEstimatedPwrSample);
    }

    return bReturn;
}

/// @brief                             Write an association between a component and device.
/// @param[in] vEstimatedPwrComponent  Power component to be written to the database
/// @returns   status                  success = the information was inserted into the database,
///                                    failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertEstimatedPowerComponent(const EstimatedPowerComponent &vEstimatedPwrComponent)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertEstimatedPowerComponent(vEstimatedPwrComponent);
    }

    return bReturn;
}

/// @brief                     Write a StackTraceInfo into the database
/// @param[in] vStackTraceInfo StackTraceInfo to write into the database
/// @returns   status          success = the information was inserted into the database,
///                            failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertStackTrace(const StackTraceInfo &vStackTraceInfo)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertStackTrace(vStackTraceInfo);
    }

    return bReturn;
}

/// @brief                       Write a StackTraceParameter into the database
/// @param[in]  vStackTraceParam StackTraceParameter to write into the database
/// @returns   status            success = the information was inserted into the database,
///                              failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertStackTraceParameter(const StackTraceParameter &vStackTraceParam)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertStackTraceParameter(vStackTraceParam);
    }

    return bReturn;
}

/// @brief                 Write an Annotation into the database
/// @param[in] vAnnotation Annotation to write into the database
/// @returns   status      success = the information was inserted into the database,
///                        failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertAnnotation(const Annotation &vAnnotation)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertAnnotation(vAnnotation);
    }

    return bReturn;
}

/// @brief                  Write a ShaderTrace into the database.
/// @param[in] vShaderTrace Annotation to write into the database.
/// @returns   status       success = the information was inserted into the database,
///                         failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertShaderTrace(const ShaderTrace &vShaderTrace)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertShaderTrace(vShaderTrace);
    }

    return bReturn;
}

/// @brief                  Write a ShaderAsmStat into the database.
/// @param[in] vShaderStat  ShaderAsmStat to write into the database.
/// @returns   status       success = the information was inserted into the database,
///                         failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertShaderAsmStat(const ShaderAsmStat &vShaderStat)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertShaderAsmStat(vShaderStat);
    }

    return bReturn;
}

/// @brief                   Write a shader-to-asm mapping into the database.
/// @param[in] vShaderToAsm  The shader-to-asm mapping to insert into the database.
/// @returns   status        success = the information was inserted into the database,
///                          failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertShaderToAsm(const ShaderToAsm &vShaderToAsm)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertShaderToAsm(vShaderToAsm);
    }

    return bReturn;
}

/// @brief                Write a GPUTimer into the database
/// @param[in] vGpuTimer  Annotation to write into the database
/// @returns   status     success = the information was inserted into the database,
///                       failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertGPUTimer(const GPUTimer &vGpuTimer)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertGPUTimer(vGpuTimer);
    }

    return bReturn;
}

/// @brief                    Write a CounterWeight into the database
/// @param[in] vCounterWeight CounterWeight to write into the database
/// @returns   status         success = the information was inserted into the database,
///                           failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertCounterWeight(const CounterWeight &vCounterWeight)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertCounterWeight(vCounterWeight);
    }

    return bReturn;
}

/// @brief                       Write a RegionOfInterest into the database    
/// @param[in] vRegionOfInterest RegionOfInterest to write into the database
/// @returns   status            success = the information was inserted into the database,
///                              failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertRegionOfInterest(const RegionOfInterest &vRegionOfInterest)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertRegionOfInterest(vRegionOfInterest);
    }

    return bReturn;
}

/// @brief                    Write a CallCategory entry into the database.
/// @param[in]  vCallCategory The CallCategory object.
/// @returns   status         success = the information was inserted into the database,
///                           failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertCallCategory(const CallCategory &vCallCategory)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertCallCategory(vCallCategory);
    }

    return bReturn;
}

/// @brief                   Write a CallSummary into the database. Please note that
///                          the RegionOfInterest for this region must exist.
/// @param[out] vCallSummary callSummary to write into the database
/// @returns    status       success = the information was inserted into the database,
///                          failure = an error has occurred during the insertion operation.
/// @warning                 The RegionOfInterest for this region must exist
///                          before inserting the call summary.
status LPGPU2DatabaseAdapter::InsertCallSummary(const CallSummary &vCallSummary)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertCallSummary(vCallSummary);
    }

    return bReturn;
}

/// @brief                   Write a CallsPerType into the database. Please not that
///                          the RegionOfInterest for this calls per type must exist.
/// @param[in] vCallsPerType CallsPerType object to write into the database
/// @returns   status        success = the information was inserted into the database,
///                          failure = an error has occurred during the insertion operation.
/// @warning                 The RegionOfInterest for this region must exist
///                          before inserting the call summary.
status LPGPU2DatabaseAdapter::InsertCallsPerType(const CallsPerType &vCallsPerType)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->InsertCallsPerType(vCallsPerType);
    }

    return bReturn;
}

/// @brief           Starts a bulk insertion. This will make multiple insertions much faster.
/// @return   status success = The transaction was initiated,
///                  failure = An error has occurred and the transaction will not take effect.
/// @warning         LPGPU2DatabaseAdapter::EndTransaction() must be called for the changes
///                  to be written into the database.
/// @see             LPGPU2DatabaseAdapter::EndTransaction().
status LPGPU2DatabaseAdapter::BeginTransaction() const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->BeginTransaction();
    }

    return bReturn;
}

/// @brief           Commits a bulk insertion. This will make multiple insertions much faster.
/// @return   status success = The transaction was finalized and the data is not in the database,
///                  failure = An error has occurred and the transaction will not take effect,
///                  this will effectively rollback all the changes.
/// @warning         LPGPU2DatabaseAdapter::BeginTransaction() must be called before ending a
///                  transaction.
/// @see             LPGPU2DatabaseAdapter::BeginTransaction().
status LPGPU2DatabaseAdapter::EndTransaction() const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->EndTransaction();
        
        if (bReturn == failure)
        {            
            // Try to rollback the database to a previous valid state.
            GT_ASSERT_EX(pDbAccessor->RollbackTransaction(), GetErrorMessage().asCharArray());
        }
    }    

    return bReturn;
}

/// @brief                      Inserts a counter category for the feedback engine.
/// @param[in] vCounterCategory The counter category info to insert.
/// @returns   status           success = the information was inserted into the database,
///                             failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::InsertCounterCategory(const InsertCounterCategoryInfo &vCounterCategory)
{
    auto bReturn = failure;
    auto pDbAcessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAcessor != nullptr)
    {
        int counterId;
        if(GetCounterIdByName(vCounterCategory.m_counterName, counterId))
        {
            const CounterCategory counterCategory{ static_cast<gtUInt64>(counterId), vCounterCategory.m_category };
            bReturn = pDbAcessor->InsertCounterCategory(counterCategory);
        }
    }

    return bReturn;
}

/// @brief          Deletes the Regions of Interest from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred the operation.
status LPGPU2DatabaseAdapter::DeleteRegionsOfInterest()
{
    auto bReturn = failure;
    auto pDbAcessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        bReturn = pDbAcessor->DeleteRegionsOfInterest();
    }

    return bReturn;
}

/// @brief          Deletes Annotations from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred during the operation.
status LPGPU2DatabaseAdapter::DeleteAnnotations()
{
    auto bReturn = failure;
    auto pDbAcessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        bReturn = pDbAcessor->DeleteAnnotations();
    }

    return bReturn;
}

/// @brief          Deletes Call Categories from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred the operation.
status LPGPU2DatabaseAdapter::DeleteCallCategories()
{
    auto bReturn = failure;
    auto pDbAcessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        bReturn = pDbAcessor->DeleteCallCategories();
    }

    return  bReturn;
}

/// @brief          Deletes Counter Categories from the database.
/// @returns status success = the information was deleted from the database,
///                 failure = an error has occurred the operation.
status LPGPU2DatabaseAdapter::DeleteCounterCategories()
{
    auto bReturn = failure;
    auto pDbAcessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        bReturn = pDbAcessor->DeleteCounterCategories();
    }

    return  bReturn;
}

/// @brief               Deletes Shader stats for the given shaderId from the database.
/// @param[in] vShaderId The shaderId that identifies the shader.
/// @param[in] vApiId    The vApiId that identifies the shader.
/// @returns   status    success = the information was deleted from the database,
///                      failure = an error has occurred during the operation.
status LPGPU2DatabaseAdapter::DeleteShaderAsmStatsForShaderId(gtUInt64 vShaderId, EShaderTraceAPIId vApiId)
{
    auto bReturn = failure;
    auto pDbAcessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(m_pDbAccessor != nullptr)
    {
        bReturn = pDbAcessor->DeleteShaderAsmStatsForShaderId(vShaderId, vApiId);
    }

    return bReturn;
}

/// @brief          Convenience function that deletes feedback data from the database.
/// @returns status success = all feedbacks where delete from the database,
///                 failure = an error has occurred during the operation.
status LPGPU2DatabaseAdapter::DeleteFeedbacks()
{
    auto bReturn = success;

    bReturn &= DeleteRegionsOfInterest();
    bReturn &= DeleteAnnotations();

    return bReturn;
}

/// @brief                    Update the LPGPU2_shader table with the information given in vShaderTrace or
///                           inserts a new record if the ShaderTrace entry does not exists in the database by
///                           calling the function LPGPU2DatabaseAdapter::InsertShaderTrace internally.
/// @param[in] vType          If the ShaderTrace record exists, this enum will tell witch information to update.
/// @param[in] vShaderTrace   The ShaderTrace object to update or insert if it does not exists.
/// @returns   status         success = The ShaderTrace was updated with the new information,
///                           failure = An error has occurred during the update process.
/// @see                      EShaderMetadaType
/// @see                      LPGPU2DatabaseAdatper::InsertShaderTrace
status lpgpu2::db::LPGPU2DatabaseAdapter::UpdateShaderTrace(EShaderMetadataType vType, const ShaderTrace &vShaderTrace)
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        // If there is an active transaction, we need to close it here because
        // this methods relies on the GetShaderTraces method. Since transactions
        // are by connection, the reads and writes must happen in the same transaction,
        // this means that this method need to be outside of a transaction to work property.
        const auto bIsTransactionActive = pDbAccessor->IsTransactionActive();
        if (bIsTransactionActive)
        {
            // End the current transaction if we are inside one
            pDbAccessor->EndTransaction();
        }

        // First, check if the trace exists
        gtVector<ShaderTrace> shaderTraces;
        if (GetShaderTraces(shaderTraces))
        {
            // Look for a shader trace with the same ShaderId and API Id.
            const auto shaderTraceIt = std::find_if(shaderTraces.begin(), shaderTraces.end(), [&](const ShaderTrace &st)
            {
                return st.m_shaderId == vShaderTrace.m_shaderId
                    && st.m_apiId == vShaderTrace.m_apiId;
            });

            if (shaderTraceIt != shaderTraces.end())
            {
                // If we already have the shader in the database, update it based on the metadata type.
                switch (vType)
                {
                    case lpgpu2::db::EShaderMetadataType::kSource:
                        bReturn = pDbAccessor->UpdateShaderTraceSource(vShaderTrace);
                        break;
                    case lpgpu2::db::EShaderMetadataType::kAsm:
                        bReturn = pDbAccessor->UpdateShaderTraceAsm(vShaderTrace);
                        break;
                    case lpgpu2::db::EShaderMetadataType::kBinary:
                        bReturn = pDbAccessor->UpdateShaderTraceBinary(vShaderTrace);
                        break;
                    case lpgpu2::db::EShaderMetadataType::kDebug:
                        bReturn = pDbAccessor->UpdateShaderTraceDebug(vShaderTrace);
                        break;
                    case lpgpu2::db::EShaderMetadataType::kSourceFromMainShaderDataPacket:
                        bReturn = pDbAccessor->UpdateShaderTraceFromMainShaderPacket(vShaderTrace);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                // Insert the new shader
                bReturn = InsertShaderTrace(vShaderTrace);
            }
        }
        else
        {
            // Insert the new shader, as it is
            bReturn = InsertShaderTrace(vShaderTrace);
        }

        if (bIsTransactionActive)
        {
            // Reopen the transaction to keep the external state the same
            pDbAccessor->BeginTransaction();
        }

    }

    return bReturn;
}

/// @brief  Updates the ShaderStats information with the given vector of stats.
/// @param[in]  vShaderStats    A list of ShaderAsmStat objects to update the information.
/// @return     status          success = all shader stats were updated,
///                             failure = an error has occurred in the update operation.
/// @warning                    This is a two step operation, first it will collect all
///                             unique shaderIds specified in the list and will delete all
///                             stats for all the specified shaders. This allows updating
///                             multiple shader stats at once.
status LPGPU2DatabaseAdapter::UpdateShaderAsmStats(const gtVector<ShaderAsmStat> &vShaderStats)
{
    auto bReturn = failure;

    // Collect all the distinct shader Ids    
    gtSet<std::pair<gtUInt64, lpgpu2::db::EShaderTraceAPIId>> uniqueShaderIds;
    for (const auto &shaderStat : vShaderStats)
    {
        uniqueShaderIds.insert({ shaderStat.m_shaderId, shaderStat.m_apiId });
    }

    bReturn = BeginTransaction();

    if (bReturn)
    {
        // Delete the previous status for all unique shaderIds
        for (const auto &shaderIdPair : uniqueShaderIds)
        {
            const auto shaderId = shaderIdPair.first;
            const auto apiId = shaderIdPair.second;

            bReturn &= DeleteShaderAsmStatsForShaderId(shaderId, apiId);
        }

        // Insert the new stats
        for (const auto &shaderStat : vShaderStats)
        {
            bReturn &= InsertShaderAsmStat(shaderStat);
        }

        bReturn &= EndTransaction();
    }        

    return bReturn;
}

/// @brief                              Inserts the entry "LPGPU2 Session" in the sessionInfo table.
/// @param[in] bIsTargetingLPGPU2Device true: the field "LPGPU2 Session" will be set to 1.
///                                     false: the field "LPGPU2 Session" will be set to 0.
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2TargetDevice(const bool bIsTargetingLPGPU2Device)
{    
    return InsertSessionInfoKeyValue(LPGPU2_STR_SESSION_KEY_W, bIsTargetingLPGPU2Device ? LPGPU2_STR_IS_LPGPU2_SESSION_W : LPGPU2_STR_IS_NOT_LPGPU2_SESSION_W);
}

/// @brief                              Inserts the entry in the sessionInfo table.
/// @param[in] s                        The value of the field
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2Platform(const gtString &s)
{    
    return InsertSessionInfoKeyValue(kLPGPU2PlatformSessionKey, s);
}
    
/// @brief                              Inserts the entry in the sessionInfo table.
/// @param[in] s                        The value of the field
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2Hardware(const gtString &s)
{    
    return InsertSessionInfoKeyValue(kLPGPU2HardwareSessionKey, s);
}

/// @brief                              Inserts the entry in the sessionInfo table.
/// @param[in] s                        The value of the field
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2DCAPI(const gtString &s)
{    
    return InsertSessionInfoKeyValue(kLPGPU2DCAPISessionKey, s);
}

/// @brief                              Inserts the entry in the sessionInfo table.
/// @param[in] s                        The value of the field
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2RAgent(const gtString &s)
{    
    return InsertSessionInfoKeyValue(kLPGPU2RagentSessionKey, s);
}

/// @brief                              Inserts the entry in the sessionInfo table.
/// @param[in] s                        The value of the field
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2HardwareID(const gtString &s)
{    
    return InsertSessionInfoKeyValue(kLPGPU2HardwareIDSessionKey, s);
}

/// @brief                              Inserts the entry in the sessionInfo table.
/// @param[in] s                        The value of the field
/// @returns   status                   success = the information was inserted into the database,
///                                     failure = an error has occurred during the insertion operation.
status LPGPU2DatabaseAdapter::SetLPGPU2BlobSize(const gtUInt32 v)
{   
    gtString asStr;
    asStr << v;
    return InsertSessionInfoKeyValue(kLPGPU2BlobSizeSessionKey, asStr);
}

/// @brief              Returns a flag indicating if this database is a valid LPGPU2 database.
/// @param[out] bool    Whether or not this database has the field "LPGPU2 Session" in the sessionInfo table.
/// @returns    status  Always returns success. 
status LPGPU2DatabaseAdapter::IsLPGPU2TargetDevice(bool& bIsTargetingLPGPU2Device)
{
    bIsTargetingLPGPU2Device = false;

    gtString isTargetingLPGPU2DeviceStr;    
    if(GetSessionInfoValue(LPGPU2_STR_SESSION_KEY_W, isTargetingLPGPU2DeviceStr))
    {
        bIsTargetingLPGPU2Device = isTargetingLPGPU2DeviceStr.compareNoCase(LPGPU2_STR_IS_LPGPU2_SESSION_W) == 0;
    }

    return success;
}
    
/// @brief                              Gets the entry from the sessionInfo table.
/// @param[out] s                       Contains the retrieved value, if any
/// @returns   status                   success = the information was retrieved from the database,
///                                     failure = an error has occurred during the retrieval operation.
status LPGPU2DatabaseAdapter::GetLPGPU2Platform(gtString &s)
{
  auto rc = failure;

  if(GetSessionInfoValue(kLPGPU2PlatformSessionKey, s))
  {
    rc = success;
  }

  return rc;
}

/// @brief                              Gets the entry from the sessionInfo table.
/// @param[out] s                       Contains the retrieved value, if any
/// @returns   status                   success = the information was retrieved from the database,
///                                     failure = an error has occurred during the retrieval operation.
status LPGPU2DatabaseAdapter::GetLPGPU2Hardware(gtString &s)
{
  auto rc = failure;

  if(GetSessionInfoValue(kLPGPU2HardwareSessionKey, s))
  {
    rc = success;
  }

  return rc;
}

/// @brief                              Gets the entry from the sessionInfo table.
/// @param[out] s                       Contains the retrieved value, if any
/// @returns   status                   success = the information was retrieved from the database,
///                                     failure = an error has occurred during the retrieval operation.
status LPGPU2DatabaseAdapter::GetLPGPU2DCAPI(gtString &s)
{
  auto rc = failure;

  if(GetSessionInfoValue(kLPGPU2DCAPISessionKey, s))
  {
    rc = success;
  }

  return rc;
}

/// @brief                              Gets the entry from the sessionInfo table.
/// @param[out] s                       Contains the retrieved value, if any
/// @returns   status                   success = the information was retrieved from the database,
///                                     failure = an error has occurred during the retrieval operation.
status LPGPU2DatabaseAdapter::GetLPGPU2RAgent(gtString &s)
{
  auto rc = failure;

  if(GetSessionInfoValue(kLPGPU2RagentSessionKey, s))
  {
    rc = success;
  }

  return rc;
}

/// @brief                              Gets the entry from the sessionInfo table.
/// @param[out] s                       Contains the retrieved value, if any
/// @returns   status                   success = the information was retrieved from the database,
///                                     failure = an error has occurred during the retrieval operation.
status LPGPU2DatabaseAdapter::GetLPGPU2HardwareID(gtString &s)
{
  auto rc = failure;

  if(GetSessionInfoValue(kLPGPU2HardwareIDSessionKey, s))
  {
    rc = success;
  }

  return rc;
}

/// @brief                              Gets the entry from the sessionInfo table.
/// @param[out] s                       Contains the retrieved value, if any
/// @returns   status                   success = the information was retrieved from the database,
///                                     failure = an error has occurred during the retrieval operation.
status LPGPU2DatabaseAdapter::GetLPGPU2BlobSize(gtString &s)
{
  auto rc = failure;

  if(GetSessionInfoValue(kLPGPU2BlobSizeSessionKey, s))
  {
    rc = success;
  }

  return rc;
}

    /// @brief                         Exports the database contents into a set of CSV files,
///                                    one per table, into the specified directory.
/// @param[in] vOutputDirectory        Name of the directory to write the contents of database.
/// @param[in] vFileNamePrefix         Prefix to be added to each file exported.
/// @param[in] vbOverrideExistingFiles true = When exporting the database, replace any file that
///                                           already exists,
///                                    false = Fail to export in case files already exists.
/// @returns   status                  success = the information was inserted into the database,
///                                    failure = an error has occurred during the insertion operation.
/// @warning                           The application must have write permissions to the directory.
/// @warning                           Files with the same name in the same directory will be overwritten.
status LPGPU2DatabaseAdapter::ExportToCSV(const gtString &vOutputDirectory, const gtString &vFileNamePrefix, const bool vbOverrideExistingFiles) const
{
    auto bReturn = failure;
    auto pDbAccessor = GetDatabaseAccessor();

    GT_IF_WITH_ASSERT(pDbAccessor != nullptr)
    {
        bReturn = pDbAccessor->ExportToCSV(vOutputDirectory, vFileNamePrefix, vbOverrideExistingFiles);
    }

    return bReturn;
}

/// @brief                            The database accessor is stored in the parent class as
///                                   as a pointer to the accessors base class. This method
///                                   casts the accessor to the type this class needs
/// @returns LPGPU2DatabaseAccessor*  Pointer to the database accessor in the type 
///                                   this class needs or null if the cast fails
LPGPU2DatabaseAccessor* LPGPU2DatabaseAdapter::GetDatabaseAccessor() const
{
    auto pTraceDbAccessor = dynamic_cast<LPGPU2DatabaseAccessor*>(m_pDbAccessor);

    GT_ASSERT_EX(m_pDbAccessor != nullptr, L"Accessor is not a LPGPU2DatabaseAccessor");

    return pTraceDbAccessor;
}

} // namespace db
} // namespace lpgpu2
