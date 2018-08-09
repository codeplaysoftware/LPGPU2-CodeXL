//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerBL.h
///
//==================================================================================

#ifndef PowerProfilerBL_h__
#define PowerProfilerBL_h__

// Local.
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
#include <AMDTPowerProfilingMidTier/include/PPDevice.h>

// Infra.
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>

// Backend.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileApi.h>
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

// Common DB access structures to query/insert profile data
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

namespace lpgpu2{ namespace db { class LPGPU2FeedbackEngine; } }

class AMDTPOWERPROFILINGMIDTIER_API PowerProfilerBL
{
public:

    PowerProfilerBL();
    ~PowerProfilerBL();

    // Causes the database to refresh with the new data.
    bool Refresh();

    // This function opens a database for reading.
    // Arguments: const gtString& dbName - a name of an already created database.
    // Note #1: If this function succeeds, all consecutive calls to functions in this API will target
    // the specified database
    // Note #2: if this function fails, the behavior of any of the following functions is undefined.
    bool OpenPowerProfilingDatabaseForRead(const gtString& dbName, bool vbIsReadOnly);

    // This function resets the object together with all of its DB connections.
    bool CloseAllConnections();

    // Retrieves the time points when the session began and ended.
    // Output args: AMDTPwrSystemTime& begin, AMDTPwrSystemTime& end.
    bool GetSessionTimeRange(SamplingTimeRange& samplingTimeRange);

    // Retrieves the sampling interval for the current session.
    bool GetSessionSamplingIntervalMs(unsigned& samplingIntervalMs);

    // Calculates the aggregated amount of energy which each counter spent since the beginning of the session (in Joule).
    // Output args: gtMap<int, double>& consumptionPerCounterInJoule.
    bool GetCurrentCumulativeEnergyConsumptionInJoule(const gtVector<int>& counterIds,
                                                      gtMap<int, double>& consumptionPerCounterInJoule, double& otherCumulativeConsumption);

    // Calculates the average amount of power which each counter spent since the beginning of the session (in Watt).
    // Output args: gtMap<int, double>& consumptionPerCounterInWatt.
    bool GetCurrentAveragePowerConsumptionInWatt(const gtVector<int>& counterIds,
                                                 unsigned int samplingIntervalMs, gtMap<int, double>& consumptionPerCounterInWatt, double& otherCounterConsumptionInWatt);

    // Calculates a bucketed  frequencies histogram for the specified counters from the beginning of the session.
    // Output args: gtMap<int, HistogramBucket>& bucketPerCounter.
    bool GetCurrentFrequenciesHistogram(unsigned int bucketWidth, const gtVector<int>& counterIds, gtMap<int, gtVector<HistogramBucket>>& bucketPerCounter);

    // Retrieves the sampled values for each counter in the specified time range.
    // Output args: gtVector<double>>& sampledValuesPerCounter.
    bool GetSampledValuesByRange(const gtVector<int>& counterIds, SamplingTimeRange& samplingTimeRange,
                                 gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter);

    // Retrieves a reduced set of sampled values for each counter in the specified time range.
    // The reduction is being made in such way that the number of sampled values per counter will
    // be less than or equal to the specified max amount.
    // Output args: gtVector<double>>& sampledValuesPerCounter.
    bool GetReducedSampledValuesByRange(const gtVector<int>& counterIds, SamplingTimeRange& samplingTimeRange,
                                        int maxValuesPerCounter, gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter);

    // Retrieves the global maximum and minimum of sampled values in the specified range.
    // Output args: double& minValue, double& maxValue.
    bool GetGlobalMinMaxValuesPerCounters(const gtVector<int> counterIds, SamplingTimeRange& samplingTimeRange,
                                          double& minValue, double& maxValue);

    // Retrieves the overall number of samples which were taken throughout the session for each of the specified counters.
    bool GetOverallNubmerOfSamples(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter);

    // Utility function that updates the current buckets with newly sampled data.
    bool UpdateCumulativeAndAverageHistograms(const gtMap<int, PPSampledValuesBatch>& newSamples, unsigned currSamplingInterval,
                                              gtMap<int, double>& accumulatedEnergyInJoule, double& cumulativeOtherCounterValue, gtMap<int, double>& averagePowerInWatt,
                                              double& averageOtherCounterValue, gtMap<int, unsigned>& aggregatedQuantizedClockTicks);

    // Utility function that updates the current frequency buckets with newly sampled data.
    bool CalculateOnlineFrequencyHistograms(unsigned int bucketWidth, unsigned int currSamplingInterval, const gtMap<int, PPSampledValuesBatch>& newSamples,
                                            const gtVector<int>& relevantCounterIds, gtMap<int, gtVector<HistogramBucket>>& bucketsPerCounter);

    // Utility function that retrieves the AMDTDeviceType for the device with id deviceId.
    bool GetDeviceType(int deviceId, AMDTDeviceType& deviceType);

    // Utility function to locate the details of a specific device within a device tree
    static const PPDevice* GetDevice(int deviceID, const PPDevice* pRootDevice);


    // Utility function that retrieves the counters which were enabled during the session and that
    // have category of type counterCategory.
    bool GetSessionCounters(AMDTPwrCategory counterCategory, gtVector<int>& counterIds);

    // Utility function that retrieves the counters which were enabled during the session and that
    // are linked to devices of type deviceType and have category of type counterCategory.
    bool GetSessionCounters(AMDTDeviceType deviceType, AMDTPwrCategory counterCategory, gtVector<int>& counterIds);

    // Utility function that retrieves a map containing all of the session's counter names and their corresponding ids.
    bool GetSessionCounterIdByName(gtMap<gtString, int>& counterNames);

    // Utility function that retrieves the counters which were enabled during the session and that
    // are linked to devices of type deviceType and have category of type counterCategory.
    bool GetSessionCounters(const gtVector<AMDTDeviceType>& deviceTypes, AMDTPwrCategory counterCategory, gtVector<int>& counterIds);

    // Utility function that retrieves the details of all of the counters which were enabled during the session.
    bool GetAllSessionCountersDescription(gtMap<int, AMDTPwrCounterDesc*>& counterDetails);

    // Utility function that retrieves the session info for this DB's session.
    bool GetSessionInfo(AMDTProfileSessionInfo& sessionInfo);

    // Utility function that retrieves the counter ID of the counter whose name is 'Total APU Power'.
    bool GetApuPowerCounterId(int& apuPowerCounterId);

    //++SRB: LPGPU2
    bool GetTraceRecords(gtVector<lpgpu2::db::ProfileTraceInfo>& records);
    //--SRB: LPGPU2    

    //++KB: LPGPU2
    bool GetAnnotations(gtVector<lpgpu2::db::Annotation>& annotations);
    //--KB: LPGPU2

//++AT:LPGPU2
    /// Retrieve the help info entry at index index
    bool GetAnnotation(gtUInt64 index, lpgpu2::db::Annotation& annotation);    
    bool GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<lpgpu2::db::Annotation> &vAnnotations);
    bool GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<lpgpu2::db::Annotation> &vAnnotations);
    bool GetGlobalAnnotations(gtVector<lpgpu2::db::Annotation>& annotations);
    bool GetUserAnnotations(gtVector<lpgpu2::db::Annotation>& annotations);
    bool GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces);
    bool GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces);
    bool GetCallsPerTypeByRegionId(gtUInt64 vRegionId, lpgpu2::db::CallsPerType &vCallsPerType);
//--AT:LPGPU2

    bool GetShaderTraces(gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces);

//++AT:LPGPU2
    bool GetAllDevicesAsVector(gtVector<PPDevice> &devices);
    bool GetStackTraceParameters(
        gtVector<lpgpu2::db::StackTraceParameter>& records);
//--AT:LPGPU2

    //++CF:LPGPU2
    bool GetRegionsOfInterest(gtVector<lpgpu2::db::RegionOfInterest>& regionsOfInterest);
    bool GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime);
    bool GetCallSummaryForRegionId(gtUInt64 vRegionId, gtVector<lpgpu2::db::CallSummary> &vCallSummaryList);
    bool GetRegionOfInterestById(gtUInt64 vRegionId, lpgpu2::db::RegionOfInterest& vRegionOfInterest);    
    bool GetSampledValuesByNanoSecRange(const gtVector<int>& counterIds, SamplingTimeRange& samplingTimeRange,
                                 gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter);
    bool GetIsLPGPU2TargetDevice(bool& vIsLPGPU2TargetDevice);
    bool GetFrameTimeRange(gtUInt64 vFrameNum, gtUInt64 &vFrameStart, gtUInt64 &vFrameEnd);
    bool GetApproxCounterSamplesAtTimePoint(gtUInt64 vTime, gtMap<AMDTPwrCounterDesc*,double>& vrCounterSamples);
    bool GetNearestSampleValueBeforeTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample);
    bool GetNearestSampleValueAfterTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample);
    bool GetTraceStartTimeOffsetMs(gtInt64& vOffsetMs);   
    bool GetEstimatedCounterFrequency(gtInt32 vCounterId, double& vFreq);
    //--CF:LPGPU2

    //++TLRS: LPGPU2
    bool IsLPGPU2Session() const;
    bool GetFeedbackEngineForSession(std::unique_ptr<lpgpu2::db::LPGPU2FeedbackEngine> &vFeedbackEnginePtr) const;
    bool GetShaderToAsmMappings(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm);
    bool GetShaderAsmStats(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderAsmStat> &vShaderAsmStats);    
    //--TLRS: LPGPU2

    // TODO: Remove this method
    void SimulateShaderCounterMatchScript(gtUInt64 vShaderId, gtUInt64 vApiId, gtUInt64 vCounterId);

private:
    class Impl;
    Impl* m_pImpl;

    // No copy.
    PowerProfilerBL(const PowerProfilerBL& other);
    PowerProfilerBL& operator=(const PowerProfilerBL& other);
};

#endif // PowerProfilerBL_h__
