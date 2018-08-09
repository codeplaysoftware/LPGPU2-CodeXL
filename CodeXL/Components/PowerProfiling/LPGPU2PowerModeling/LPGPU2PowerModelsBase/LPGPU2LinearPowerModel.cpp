/*
 * lpgpuLinearPowerModel.cc
 *
 *  Created on: Nov 13, 2017
 *      Author: Sohan Lal
 *      Research Assistant
 *      Technical University of Berlin
 */

#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2LinearPowerModel.h>

#include <AMDTBaseTools/Include/gtAssert.h>

//#include "Common/Src/AMDTCommonHeaders/AMDTCommonProfileDataTypes.h"

// AMDTDbAdapter:
//#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

//LPGPU2DatabaseAccessor* lpgpuLinearPowerModel::GetDatabaseAccessor() const
//{
//    auto pTraceDbAccessor = dynamic_cast<LPGPU2DatabaseAccessor*>(m_pDbAccessor);

//    if (m_pDbAccessor != nullptr)
//    	return pTraceDbAccessor;
//    else
//    	return nullptr;
//}

void lpgpuLinearPowerModel::estimatePower() {

    // Get sampling time range to estimate power for
    SamplingTimeRange samplingTimeRange(0,0);

    GT_IF_WITH_ASSERT_EX(mpDbAdapter->GetSessionTimeRange(samplingTimeRange), L"Cannot get session time range") {

    }



    // Get the Ids for all required counters
    std::map<std::string,int> reqCountersMap;
    // Vector of counters id for the linear power model
    gtVector<int> counterIds;

    gtString name;

    for (auto& reqCounter : mDescriptor.requiredCounters) {
        counterIds.push_back(reqCounter.counterId);

        if (reqCounter.modelMappedName.compare(name.fromUtf8String("renderer_active") ) == 0) {
            reqCountersMap["renderer_active"] = reqCounter.counterId;

        } else if (reqCounter.modelMappedName.compare(name.fromUtf8String("tiler_active") ) == 0) {
            reqCountersMap["tiler_active"] = reqCounter.counterId;

        } else if (reqCounter.modelMappedName.compare(name.fromUtf8String("compute_active") ) == 0) {
            reqCountersMap["compute_active"] = reqCounter.counterId;

        }
    }

    // Get parameters for linear power model from XML file
    // TODO: Call a method to get them

    float coefficients[4];

    unsigned i = 0;
    for (auto& parameter : mDescriptor.parameters) {
        if (parameter.type.compare(name.fromUtf8String("float")) == 0) 
        {            
            coefficients[i++] = std::stof(parameter.value.asASCIICharArray());
        }		
    }

    gtMap<int, gtVector<SampledValue>>sampledValuesPerCounter;

    GT_IF_WITH_ASSERT_EX(mpDbAdapter->GetSampledValuesByRange(counterIds, samplingTimeRange, sampledValuesPerCounter), L"Cannot get sampled values by range") {

        double perfCountHwCpuCycles, gpuUtil, computeActive;
        
        gtUInt64 sampleTime;

        // TODO: could get number of values for each counter id
        unsigned numOfValues;

        numOfValues = sampledValuesPerCounter[reqCountersMap["compute_active"]].size(); // FIXME

        // Loop over all sampled values of counters and estimate power
        mpDbAdapter->BeginTransaction();
        for (unsigned i = 0; i < numOfValues; ++i) {

            // Replace 0, 1, 2 with counter ids from XML
            perfCountHwCpuCycles = sampledValuesPerCounter[reqCountersMap["compute_active"]][i].m_sampleValue;
            gpuUtil = sampledValuesPerCounter[reqCountersMap["tiler_active"]][i].m_sampleValue;
            computeActive = sampledValuesPerCounter[reqCountersMap["compute_active"]][i].m_sampleValue;

            sampleTime = sampledValuesPerCounter[reqCountersMap["compute_active"]][i].m_sampleTime;

            // Write power estimation equation here
            lpgpu2::db::EstimatedPowerSample pwrSample;

            // Change with linear power model here
            const double estimatedPower = coefficients[0] * perfCountHwCpuCycles + coefficients[1] * gpuUtil + coefficients[2] * computeActive + coefficients[3];
            pwrSample.m_estimatedPower = static_cast<gtFloat32>(estimatedPower);

            pwrSample.m_sampleTime = sampleTime;

            // Get from XML parsing
            //pwrSample.m_modelId = mDescriptor.modelUuid.toIntNumber(); // FIXME
            pwrSample.m_modelId = 0;
            
            // Get from XML parsing
            pwrSample.m_unit = L"mWatts"; // FIXME

            // Get from XML parsing
            pwrSample.m_componentId = 0;

            // Write estimated power sample to database
            GT_ASSERT(mpDbAdapter->InsertEstimatedPowerSample(pwrSample));

            
            //Write the samples table
            gtVector<AMDTProfileTimelineSample*> ppSample;

            AMDTProfileCounterValue counterData;
            counterData.m_counterId = 4178; //4819;
            counterData.m_counterValue = pwrSample.m_estimatedPower;

            AMDTProfileTimelineSample sample;
            sample.m_sampleSystemTime = AMDTProfileSystemTime();
            sample.m_sampleElapsedTimeMs = pwrSample.m_sampleTime;
            sample.m_sampleValues.push_back(counterData);
            ppSample.push_back(&sample);
            
            auto ret = mpDbAdapter->InsertSamples(ppSample);

            if (!ret)
            {
                // ...
            }
            
        }
        mpDbAdapter->EndTransaction();

        // TODO add a component with ID 0 called estimated whole power to the components table
        // if it's not already added

    }

    // Read samples table for three counters 1)PERF_COUNT_HW_CPU_CYCLES 2) gpu_util 3) Compute active using method: APIs from AMDTDatabaseAccesor
    // Power sample insertion method InsertSamples(). Method to read samples unclear
    // Get all counters
    // Get counters with required counter ids for power model

    // Estimate power using liner power model
    // Write estimated power using method: bool InsertEstimatedPowerSample(const EstimatedPowerSample& vEstimatedPwrSample)

}



