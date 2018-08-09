// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  Defines the object responsible for executing ADB commands
///
/// LPGPU2ppADBCommandExecutor implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommandsExecutor.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// STL:
#include <chrono>

/// @brief  Interval between ADB system queries.
static constexpr auto gs_ADBMonitorTimerInterval = std::chrono::milliseconds{ 2000 };

/// @brief The flag that can be used to cancel the osExecAndGrabOutput calls.
static constexpr auto gs_bCancelFlag = false;

/// @brief  Class constructor. No work is done here. Just calls the parent constructor.
/// @param[in]  vAdbPath    The path the ADB executable.
/// @param[in]  vpParent    The parent object of this.
LPGPU2ppADBCommandExecutor::LPGPU2ppADBCommandExecutor(const osFilePath &vAdbPath, QObject *vpParent /* = nullptr */)
    : QObject{ vpParent }
    , m_ADBPath{ vAdbPath }
{
    ValidateADBPath();
    QueryADBAndRAgentStatus();
}

/// @brief  Class destructor. Kill the timer used to monitor the ADB and RAgent status, if necessary.
LPGPU2ppADBCommandExecutor::~LPGPU2ppADBCommandExecutor()
{
    if (m_adbObserverTimerId != -1)
    {
        killTimer(m_adbObserverTimerId);
    }
}

bool LPGPU2ppADBCommandExecutor::IsADBPathValid() const
{
    return m_bIsADBPathValid;
}

const gtString& LPGPU2ppADBCommandExecutor::GetADBVersion() const
{
    return m_ADBVersion;
}

const gtString& LPGPU2ppADBCommandExecutor::GetADBRevision() const
{
    return m_ADBRevision;
}

const gtVector<gtString>& LPGPU2ppADBCommandExecutor::GetADBDevices() const
{
    return m_ADBDevices;
}

bool LPGPU2ppADBCommandExecutor::IsADBRunning() const
{
    return m_bIsADBRunning;
}

bool LPGPU2ppADBCommandExecutor::IsRAgentInstalled() const
{
    return m_bIsRAgentInstalled;
}

bool LPGPU2ppADBCommandExecutor::IsRAgentRunning() const
{
    return m_bIsRAgentRunning;
}

void LPGPU2ppADBCommandExecutor::OnSetADBPath(const QString &vADBPath)
{
    const osFilePath newADBPath{ acQStringToGTString(vADBPath) };
    if (m_ADBPath != newADBPath)
    {
        m_ADBPath = newADBPath;
        ValidateADBPath();

        emit ADBPathChanged();
    }
}

/// @brief  Handles the timer event that is used to query the status of ADB
///         and the RAgent. Caches the state and emit the appropriate signals
///         when a state changed was detected.
/// @param[in]  vTimerEvent The Qt timer event. Not currently used.
void LPGPU2ppADBCommandExecutor::timerEvent(QTimerEvent *vTimerEvent)
{
    GT_IF_WITH_ASSERT(vTimerEvent != nullptr)
    {
        QueryADBAndRAgentStatus();
    }
}

/// @brief  Starts the timer used to monitor the ADB and RAgent status.
void LPGPU2ppADBCommandExecutor::OnStartMonitoring()
{
    // Newer versions of Qt have an overloaded startTimer that takes a std::chrono::milliseconds value.
    m_adbObserverTimerId = startTimer(gs_ADBMonitorTimerInterval.count());
}

/// @brief  Handles request to start ADB as a server.
/// @warning    If ADB is already running this method does nothing.
void LPGPU2ppADBCommandExecutor::OnStartADB()
{
    if (!m_bIsADBRunning)
    {
        gtString output;
        if (ExecADBCommand(L"start-server", output) == lpgpu2::PPFnStatus::success)
        {
            const auto bIsADBRunning = GetADBRunningStatus();
            SetADBRunning(bIsADBRunning);
        }
    }
}

/// @brief Handles the request to stop the ADB server.
/// @warning If ADB is not running this method does nothing.
void LPGPU2ppADBCommandExecutor::OnStopADB()
{
    if (m_bIsADBRunning)
    {
        gtString output;
        if (ExecADBCommand(L"kill-server", output) == lpgpu2::PPFnStatus::success)
        {
            const auto bIsADBRunning = GetADBRunningStatus();
            SetADBRunning(bIsADBRunning);
        }
    }
}

/// @brief  Handles the request to start he RAgent on the device.
/// @warning This method requires ADB to be running and the RAgent to be installed and not running.
void LPGPU2ppADBCommandExecutor::OnStartRAgent()
{
    if (m_bIsADBRunning && m_bIsRAgentInstalled && !m_bIsRAgentRunning)
    {
        gtString output;
        if (ExecADBCommand(L"shell am start -W -n agent.remote.lpgpu2.lpgpu2ragent/agent.remote.lpgpu2.lpgpu2ragent.MainActivity -a android.intent.action.MAIN -c android.intent.category.LAUNCHER", output) == lpgpu2::PPFnStatus::success)
        {
            const auto bIsRAgentRunning = GetRAgentRunningStatus();
            SetRAgentRunning(bIsRAgentRunning);
        }
    }
}

/// @brief  Handles the request to stop the RAgent on the device.
/// @warning    This method requires ADB to be running, the RAgent to be installed and running.
void LPGPU2ppADBCommandExecutor::OnStopRAgent()
{
    if (m_bIsADBRunning && m_bIsRAgentInstalled && m_bIsRAgentRunning)
    {
        gtString output;
        ExecADBCommand(L"shell am force-stop agent.remote.lpgpu2.lpgpu2ragent", output);
        const auto bIsRAgentRunning = GetRAgentRunningStatus();
        SetRAgentRunning(bIsRAgentRunning);
    }
}

/// @brief  Handles the request to restart the RAgent. This effectively calls OnStopRAgent() and OnStartRAgent().
/// @see LPGPU2ppADBCommandExecutor::OnStartRAgent()
/// @see LPGPU2ppADBCommandExecutor::OnStopRAgent().
void LPGPU2ppADBCommandExecutor::OnRestartRAgent()
{
    OnStopRAgent();
    OnStartRAgent();
}

/// @brief  Handles the request to do a ADB port forward command.
/// @param[in] vOrigPort The port of origin.
/// @param[in] vDestPort The port of destiny.
/// @warning    This method requires ADB to be running.
void LPGPU2ppADBCommandExecutor::OnPortForward(unsigned short vOrigPort, unsigned short vDestPort)
{
    if (m_bIsADBRunning)
    {
        gtString portForwardArgs;
        portForwardArgs << L"forward tcp:" << vOrigPort << " tcp:" << vDestPort;

        gtString output;
        ExecADBCommand(portForwardArgs, output);
    }
}

/// @brief  Handles the request to enable the LPGPU2 Vulkan Layer on the connected device.
/// @warning This method requires ADB to be running.
void LPGPU2ppADBCommandExecutor::OnEnableLPGPU2VulkanLayer()
{
    if (m_bIsADBRunning)
    {
        if (IsDeviceAttached())
        {
            gtString output;
            ExecADBCommand(L"shell setprop debug.vulkan.layers VK_LAYER_LPGPU2_interposer", output);
        }
    }
}

/// @brief  Handles the request to install an APK on the device.
/// @param[in]  vApkFilePath    The path of the APK to install on the device.
/// @warning    This method requires ADB to be running and a device to be attached.
/// @warning    This emits the signal RAgentInstalledStatusChanged(false) in case no
///             devices are attached.
void LPGPU2ppADBCommandExecutor::OnInstallAPK(const QString &vApkFilePath)
{
    if (m_bIsADBRunning)
    {
        if (IsDeviceAttached())
        {
            gtString installCmdArgs;
            installCmdArgs << "install -r " << acQStringToGTString(vApkFilePath);

            gtString output;
            ExecADBCommand(installCmdArgs, output);
        }
        else
        {
            // Force the signal to be emitted.
            const auto bRAgentInstalled = false;
            emit RAgentInstalledStatusChanged(bRAgentInstalled);
        }
    }
}

/// @brief              Execute the adb command line tool, from the current configured ADB path with the
///                     specified arguments.
/// @param[in]  vArgs   The arguments to the ADB command.
/// @param[out] vOutput The output of the ADB command executed with the provided arguments.
/// @param[in]  vNoDevice   When set to true, does not execute the command on a specific device even if
///                         one is selected. Defaults to false.
/// @return     status  success = The command was executed successfully and the output will be in vOutput,
///                     failure = The command failed to execute.
/// @warning    The error message for the command will be written to CodeXL's log.
lpgpu2::PPFnStatus LPGPU2ppADBCommandExecutor::ExecADBCommand(const gtString &vArgs, gtString &vOutput, bool vNoDevice /*=false*/) const
{
    auto bReturn = lpgpu2::PPFnStatus::failure;

    if (m_bIsADBPathValid)
    {
        auto adbCommand = m_ADBPath.asString();
        if (m_ADBActiveDevice != L"" && !vNoDevice)
        {
            adbCommand << L" -s " << m_ADBActiveDevice;
        }
        adbCommand << L" " << vArgs;
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Executing ADB command: %ls", adbCommand.asCharArray());

        bReturn = LPGPU2osExecAndGrabOutput(adbCommand.asASCIICharArray(), gs_bCancelFlag, vOutput) ? lpgpu2::PPFnStatus::success : lpgpu2::PPFnStatus::failure;

        if (bReturn == lpgpu2::PPFnStatus::failure)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Error: %ls", vOutput.asCharArray());
        }
    }

    return bReturn;
}

/// @brief   Utility function used to query whether or not the ADB process is running on the current machine.
/// @return  bool   true = ADB is running (the process "adb.exe" was found on Windows, and "adb" on Linux,
///                 false = ADB is not running.
bool LPGPU2ppADBCommandExecutor::GetADBRunningStatus() const
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS    
    return osIsProcessAlive(L"adb.exe");
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    return osIsProcessAlive(L"adb");
#else
#error Unsupported OS
#endif
}

/// @brief  Utility function used to query the installation status of the RAgent.
/// @return bool    true = The RAgent is installed on the connected device,
///                 false = The RAgent is not installed on the connected device.
bool LPGPU2ppADBCommandExecutor::GetRAgentInstallationStatus() const
{
    auto bReturn = false;

    gtString output;
    if (ExecADBCommand(L"shell pm list packages | grep package:agent.remote.lpgpu2.lpgpu2ragent", output) == lpgpu2::PPFnStatus::success)
    {
        bReturn = output.trim().compareNoCase(L"package:agent.remote.lpgpu2.lpgpu2ragent") == 0;
    }

    return bReturn;
}

/// @brief  Utility function used to query the running status of the RAgent on the connected device.
/// @return bool    true = The RAgent is running on the connected device,
///                 false = The RAgent is not running on the connected device.
bool LPGPU2ppADBCommandExecutor::GetRAgentRunningStatus() const
{
    auto bReturn = false;

    gtString output;
    if (ExecADBCommand(L"shell ps | grep agent.remote.lpgpu2.lpgpu2ragent", output) == lpgpu2::PPFnStatus::success)
    {
        bReturn = !output.isEmpty();
    }

    return bReturn;
}

/// @brief  Utility function to query if there is, at least, one device connected to the local machine.
/// @return bool    true = There is, at least, one device connected to the local machine,
///                 false = No devices are connected on the local machine.
bool LPGPU2ppADBCommandExecutor::IsDeviceAttached() const
{
    auto bReturn = false;

    gtString output;
    if (ExecADBCommand(L"devices", output) == lpgpu2::PPFnStatus::success)
    {
        const auto numberOfLines = output.count(L'\n');
        bReturn = numberOfLines > 1;
    }

    return bReturn;
}

/// @brief Utility function to get the list of all attached devices.
/// @return Vector of all found devices.
gtVector<gtString> LPGPU2ppADBCommandExecutor::GetAttachedDevices() const
{
    gtVector<gtString> vecReturn;

    gtString output;
    if (ExecADBCommand(L"devices", output) == lpgpu2::PPFnStatus::success)
    {
        gtStringTokenizer lineTokenizer {output, L'\n'};
        gtString line;

        // Ignore the first line as it contains the list heading:
        lineTokenizer.getNextToken(line);

        while (lineTokenizer.getNextToken(line))
        {
            gtStringTokenizer tabTokenizer {line, L'\t'};
            gtString deviceName;
            tabTokenizer.getNextToken(deviceName);

            // Get readable model name:
            const auto bNoDevice = true;
            gtString deviceModelCmd = L"-s ";
            deviceModelCmd += deviceName;
            deviceModelCmd += L" shell getprop ro.product.model";
            if (ExecADBCommand(deviceModelCmd, output, bNoDevice) == lpgpu2::PPFnStatus::success)
            {
                deviceName += L" (";
                deviceName += output.trim();
                deviceName += L")";
            }

            vecReturn.push_back(deviceName);
        }
    }

    return vecReturn;
}

/// @brief Set the name of the device to send all ADB commands to.
/// @param deviceName The name of the device, as it appears in ADB.
void LPGPU2ppADBCommandExecutor::SetActiveDevice(const gtString& deviceName)
{
    // The format is 'serialno (devicename)', we just want the serial:
    gtStringTokenizer spaceTokenizer {deviceName, L' '};
    spaceTokenizer.getNextToken(m_ADBActiveDevice);
}

/// @brief  Try to validate the current ADB path by running the command "adb version" and
///         parsing the output. will try to extract the Version and Revision as printed by
///         the command.
void LPGPU2ppADBCommandExecutor::ValidateADBPath()
{
    m_bIsADBPathValid = false;

    gtString adbVersionCmd;
    adbVersionCmd << m_ADBPath.asString() << L" version";

    // Try to get the adb version
    gtString output;
    if (LPGPU2osExecAndGrabOutput(adbVersionCmd.asASCIICharArray(), gs_bCancelFlag, output))
    {
        gtStringTokenizer tokenizer{ output, L"\n" };
        gtString line;
        while (tokenizer.getNextToken(line))
        {
            gtString searchText = L"Android Debug Bridge version";
            if (line.find(searchText) != -1)
            {
                line.getSubString(searchText.length() + 1, line.length(), m_ADBVersion);
            }

            searchText = L"Revision";
            if (line.find(searchText) != -1)
            {
                line.getSubString(searchText.length() + 1, line.length(), m_ADBRevision);
            }

            searchText = L"Version";
            if (line.find(searchText) != -1)
            {
                line.getSubString(searchText.length() + 1, line.length(), m_ADBRevision);
            }
        }

        m_bIsADBPathValid = !m_ADBVersion.isEmpty();
    }
}

/// @brief  Utility function to query the status of ADB and the RAgent on the
///         currently connected device.
void LPGPU2ppADBCommandExecutor::QueryADBAndRAgentStatus()
{
    if (m_bIsADBPathValid)
    {
        const auto bIsADBRunning = GetADBRunningStatus();
        SetADBRunning(bIsADBRunning);

        if (bIsADBRunning)
        {
            auto&& vecActiveDevices = GetAttachedDevices();

            // If no device is active, select the first one available:
            if (m_ADBActiveDevice == L"")
            {                
                if (!vecActiveDevices.empty())
                {
                    SetActiveDevice(vecActiveDevices.front());
                }                
            }

            SetADBDevices(std::move(vecActiveDevices));

            const auto bIsRAgentInstalled = GetRAgentInstallationStatus();
            SetRAgentInstalledStatus(bIsRAgentInstalled);

            if (bIsRAgentInstalled)
            {
                const auto bIsRAgentRunning = GetRAgentRunningStatus();
                SetRAgentRunning(bIsRAgentRunning);
            }
        }
    }
}


/// @brief  Set the running status of ADB and emits the signal ADBStatusChaged
///         with the new status.
/// @param[in]  vbIsRunning Whether or not ADB is running.
void LPGPU2ppADBCommandExecutor::SetADBRunning(bool vbIsRunning)
{
    if (m_bIsADBRunning != vbIsRunning)
    {
        m_bIsADBRunning = vbIsRunning;
        emit ADBStatusChanged(m_bIsADBRunning);
    }
}

/// @brief  Set the installed status of the RAgent and emits the signal
///         RAgentInstalledStatusChanged with the new status.
/// @param[in]  vbIsInstalled   Whether or not the RAgent is installed.
void LPGPU2ppADBCommandExecutor::SetRAgentInstalledStatus(bool vbIsInstalled)
{
    if (m_bIsRAgentInstalled != vbIsInstalled)
    {
        m_bIsRAgentInstalled = vbIsInstalled;
        emit RAgentInstalledStatusChanged(m_bIsRAgentInstalled);
    }
}

///@brief  Set the RAgent running status and emits the signal RAgentStatusChanged
///        with the new status.
/// @param[in] vbIsRunning  Whether or not the RAgent is running.
void LPGPU2ppADBCommandExecutor::SetRAgentRunning(bool vbIsRunning)
{
    if (m_bIsRAgentRunning != vbIsRunning)
    {
        m_bIsRAgentRunning = vbIsRunning;
        emit RAgentStatusChanged(m_bIsRAgentRunning);
    }
}

void LPGPU2ppADBCommandExecutor::SetADBDevices(gtVector<gtString> &&vNewDevices)
{
    gtVector<gtString> newDevices;

    if (!m_ADBDevices.empty() && vNewDevices.empty())
    {
        // All devices removed
        m_ADBDevices.clear();
        SetActiveDevice(L"");
        emit ADBDevicesChanged();
    }
    else
    {
        // See if we have new devices available.
        if (vNewDevices.size() != m_ADBDevices.size())
        {
            m_ADBDevices = vNewDevices;
            emit ADBDevicesChanged();
        }
    }
}

/// @brief Slot called when a change to the active ADB device is requested.
/// @param vDeviceName  The name of the device to switch to.
void LPGPU2ppADBCommandExecutor::OnSetADBDevice(const QString &vDeviceName)
{
    // The format is 'serialno (devicename)', we just want the serial:
    const auto deviceName = acQStringToGTString(vDeviceName);
    gtString deviceSerial;
    gtStringTokenizer spaceTokenizer {deviceName, L' '};
    spaceTokenizer.getNextToken(deviceSerial);

    if (deviceSerial != m_ADBActiveDevice)
    {
        if (m_bIsADBRunning)
        {
            // First cancel existing port forwards that could conflict:
            gtString output;
            ExecADBCommand(L"forward --remove-all", output);
        }

        m_ADBActiveDevice = deviceSerial; //acQStringToGTString(vDeviceName);
        emit ADBActiveDeviceChanged(acGTStringToQString(deviceSerial));
        emit ADBStatusChanged(m_bIsADBRunning);
    }
}
