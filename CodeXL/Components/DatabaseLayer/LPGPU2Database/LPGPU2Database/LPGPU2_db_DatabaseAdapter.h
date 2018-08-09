// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Database is an extension to the CodeXL's Database Layer to
///         allow customisation of the CodeXL power profiling database.It
///         has the API to read and write the tables used by the LPGPU2
///         consortium
///
/// LPGPU2DatabaseAdapter interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DB_DATABASE_ADAPTER_H_INCLUDE
#define LPGPU2_DB_DATABASE_ADAPTER_H_INCLUDE

// AMDTDbAdapter:
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

// Local:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAPIDefs.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_FnStatus.h>

namespace lpgpu2 {
namespace db {

// Declarations:
class LPGPU2DatabaseAccessor;

using namespace fnstatus;

/// @brief      This is an extension of the amdtProfileDbAdapter that shows how
///             to read/write extra information from the power profiling 
///             database
/// @warning    None.
/// @see        amdtProfileDbAdapter
/// @see        LPGPU2DatabaseAccessor
/// @date       08/08/2017
/// @author     Thales Sabino
// clang-format off
class LPGPU2_DATABASE_API LPGPU2DatabaseAdapter : public amdtProfileDbAdapter
{
// Methods:
public:
    LPGPU2DatabaseAdapter();

// Overridden:
public:

    // From amdtProfileDbAdapter
    virtual bool Initialise() override;

// Methods:
public:

    gtString GetErrorMessage() const;

    // Disable copy/move of this object
    LPGPU2DatabaseAdapter(const LPGPU2DatabaseAdapter&) = delete;
    LPGPU2DatabaseAdapter(LPGPU2DatabaseAdapter&&) = delete;
    LPGPU2DatabaseAdapter& operator= (const LPGPU2DatabaseAdapter&) = delete;
    LPGPU2DatabaseAdapter& operator== (LPGPU2DatabaseAdapter&&) = delete;

    // Methods to read from the database
    status GetAllSamples(gtVector<PPSampleData> &vSamples) const;
    status GetProfileTraceInfo(gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetProfileTraceInfoByCallName(const gtString& callName, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetProfileTraceInfoMaxCpuEndTimeByFrameNumber(gtUInt64 vFrameNum, ProfileTraceInfo &vProfileTraceInfo) const;
    status GetProfileTraceInfoByCallCategory(const gtString &vCallCategory, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;
    status GetProfileTraceInfoByAPIName(const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;
    status GetProfileTraceInfoByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<ProfileTraceInfo> &vProfileTraceInfoList) const;
    status GetProfileTraceInfoByCallCategoryWithNumberOfCallsAboveThreshold(const gtString &vCounterCategory, gtUInt64 vThreshold, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetFrameTimeRange(gtUInt64 vFrameNum, gtUInt64 &vStartTime, gtUInt64 &vEndTime) const;
    status GetUniqueProfileTraceFrameNumbers(gtVector<gtUInt64> &vUniqueFrameNumbers) const;
    status GetFirstTraceCallTimestamp(gtUInt64 &vCpuStartTimestamp) const;
    status GetProfileTraceInfoInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ProfileTraceInfo> &vProfileTraceInfo) const;
    status GetAnnotations(gtVector<Annotation> &vAnnotations) const;
    status GetAnnotationById(gtUInt64 vAnnotationId, Annotation &vAnnotation) const;
    status GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<Annotation> &vAnnotations) const;
    status GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<Annotation> &vAnnotations) const;
    status GetGlobalAnnotations(gtVector<Annotation> &vAnnotations) const;
    status GetUserAnnotations(gtVector<Annotation> &vAnnotations) const;
    status GetEstimatedPowerSamples(gtVector<EstimatedPowerSample> &vEstimatedPwrSample) const;
    status GetEstimatedPowerComponents(gtMap<gtUInt64, gtVector<EstimatedPowerComponent>> &vPwrComponentsByDevice) const;
    status GetShaderTraces(gtVector<ShaderTrace> &vShaderTraces) const;    
    status GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<ShaderTrace> &vShaderTraces) const;
    status GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<ShaderTrace> &vShaderTraces) const;    
    status GetShaderTracesByAPIID(EShaderTraceAPIId vApiId, gtVector<ShaderTrace> &vShaderTraces) const;
    status GetShaderTracesByAPIName(const gtString &vApiName, gtVector<ShaderTrace> &vShaderTraces) const;
    status GetShaderAsmStatsByShaderId(gtUInt64 vShaderId, gtVector<ShaderAsmStat> &vShaderStats) const;
    status GetShaderToAsmByShaderId(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm) const;
    status GetGPUTimers(gtVector<GPUTimer> &vGpuTimers) const;
    status GetGPUTimersInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<GPUTimer> &vGpuTimers) const;
    status GetCounterWeights(gtVector<CounterWeight> &vCounterWeights) const;
    status GetCounterWeightForCounterByName(const gtString &vCounterName, CounterWeight &vCounterWeight) const;
    status GetWeightedSumForCounterByNameInRange(const gtString &vCounterName, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtFloat32 &vWeightedSum) const;
    status GetWeightedSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const;
    status GetStackTraces(gtVector<StackTraceInfo> &vStackTraces) const;    
    status GetStackTraceParameters(gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetStackTraceParametersByCallCategoryAndAPIName(const gtString &vCallCategory, const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetStackTraceParametersByCallCategory(const gtString &vCallCategory, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetStackTraceParametersByAPIName(const gtString &vApiName, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetNumThreadsFromTraceParameters(gtUInt64 &vNumThreads) const;
    status GetStackTraceParametersByFrameAndDrawNumber(gtUInt64 vFrameNum, gtUInt64 vDrawNum, gtVector<StackTraceParameter> &vTraceParameters) const;
    status GetRegionsOfInterest(gtVector<RegionOfInterest> &vRegionsOfInterest) const;
    status GetRegionOfInterestByRegionId(gtUInt64 vRegionId, RegionOfInterest &vRegionOfInterest) const;
    status GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime) const;
    status GetCallSummaryForRegionById(gtUInt64 vRegionId, gtVector<CallSummary> &vCallSummaryList) const;    
    status GetCallsPerTypeForRegionById(gtUInt64 vRegionId, CallsPerType &vCallsPerTypeList) const;
    status GetUniqueQuantizedTimesFromSamples(gtVector<gtUInt64> &vUniqueTimes) const;
    status GetUniqueQuantizedTimesFromSamplesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const;    
    status GetUniqueQuantizedTimesFromSamplesByCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<gtUInt64> &vUniqueTimes) const;
    status GetCallNamesByCategory(const gtString &vCategory, gtVector<CallCategory> &vCallCategories) const;    
    status GetSamplesForCounterByName(const gtString &vCounterName, gtVector<PPSampleData> &vSampleValues) const;
    status GetAverageSamplesByCounterCategoryInRange(const gtString &vCounterCategory, gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<PPSampleData> &vSampleValues) const;
    status GetMaxSampleValuesByCounterCategory(const gtString &vCounterCategory, gtVector<PPSampleData> &vSampleValues) const;    
    status GetNearestSampleValueBeforeTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample);
    status GetNearestSampleValueAfterTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample);
    status GetAllDevicesAsVector(gtVector<Device> &devices) const;
    status GetEstimatedCounterFrequency(gtInt32 vCounterId, double& freq) const;

    // Methods to write into the database
    status InsertLPGPU2CounterSampleBlobs(const CounterSampleBlobsVec &blobs);
    status InsertProfileTraceInfo(const ProfileTraceInfo &vProfileTraceInfo);
    status InsertAnnotation(const Annotation &vAnnotation);
    status InsertEstimatedPowerSample(const EstimatedPowerSample& vEstimatedPwrSample);
    status InsertEstimatedPowerComponent(const EstimatedPowerComponent &vEstimatedPwrComponent);
    status InsertStackTrace(const StackTraceInfo &vStackTraceInfo);
    status InsertStackTraceParameter(const StackTraceParameter &vStackTraceParam);
    status InsertShaderTrace(const ShaderTrace &vShaderTrace);
    status InsertShaderAsmStat(const ShaderAsmStat &vShaderStat);
    status InsertShaderToAsm(const ShaderToAsm &vShaderToAsm);
    status InsertGPUTimer(const GPUTimer &vGpuTimer);
    status InsertCounterWeight(const CounterWeight &vCounterWeight);
    status InsertRegionOfInterest(const RegionOfInterest &vRegionOfInterest);
    status InsertCallCategory(const CallCategory &vCallCategory);    
    status InsertCallSummary(const CallSummary &vCallSummary);
    status InsertCallsPerType(const CallsPerType &vCallsPerType);
    status InsertCounterCategory(const InsertCounterCategoryInfo &vCounterCategory);

    status UpdateShaderTrace(EShaderMetadataType vType, const ShaderTrace &vShaderTrace);
    status UpdateShaderAsmStats(const gtVector<ShaderAsmStat> &vShaderStats);

    status SetLPGPU2TargetDevice(const bool bIsTargetingLPGPU2Device);
    status SetLPGPU2Platform(const gtString &s);
    status SetLPGPU2Hardware(const gtString &s);
    status SetLPGPU2DCAPI(const gtString &s);
    status SetLPGPU2RAgent(const gtString &s);
    status SetLPGPU2HardwareID(const gtString &s);
    status SetLPGPU2BlobSize(const gtUInt32 v);
    
    status IsLPGPU2TargetDevice(bool &bIsTargetingLPGPU2Device);
    status GetLPGPU2Platform(gtString &s);
    status GetLPGPU2Hardware(gtString &s);
    status GetLPGPU2DCAPI(gtString &s);
    status GetLPGPU2RAgent(gtString &s);
    status GetLPGPU2HardwareID(gtString &s);
    status GetLPGPU2BlobSize(gtString &s);
    
    // Methods to delete data from the database
    status DeleteRegionsOfInterest();
    status DeleteAnnotations();    
    status DeleteCallCategories();
    status DeleteCounterCategories();
    status DeleteShaderAsmStatsForShaderId(gtUInt64 vShaderId, EShaderTraceAPIId vApiId);

    status DeleteFeedbacks();    

    // Transaction methods
    status BeginTransaction() const;
    status EndTransaction() const;

    // Methods to export the database
    status ExportToCSV(const gtString &vOutputDirectory, const gtString &vFileNamePrefix, bool vbOverrideExistingFiles) const;

// Attributes:
private:
    LPGPU2DatabaseAccessor* GetDatabaseAccessor() const;
};
// clang-format on

} // namespace db
} // namespace lpgpu2

#endif // LPGPU2_DB_DATABASE_ADAPTER_H_INCLUDE
