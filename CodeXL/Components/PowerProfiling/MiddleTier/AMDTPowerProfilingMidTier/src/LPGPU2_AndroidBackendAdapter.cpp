// Local.
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
#include <AMDTPowerProfilingMidTier/include/BackendDataConvertor.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_AndroidBackendAdapter.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppTargetDefinition.h>
#include <AMDTPowerProfiling/src/ppLPGPU2CollectionDefs.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppTargetDefinition.h>
#include <AMDTPowerProfilingMidTier/include/ppLPGPU2TargetCharacteristics.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
using lpgpu2::PPFnStatus;

// Power profiling backend definitions.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Network.
#include <AMDTOSWrappers/Include/osPortAddress.h>

// STL
// Introduce:
// std::move
// std::find_if
// std::count_if
#include <utility>
#include <algorithm>

namespace lpgpu2 {

/// @brief Class ctor. Initialise *this. No work is done here
/// @note: We do not use an Init method because the pre-existing
///  CXL backend classes don't have that either and hence we
///  do so to conform with the pre-existing style
AndroidBackendAdapter::AndroidBackendAdapter(
    const CollectionDefs &collectionDefs,
    const lpgpu2::TargetDefinition &targetDefinition,
    const TargetCharacteristics &targetCharacteristics)
  : IPowerProfilerBackendAdapter{},
    m_remoteHostName{},
    m_remoteTargetPort{},
    m_appLaunchStatus{},
    m_lastErrorMsg{},
    m_collectionDefs{new CollectionDefs{collectionDefs}},
    m_targetDefinition{new lpgpu2::TargetDefinition{targetDefinition}},
    m_targetCharacteristics{new TargetCharacteristics{targetCharacteristics}}
{
}

/// @brief Class dtor. Closes an existing connection with the remote agent, if any
AndroidBackendAdapter::~AndroidBackendAdapter()
{
    // Close the connection.
    // TODO Uncomment this once the android RAgent implements disconnection
    // and reconnection correctly
    //CXLDaemonClient::Close();
}

/// @brief Initialise the backend by initialising the remote agent client class
///        and connecting to the remote agent
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::InitializeBackend()
{
  // Initialize the remote client.
  osPortAddress remoteTargetAddr(m_remoteHostName, m_remoteTargetPort);
  auto rcClient = CXLDaemonClient::IsInitialized(remoteTargetAddr);

  if (rcClient == false)
  {
    rcClient = CXLDaemonClient::Init(remoteTargetAddr,
                  static_cast<long>(kTCPTimeoutDefault));
    GT_ASSERT_EX(rcClient, L"ERROR: Setting the remote power profiling target address");
  }

  if (rcClient)
  {
    auto pRemoteClient = CXL_DAEMON_CLIENT;
    GT_IF_WITH_ASSERT(pRemoteClient != nullptr)
    {
      osPortAddress clientAddr;
      rcClient = pRemoteClient->ConnectToDaemon(clientAddr);// Function takes an output parameter in which it returns the address and port number of the remote end after the connection was established.
      GT_IF_WITH_ASSERT(rcClient)
      {
        // Test if the device is still available; when calling this method we should have
        // already initialised the device; by initialisation we mean that the device info has been retrieved (when
        // in the project settings) and the collection options have been sent over
        const auto rc = pRemoteClient->LPGPU2_IsAndroidClientAlive();

        if (!rc)
        {
          pRemoteClient->Close();
          return PPR_REMOTE_CONNECTION_ERROR;
        }

        return PPR_NO_ERROR;
      }
      else
      {
        return PPR_REMOTE_CONNECTION_ERROR;
      }
    }
    else
    {
      return PPR_UNKNOWN_FAILURE;
    }
  }
  else
  {
    return PPR_REMOTE_CONNECTION_ERROR;
  }
}

/// @brief Return the system topology as a tree
/// @param systemDevices Out parameter which contains the list of devices to
///        return
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::GetSystemTopology(gtList<PPDevice*>& systemDevices)
{
  // Aware that allocating memory on the heap using "new" is to be avoided; however it is necessary
  // to conform with how CXL does things already

  // List of supported counters.
  gtList<AMDTPwrCounterDesc*> supportedCounters;
  GetDeviceCounters(0, supportedCounters);

  // Create only two devices (one is a sub-device) because we know we are
  // working with an Android device which has a GPU. We don't need to query
  // the hardware drivers because we already know the layout
  auto pDevice = new PPDevice{};
  GT_ASSERT(pDevice);
  pDevice->m_deviceId = 1;
  pDevice->m_deviceType = AMDT_PWR_DEVICE_INTERNAL_GPU;
  pDevice->m_deviceName =
    m_targetDefinition->GetTargetElement().GetPlatform();
  pDevice->m_deviceDescription =
    m_targetDefinition->GetTargetElement().GetHardware();
  pDevice->m_supportedCounters = supportedCounters;
  gtList<PPDevice*> subDevices;
  subDevices.push_back(pDevice);

  // Creating a root device because this is the system used by CXL to represent
  // system topologies: there is always a root dummy device
  auto pRootDevice = new PPDevice{};
  GT_ASSERT(pRootDevice);
  pRootDevice->m_deviceId = 0;
  pRootDevice->m_deviceType = AMDT_PWR_DEVICE_SYSTEM;
  pRootDevice->m_deviceName = L"Root system";
  pRootDevice->m_deviceDescription= L"Root system";

  // For all the counter sets, set them as "devices", as they contain counters.
  const auto &counterSets =
    m_targetDefinition->GetTargetElement().GetCounterElemsVec();
  for (const auto &counterSet : counterSets)
  {
    auto pChildDevice = new PPDevice{};
    GT_ASSERT(pChildDevice);
    pChildDevice->m_deviceId = counterSet.GetId();
    pChildDevice->m_deviceType = AMDT_PWR_DEVICE_INTERNAL_GPU;
    pChildDevice->m_deviceName = counterSet.GetName();
    pChildDevice->m_deviceDescription = counterSet.GetDescription();
    subDevices.push_back(pChildDevice);
  }

  pRootDevice->m_subDevices = subDevices;
  systemDevices.push_back(pRootDevice);

  return PPR_NO_ERROR;
}

/// @brief Return the system topology as a tree
/// @param deviceID Legacy param, kept for iface consistency with what expected
///        by CXL
/// @param supportedCounters Out parameter, list of counters supported by the device
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::GetDeviceCounters(int deviceID, gtList<AMDTPwrCounterDesc*>& supportedCounters)
{
  GT_UNREFERENCED_PARAMETER(deviceID);

  const auto &counterSets =
    m_targetDefinition->GetTargetElement().GetCounterElemsVec();
  for (const auto &counterSet : counterSets)
  {
    gtUInt32 id = counterSet.GetId();
    const auto &counters = counterSet.GetCounterElemsVec();
    for (const auto& counter : counters)
    {
      auto pCurrentCounter = BackendDataConvertor::ConvertToPwrCounterDesc(id, counter);

      supportedCounters.push_back(pCurrentCounter);
    }
  }

  return PPR_NO_ERROR;
}

/// @brief Retrieve the minimum sampling period in milliseconds
/// @warning Currently unsupported for Android
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::GetMinTimerSamplingPeriodMS(unsigned int& samplingPeriodBufferMs)
{
  GT_UNREFERENCED_PARAMETER(samplingPeriodBufferMs);
  return PPR_NO_ERROR;
}

/// @brief Retrieve the state buffer
/// @warning Currently unsupported for Android
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::GetProfilingState(AMDTPwrProfileState& stateBuffer)
{
  PPResult ret = PPR_NO_ERROR;

  GT_UNREFERENCED_PARAMETER(stateBuffer);
  GT_ASSERT_EX(false, L"The method or operation is not implemented.");

  return ret;
}

/// @brief Retrieve the current sampling interval in milliseconds
/// @param samplingIntervalMs Output param, returns the sampling interval in ms
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::GetCurrentSamplingInterval(unsigned int& samplingIntervalMs)
{
  samplingIntervalMs = static_cast<unsigned int>(
    1.f / static_cast<gtFloat32>(m_collectionDefs->GetIntervalHz()) * 1000.f
  );
  return PPR_NO_ERROR;
}

/// @brief Retrieve whether a given counter is enabled
/// @counterID The ID of the counter of which to read if it is enabled
/// @isEnabled Output param, whether the counter is enabled or not
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::IsCounterEnabled(unsigned int counterId, bool& isEnabled)
{
  // Loop over each counter set
  for (auto &&set : m_collectionDefs->GetCounterSets())
  {
    auto counter_it = std::find_if(set.counters.cbegin(), set.counters.cend(),
        [counterId](const CollectionDefCounter &el) {
          unsigned int to_num;
          auto rc = el.id.toUnsignedIntNumber(to_num);
          if (!rc) {
            return false;
          }
          return to_num == counterId && el.enabled;
        });

    if (counter_it != set.counters.cend())
    {
      isEnabled = true;
      return PPR_NO_ERROR;
    }
  }

  return PPR_COUNTERS_NOT_ENABLED;
}

/// @brief Retrieve the number of available counters
/// @param numOfAvailableCounters Output param, number of available counters
/// @return The return status of the function once it completes.
///         Always PPR_NO_ERROR
PPResult AndroidBackendAdapter::GetNumOfEnabledCounters(int& numOfAvailableCounters)
{
  // Loop over each counter set
  for (auto &&set : m_collectionDefs->GetCounterSets())
  {
    auto count = std::count_if(set.counters.cbegin(), set.counters.cend(),
        [](const CollectionDefCounter &el) {
          return el.enabled;
        });
    numOfAvailableCounters += static_cast<int>(count);
  }

  return PPR_NO_ERROR;
}

/// @brief Begin profilig
/// @warning Currently unsupported for Android
/// @return The return status of the function once it completes.
///          Always PPR_NOT_SUPPORTED
PPResult AndroidBackendAdapter::StartProfiling()
{
  // This function should not be called anywhere because we call directly into the CXL Daemon Client functions
  // from the Android Polling thread
  GT_ASSERT_EX(false, L"The method or operation is not implemented and should not have been called.");

  return PPR_NOT_SUPPORTED;
}

/// @brief Stop profiling session by stopping collection and closing connection
///        with the server. Implemented only to conform with the interface
///        expected by CodeXL
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::StopProfiling()
{
  return PPR_NO_ERROR;
}

/// @brief Pause profiling session
/// @warning Currently unsupported for Android
/// @return The return status of the function once it completes.
///          Always PPR_NOT_SUPPORTED
PPResult AndroidBackendAdapter::PauseProfiling()
{
  // This function should not be called anywhere because we don't support this on Android and
  // CXL does not use this function apart from the CLI version of the power profiler,
  // in which we don't support android profiling
  GT_ASSERT_EX(false, L"The method or operation is not implemented and should not have been called.");

  return PPR_NOT_SUPPORTED;
}

/// @brief Resume profiling session
/// @warning Currently unsupported for Android
/// @return The return status of the function once it completes.
///          Always PPR_NOT_SUPPORTED
PPResult AndroidBackendAdapter::ResumeProfiling()
{
    // This function should not be called anywhere because we don't support this on Android and
    // CXL does not use this function apart from the CLI version of the power profiler,
    // in which we don't support android profiling
    GT_ASSERT_EX(false, L"The method or operation is not implemented and should not have been called.");

    return PPR_NOT_SUPPORTED;
}

/// @brief Enable a counter
/// @warning Currently unsupported for Android
/// @param counterID The ID of the counter to enable
/// @return The return status of the function once it completes.
///          Always PPR_NO_ERROR
PPResult AndroidBackendAdapter::EnableCounter(int counterId)
{
  GT_UNREFERENCED_PARAMETER(counterId);
  OS_OUTPUT_DEBUG_LOG(L"In Android profile mode EnableCounter is not supported. The counters are set before starting the session only via the XML configuration file.", OS_DEBUG_LOG_DEBUG);
  return PPR_NO_ERROR;
}

/// @brief Disable a counter
/// @warning Currently unsupported for Android
/// @param counterID The ID of the counter to disable
/// @return The return status of the function once it completes.
///          Always PPR_NO_ERROR
PPResult AndroidBackendAdapter::DisableCounter(int counterId)
{
  GT_UNREFERENCED_PARAMETER(counterId);
  OS_OUTPUT_DEBUG_LOG(L"In Android profile mode DisableCounter is not supported. The counters are set before starting the session only via the XML configuration file.", OS_DEBUG_LOG_DEBUG);
  return PPR_NO_ERROR;
}

/// @brief Set sampling interval
/// @warning Currently unsupported for Android
/// @param interval The interval to set
/// @return The return status of the function once it completes.
///         Always PPR_NO_ERROR
PPResult AndroidBackendAdapter::SetTimerSamplingInterval(unsigned int interval)
{
  GT_UNREFERENCED_PARAMETER(interval);
  OS_OUTPUT_DEBUG_LOG(L"In Android profile mode SetTimerSamplingInterval is not supported. The settings are set before starting the session only via the XML configuration file.", OS_DEBUG_LOG_DEBUG);
  return PPR_NO_ERROR;
}

/// @brief Close the profling session
/// @return The return status of the function once it completes. PPR_NO_ERROR =
///         no issues, else issues.
PPResult AndroidBackendAdapter::CloseProfileSession()
{
  auto pRemoteClient = CXL_DAEMON_CLIENT;
  if(pRemoteClient != nullptr)
  {
    return  StopProfiling();
  }

  return PPR_UNKNOWN_FAILURE;
}

/// @brief Sample data from all the enabled counters
/// @warning Currently unsupported for Android
/// @return The return status of the function once it completes.
///          Always PPR_NOT_SUPPORTED
PPResult AndroidBackendAdapter::ReadAllEnabledCounters(gtVector<AMDTProfileTimelineSample*>& buffer)
{
    // This function should not be called anywhere because we don't support it on Android and istead use
    // the CXLDaemonClient class directly in the Android polling thread
    GT_UNREFERENCED_PARAMETER(buffer);
    GT_ASSERT_EX(false, L"The method or operation is not implemented.");

    return PPR_NOT_SUPPORTED;
}

/// @brief Set the remote target host name and port number
/// @param remoteTargetHostName The name of the remote host to
///         set 
/// @param remoteTargetPortNumber The port of the remote host to
///         set 
/// @return The return status of the function once it completes.
///         Status::success if success, Status::failure if failure
PPFnStatus AndroidBackendAdapter::SetRemoteTarget(const gtString& remoteTargetHostName, unsigned short remoteTargetPortNumber)
{
    if (remoteTargetHostName.isEmpty() || (remoteTargetPortNumber == 0))
    {
      return PPFnStatus::failure;
    }

    // Set the relevant data members.
    m_remoteHostName = remoteTargetHostName;
    m_remoteTargetPort = remoteTargetPortNumber;

    return PPFnStatus::success;
}

/// @brief Set the application launch details
/// @warning Currently unsupported for Android
/// @param appLaunchDetails The application launch details to set
/// @return The return status of the function once it completes.
///         Always PPR_NO_ERROR.
PPResult AndroidBackendAdapter::SetApplicationLaunchDetails(const ApplicationLaunchDetails& appLaunchDetails)
{
    // We do nothing here; all the information we need has (SHOULD HAVE) already been set in the project settings
    // widget
    GT_UNREFERENCED_PARAMETER(appLaunchDetails);
    OS_OUTPUT_DEBUG_LOG(L"In Android profile mode SetApplicationLaunchDetails is not supported. The settings are set before starting the session only via the XML configuration file.", OS_DEBUG_LOG_DEBUG);
    return PPR_NO_ERROR;
}

/// @brief Get the status of the application which was launched
/// @warning Currently unsupported for Android
/// @return The status of the application. Always rasOk.
AppLaunchStatus AndroidBackendAdapter::GetApplicationLaunchStatus()
{
    // Always return ok
    OS_OUTPUT_DEBUG_LOG(L"In Android profile mode GetApplicationLaunchStatus() is not used. Always return ok because CLX code calls into it.", OS_DEBUG_LOG_DEBUG);
    return rasOk;
}

/// @brief Get the latest error message
/// @param msg The error message returned
void AndroidBackendAdapter::GetLastErrorMessage(gtString& msg)
{
    msg = m_lastErrorMsg;
}

} // namespace lpgpu2
