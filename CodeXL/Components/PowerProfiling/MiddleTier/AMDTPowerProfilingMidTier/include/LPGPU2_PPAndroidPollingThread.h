// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Polling Thread to be used when the mode is to profile
///        on an Android remote device.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PPANDROIDPOLLINGTHREAD
#define LPGPU2_PPANDROIDPOLLINGTHREAD

//Infra
#include <AMDTOSWrappers/Include/osThread.h>

//Local
#include <AMDTPowerProfilingMidTier/include/PPPollingThread.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>

// Forward declarations.
enum AppLaunchStatus;
class PowerProfilerCore;
namespace lpgpu2 {
  class DataPacketParser;

namespace db {
  class LPGPU2DatabaseAdapter;
}
}

namespace lpgpu2 {

/// @brief    Impl of the Polling Thread for Android power profiling.
///
/// @warning  This works fairly differently from the PPPollingThread since it
///           does not use a backend but directly interacts with the
///           CXLDaemonClient class.
/// @see      PPPollingThread, CXLDaemonClient
/// @date     06/09/2017.
/// @author   Alberto Taiuti.
// clang-format off
class PPAndroidPollingThread final
: public PPPollingThread
{
// Methods
public:
  PPAndroidPollingThread(
      unsigned pollingInterval,
      PPSamplesDataHandler cb,
      void* pDataCbParams,
      PPFatalErrorHandler cbErr,
      void* pErrorCbParams,
      PowerProfilerCore* m_pPPcore,
      db::LPGPU2DatabaseAdapter* pDataAdapter);

// Default
public:
  ~PPAndroidPollingThread() = default;

// Deleted
public:
  PPAndroidPollingThread() = delete;
  PPAndroidPollingThread(PPAndroidPollingThread &&) = delete;
  PPAndroidPollingThread& operator=(PPAndroidPollingThread &&) = delete;
  PPAndroidPollingThread(const PPAndroidPollingThread&) = delete;
  PPAndroidPollingThread& operator=(const PPAndroidPollingThread&) = delete;

// Overridden
public:
  int entryPoint() override;

// Overridden
protected:
  void beforeTermination() override;

// Methods:
private:
    db::LPGPU2DatabaseAdapter* GetDatabaseAdapter() const;

// Attributes
private:
  PowerProfilerCore *m_pPPcore = nullptr;
  std::unique_ptr<lpgpu2::DataPacketParser> m_pDataParser;

}; // class LPGPU2_PPAndroidPollingThread
// clang-format on

} // namespace lpgpu2
#endif // LPGPU2_PPANDROIDPOLLINGTHREAD
