// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Power Profiling backend to be used when the project is to profile
///        on an Android remote device.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_ANDROIDBACKEND_H
#define LPGPU2_ANDROIDBACKEND_H

// Local.
#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Common DB related structures
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

// Remote client.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// STL
// Introduce:
// std::unique_ptr
#include <memory>

// Forward declarations
class CollectionDefs;
class TargetCharacteristics;
namespace lpgpu2 {
  class TargetDefinition;
  enum class PPFnStatus;
}

namespace lpgpu2 {

/// @brief    Impl of the Android remote backend.
///
/// @warning  Some of the iface is redundant for the SAMS ANDROID protocol, but
///           we need to keep it as such to conform with the iface expected by
///           CXL of a Backend.
/// @see      IPowerProfilerBackendAdapter, RemoteBackend,
///           LocalBackend
/// @date     04/09/2017.
/// @author   Alberto Taiuti.
// clang-format off
class AndroidBackendAdapter final
: public IPowerProfilerBackendAdapter
{
// Type aliases
public:
  using CollDefsUP = std::unique_ptr<CollectionDefs>;
  using TargDefUP = std::unique_ptr<lpgpu2::TargetDefinition>;
  using TargCharUP = std::unique_ptr<TargetCharacteristics>;

// Methods:
public:
  AndroidBackendAdapter(const CollectionDefs &collectionDefs,
    const lpgpu2::TargetDefinition &targetDefinition,
    const TargetCharacteristics &targetCharacteristics);
  ~AndroidBackendAdapter();

  // Rule of 5
  AndroidBackendAdapter(AndroidBackendAdapter &&) = delete;
  AndroidBackendAdapter &operator=(AndroidBackendAdapter &&) = delete;
  AndroidBackendAdapter(const AndroidBackendAdapter &) = delete;
  AndroidBackendAdapter &operator=(const AndroidBackendAdapter &) = delete;

  lpgpu2::PPFnStatus SetRemoteTarget(const gtString& remoteTargetHostName,
                         unsigned short remoteTargetPortNumber);

  const CollectionDefs *GetCollectionDefs() const {
    return m_collectionDefs.get(); }
  const TargetCharacteristics *GetTargetCharacteristics() const {
    return m_targetCharacteristics.get(); }
  const TargetDefinition *GetTargetDefinition() const {
    return m_targetDefinition.get(); }

// Overridden:
public:
  PPResult InitializeBackend() override;
  PPResult GetSystemTopology(gtList<PPDevice*>& systemDevices) override;
  PPResult GetDeviceCounters(int deviceID, gtList<AMDTPwrCounterDesc*>& countersListBuffer) override;
  PPResult GetMinTimerSamplingPeriodMS(unsigned int& samplingPeriodBufferMs) override;
  PPResult GetProfilingState(AMDTPwrProfileState& stateBuffer) override;
  PPResult GetCurrentSamplingInterval(unsigned int& samplingIntervalMs) override;
  PPResult IsCounterEnabled(unsigned int counterID, bool& isEnabled) override;
  PPResult GetNumOfEnabledCounters(int& numOfAvailableCounters) override;
  PPResult StartProfiling() override;
  PPResult StopProfiling() override;
  PPResult PauseProfiling() override;
  PPResult ResumeProfiling() override;
  PPResult EnableCounter(int counterId) override;
  PPResult DisableCounter(int counterId) override;
  PPResult SetTimerSamplingInterval(unsigned int interval) override;
  PPResult CloseProfileSession() override;
  PPResult ReadAllEnabledCounters(gtVector<AMDTProfileTimelineSample*>& buffer) override;
  PPResult SetApplicationLaunchDetails(const ApplicationLaunchDetails& appLaunchDetails) override;
  AppLaunchStatus GetApplicationLaunchStatus() override;
  void GetLastErrorMessage(gtString& msg) override;

// Attributes
private:
  gtString m_remoteHostName;
  unsigned short m_remoteTargetPort;
  AppLaunchStatus m_appLaunchStatus;
  gtString m_lastErrorMsg;
  CollDefsUP m_collectionDefs;
  TargDefUP m_targetDefinition;
  TargCharUP m_targetCharacteristics;

}; // class LPGPU2_AndroidBackendAdapter

} // namespace lpgpu2

// clang-format on

#endif // LPGPU2_ANDROIDBACKEND_H
