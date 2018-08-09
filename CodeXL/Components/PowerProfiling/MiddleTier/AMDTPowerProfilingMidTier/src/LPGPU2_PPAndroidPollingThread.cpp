// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Power Profiling backend to be used when the project is to profile
///        on an Android remote device.
/// @warning Some of the iface had to be implemented even though it is not
///          necessary for the Android profiling. This was necessary to conform
///          with the iface expected by CXL of a Backend. In such cases, the
///          methods either return a NOT_SUPPORTED code or simply a success
///          code in cases in which it would cause CXL to stop if an error code
///          was returned.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTPowerProfilingMidTier/include/LPGPU2_PPAndroidPollingThread.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_AndroidBackendAdapter.h>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// STL
// Introduce:
// std::unique_ptr
// std::vector
// std::move
#include <memory>
#include <vector>
#include <utility>
#include <chrono>

// Remote
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_DataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_CounterDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_TraceParamsDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_TraceDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_ShaderDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_ShaderMetaDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_AnnotationsDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_GPUTimingDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_StackDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>

#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

// Constants
const auto kDurationDatabaseFlush = std::chrono::milliseconds{50};
const auto kDurationDataFlush = std::chrono::seconds{5};

namespace lpgpu2 {

/// @brief Class ctor. Initialise *this; no work is done here
PPAndroidPollingThread::PPAndroidPollingThread(
  unsigned pollingInterval,
  PPSamplesDataHandler cb,
  void* pDataCbParams,
  PPFatalErrorHandler cbErr,
  void* pErrorCbParams,
  PowerProfilerCore* pPPcore,
  db::LPGPU2DatabaseAdapter* pDataAdapter)
: PPPollingThread{pollingInterval, cb, pDataCbParams, cbErr, pErrorCbParams,
    nullptr, pDataAdapter, true},
  m_pPPcore{pPPcore}, m_pDataParser{new lpgpu2::DataPacketParser}
{
  m_profResult = PPR_NO_ERROR;
  m_profilingErr = false;
}

/// @brief Called by the polling thread manager after the thread has been
///        spawned. Represents the work done by the thread
/// @warning The return code is always 0 because that is how CXL expects it.
/// @return The return status of the function once it completes. Note: always 0!
int PPAndroidPollingThread::entryPoint()
{
  GT_IF_WITH_ASSERT((m_pDataAdapter != NULL) && (m_dataCb != NULL))
  {
    auto *CXLDmnCli = CXLDaemonClient::GetInstance();
    const auto *pAndroidBackend =
      static_cast<const lpgpu2::AndroidBackendAdapter *>(
          m_pPPcore->GetBEAdapter());

    // Send collection definitions
    gtString collDefsFileName =
      pAndroidBackend->GetCollectionDefs()->GetFileName();
    GT_ASSERT(collDefsFileName.length() != 0);
    if(collDefsFileName.length() == 0)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }

    // The app to launch for profiling
    gtString appName =
      pAndroidBackend->GetCollectionDefs()->GetProcessName();
    OS_OUTPUT_DEBUG_LOG(L"At start of entrypoint", OS_DEBUG_LOG_INFO);


    if (CXLDmnCli && CXLDmnCli->LPGPU2_SendAndroidCollectionOptions(collDefsFileName) &&
        CXLDmnCli->LPGPU2_StartAndroidCollection(appName))
    {
      m_IsStopped = false;
    }
    else
    {
      m_profilingErr = true;
      m_profResult = PPR_UNKNOWN_FAILURE;
      m_IsStopped = true;
      return 0;
    }

    if (!m_IsStopped)
    {
      // We set the values for the status variables checked by CXL here
      // after we know that the android collection started successfully
      m_sessionStartedStatus = PPR_NO_ERROR;
      m_targetAppLaunchStatus = rasOk;
    }

    // Using polymorphism here
    lpgpu2::BDPUPtr counterParser{new (std::nothrow) lpgpu2::CounterDataPacketParser{
      GetDatabaseAdapter(), m_dataCb,
        m_pDataCbParams, m_pPPcore,
        pAndroidBackend->GetTargetDefinition()->GetTargetElement().GetBlobSize()}};
    if (!counterParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr traceParamsParser{new (std::nothrow) lpgpu2::TraceParamsDataPacketParser{
        // We can safely cast here because we know that in LPGPU2 CXL the
        // database adapter created by the PowerProfilerCore is of type
        // LPGPU2DatabaseAdapter
        GetDatabaseAdapter(),
        pAndroidBackend->GetTargetCharacteristics()
    }};
    if (!traceParamsParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr tracesParser{new (std::nothrow) lpgpu2::TraceDataPacketParser{
        // We can safely cast here because we know that in LPGPU2 CXL the
        // database adapter created by the PowerProfilerCore is of type
        // LPGPU2DatabaseAdapter
        GetDatabaseAdapter(),
        pAndroidBackend->GetTargetCharacteristics()
    }};
    if (!tracesParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr shadersParser{new (std::nothrow) lpgpu2::ShaderDataPacketParser{
        GetDatabaseAdapter()
    }};
    if (!shadersParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr gpuTimingParser{new (std::nothrow) lpgpu2::GPUTimingDataPacketParser{
        GetDatabaseAdapter()
    }};
    if (!gpuTimingParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr annotationsParser{new (std::nothrow) lpgpu2::AnnotationsDataPacketParser{
        GetDatabaseAdapter()
    }};
    if (!annotationsParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr stackParser{new (std::nothrow) lpgpu2::StackDataPacketParser{
        GetDatabaseAdapter()
    }};
    if (!stackParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }
    lpgpu2::BDPUPtr shaderMetaParser{new (std::nothrow) lpgpu2::ShaderMetaDataPacketParser{
        GetDatabaseAdapter()
    }};
    if (!shaderMetaParser)
    {
        m_profilingErr = true;
        m_profResult = PPR_UNKNOWN_FAILURE;
        m_IsStopped = true;

        return 0;
    }

    // Associate a given parser with a type of packet
    m_pDataParser->AddHandler(DataChunkType::COUNTERV2,
        std::move(counterParser));
    m_pDataParser->AddHandler(DataChunkType::PARAMS,
        std::move(traceParamsParser));
    m_pDataParser->AddHandler(DataChunkType::TRACE,
        std::move(tracesParser));
    m_pDataParser->AddHandler(DataChunkType::SHADER,
        std::move(shadersParser));
    m_pDataParser->AddHandler(DataChunkType::GPUTIMER,
        std::move(gpuTimingParser));
    m_pDataParser->AddHandler(DataChunkType::ANNOTATIONS,
        std::move(annotationsParser));
    m_pDataParser->AddHandler(DataChunkType::STACK,
        std::move(stackParser));
    m_pDataParser->AddHandler(DataChunkType::SHADER_META,
        std::move(shaderMetaParser));

    auto startTime  = std::chrono::system_clock::now();
    std::vector<gtByte> chunk;

    GetDatabaseAdapter()->BeginTransaction();

    // Poll for new data until we get killed.
    while (m_IsStopped == false)
    {
      // Read the value of the sampled counters.
      bool rc = CXLDmnCli->LPGPU2_ReceiveCollectedAndroidChunks(chunk);

      if (rc)
      {
        bool isSessionFinished = false;
        auto dpRc = m_pDataParser->ConsumeData(chunk, isSessionFinished);
        if (dpRc != lpgpu2::PPFnStatus::success)
        {
          // Log this event.
          OS_OUTPUT_DEBUG_LOG(L"There was an error consuming the data",
              OS_DEBUG_LOG_INFO);

          // Set the flags which will be read by the manager of the
          // polling thread
          m_profilingErr = false;
          m_profResult = PPR_COMMUNICATION_FAILURE;
          m_IsStopped = true;
          continue;
        }

        if (isSessionFinished)
        {
          // Set the flags which will be read by the manager of the
          // polling thread
          m_profilingErr = false;
          m_profResult = PPR_REMOTE_APP_STOPPED;
          m_IsStopped = true;

          // Log this event.
          OS_OUTPUT_DEBUG_LOG(L"Session finished by the RAgent",
              OS_DEBUG_LOG_INFO);
          continue;
        }

        // Send an ACK to the remote agent every time a Data packet is read
        // successfully and there are more to be read in the future
        rc = CXLDmnCli->LPGPU2_WriteChunk(RemProtoToken::CRP_RESPONSE_ACK);
        if(!rc)
        {
          OS_OUTPUT_DEBUG_LOG(L"Chunk was not acked as there was error", OS_DEBUG_LOG_INFO);

          // Set the flags which will be read by the manager of the
          // polling thread
          m_profilingErr = true;
          m_profResult = PPR_COMMUNICATION_FAILURE;
          m_IsStopped = true;
          continue;
        }
      }
      // If the reception of chunks failed
      else
      {
        // Log this event.
        OS_OUTPUT_DEBUG_LOG(L"Error occurred when receiving capture data", OS_DEBUG_LOG_ERROR);

        GT_ASSERT_EX(m_errorCb, L"The error callback handler cannot be null!");
        // Set the flags which will be read by the manager of the
        // polling thread
        m_profilingErr = true;
        m_profResult = PPR_COMMUNICATION_FAILURE;
        m_IsStopped = true;

        continue;
      }

      auto frameEndTime = std::chrono::system_clock::now();

      auto durationUntilLastFlush = frameEndTime - startTime;
      if (durationUntilLastFlush > kDurationDatabaseFlush)
      {
        auto fRC = m_pDataParser->FlushData();
        if (fRC != lpgpu2::PPFnStatus::success)
        {
          // Set the flags which will be read by the manager of the
          // polling thread
          m_profilingErr = false;
          m_profResult = PPR_COMMUNICATION_FAILURE;
          m_IsStopped = true;

          continue;
        }

        // Restart timer
        startTime = frameEndTime;

      }

      OS_OUTPUT_DEBUG_LOG(L"At end of loop internally",
          OS_DEBUG_LOG_INFO);
    }//while (m_IsStopped == false)

    GetDatabaseAdapter()->EndTransaction();

    OS_OUTPUT_DEBUG_LOG(L"At end of loop", OS_DEBUG_LOG_INFO);
  }

  OS_OUTPUT_DEBUG_LOG(L"At end of entry point", OS_DEBUG_LOG_INFO);
  return 0;
}

/// @brief Called by the thread manager before the termination of the thread
/// @note We do not return something here because CXL expects the signature of
/// this function to be void
void PPAndroidPollingThread::beforeTermination()
{
  GT_ASSERT_EX(m_errorCb, L"The error callback handler cannot be null!");

  OS_OUTPUT_DEBUG_LOG(L"Beginning run beforeTermination", OS_DEBUG_LOG_INFO);

  auto pRemoteClient = CXL_DAEMON_CLIENT;
  if (pRemoteClient)
  {
    if ((true == m_profilingErr))
    {
      pRemoteClient->Close();
      if(m_profResult == PPR_COMMUNICATION_FAILURE)
      {
#ifdef WIN32
        (void)pRemoteClient;
#endif // WIN32

        OS_OUTPUT_DEBUG_LOG(L"Error, closed CXL daemon CLI", OS_DEBUG_LOG_INFO);

        // Notify the system.
        m_errorCb(m_profResult, m_pDataCbParams);
        return;
      }

      return;
    }

    if (pRemoteClient->IsConnected() && m_profResult != PPR_REMOTE_APP_STOPPED)
    {
      bool isDataFlushed = false;
      std::vector<gtByte> chunk;

      // Read the value of the sampled counters.
      auto rc = pRemoteClient->LPGPU2_ReceiveCollectedAndroidChunks(chunk);
      if (!rc)
      {
        OS_OUTPUT_DEBUG_LOG(L"Error flushin Android collection",
            OS_DEBUG_LOG_ERROR);
        pRemoteClient->Close();
        return;
      }

      if (chunk.size() == 0)
      {
        pRemoteClient->Close();
        return;
      }

      GetDatabaseAdapter()->BeginTransaction();
      auto dpRc = m_pDataParser->ConsumeData(chunk, isDataFlushed);
      if (dpRc != lpgpu2::PPFnStatus::success)
      {
        // Log this event.
        OS_OUTPUT_DEBUG_LOG(L"There was an error consuming the data",
            OS_DEBUG_LOG_ERROR);

        pRemoteClient->Close();
        return;
      }

       //Send an ACK to the remote agent every time a Data packet is read
       //successfully and there are more to be read in the future
      rc = pRemoteClient->LPGPU2_WriteChunk(RemProtoToken::CRP_RESPONSE_ACK);
      if(!rc)
      {
        OS_OUTPUT_DEBUG_LOG(L"Chunk was not acked as there was error",
            OS_DEBUG_LOG_ERROR);
        pRemoteClient->Close();
        return;
      }

      // We need to also send the STOP_COLLECTION command
      rc = pRemoteClient->LPGPU2_StopAndroidCollection();
      if(!rc)
      {
        OS_OUTPUT_DEBUG_LOG(L"Error closing Android collection",
            OS_DEBUG_LOG_ERROR);
        pRemoteClient->Close();
        return;
      }


      // Use a timer to stop receiving flushed data if it might take too long
      auto timerStart = std::chrono::system_clock::now();
      auto timerEnd = std::chrono::system_clock::now();
      size_t loopsCounter = 0;
      while (!isDataFlushed &&
             std::chrono::duration_cast<std::chrono::seconds>(
               timerEnd - timerStart) < kDurationDataFlush)
      {
        // Read the value of the sampled counters.
        rc = pRemoteClient->LPGPU2_ReceiveCollectedAndroidChunks(chunk);
        if (!rc)
        {
          OS_OUTPUT_DEBUG_LOG(L"Error flushin Android collection",
              OS_DEBUG_LOG_ERROR);
          pRemoteClient->Close();
          break;
        }

        if (chunk.size() == 0)
        {
          pRemoteClient->Close();
          break;
        }

        dpRc = m_pDataParser->ConsumeData(chunk, isDataFlushed);
        if (dpRc != lpgpu2::PPFnStatus::success)
        {
          // Log this event.
          OS_OUTPUT_DEBUG_LOG(L"There was an error consuming the data",
              OS_DEBUG_LOG_ERROR);

          pRemoteClient->Close();
          break;
        }

        // Send an ACK to the remote agent every time a Data packet is read
        // successfully and there are more to be read in the future
        rc = pRemoteClient->LPGPU2_WriteChunk(RemProtoToken::CRP_RESPONSE_ACK);
        if(!rc)
        {
          OS_OUTPUT_DEBUG_LOG(L"Chunk was not acked as there was error",
              OS_DEBUG_LOG_ERROR);
          pRemoteClient->Close();
          break;
        }

        timerEnd = std::chrono::system_clock::now();

        ++loopsCounter;
      }

      OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO,
          L"Loops: %u", loopsCounter);

      GetDatabaseAdapter()->EndTransaction();
    }
  }

  pRemoteClient->Close();
  OS_OUTPUT_DEBUG_LOG(L"At end of beforeTermination", OS_DEBUG_LOG_INFO);
}

/// @brief  Get a pointer to the database adapter from the base class as a db::LPGPU2DatabaseAdapter
/// @returns db::LPGPU2DatabaseAdapter* The base class database adapter as an LPGPU2DatabaseAdaper.
/// @see db::LPGPU2DatabaseAdapter.
db::LPGPU2DatabaseAdapter* lpgpu2::PPAndroidPollingThread::GetDatabaseAdapter() const
{
    auto *dbAdapter = dynamic_cast<db::LPGPU2DatabaseAdapter*>(m_pDataAdapter);

    GT_ASSERT_EX(dbAdapter != nullptr, L"The current database adapter is not an LPGPU2DatabaseAdapter");

    return dbAdapter;
}


} // namespace lpgpu2
