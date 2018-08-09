//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerBL.cpp
///
//==================================================================================

#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>
#include <AMDTPowerProfilingMidTier/include/BackendDataConvertor.h>

#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEngine.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtSet.h>

//++AT:LPGPU2
// Imports:
// std::transform
// std::back_inserter
#include <algorithm>
#include <iterator>
#include <chrono>
//--AT:LPGPU2

#include <random>

class PowerProfilerBL::Impl
{
public:
    Impl() : m_apuPowerCounterId(-1), m_samplingIntervalMs(0) {}

    bool Refresh()
    {
        return m_dataAdapter.FlushDbAsync();
    }

    bool CacheSessionSelectedCounters()
    {
        gtVector<int> pwrCounters;
        bool ret = GetSessionCounters(AMDT_PWR_CATEGORY_POWER, pwrCounters);

        if (ret)
        {
            for (int& cid : pwrCounters)
            {
                m_sessionSelectedPowerCounters.insert(cid);
            }
        }

        return ret;
    }

    // Note: In 2.1, this should be part of the db migration tool
    bool MigrateDatabaseForNewVersion(const gtString& dbName)
    {
        lpgpu2::db::LPGPU2DatabaseAdapter dbAdapter;
        bool ret = dbAdapter.Initialise();

        //++TLRS: LPGPU2: Fix bug that could cause a crash in CodeXL is a LPGPU2 session is created with no options set. 
        if (!ret)
        {
            return ret;
        }
        //--TLRS: LPGPU2: Fix bug that could cause a crash in CodeXL is a LPGPU2 session is created with no options set.

        ret = dbAdapter.MigrateDb(dbName);

        if (ret)
        {
            gtMap<gtString, int> infoMap;

            if (BackendDataConvertor::GetPwrDeviceIdStringMap(infoMap))
            {
                ret = dbAdapter.UpdateDeviceTypeId(infoMap);
                infoMap.clear();
            }

            if (ret && BackendDataConvertor::GetPwrCategoryIdStringMap(infoMap))
            {
                ret = dbAdapter.UpdateCounterCategoryId(infoMap);
                infoMap.clear();
            }

            if (ret && BackendDataConvertor::GetPwrAggregationIdStringMap(infoMap))
            {
                ret = dbAdapter.UpdateCounterAggregationId(infoMap);
                infoMap.clear();
            }

            if (ret && BackendDataConvertor::GetPwrUnitIdStringMap(infoMap))
            {
                ret = dbAdapter.UpdateCounterUnitId(infoMap);
                infoMap.clear();
            }
        }

        dbAdapter.CloseDb();

        return ret;
    }

    bool OpenPowerProfilingDatabaseForRead(const gtString& dbName, bool vbIsReadOnly)
    {
        // Clear cached values.
        m_sessionSelectedPowerCounters.clear();
        
        bool ret = m_dataAdapter.OpenDb(dbName, AMDT_PROFILE_MODE_TIMELINE, vbIsReadOnly);

        if (! ret)
        {
            int version = -1;
            m_dataAdapter.GetDbVersion(version);

            if (version < m_dataAdapter.GetSupportedDbVersion())
            {
                m_dataAdapter.CloseDb();

                ret = MigrateDatabaseForNewVersion(dbName);

                ret = ret && m_dataAdapter.OpenDb(dbName, AMDT_PROFILE_MODE_TIMELINE);
            }
        }

        if (ret)
        {
            // Cache the session Power counters.
            ret = CacheSessionSelectedCounters();
//++AT:LPGPU2
            //GT_ASSERT(ret);
//--AT:LPGPU2

            // Cache the APU Power counter ID.
            ret = GetApuPowerCounterId(m_apuPowerCounterId);

            // Cache the session sampling interval.
            ret = GetSessionSamplingIntervalMs(m_samplingIntervalMs);
//++AT:LPGPU2
            //GT_ASSERT(ret);
//--AT:LPGPU2
        }

        return ret;
    }

    bool GetSessionTimeRange(SamplingTimeRange& samplingTimeRange)
    {
        return m_dataAdapter.GetSessionTimeRange(samplingTimeRange);
    }

    bool GetFrequencyCountersValueRange(const AMDTDeviceType& devideType, double& minValue, double& maxValue)
    {
        const double FREQ_COUNTERS_MIN_VALUE_MHZ = 0.0;
        const double CPU_FREQ_COUNTERS_MAX_VALUE_MHZ = 4900;
        const double GPU_FREQ_COUNTERS_MAX_VALUE_MHZ = 1200;

        // Reset the output variables.
        minValue = maxValue = FREQ_COUNTERS_MIN_VALUE_MHZ;

        bool ret = true;

        switch (devideType)
        {
            case AMDT_PWR_DEVICE_CPU_CORE:
                maxValue = CPU_FREQ_COUNTERS_MAX_VALUE_MHZ;
                break;

            case AMDT_PWR_DEVICE_INTERNAL_GPU:
            case AMDT_PWR_DEVICE_EXTERNAL_GPU:
                maxValue = GPU_FREQ_COUNTERS_MAX_VALUE_MHZ;
                break;

            default:
                ret = false;
                break;
        }

        return ret;
    }

    bool GetCurrentCumulativeEnergyConsumptionInJoule(const gtVector<int>& counterIds,
                                                      gtMap<int, double>& consumptionPerCounterInJoule, double& otherCumulativeConsumption)
    {
        //bool ret = m_dataAdapter.GetCurrentCumulativeEnergyConsumptionInJoule(counterIds, consumptionPerCounterInJoule);

        bool ret = false;

        // First retrieve the data from the DB.
        ret = m_dataAdapter.GetSamplesGroupByCounterId(counterIds, consumptionPerCounterInJoule);

        if (ret)
        {
            unsigned currSamplingIntervalMs = 0;
            ret = m_dataAdapter.GetSessionSamplingIntervalMs(currSamplingIntervalMs);

            if (ret)
            {
                // Convert the values from Watt to Joule (formula is Sigma(Avg(Watt))*timeInSec=EnergyInJoule).
                for (auto& pair : consumptionPerCounterInJoule)
                {
                    pair.second = (pair.second * (currSamplingIntervalMs / 1000.0));
                }
            }
        }

        if (ret)
        {
            int apuPowerCid = -1;
            ret = GetApuPowerCounterId(apuPowerCid);

            if (ret)
            {
                // Now add the 'Other' counter's data.
                for (const auto& pair : consumptionPerCounterInJoule)
                {
                    if (pair.first == apuPowerCid)
                    {
                        otherCumulativeConsumption += pair.second;
                    }
                    else
                    {
                        otherCumulativeConsumption -= pair.second;
                    }
                }

                // This is a workaround until the Backend accuracy issue gets solved.
                // The thing is that
                if (otherCumulativeConsumption < 0.0)
                {
                    otherCumulativeConsumption = 0.0;
                }
            }
        }

        return ret;
    }

    bool GetCurrentAveragePowerConsumptionInWatt(const gtVector<int>& counterIds, unsigned int samplingIntervalMs, gtMap<int, double>& consumptionPerCounterInWatt, double& otherCounterConsumptionInWatt)
    {
        //return m_dataAdapter.GetCurrentAveragePowerConsumptionInWatt(counterIds, samplingIntervalMs, consumptionPerCounterInWatt, otherCounterConsumptionInWatt);

        bool ret = false;
        otherCounterConsumptionInWatt = 0.0;

        if (samplingIntervalMs > 0)
        {
            // First, count the number of samples for each counter.
            gtMap<int, int> numOfSamples;
            ret = m_dataAdapter.GetSampleCountByCounterId(counterIds, numOfSamples);

            GT_IF_WITH_ASSERT(ret)
            {
                // Get the total power consumed in Watt.
                gtMap<int, double> cumulativePowerInWatt;
                ret = m_dataAdapter.GetSamplesGroupByCounterId(counterIds, cumulativePowerInWatt);

                int counterId = 0;
                double avgValueInWatt = 0.0;

                GT_IF_WITH_ASSERT(ret)
                {
                    // Get the Total APU Power counter id.
                    int apuPowerCounterId = -1;
                    gtString apuName(L"Total APU Power");
                    bool doesAPUExist = m_dataAdapter.GetCounterIdByName(apuName, apuPowerCounterId);

                    // Go over the cumulative values, and calculate the average:
                    for (const auto& pair : cumulativePowerInWatt)
                    {
                        counterId = pair.first;
                        avgValueInWatt = cumulativePowerInWatt[counterId] / numOfSamples[counterId];
                        consumptionPerCounterInWatt.insert(std::pair<int, double>(counterId, avgValueInWatt));

                        // Calculate the other counter value, only when there is an APU:
                        if (doesAPUExist)
                        {
                            // Aggregate the 'Other' counter's data.
                            if (pair.first == apuPowerCounterId)
                            {
                                otherCounterConsumptionInWatt += pair.second;
                            }
                            else
                            {
                                otherCounterConsumptionInWatt -= pair.second;
                            }
                        }
                    }

                    if (doesAPUExist)
                    {
                        // Just to be on the safe side until the Backend accuracy issue is solved.
                        if (otherCounterConsumptionInWatt < 0.0)
                        {
                            otherCounterConsumptionInWatt = 0.0;
                        }

                        // Calculate the 'Other' counter's value.
                        if (otherCounterConsumptionInWatt > 0.0)
                        {
                            otherCounterConsumptionInWatt = otherCounterConsumptionInWatt / numOfSamples[apuPowerCounterId];
                        }
                    }
                }
            }
        }

        return ret;
    }

    bool GetSampledValuesByRange(const gtVector<int>& counterIds,
                                 SamplingTimeRange& samplingTimeRange, gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
    {
        return m_dataAdapter.GetSampledValuesByRange(counterIds, samplingTimeRange, sampledValuesPerCounter);
    }

//++AT:LPGPU2
    /// @brief Return all the devices in the system inside a vector
    /// @param devices The returned vector of devices
    /// @return Always true
    bool GetAllDevicesAsVector(gtVector<PPDevice> &devices)
    {
      gtVector<lpgpu2::db::Device> devicesFromDBLayer;
      m_dataAdapter.GetAllDevicesAsVector(devicesFromDBLayer);

      std::transform(devicesFromDBLayer.cbegin(), devicesFromDBLayer.cend(),
          std::back_inserter(devices),
          [](const lpgpu2::db::Device &devDB) {
            PPDevice devOut;
            devOut.m_deviceId = devDB.m_deviceId;
            devOut.m_deviceType =
              static_cast<AMDTDeviceType>(devDB.m_deviceTypeId);
            devOut.m_deviceName = devDB.m_deviceName;
            devOut.m_deviceDescription = devDB.m_deviceDescription;

            return devOut;
          });

      return true;
    }

    bool GetStackTraceParameters(
        gtVector<lpgpu2::db::StackTraceParameter>& records)
    {
      return m_dataAdapter.GetStackTraceParameters(records);
    }
//--AT:LPGPU2


//++CF:LPGPU2
    bool GetSampledValuesByNanoSecRange(const gtVector<int>& counterIds,
                                 SamplingTimeRange& samplingTimeRange, gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
    {
        // TODO: Is this function really needed?
        // Millisec range from nanosec
        const std::chrono::nanoseconds fromTimeMs{ samplingTimeRange.m_fromTime };
        const std::chrono::nanoseconds toTimeMs{ samplingTimeRange.m_toTime };        

        const auto fromTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(fromTimeMs);
        const auto toTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(toTimeMs);

//++AT:LPGPU2
        SamplingTimeRange msTimeRange{ static_cast<gtInt64>(fromTimeNs.count()), static_cast<gtInt64>(toTimeNs.count()) };
//--AT:LPGPU2
        return m_dataAdapter.GetSampledValuesByRange(counterIds, msTimeRange, sampledValuesPerCounter);
    }

    bool GetEstimatedCounterFrequency(gtInt32 vCounterId, double& vFreq)
    {
        return m_dataAdapter.GetEstimatedCounterFrequency(vCounterId, vFreq);
    }
//--CF:LPGPU2
    bool GetGlobalMinMaxValuesPerCounters(const gtVector<int> counterIds,
                                          SamplingTimeRange& samplingTimeRange, double& minValue, double& maxValue)
    {
        return m_dataAdapter.GetGlobalMinMaxValuesPerCounters(counterIds, samplingTimeRange, minValue, maxValue);
    }

    bool GetOverallNubmerOfSamples(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter)
    {
        return m_dataAdapter.GetOverallNubmerOfSamples(counterIds, numberOfSamplesPerCounter);
    }

    bool GetCurrentFrequenciesHistogram(unsigned int bucketWidth, const gtVector<int>& counterIds,
                                        gtMap<int, gtVector<HistogramBucket>>& bucketPerCounter)
    {
        // return m_dataAdapter.GetCurrentFrequenciesHistogram(bucketWidth, m_samplingIntervalMs, counterIds, bucketPerCounter);
        bool ret = false;

        // Make sure our output map is clear.
        bucketPerCounter.clear();

        if (bucketWidth > 0)
        {
            // First, fill up the containers with empty buckets.
            // This is not required for correctness, only to ensure that even empty buckets are presented.
            // If this is the first time, we need to build the empty histograms.
            // First, get the range of values for these counters.
            double minValue = 0.0;
            double maxValue = 0.0;

            // Get the device type for this counter id
            int deviceType;
            ret = m_dataAdapter.GetDeviceTypeByCounterId(counterIds[0], deviceType);
            ret = GetFrequencyCountersValueRange((AMDTDeviceType)(deviceType), minValue, maxValue);

            GT_IF_WITH_ASSERT(ret)
            {
                // Currently ignore the minimum value, take zero as the minimum value.
                double currentMax = 0.0;
                double currentUpperBound = currentMax + bucketWidth;

                while ((currentUpperBound < maxValue) || (currentUpperBound - maxValue) < bucketWidth)
                {
                    // Create the current bucket.
                    HistogramBucket currBucket;
                    currBucket.m_lowerBound = currentMax;
                    currBucket.m_upperBound = currentMax + bucketWidth;
                    currBucket.m_value = 0.0;

                    // Add the bucket to each of our counters.
                    for (int cid : counterIds)
                    {
                        bucketPerCounter[cid].push_back(currBucket);
                    }

                    // Increment our current maximum.
                    currentMax += bucketWidth;
                    currentUpperBound = currentMax + bucketWidth;
                }
            }

            gtVector<int> dbCids;
            gtVector<double> dbBucketBottoms;
            gtVector<int> dbBucketCount;

            ret = m_dataAdapter.GetBucketizedSamplesByCounterId(bucketWidth, counterIds, dbCids, dbBucketBottoms, dbBucketCount);

            if (ret)
            {
                // Now, after the DB is released, let's create the histograms.
                const size_t numOfResults = dbCids.size();
                ret = (numOfResults > 0) && (dbBucketBottoms.size() == numOfResults) && (dbBucketCount.size() == numOfResults);

                if (ret)
                {
                    for (size_t i = 0; i < numOfResults; ++i)
                    {
                        int currCid = dbCids[i];
                        double currBucketBottom = dbBucketBottoms[i];
                        gtVector<HistogramBucket>& currBucketVector = bucketPerCounter[currCid];

                        for (auto& bucket : currBucketVector)
                        {
                            if (bucket.m_lowerBound == currBucketBottom)
                            {
                                // Add the amount of seconds to the bucket.
                                bucket.m_value += (m_samplingIntervalMs * dbBucketCount[i]) / 1000.0;
                                break;
                            }
                        }
                    }

                    ret = true;
                }
            }
        }

        return ret;
    }

    bool UpdateCumulativeAndAverageHistograms(const gtMap<int, PPSampledValuesBatch>& newSamples, unsigned currSamplingInterval,
                                              gtMap<int, double>& accumulatedEnergyInJoule, double& cumulativeOtherCounterValue, gtMap<int, double>& averagePowerInWatt, double& averageOtherCounterValueWatt,
                                              gtMap<int, unsigned>& aggregatedQuantizedClockTicks)
    {
        bool ret = true;
        averageOtherCounterValueWatt = 0.0;
        cumulativeOtherCounterValue = 0.0;

        // First, verify that power counters data is cached.
        if (m_sessionSelectedPowerCounters.empty())
        {
            CacheSessionSelectedCounters();
//++AT:LPGPU2
// Aware of this. Because in LPGPU2 mode we might have setups without
// power counters, this assertion gets triggered in those cases.
            //GT_ASSERT(!m_sessionSelectedPowerCounters.empty());
//--AT:LPGPU2
        }

        // Update the accumulated energy map.
        double currSampleValue = 0.0;
        int currCounterId = 0;

        for (const auto& samplePerCounter : newSamples)
        {
            // Take the current sample value.
            currSampleValue = samplePerCounter.second.m_sampleValues[0];
            currCounterId = samplePerCounter.first;

            // First, verify that the current counter is a Power counter.
            if (m_sessionSelectedPowerCounters.find(currCounterId) != m_sessionSelectedPowerCounters.end())
            {
                // Increment the aggregated clock ticks counter.
                auto timeMapIter = aggregatedQuantizedClockTicks.find(currCounterId);

                if (timeMapIter != aggregatedQuantizedClockTicks.end())
                {
                    aggregatedQuantizedClockTicks[currCounterId] += currSamplingInterval;
                }
                else
                {
                    aggregatedQuantizedClockTicks[currCounterId] = currSamplingInterval;
                }

                // Update the accumulated energy.
                // Take the buckets vector that is relevant to our counter.
                auto accumMapIter = accumulatedEnergyInJoule.find(currCounterId);

                if (accumMapIter != accumulatedEnergyInJoule.end())
                {
                    // The formula that we used is:
                    // Sigma(W)*timeInSeconds=EnergyInJaul.
                    // timeInSeconds in this case is currentSamplingInterval/1000.0, because currentSamplingInterval
                    // is in milliseconds and it is the time for which Wi was calculated as the average power.
                    // Note that the division by 1000.0 is there because currentSamplingInterval is in milliseconds.
                    // Since timeInSeconds is timeInMS/1000, and since the values are reported by the BE in Watt,
                    accumulatedEnergyInJoule[currCounterId] += ((currSampleValue * currSamplingInterval) / 1000.0);
                }
                else
                {
                    accumulatedEnergyInJoule[currCounterId] = ((currSampleValue * currSamplingInterval) / 1000.0);
                }

                // Update the average power consumption.
                // Take the buckets vector that is relevant to our counter.
                const unsigned currAggregatedSamplingTimeMs = aggregatedQuantizedClockTicks[currCounterId];

                if (currAggregatedSamplingTimeMs > 0)
                {
                    // Watt is Joule to Sec.
                    // First, get the accumulated value in Watt.
                    // Then, calculate the average power in Watt by dividing the accumulated value in Watt by the time of the sampling session.
                    double accumulatedValueInWatt = (accumulatedEnergyInJoule[currCounterId] * 1000.0) / currSamplingInterval;
                    averagePowerInWatt[currCounterId] = (accumulatedValueInWatt) / (aggregatedQuantizedClockTicks[currCounterId] / currSamplingInterval);
                }
            }
        }

        int apuPowerCounterId = 0;
        ret = GetApuPowerCounterId(apuPowerCounterId);

        if (ret)
        {
            // Now take care of the 'Other' counters.
            for (const auto& pair : averagePowerInWatt)
            {
                if (pair.first == apuPowerCounterId)
                {
                    averageOtherCounterValueWatt += pair.second;
                }
                else
                {
                    averageOtherCounterValueWatt -= pair.second;
                }
            }

            for (const auto& pair : accumulatedEnergyInJoule)
            {
                if (pair.first == apuPowerCounterId)
                {
                    cumulativeOtherCounterValue += pair.second;
                }
                else
                {
                    cumulativeOtherCounterValue -= pair.second;
                }
            }

            // Just to be on the safe side until the Backend accuracy issue is resolved.
            if (cumulativeOtherCounterValue < 0.0)
            {
                cumulativeOtherCounterValue = 0.0;
            }

            if (averageOtherCounterValueWatt < 0.0)
            {
                averageOtherCounterValueWatt = 0.0;
            }
        }

        return ret;
    }

    bool CalculateOnlineFrequencyHistograms(unsigned int bucketWidth, unsigned int currSamplingIntervalMs, const gtMap<int, PPSampledValuesBatch>& newSamples,
                                            const gtVector<int>& relevantCounterIds, gtMap<int, gtVector<HistogramBucket>>& bucketsPerCounter)
    {
        bool ret = true;

        if (bucketsPerCounter.size() == 0)
        {
            // If this is the first time, we need to build the empty histograms.
            // First, get the range of values for these counters.
            double minValue = 0.0;
            double maxValue = 0.0;

            int deviceType;
            ret = m_dataAdapter.GetDeviceTypeByCounterId(relevantCounterIds[0], deviceType);
            ret = GetFrequencyCountersValueRange((AMDTDeviceType)deviceType, minValue, maxValue);

            GT_IF_WITH_ASSERT(ret)
            {
                // Currently ignore the minimum value, take zero as the minimum value.
                double currentMax = 0.0;
                double currentUpperBound = currentMax + bucketWidth;

                while ((currentUpperBound < maxValue) || (currentUpperBound - maxValue) < bucketWidth)
                {
                    // Create the current bucket.
                    HistogramBucket currBucket;
                    currBucket.m_lowerBound = currentMax;
                    currBucket.m_upperBound = currentMax + bucketWidth;
                    currBucket.m_value = 0.0;

                    // Add the bucket to each of our counters.
                    for (int cid : relevantCounterIds)
                    {
                        bucketsPerCounter[cid].push_back(currBucket);
                    }

                    // Increment our current maximum.
                    currentMax += bucketWidth;
                    currentUpperBound = currentMax + bucketWidth;
                }

                // Check if we got a tail (a bucket whose width is smaller than BUCKET_WIDTH).
                if (currentMax < maxValue)
                {
                    // Create the current bucket.
                    HistogramBucket currBucket;
                    currBucket.m_lowerBound = currentMax;
                    currBucket.m_upperBound = maxValue;
                    currBucket.m_value = 0.0;

                    // Add the bucket to each of our counters.
                    for (int cid : relevantCounterIds)
                    {
                        bucketsPerCounter[cid].push_back(currBucket);
                    }
                }
            }
        }

        // Now handle the values.
        double currSampledValue = 0.0;

        for (int cid : relevantCounterIds)
        {
            // Get the relevant buckets vector.
            gtVector<HistogramBucket>& currBucketsVector = bucketsPerCounter[cid];

            // Get the relevant sample.
            const auto iter = newSamples.find(cid);

            if (iter != newSamples.end())
            {
                currSampledValue = iter->second.m_sampleValues[0];

                // Increment the relevant bucket.
                for (HistogramBucket& bucket : currBucketsVector)
                {
                    if (currSampledValue >= bucket.m_lowerBound && currSampledValue <= bucket.m_upperBound)
                    {
                        // Add the number of seconds.
                        bucket.m_value += currSamplingIntervalMs / 1000.0;
                        break;
                    }
                }
            }
        }

        return ret;
    }

    bool GetDeviceType(int deviceId, AMDTDeviceType& deviceType)
    {
        return m_dataAdapter.GetDeviceType(deviceId, reinterpret_cast<int&>(deviceType));
    }

    bool GetSessionCounters(AMDTDeviceType deviceType, AMDTPwrCategory counterCategory, gtVector<int>& counterIds)
    {
        return m_dataAdapter.GetSessionCounters(deviceType, counterCategory, counterIds);
    }

    bool GetSessionCounters(const gtVector<AMDTDeviceType>& deviceTypes, AMDTPwrCategory counterCategory, gtVector<int>& counterIds)
    {
        //return m_dataAdapter.GetSessionCounters(deviceTypes, counterCategory, counterIds);
        gtVector<int> deviceTypeIds;

        for (auto deviceId : deviceTypes)
        {
            deviceTypeIds.push_back((int)(deviceId));
        }

        return m_dataAdapter.GetSessionCounters(deviceTypeIds, counterCategory, counterIds);
    }

    bool GetSessionCounters(AMDTPwrCategory counterCategory, gtVector<int>& counterIds)
    {
        return m_dataAdapter.GetSessionCounters(counterCategory, counterIds);
    }

    bool GetSessionInfo(AMDTProfileSessionInfo& sessionInfo)
    {
        return m_dataAdapter.GetSessionInfo(sessionInfo);
    }

    bool GetApuPowerCounterId(int& apuPowerCounterId)
    {
        bool ret = false;

        if (m_apuPowerCounterId > -1)
        {
            apuPowerCounterId = m_apuPowerCounterId;
            ret = true;
        }
        else
        {
            int counterId = -1;
            gtString apuName(L"Total APU Power");

            if (m_dataAdapter.GetCounterIdByName(apuName, counterId))
            {
                apuPowerCounterId = counterId;
                ret = true;
            }
        }

        return ret;
    }

    bool GetSessionSamplingIntervalMs(unsigned& samplingIntervalMs)
    {
        bool ret = true;

        if (m_samplingIntervalMs > 0)
        {
            samplingIntervalMs = m_samplingIntervalMs;
        }
        else
        {
            ret = m_dataAdapter.GetSessionSamplingIntervalMs(samplingIntervalMs);

            if (ret)
            {
                // Cache the sampling interval for future use.
                m_samplingIntervalMs = samplingIntervalMs;
            }
        }

        return ret;
    }

    bool GetSessionCounterIds(gtMap<gtString, int>& counterNames)
    {
        return m_dataAdapter.GetCounterNames(counterNames);
    }

    bool GetAllSessionCountersDescription(gtMap<int, AMDTPwrCounterDesc*>& counterDetails)
    {
        gtMap<int, AMDTProfileCounterDesc> counterDescMap;

        bool ret = m_dataAdapter.GetCountersDescription(counterDescMap);

        if (ret)
        {
            for (auto& counterDescPair : counterDescMap)
            {
                AMDTPwrCounterDesc* pwrCounterDesc = BackendDataConvertor::ConvertToPwrCounterDesc(counterDescPair.second);
                counterDetails[counterDescPair.first] = pwrCounterDesc;
            }
        }

        return ret;
    }

    //++SRB: LPGPU2DEMO_V
    bool GetTraceRecords(gtVector<lpgpu2::db::ProfileTraceInfo>& records) 
    {
        return m_dataAdapter.GetProfileTraceInfo(records);        
    }
    //--SRB: LPGPU2DEMO_V

    //++KB: LPGPU2DEMO_V
    bool GetAnnotations(gtVector<lpgpu2::db::Annotation>& annotations)
    {
      return m_dataAdapter.GetAnnotations(annotations);
    }
    //--KB: LPGPU2DEMO_V

    //++AT: LPGPU2DEMO_V
    bool GetAnnotationById(gtUInt64 annotationId, lpgpu2::db::Annotation& annotation)
    {
      return m_dataAdapter.GetAnnotationById(annotationId, annotation);
    }

    bool GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<lpgpu2::db::Annotation>& vAnnotations)
    {
      return m_dataAdapter.GetAnnotationsInRange(vRangeStart, vRangeEnd, vAnnotations);
    }
    
    bool GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<lpgpu2::db::Annotation>& vAnnotations)
    {
      return m_dataAdapter.GetAnnotationsInFrameRange(vFrameStart, vFrameEnd, vAnnotations);
    }

    bool GetGlobalAnnotations(gtVector<lpgpu2::db::Annotation>& annotations)
    {
      return m_dataAdapter.GetGlobalAnnotations(annotations);
    }

    bool GetUserAnnotations(gtVector<lpgpu2::db::Annotation>& annotations)
    {
      return m_dataAdapter.GetUserAnnotations(annotations);
    }
    
    bool GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces)
    {
      return m_dataAdapter.GetShaderTracesInRange(vRangeStart, vRangeEnd, vShaderTraces);
    }
    
    bool GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces)
    {
      return m_dataAdapter.GetShaderTracesInFrameRange(vFrameStart, vFrameEnd, vShaderTraces);
    }

    bool GetShaderTraces(gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces)
    {
        return m_dataAdapter.GetShaderTraces(vShaderTraces);
    }

    bool GetCallsPerTypeForRegionById(gtUInt64 vRegionId, lpgpu2::db::CallsPerType& vCallsPerType)
    {
      return m_dataAdapter.GetCallsPerTypeForRegionById(vRegionId, vCallsPerType);
    }
    //--AT: LPGPU2DEMO_V
    
    //++CF:LPGPU2
    bool GetRegionsOfInterest(gtVector<lpgpu2::db::RegionOfInterest>& regionsOfInterest)
    {
      return m_dataAdapter.GetRegionsOfInterest(regionsOfInterest);
    }

    bool GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime)
    {
        return m_dataAdapter.GetAccumulatedCallSummaryTime(vAccumulatedTime);
    }

    bool GetCallSummaryForRegionById(gtUInt64 vRegionId, gtVector<lpgpu2::db::CallSummary> &vCallSummaryList)
    {
        return m_dataAdapter.GetCallSummaryForRegionById(vRegionId, vCallSummaryList);
    }
    
    bool GetRegionOfInterestById(gtUInt64 vRegionId, lpgpu2::db::RegionOfInterest &vRegionOfInterest)
    {
        return m_dataAdapter.GetRegionOfInterestByRegionId(vRegionId, vRegionOfInterest);
    }

    bool GetFrameTimeRange(gtUInt64 vFrameNum, gtUInt64 &vFrameStart, gtUInt64 &vFrameEnd)
    {
        return m_dataAdapter.GetFrameTimeRange(vFrameNum, vFrameStart, vFrameEnd);
    }
    
    bool GetNearestSampleValueBeforeTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample)
    {
      return m_dataAdapter.GetNearestSampleValueBeforeTime(vTime, vCounter, vrNearestSample);
    }
    
    bool GetNearestSampleValueAfterTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample)
    {
      return m_dataAdapter.GetNearestSampleValueAfterTime(vTime, vCounter, vrNearestSample);
    }
    
    // It is sometimes needed to fetch counter values at a higher resolution than they are sampled,
    // so the interpolated values of the nearest samples are calculated.
    bool GetApproxCounterSamplesAtTimePoint(gtUInt64 vTime, gtMap<AMDTPwrCounterDesc*,double>& vrCounterSamples)
    {
      auto bOk = true;
      
      // Convert target time to milliseconds:
      const std::chrono::nanoseconds targetTimeNs{ vTime };
      const auto targetTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(targetTimeNs);
      vTime = targetTimeMs.count();
      
      gtMap<int, AMDTProfileCounterDesc> counterDescMap;
      bOk = m_dataAdapter.GetCountersDescription(counterDescMap);

      if (!bOk)
      {
        return false;
      }
      
      for (auto& counterDescPair : counterDescMap)
      {
        const auto counterId = counterDescPair.first;
        const auto counterDesc = counterDescPair.second;
        AMDTPwrCounterDesc* pwrCounterDesc = BackendDataConvertor::ConvertToPwrCounterDesc(counterDesc);
        
        // Lookup nearest samples
        SampledValue sampleBefore{ 0, 0 };
        SampledValue sampleAfter{ 0, 0 };

        bOk = m_dataAdapter.GetNearestSampleValueAfterTime(vTime, counterId, sampleAfter);
        bOk &= m_dataAdapter.GetNearestSampleValueBeforeTime(vTime, counterId, sampleBefore);
            
        if (!bOk)
        {
          return false;
        }
        
        const auto timeBefore = sampleBefore.m_sampleTime;
        const auto timeAfter = sampleAfter.m_sampleTime;
        const auto valueBefore = sampleBefore.m_sampleValue;
        const auto valueAfter = sampleAfter.m_sampleValue;
        
        // If the nearest points are actually at the same time, don't interpolate:
        if (timeBefore == timeAfter)
        {
          vrCounterSamples[pwrCounterDesc] = valueBefore;
        }
        else
        {
          // Otherwise, perform linear interpolation of the values
          const auto totalDiff = timeAfter - timeBefore;
          const auto beforeRatio = (vTime - timeBefore) / static_cast<double>(totalDiff);
          vrCounterSamples[pwrCounterDesc] = beforeRatio * valueBefore + (1.0 - beforeRatio) * valueAfter;
        }
      }
      
      return bOk;
    }
    
    // Get the offset of the start of the trace records from the start of the samples
    bool GetTraceStartTimeOffsetMs(gtInt64& vOffsetMs)
    {
      bool bOk = true;
      gtUInt64 traceStart;
      gtUInt64 sampleStart;
      SamplingTimeRange sampleRange{ 0, 0 };
      
      // Get traces start time
      bOk = m_dataAdapter.GetFirstTraceCallTimestamp(traceStart);
      
      if (!bOk)
      {
        return false;
      }
      const std::chrono::nanoseconds traceStartNs{ traceStart };
      const auto traceStartMs = std::chrono::duration_cast<std::chrono::milliseconds>(traceStartNs);
      
      // Get samples start time
      bOk = GetSessionTimeRange(sampleRange);
      sampleStart = sampleRange.m_fromTime;
      const std::chrono::milliseconds sampleStartMs{ sampleStart };
      
      if (!bOk)
      {
        return false;
      }
      
      // Return offset
      vOffsetMs = traceStartMs.count() - sampleStartMs.count();
      return bOk;
    }
    //--CF:LPGPU2

    //++TLRS: LPGPU2
    /// @brief          Returns if this is a LPGPU2 session or not.
    /// @returns bool   true: This is a LPGPU2 session,
    ///                 false: This is not a LPGPU2 session.
    bool IsLPGPU2Session()
    {
        auto bIsLPGPU2TargetDevice = false;
        if (m_dataAdapter.IsLPGPU2TargetDevice(bIsLPGPU2TargetDevice))
        {
            return bIsLPGPU2TargetDevice;
        }

        return false;
    }

    bool GetFeedbackEngineForSession(std::unique_ptr<lpgpu2::db::LPGPU2FeedbackEngine> &vFeedbackEnginePtr)
    {
        vFeedbackEnginePtr.reset(new lpgpu2::db::LPGPU2FeedbackEngine{ m_dataAdapter });
        return vFeedbackEnginePtr != nullptr;
    }
    
    bool GetShaderToAsmMappings(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm)
    {
        return m_dataAdapter.GetShaderToAsmByShaderId(vShaderId, vShaderToAsm);
    }

    bool GetShaderAsmStats(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderAsmStat> &vShaderAsmStats)
    {
        return m_dataAdapter.GetShaderAsmStatsByShaderId(vShaderId, vShaderAsmStats);
    }

    void SimulateShaderCounterMatchScript(gtUInt64 vShaderId, gtUInt64 vApiId, gtUInt64 vCounterId)
    {
        (void) vCounterId;

        // Look for the shader
        gtVector<lpgpu2::db::ShaderTrace> shaderTraces;
        m_dataAdapter.GetShaderTraces(shaderTraces);

        const auto stIt = std::find_if(shaderTraces.begin(), shaderTraces.end(), [=](const lpgpu2::db::ShaderTrace &st)
        {
            return st.m_shaderId == vShaderId && st.m_apiId == static_cast<lpgpu2::db::EShaderTraceAPIId>(vApiId);
        });

        if (stIt != shaderTraces.end())
        {
            // Read the shader assembly
            const auto& asmCode = stIt->m_asmCode;
            
            // For each line, generate a ShaderAsmStat object
            const auto numberOfAsmLines = asmCode.count(L'\n');

            std::random_device rd;
            std::mt19937 eng(rd());
            std::uniform_real_distribution<gtFloat32> dist{ 0.0f, 100.0f };

            gtVector<lpgpu2::db::ShaderAsmStat> shaderAsmStats;
            for (auto iLineIdx = 1; iLineIdx <= numberOfAsmLines; ++iLineIdx)
            {
                lpgpu2::db::ShaderAsmStat asmStat;
                asmStat.m_shaderId = vShaderId;
                asmStat.m_apiId = static_cast<lpgpu2::db::EShaderTraceAPIId>(vApiId);
                asmStat.m_asmLine = iLineIdx;
                
                // Generate some random percentage
                asmStat.m_percentage = dist(eng);

                shaderAsmStats.push_back(asmStat);
            }

            m_dataAdapter.UpdateShaderAsmStats(shaderAsmStats);

            // Generate some ShaderToAsm if there is none
            gtVector<lpgpu2::db::ShaderToAsm> shaderToAsm;
            m_dataAdapter.GetShaderToAsmByShaderId(vShaderId, shaderToAsm);

            if (shaderToAsm.empty())
            {
                // For each line of shader src code, generate a ShaderToAsm
                const auto numberOfShaderCodeLines = stIt->m_srcCode.count(L'\n');

                for (auto iLineIdx = 1; iLineIdx < numberOfShaderCodeLines; ++iLineIdx)
                {
                    lpgpu2::db::ShaderToAsm sToA;
                    sToA.m_shaderId = vShaderId;
                    sToA.m_shaderLine = iLineIdx;
                    sToA.m_asmStartLine = iLineIdx % numberOfAsmLines;
                    sToA.m_asmEndLine = std::max(iLineIdx % numberOfAsmLines + 2, iLineIdx % numberOfAsmLines);

                    m_dataAdapter.InsertShaderToAsm(sToA);
                }                
            }
        }

    }
    //--TLRS: LPGPU2

private:
    lpgpu2::db::LPGPU2DatabaseAdapter m_dataAdapter;

    // Holds the Power counters which were enabled for this session.
    gtSet<int> m_sessionSelectedPowerCounters;

    // Holds the APU power counter ID.
    int m_apuPowerCounterId;
    
    // Holds the sampling interval.
    unsigned int m_samplingIntervalMs;
};


PowerProfilerBL::PowerProfilerBL() : m_pImpl(new PowerProfilerBL::Impl())
{
}


PowerProfilerBL::~PowerProfilerBL()
{
    delete m_pImpl;
    m_pImpl = NULL;
}

bool PowerProfilerBL::OpenPowerProfilingDatabaseForRead(const gtString& dbName, bool vbIsReadOnly)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->OpenPowerProfilingDatabaseForRead(dbName, vbIsReadOnly);
    }
    return ret;
}

bool PowerProfilerBL::GetSessionTimeRange(SamplingTimeRange& samplingTimeRange)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionTimeRange(samplingTimeRange);
    }
    return ret;
}


bool PowerProfilerBL::GetCurrentCumulativeEnergyConsumptionInJoule(const gtVector<int>& counterIds,
                                                                   gtMap<int, double>& consumptionPerCounterInJoule, double& otherCumulativeConsumption)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetCurrentCumulativeEnergyConsumptionInJoule(counterIds, consumptionPerCounterInJoule, otherCumulativeConsumption);
    }
    return ret;
}


bool PowerProfilerBL::GetCurrentAveragePowerConsumptionInWatt(const gtVector<int>& counterIds, unsigned int samplingIntervalMs,
                                                              gtMap<int, double>& consumptionPerCounterInWatt, double& otherCounterConsumptionInWatt)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetCurrentAveragePowerConsumptionInWatt(counterIds, samplingIntervalMs, consumptionPerCounterInWatt, otherCounterConsumptionInWatt);
    }
    return ret;
}

bool PowerProfilerBL::GetSampledValuesByRange(const gtVector<int>& counterIds,
                                              SamplingTimeRange& samplingTimeRange, gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSampledValuesByRange(counterIds, samplingTimeRange, sampledValuesPerCounter);
    }
    return ret;
}

//++CF:LPGPU2
bool PowerProfilerBL::GetSampledValuesByNanoSecRange(const gtVector<int>& counterIds,
                                              SamplingTimeRange& samplingTimeRange, gtMap<int, gtVector<SampledValue>>& sampledValuesPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSampledValuesByNanoSecRange(counterIds, samplingTimeRange, sampledValuesPerCounter);
    }
    return ret;
}
//--CF:LPGPU2

bool PowerProfilerBL::GetGlobalMinMaxValuesPerCounters(const gtVector<int> counterIds,
                                                       SamplingTimeRange& samplingTimeRange, double& minValue, double& maxValue)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetGlobalMinMaxValuesPerCounters(counterIds, samplingTimeRange, minValue, maxValue);
    }
    return ret;
}

bool PowerProfilerBL::GetOverallNubmerOfSamples(const gtVector<int>& counterIds, gtMap<int, int>& numberOfSamplesPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetOverallNubmerOfSamples(counterIds, numberOfSamplesPerCounter);
    }
    return ret;
}

bool PowerProfilerBL::Refresh()
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->Refresh();
    }
    return ret;
}


bool PowerProfilerBL::GetCurrentFrequenciesHistogram(unsigned int bucketWidth, const gtVector<int>& counterIds,
                                                     gtMap<int, gtVector<HistogramBucket>>& bucketPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetCurrentFrequenciesHistogram(bucketWidth, counterIds, bucketPerCounter);
    }
    return ret;
}


bool PowerProfilerBL::UpdateCumulativeAndAverageHistograms(const gtMap<int, PPSampledValuesBatch>& newSamples, unsigned currSamplingInterval,
                                                           gtMap<int, double>& accumulatedEnergyInJoule, double& cumulativeOtherCounterValue, gtMap<int, double>& averagePowerInWatt, double& averageOtherCounterValueWatt,
                                                           gtMap<int, unsigned>& aggregatedQuantizedClockTicks)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->UpdateCumulativeAndAverageHistograms(newSamples, currSamplingInterval, accumulatedEnergyInJoule,
                                                            cumulativeOtherCounterValue, averagePowerInWatt, averageOtherCounterValueWatt, aggregatedQuantizedClockTicks);
    }
    return ret;
}

bool PowerProfilerBL::GetDeviceType(int deviceId, AMDTDeviceType& deviceType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetDeviceType(deviceId, deviceType);
    }
    return ret;
}

const PPDevice* PowerProfilerBL::GetDevice(int deviceID, const PPDevice* pRootDevice)
{
    const PPDevice* deviceThatWasFound = NULL;

    if (NULL != pRootDevice)
    {
        // If we found a device with a matching device ID
        if (pRootDevice->m_deviceId == deviceID)
        {
            deviceThatWasFound = pRootDevice;
        }
        else
        {
            // Look for a match among the subdevices
            for (const PPDevice* pSubDevice : pRootDevice->m_subDevices)
            {
                deviceThatWasFound = GetDevice(deviceID, pSubDevice);

                if (deviceThatWasFound != NULL)
                {
                    break;
                }
            }
        }
    }

    return deviceThatWasFound;
}

bool PowerProfilerBL::GetSessionCounters(AMDTDeviceType deviceType, AMDTPwrCategory counterCategory, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionCounters(deviceType, counterCategory, counterIds);
    }
    return ret;
}

bool PowerProfilerBL::GetSessionCounters(AMDTPwrCategory counterCategory, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionCounters(counterCategory, counterIds);
    }
    return ret;
}

bool PowerProfilerBL::GetSessionCounters(const gtVector<AMDTDeviceType>& deviceTypes, AMDTPwrCategory counterCategory, gtVector<int>& counterIds)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionCounters(deviceTypes, counterCategory, counterIds);
    }
    return ret;
}

bool PowerProfilerBL::GetSessionInfo(AMDTProfileSessionInfo& sessionInfo)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionInfo(sessionInfo);
    }
    return ret;
}

bool PowerProfilerBL::GetApuPowerCounterId(int& apuPowerCounterId)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetApuPowerCounterId(apuPowerCounterId);
    }
    return ret;

}

bool PowerProfilerBL::CloseAllConnections()
{
    delete m_pImpl;
    m_pImpl = new PowerProfilerBL::Impl();
    return true;
}

bool PowerProfilerBL::CalculateOnlineFrequencyHistograms(unsigned int bucketWidth, unsigned int currSamplingInterval, const gtMap<int, PPSampledValuesBatch>& newSamples,
                                                         const gtVector<int>& relevantCounterIds, gtMap<int, gtVector<HistogramBucket>>& bucketsPerCounter)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->CalculateOnlineFrequencyHistograms(bucketWidth, currSamplingInterval, newSamples, relevantCounterIds, bucketsPerCounter);
    }
    return ret;
}

bool PowerProfilerBL::GetSessionSamplingIntervalMs(unsigned& samplingIntervalMs)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionSamplingIntervalMs(samplingIntervalMs);
    }
    return ret;
}

bool PowerProfilerBL::GetSessionCounterIdByName(gtMap<gtString, int>& counterNames)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionCounterIds(counterNames);
    }
    return ret;
}

bool PowerProfilerBL::GetAllSessionCountersDescription(gtMap<int, AMDTPwrCounterDesc*>& counterDetails)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetAllSessionCountersDescription(counterDetails);
    }
    return ret;
}

//++SRB: LPGPU2
bool PowerProfilerBL::GetTraceRecords(gtVector<lpgpu2::db::ProfileTraceInfo>& records) {
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetTraceRecords(records);
    }
    return ret;
}
//--SRB: LPGPU2

//++KB: LPGPU2DEMO_V
bool PowerProfilerBL::GetAnnotations(gtVector<lpgpu2::db::Annotation>& annotations)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
            ret = m_pImpl->GetAnnotations(annotations);
    }
    return ret;
}
//--KB: LPGPU2DEMO_V

//++AT: LPGPU2DEMO_V
bool PowerProfilerBL::GetAnnotation(gtUInt64 annotationId, lpgpu2::db::Annotation& annotation)
{
  bool ret = false;
  GT_IF_WITH_ASSERT(m_pImpl != NULL)
  {
    ret = m_pImpl->GetAnnotationById(annotationId, annotation);
  }
  return ret;
}

bool PowerProfilerBL::GetAnnotationsInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<lpgpu2::db::Annotation>& vAnnotations)
{
  bool bReturn = false;
  GT_IF_WITH_ASSERT(m_pImpl != NULL)
  {
    bReturn = m_pImpl->GetAnnotationsInRange(vRangeStart, vRangeEnd, vAnnotations);
  }
  return bReturn;
}

bool PowerProfilerBL::GetAnnotationsInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<lpgpu2::db::Annotation>& vAnnotations)
{
  bool bReturn = false;
  GT_IF_WITH_ASSERT(m_pImpl != NULL)
  {
    bReturn = m_pImpl->GetAnnotationsInFrameRange(vFrameStart, vFrameEnd, vAnnotations);
  }
  return bReturn;
}

bool PowerProfilerBL::GetGlobalAnnotations(
    gtVector<lpgpu2::db::Annotation>& annotations)
{
	bool ret = false;
	GT_IF_WITH_ASSERT(m_pImpl != NULL)
	{
		ret = m_pImpl->GetGlobalAnnotations(annotations);
	}
	return ret;
}

bool PowerProfilerBL::GetUserAnnotations(
    gtVector<lpgpu2::db::Annotation>& annotations)
{
	bool ret = false;
	GT_IF_WITH_ASSERT(m_pImpl != NULL)
	{
		ret = m_pImpl->GetUserAnnotations(annotations);
	}
	return ret;
}


bool PowerProfilerBL::GetShaderTracesInRange(gtUInt64 vRangeStart, gtUInt64 vRangeEnd, gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetShaderTracesInRange(vRangeStart, vRangeEnd, vShaderTraces);
    }
    return bReturn;
}

bool PowerProfilerBL::GetShaderTracesInFrameRange(gtUInt64 vFrameStart, gtUInt64 vFrameEnd, gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetShaderTracesInFrameRange(vFrameStart, vFrameEnd, vShaderTraces);
    }
    return bReturn;
}

bool PowerProfilerBL::GetCallsPerTypeByRegionId(gtUInt64 vRegionId, lpgpu2::db::CallsPerType &vCallsPerType)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        ret = m_pImpl->GetCallsPerTypeForRegionById(vRegionId, vCallsPerType);
    }
    return ret;
}

bool PowerProfilerBL::GetShaderTraces(gtVector<lpgpu2::db::ShaderTrace>& vShaderTraces)
{
    auto ret = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        ret = m_pImpl->GetShaderTraces(vShaderTraces);
    }

    return ret;
}
//--AT: LPGPU2DEMO_V

//++CF:LPGPU2
bool PowerProfilerBL::GetRegionsOfInterest(gtVector<lpgpu2::db::RegionOfInterest>& regionsOfInterest)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
            ret = m_pImpl->GetRegionsOfInterest(regionsOfInterest);
    }
    return ret;
}

bool PowerProfilerBL::GetAccumulatedCallSummaryTime(gtUInt64 &vAccumulatedTime)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetAccumulatedCallSummaryTime(vAccumulatedTime);
    }
    return bReturn;
}

bool PowerProfilerBL::GetCallSummaryForRegionId(gtUInt64 vRegionId, gtVector<lpgpu2::db::CallSummary>& vCallSummaryList)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetCallSummaryForRegionById(vRegionId, vCallSummaryList);
    }
    return bReturn;
}

bool PowerProfilerBL::GetRegionOfInterestById(gtUInt64 vRegionId, lpgpu2::db::RegionOfInterest& vRegionOfInterest)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetRegionOfInterestById(vRegionId, vRegionOfInterest);
    }
    return bReturn;
}

bool PowerProfilerBL::GetFrameTimeRange(gtUInt64 vFrameNum, gtUInt64 &vFrameStart, gtUInt64 &vFrameEnd)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetFrameTimeRange(vFrameNum, vFrameStart, vFrameEnd);        
    }
    return bReturn;
}

bool PowerProfilerBL::GetApproxCounterSamplesAtTimePoint(gtUInt64 vTime, gtMap<AMDTPwrCounterDesc*,double>& vrCounterSamples)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetApproxCounterSamplesAtTimePoint(vTime, vrCounterSamples);
    }
    return bReturn;
}

bool PowerProfilerBL::GetNearestSampleValueBeforeTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetNearestSampleValueBeforeTime(vTime, vCounter, vrNearestSample);
    }
    return bReturn;
}

bool PowerProfilerBL::GetNearestSampleValueAfterTime(gtUInt64 vTime, gtInt32 vCounter, SampledValue& vrNearestSample)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetNearestSampleValueAfterTime(vTime, vCounter, vrNearestSample);
    }
    return bReturn;
}

bool PowerProfilerBL::GetTraceStartTimeOffsetMs(gtInt64& vOffsetMs)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetTraceStartTimeOffsetMs(vOffsetMs);
    }
    return bReturn;
}


bool PowerProfilerBL::GetEstimatedCounterFrequency(gtInt32 vCounterId, double& vFreq)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetEstimatedCounterFrequency(vCounterId, vFreq);
    }
    return bReturn;
}
//--CF:LPGPU2

//++TLRS: LPGPU2
/// @brief          Returns if this is a LPGPU2 session or not.
/// @returns bool   true: This is a LPGPU2 session,
///                 false: This is not a LPGPU2 session.
bool PowerProfilerBL::IsLPGPU2Session() const
{
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        return m_pImpl->IsLPGPU2Session();
    }

    return false;
}
bool PowerProfilerBL::GetFeedbackEngineForSession(std::unique_ptr<lpgpu2::db::LPGPU2FeedbackEngine>& vFeedbackEnginePtr) const
{
    auto bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        bReturn = m_pImpl->GetFeedbackEngineForSession(vFeedbackEnginePtr);
    }

    return bReturn;
}

bool PowerProfilerBL::GetShaderToAsmMappings(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm)
{
    auto bReturn = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        bReturn = m_pImpl->GetShaderToAsmMappings(vShaderId, vShaderToAsm);
    }

    return bReturn;
}

bool PowerProfilerBL::GetShaderAsmStats(gtUInt64 vShaderId, gtVector<lpgpu2::db::ShaderAsmStat> &vShaderAsmStats)
{
    auto bReturn = false;

    GT_IF_WITH_ASSERT(m_pImpl != nullptr)
    {
        bReturn = m_pImpl->GetShaderAsmStats(vShaderId, vShaderAsmStats);
    }

    return bReturn;
}

void PowerProfilerBL::SimulateShaderCounterMatchScript(gtUInt64 vShaderId, gtUInt64 vApiId, gtUInt64 vCounterId)
{
    m_pImpl->SimulateShaderCounterMatchScript(vShaderId, vApiId, vCounterId);
}
//--TLRS: LPGPU2

//++AT:LPGPU2
/// @brief Return all the devices in the system inside a vector
/// @param devices The returned vector of devices
/// @returns bool   true: This is a LPGPU2 session,
///                 false: This is not a LPGPU2 session.
bool PowerProfilerBL::GetAllDevicesAsVector(gtVector<PPDevice> &devices)
{
    bool bReturn = false;
    GT_IF_WITH_ASSERT(m_pImpl != NULL)
    {
        bReturn = m_pImpl->GetAllDevicesAsVector(devices);
    }
    return bReturn;
}

/// @brief Return all the trace stack parameters entries from the db
/// @param records The returned vector of parameter entries
/// @returns bool   true: This is a LPGPU2 session,
///                 false: This is not a LPGPU2 session.
bool PowerProfilerBL::GetStackTraceParameters(
    gtVector<lpgpu2::db::StackTraceParameter>& records) 
{
  bool bReturn = false;
  GT_IF_WITH_ASSERT(m_pImpl != NULL)
  {
    bReturn = m_pImpl->GetStackTraceParameters(records);
  }
  return bReturn;
}
//--AT:LPGPU2
