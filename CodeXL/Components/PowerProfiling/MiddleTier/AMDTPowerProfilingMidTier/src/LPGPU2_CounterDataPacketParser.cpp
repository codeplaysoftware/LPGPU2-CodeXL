// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_CounterDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles COUNTER (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_CounterDataPacketParser.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// STL
// Introduces:
// std::move
// assert
// std::find
// std::duration
#include <utility>
#include <cassert>
#include <algorithm>
#include <chrono>

namespace lpgpu2 {

/// @brief Ctor; obtains the list of counters for later use.
/// @param pDataAdapter Handle to the database connection object
/// @param dataHandler Data ready callback, used to send data to the
///        visualisations
/// @param pDataHandlerParams Params to pass to the callback function
/// @param pPPcore Ptr to the Power Profiler Core class instance
CounterDataPacketParser::CounterDataPacketParser(
    db::LPGPU2DatabaseAdapter* pDataAdapter,
    PPSamplesDataHandlerFunc dataHandler,
    void *pDataHandlerParams,
    PowerProfilerCore* pPPcore,
    // blobSize = 0 in Ctor declaration
    gtUInt32 blobSize)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter},
  m_dataHandler{std::move(dataHandler)},
  m_pDataHandlerParams{pDataHandlerParams}, m_pPPcore{pPPcore},
  m_enabledCounters{}, m_samplesBuffer{}, m_blobSize{blobSize}
{
  m_pPPcore->GetEnabledCounters(m_enabledCounters);
}

/// @brief  Used by the LPGPU2_PPAndroidPollingThread to map a power profiling
///         sample (which contains samples per-counter) to the counters.
/// @warning Not used at the moment; will be used and updated once the protocol
///          (remote SAMS ANDROID) is fully defined
/// @param beSample The single sample which contains data from multiple counters
/// @param outMap Ref to a gtMap containing samples per-counter
void CreateCounterToSampleMap(const AMDTProfileTimelineSample& beSample,
    gtMap<int, PPSampledValuesBatch>& outMap)
{
  const gtVector<AMDTProfileCounterValue> &beValsVector =
    beSample.m_sampleValues;

  for (size_t i = 0; i < beValsVector.size(); ++i)
  {
    AMDTUInt32 currCounterId = beValsVector[i].m_counterId;
    auto iter = outMap.find(currCounterId);

    if (iter == outMap.end())
    {
      // If we don't yet have a batch for that counter id, let's create it.
      outMap[currCounterId] =
        PPSampledValuesBatch(beSample.m_sampleElapsedTimeMs);
      iter = outMap.find(currCounterId);
    }

    // Add the value to the counter's sampled values batch.
    iter->second.m_sampleValues.push_back(
        beValsVector[i].m_counterValue);
  }
}

/// @brief Consume the data contained in bp, write data to BD, call
///        visualisation callbacks.
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus CounterDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  // Read the time at which the sample was taken
  //
  // We do not use the READ_TIME macro here because the counter time is in ms
  // and all the other counters use ns, so we convert it to ns before
  // distributing it to the other parsers
  gtUInt64 sampleTime = 0U;
  READ_TIME(sampleTime);

  // Read the number of enabled counters
  gtUInt32 numEnabledCounters = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(numEnabledCounters);

  // Read the value of the counters
  m_samplesBuffer.push_back(AMDTProfileTimelineSample{});
  auto *sample = &m_samplesBuffer.back();
  // Convert back to ms before sending data to graphs
  auto sampleElapsedTimeMs =
    std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::nanoseconds{sampleTime}).count();
  sample->m_sampleElapsedTimeMs = sampleElapsedTimeMs;

  db::CounterSampleBlobsVec blobs;

  // Then read the ones which are active and set their value
  for (auto i = 0U; i < numEnabledCounters; ++i)
  {
    // Counter ID
    gtUInt32 counterId = 0U;
    READ_BYTES_AND_CHECK_NO_SWAP(counterId);

    // Value
    gtUInt32 v = 0U;
    READ_BYTES_AND_CHECK_NO_SWAP(v);

    sample->m_sampleValues.push_back(AMDTProfileCounterValue {
        counterId,
        static_cast<AMDTFloat32>(v)});

    // Blob
    PPSampleData::BlobVec binaryBlob;
    if (m_blobSize > 0)
    {
        binaryBlob.resize(m_blobSize);

        memcpy(binaryBlob.data(), &bp.data[bp.off], m_blobSize);
        bp.off += m_blobSize;
    }   

    blobs.emplace_back(std::move(binaryBlob));
  }

  // We have to use a vector with raw ptrs here because that is how CXL
  // does it normally and we have to respect the iface
  SamplesPtrVec dbSamplesBuffer;
  dbSamplesBuffer.push_back(sample);

  auto rcDb = m_pDataAdapter->InsertSamples(dbSamplesBuffer);
  if (!rcDb)
  {
    auto errMsg = m_pDataAdapter->GetErrorMessage();
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Errmsg: %s",
        errMsg.asASCIICharArray());
    return PPFnStatus::failure;
  }
  rcDb = m_pDataAdapter->InsertLPGPU2CounterSampleBlobs(blobs);
  if (!rcDb)
  {
    auto errMsg = m_pDataAdapter->GetErrorMessage();
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Errmsg: %s",
        errMsg.asASCIICharArray());
    return PPFnStatus::failure;
  }

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus CounterDataPacketParser::FlushDataImpl()
{
  // Send the data to the graphs via callback
  for (auto &i : m_samplesBuffer)
  {
    // Create a map to hold the sample values by counter id.
    auto pCounterIdToSampleMap =
      std::make_shared<gtMap<int, PPSampledValuesBatch>>();

    // Fill the counter to sample map.
    CreateCounterToSampleMap(i, *pCounterIdToSampleMap);

    // Raise the event.
    m_dataHandler(pCounterIdToSampleMap, m_pDataHandlerParams);
  }

  m_samplesBuffer.clear();

// TODO Investigate why this method locks the DB and stops from calling
// read methods
// TODO Uncomment once above is done
  //auto rcDb = m_pDataAdapter->FlushDb();
  //if (!rcDb)
  //{
    //return PPFnStatus::failure;
  //}

  return PPFnStatus::success;
}

} // namespace lpgpu2
