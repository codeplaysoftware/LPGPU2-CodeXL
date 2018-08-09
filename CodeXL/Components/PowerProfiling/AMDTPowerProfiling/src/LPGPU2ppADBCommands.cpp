// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  Defines the object responsible for executing ADB commands
///
/// LPGPU2ppADBCommands implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommands.h>
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommandsExecutor.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Singleton declaration:
std::unique_ptr<LPGPU2ppADBCommands> LPGPU2ppADBCommands::m_spMySingleInstance = nullptr;

// Static Initialisation:
const std::chrono::seconds LPGPU2ppADBCommands::MaxStartAdbWaitTime{ 5 };
const std::chrono::seconds LPGPU2ppADBCommands::MaxStopRAgentWaitTime{ 5 };
const std::chrono::seconds LPGPU2ppADBCommands::MaxStartRAgentWaitTime{ 5 };

/// @brief Class constructor. Initialises the thread that executes ADB commands.
/// @param[in]  vpParent The parent object of this object.
/// @note This class is designed to be a singleton, so no one should need to
///       construct this object explicitly.
LPGPU2ppADBCommands::LPGPU2ppADBCommands(QObject *vpParent /* = nullptr */)
    : QObject{ vpParent }
    , m_ADBCommandExecutorThread{ this }
    , m_ADBPath{ L"adb" }
{
}

/// @brief Class destructor. Stops the monitoring thread and wait for it to
///        finishes.
LPGPU2ppADBCommands::~LPGPU2ppADBCommands()
{
    StopMonitoring();
    m_ADBCommandExecutorThread.wait();
}

/// @brief  Return the singleton instance of this class as a reference.
///         Constructs the object in the first call.
/// @return LPGPU2ppADBCommands A single instance of the LPGPU2ppADBCommands class.
/// @warning This object requires a call to LPGPU2ppADBCommands::StartMonitoring()
///          to create the monitoring object.
/// @see     LPGPU2ppADBCommands::StartMonitoring().
LPGPU2ppADBCommands& LPGPU2ppADBCommands::Instance()
{
    if (m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = std::unique_ptr<LPGPU2ppADBCommands>(new (std::nothrow) LPGPU2ppADBCommands);
        GT_ASSERT(m_spMySingleInstance != nullptr);
    }

    return *m_spMySingleInstance;
}

/// @brief  Return whether or not the current ADB path is valid.
/// @return bool    true = the path is valid,
///                 false = the path is not valid.
/// @warning    For the current ADB path to be validated, call StartMonitoring() first.
/// @see        LPGPU2ppADBCommands::StartMonitoring().
bool LPGPU2ppADBCommands::IsADBPathValid() const
{
    auto bReturn = false;

    if (m_pADBCommandExecutor != nullptr)
    {
        bReturn = m_pADBCommandExecutor->IsADBPathValid();
    }

    return bReturn;
}

/// @brief  Get the ADB version if the current ADB path is valid.
/// @param[out] vADBVersion The version of adb as returned by the command "adb version".
/// @return PPFnStauts  success = The ADB path is valid and the version is returned in vADBVersion,
///                     failure = The ADB path is invalid and no version information is available.
/// @warning    For the current ADB path to be validated, call StartMonitoring() first.
/// @see        LPGPU2ppADBCommands::StartMonitoring().
lpgpu2::PPFnStatus LPGPU2ppADBCommands::GetADBVersion(gtString &vADBVersion) const
{
    auto bReturn = lpgpu2::PPFnStatus::failure;

    if (IsADBPathValid())
    {
        vADBVersion = m_pADBCommandExecutor->GetADBVersion();
        bReturn = lpgpu2::PPFnStatus::success;
    }

    return bReturn;
}

/// @brief Get the ADB revision if the current ADB path is valid.
/// @param[out] vADBRevision    The revision of ADB as returned by the command "adb version".
/// @return     PPFnStauts      success = The ADB path is valid and the version is returned in vADBRevision,
///                             failure = The ADB path is invalid and no revision information is available.
/// @warning    For the current ADB path to be validated, call StartMonitoring() first.
/// @see        LPGPU2ppADBCommands::StartMonitoring().
lpgpu2::PPFnStatus LPGPU2ppADBCommands::GetADBRevision(gtString &vADBRevision) const
{
    auto bReturn = lpgpu2::PPFnStatus::failure;

    if (IsADBPathValid())
    {
        vADBRevision = m_pADBCommandExecutor->GetADBRevision();
        bReturn = lpgpu2::PPFnStatus::success;
    }

    return bReturn;
}

/// @brief Get the currently available ADB devices if the current ADB path is valid.
/// @param[out] vADBDevices     The available devices as returned by the command "adb devices".
/// @return     PPFnStauts      success = The ADB path is valid and the devices are returned in vADBDevices,
///                             failure = The ADB path is invalid and no revision information is available.
/// @warning    For the current ADB path to be validated, call StartMonitoring() first.
/// @see        LPGPU2ppADBCommands::StartMonitoring().
lpgpu2::PPFnStatus LPGPU2ppADBCommands::GetADBDevices(gtVector<gtString> &vADBDevices) const
{
    auto bReturn = lpgpu2::PPFnStatus::failure;

    if (IsADBPathValid())
    {
        vADBDevices = m_pADBCommandExecutor->GetADBDevices();
        bReturn = lpgpu2::PPFnStatus::success;
    }

    return bReturn;
}

/// @brief Get the currently selected ADB device if the current ADB path is valid.
/// @param[out] vADBDevices     The available devices as returned by the command "adb devices".
lpgpu2::PPFnStatus LPGPU2ppADBCommands::GetADBActiveDevice(gtString& vADBDevice) const
{
    auto bReturn = lpgpu2::PPFnStatus::failure;

    if (IsADBPathValid())
    {
        vADBDevice = m_ADBActiveDevice;
        bReturn = lpgpu2::PPFnStatus::success;
    }

    return bReturn;
}

/// @brief                    Request a change in the current ADB path.
/// @param[in]  vAdbFilePath  The new ADB path to be registered in the observer object.
/// @warning    For the current ADB path to be validated, call StartMonitoring() first.
/// @see        LPGPU2ppADBCommands::StartMonitoring().
void LPGPU2ppADBCommands::SetADBPath(const osFilePath &vAdbFilePath)
{
    if (m_ADBPath != vAdbFilePath)
    {
        m_ADBPath = vAdbFilePath;
        emit SetADBPathRequested(acGTStringToQString(m_ADBPath.asString()));
    }
}

/// @brief   Start the process of monitoring ADB and the RAgent. This will, in fact,
///          create an object and move it to the monitoring thread in order to execute
///          ADB commands without blocking the main thread.
void LPGPU2ppADBCommands::StartMonitoring()
{
    // Create the monitoring object if it does not exists.
    if (m_pADBCommandExecutor == nullptr)
    {
        m_pADBCommandExecutor = new LPGPU2ppADBCommandExecutor{ m_ADBPath };        

        // Cache the state as the executor constructor will have the updated adb and ragent states
        m_bIsADBRunning = m_pADBCommandExecutor->IsADBRunning();
        m_bIsRAgentInstalled = m_pADBCommandExecutor->IsRAgentInstalled();
        m_bIsRAgentRunning = m_pADBCommandExecutor->IsRAgentRunning();
    }

    // Move it to the monitoring thread.
    m_pADBCommandExecutor->moveToThread(&m_ADBCommandExecutorThread);

    // Connect all the required signals.
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::ADBPathChanged, this, &LPGPU2ppADBCommands::OnADBPathChanged, Qt::QueuedConnection);
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::ADBStatusChanged, this, &LPGPU2ppADBCommands::OnADBStatusChanged, Qt::QueuedConnection);
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::RAgentInstalledStatusChanged, this, &LPGPU2ppADBCommands::OnRAgentInstalledStatusChanged, Qt::QueuedConnection);
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::RAgentStatusChanged, this, &LPGPU2ppADBCommands::OnRAgentStatusChanged, Qt::QueuedConnection);
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::PreviousActionsCompleted, this, &LPGPU2ppADBCommands::OnPreviousActionsCompleted, Qt::QueuedConnection);
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::ADBActiveDeviceChanged, this, &LPGPU2ppADBCommands::OnADBActiveDeviceChanged, Qt::QueuedConnection);
    connect(m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::ADBDevicesChanged, this, &LPGPU2ppADBCommands::OnADBDevicesChanged, Qt::QueuedConnection);

    connect(&m_ADBCommandExecutorThread, &QThread::started, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnStartMonitoring, Qt::QueuedConnection);
    connect(&m_ADBCommandExecutorThread, &QThread::finished, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::deleteLater, Qt::QueuedConnection);

    connect(this, &LPGPU2ppADBCommands::SetADBPathRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnSetADBPath, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::StartADBRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnStartADB, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::StopADBRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnStopADB, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::StartRAgentRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnStartRAgent, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::StopRAgentRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnStopRAgent, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::RestartRAgentRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnRestartRAgent, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::InstallAPKRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnInstallAPK, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::PortForwardRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnPortForward, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::EnableLPGPU2VulkanLayerRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnEnableLPGPU2VulkanLayer, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::WaitForPreviousActionsRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::PreviousActionsCompleted, Qt::QueuedConnection);
    connect(this, &LPGPU2ppADBCommands::ADBDeviceChangeRequested, m_pADBCommandExecutor, &LPGPU2ppADBCommandExecutor::OnSetADBDevice, Qt::QueuedConnection);    

    // Start the monitoring thread event loop.
    m_ADBCommandExecutorThread.start();
}

/// @brief  Stops the monitoring thread to avoid wasting of resources.
void LPGPU2ppADBCommands::StopMonitoring()
{
    m_ADBCommandExecutorThread.quit();

    // The thread "finished" signal will take care of deleting the ADB observer
    m_pADBCommandExecutor = nullptr;
}

/// @brief   Request ADB to start a server.
/// @return  bool  true = The request to start the ADB server has been placed and
///                       the caller might need to wait for it to finishes.
///                false = The ADB server is already running.
/// @warning       This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::StartADB()
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (!IsADBRunning())
    {
        emit StartADBRequested();
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief     Request ADB to stop its server.
/// @return    bool     true = The request to stop the ADB sever has been placed and
///                            the caller might need to wait for it to finish.
///                     false = The ADB server is already stopped.
/// @warning   This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::StopADB()
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (IsADBRunning())
    {
        emit StopADBRequested();
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief  Request the RAgent to start in the connected device.
/// @return bool    true = The request to start the RAgent has been placed and
///                        the caller might need to wait for it to finish.
///                 false = ADB is not running or the RAgent is not installed.
/// @warning        This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::StartRAgent()
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (IsADBRunning() && !IsRAgentRunning())
    {
        emit StartRAgentRequested();
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief  Request the RAgent to stop in the connected device.
/// @return bool    true = The request to stop the RAgent has been placed and
///                        the caller might need to wait for it to finish.
///                 false = ADB is not running, the RAgent is not installed or it is not running.
/// @warning        This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::StopRAgent()
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (IsADBRunning() && IsRAgentInstalled() && IsRAgentRunning())
    {
        emit StopRAgentRequested();
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief  Request the RAgent to restart in the connected device.
/// @return bool    true = The request to restart the RAgent has been placed and
///                        the caller might need to wait for it to finish.
///                 false = ADB is not running, the RAgent is not installed or the RAgent
///                         is already stopped.
/// @warning        This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::RestartRAgent()
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (IsADBRunning() && IsRAgentInstalled())
    {
        emit RestartRAgentRequested();
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief  Request the RAgent to restart in the connected device.
/// @return bool    true = The request to change the device has been placed and
///                        the caller might need to wait for it to finish.
///                 false = ADB is not running.
/// @warning        This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::ChangeADBDevice(const QString &vDeviceName)
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (IsADBRunning() && vDeviceName != acGTStringToQString(m_ADBActiveDevice))
    {
        emit ADBDeviceChangeRequested(vDeviceName);
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief       Returns whether or not ADB is running as a server.
/// @return bool true = ADB is running as a server,
///              false = ADB is not running.
/// @warning     This is a cached state, it might not reflect the real state of the ADB
///              server in the local machine, but it will eventually be consistent.
bool LPGPU2ppADBCommands::IsADBRunning() const
{
    return m_bIsADBRunning;
}

/// @brief  Returns whether or not the RAgent is installed on the connected device.
/// @return bool    true = The RAgent is installed in the connected device,
///                 false = The RAgent is not installed in the connected device.
/// @warning    This is a cached state, it might not reflect the real state of the RAgent
///             on the device, but it will eventually be consistent.
bool LPGPU2ppADBCommands::IsRAgentInstalled() const
{
    return m_bIsRAgentInstalled;
}

/// @brief  Returns whether or not the RAgent is running on the connected device.
/// @return bool    true = The RAgent is running in the connected device,
///                 false = The RAgent is not running in the connected device.
/// @warning    This is a cached state, it might not reflect the real state of the RAgent
///             on the device, but it will eventually be consistent.
bool LPGPU2ppADBCommands::IsRAgentRunning() const
{
    return m_bIsRAgentRunning;
}

/// @brief                  Request the installation of the specified 
///                         APK in the connected device.
/// @param[in] vApkFilePath The file path of the APK to be installed.
/// @return    PPFnStauts   success = The installation was requested and the caller,
///                         failure = might need to wait for it to finishes.
///                         false = The APK file was not found or ADB is not running.
/// @warning                This is an asynchronous operation, it will return immediately.
/// @warning                This requires that the APK file exists and that ADB is running.
lpgpu2::PPFnStatus LPGPU2ppADBCommands::InstallAPK(const osFilePath &vApkFilePath)
{
    auto bReturn = lpgpu2::PPFnStatus::failure;

    if (vApkFilePath.exists() && IsADBRunning())
    {
        emit InstallAPKRequested(acGTStringToQString(vApkFilePath.asString()));
        bReturn = lpgpu2::PPFnStatus::success;
    }

    return bReturn;
}


/// @brief  Request the ADB do to a port forward to allow the connection to the RAgent
///         via a USB cable. It will execute the command "adb forward tcp:vOrigPort tcp:vDestPort"
/// @param[in]  vOrigPort   The TCP port of origin.
/// @param[in]  vDestPort   The TCP port of destiny.
/// @return     bool        true = The request to do the port forward has been placed,
///             false       false = ADB is not running.
/// @warning  This is an asynchronous operation, it will return immediately.
EWaitStatus LPGPU2ppADBCommands::PortFoward(unsigned short vOrigPort, unsigned short vDestPort)
{
    auto bCallerNeedToWait = EWaitStatus::kNoNeedToWait;

    if (IsADBRunning())
    {
        emit PortForwardRequested(vOrigPort, vDestPort);
        bCallerNeedToWait = EWaitStatus::kNeedToWait;
    }

    return bCallerNeedToWait;
}

/// @brief  Request the ADB to enable the LPGPU2 Vulkan layer. This allows the RAgent to
///         capture Vulkan API calls on the Android device.
/// @warning    This method requires ADB to be running. Since the ADB command used
///             "adb setprop" never fails, there is no way to return an error code here.
void LPGPU2ppADBCommands::EnableLPGPU2VulkanLayer()
{
    if (IsADBRunning())
    {
        emit EnableLPGPU2VulkanLayerRequested();
    }
}

/// @brief  Generates a CommandGroupId that be used to check if a group of actions
///         has been completed. This is useful, for example, in cases where one 
///         want to ensure two or more actions were executed before updating the GUI.
///         The returned CommandGroupId can be used to check if the previous actions,
///         were in fact executed. This method will cause the signal OnPreviousActionsCompleted
///         to be emitted. With the signal, will be the same CommandGroupId that was returned
///         when calling this. With this, it is possible to uniquely identify groups of actions.
/// @return CommandGroupId an unique identifier that can be used to make sure all previous actions
///         that took place before the call to this method.
LPGPU2ppADBCommands::CommandGroupId LPGPU2ppADBCommands::WaitForPreviousActions()
{
    // Just to make sure we don't overflow this integer, very unlikely but possible.
    if (m_lastActionsGroupId + 1 == std::numeric_limits<CommandGroupId>::max())
    {
        m_lastActionsGroupId = 0;
    }

    m_lastActionsGroupId++;

    emit WaitForPreviousActionsRequested(m_lastActionsGroupId);

    return m_lastActionsGroupId;
}

/// @brief  Internal slot used to cache ADB information. The cached information consists
///         of whether or not the current path is valid, the current ADB version and revision.
///         This will emit the signal ADBPathChanged so subscribers can read the new results.
void LPGPU2ppADBCommands::OnADBPathChanged()
{
    // Cache the ADB info
    m_bIsADBPathValid = m_pADBCommandExecutor->IsADBPathValid();
    m_ADBVersion = m_pADBCommandExecutor->GetADBVersion();
    m_ADBRevision = m_pADBCommandExecutor->GetADBRevision();

    emit ADBPathChanged();
}

/// @brief  Internal slot used to cache the current status of the ADB server.
/// @param[in]  vbIsADBRunning  Whether of not ADB is running as a server on the local machine.
void LPGPU2ppADBCommands::OnADBStatusChanged(bool vbIsADBRunning)
{
    if (m_bIsADBRunning != vbIsADBRunning)
    {
        m_bIsADBRunning = vbIsADBRunning;
        emit ADBStatusChanged(m_bIsADBRunning);
    }
}

/// @brief Internal slot used to update the currently set ADB device.
/// @param vDeviceName Name of the active device.
void LPGPU2ppADBCommands::OnADBActiveDeviceChanged(const QString &vDeviceName)
{
    m_ADBActiveDevice = acQStringToGTString(vDeviceName);
}

void LPGPU2ppADBCommands::OnADBDevicesChanged()
{
    m_ADBDevices = m_pADBCommandExecutor->GetADBDevices();
    emit ADBDevicesChanged();
}

/// @brief  Internal slot used to cache the installed status the RAgent on the connected device.
/// @param[in]  vbIsADBRunning  Whether of not the RAgent is running in the connected device.
void LPGPU2ppADBCommands::OnRAgentInstalledStatusChanged(bool vbIsRAgentInstalled)
{
    if (m_bIsRAgentInstalled != vbIsRAgentInstalled)
    {
        m_bIsRAgentInstalled = vbIsRAgentInstalled;
        emit RAgentInstalledStatusChanged(m_bIsRAgentInstalled);
    }
}

/// @brief  Internal slot used to cache the current running status of the remote agent on the connect device.
/// @param[in]  vbIsRAgentRunning   Whether or not the RAgent is running on the connected device.
void LPGPU2ppADBCommands::OnRAgentStatusChanged(bool vbIsRAgentRunning)
{
    if (m_bIsRAgentRunning != vbIsRAgentRunning)
    {
        m_bIsRAgentRunning = vbIsRAgentRunning;
        emit RAgentStatusChanged(m_bIsRAgentRunning);
    }
}
