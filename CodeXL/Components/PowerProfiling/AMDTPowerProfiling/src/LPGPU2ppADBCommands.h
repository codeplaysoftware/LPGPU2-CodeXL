// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  Defines the object responsible for executing ADB commands
///
/// LPGPU2ppADBCommands class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_ADB_COMMANDS_H_INCLUDE
#define LPGPU2_PP_ADB_COMMANDS_H_INCLUDE

// Qt:
#include <QThread>

// STL:
#include <chrono>
#include <memory>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Forward declarations:
class osFilePath;
class LPGPU2ppADBCommandExecutor;

/// @brief Enum used to check the status of functions that can be asynchronous.
enum class EWaitStatus
{
    kNeedToWait,    //!< The caller has to wait the command to complete.
    kNoNeedToWait   //!< The caller does not need to wait and the action is completed already.
};

/// @brief  LPGPU2ppADBCommands is the interface that can be used by CodeXL to
///         issue commands to a connected Android device and to monitor the status
///         of device. It uses the class LPGPU2ppADBCommandExecutor in a separate
///         thread and signal/slots to be able to issue the commands and wait for
///         result without blocking the GUI. This is a singleton class.
/// @see    LPGPU2ppADBCommandExecutor.
/// @date   18/01/2018
/// @author Thales Sabino.
// clang-format off
class LPGPU2ppADBCommands final : public QObject
{
    Q_OBJECT
// Typedefs:
public:
    using CommandGroupId = int;

// Declarations:
public:
    static constexpr CommandGroupId InvalidCommandGroupId = -1;
    static const std::chrono::seconds MaxStartAdbWaitTime;
    static const std::chrono::seconds MaxStopRAgentWaitTime;
    static const std::chrono::seconds MaxStartRAgentWaitTime;

// Methods:
public:
    // singleton access:
    static LPGPU2ppADBCommands& Instance();    

    void SetADBPath(const osFilePath &vAdbFilePath);

    bool IsADBPathValid() const;
    lpgpu2::PPFnStatus GetADBVersion(gtString &vADBVersion) const;
    lpgpu2::PPFnStatus GetADBRevision(gtString &vADBRevision) const;
    lpgpu2::PPFnStatus GetADBDevices(gtVector<gtString> &vADBDevices) const;
    lpgpu2::PPFnStatus GetADBActiveDevice(gtString& vADBDevice) const;

    void StartMonitoring();
    void StopMonitoring();

    EWaitStatus StartADB();
    EWaitStatus StopADB();

    EWaitStatus StartRAgent();
    EWaitStatus StopRAgent();
    EWaitStatus RestartRAgent();
    EWaitStatus ChangeADBDevice(const QString &vDeviceName);

    bool IsADBRunning() const;
    bool IsRAgentInstalled() const;
    bool IsRAgentRunning() const;

    lpgpu2::PPFnStatus InstallAPK(const osFilePath &vApkFilePath);
    EWaitStatus PortFoward(unsigned short vOrigPort, unsigned short vDestPort);
    void EnableLPGPU2VulkanLayer();

    CommandGroupId WaitForPreviousActions();

// Signals:
signals:
    void ADBPathChanged();
    void ADBStatusChanged(bool vbIsAdbRunning);
    void RAgentInstalledStatusChanged(bool vbIsInstalled);
    void RAgentStatusChanged(bool vbIsRunning);    
    void SetADBPathRequested(const QString &vADBPath);
    void StartADBRequested();
    void StopADBRequested();
    void StartRAgentRequested();
    void StopRAgentRequested();
    void RestartRAgentRequested();
    void PortForwardRequested(unsigned short vOrigPort, unsigned short vDestPort);
    void EnableLPGPU2VulkanLayerRequested();
    void WaitForPreviousActionsRequested(CommandGroupId vActionsGroupId);
    void OnPreviousActionsCompleted(CommandGroupId groupId);
    void InstallAPKRequested(const QString &vApkPath);
    void ADBDeviceChangeRequested(const QString &vDeviceName);
    void ADBDevicesChanged();

// Slots:
private slots:
    void OnADBPathChanged();
    void OnADBStatusChanged(bool vbIsAdbRunning);
    void OnADBActiveDeviceChanged(const QString &vDeviceName);
    void OnADBDevicesChanged();
    void OnRAgentInstalledStatusChanged(bool vbIsRAgentInstalled);
    void OnRAgentStatusChanged(bool vbIsRAgentRunning);

// Methods:
private:    
    // In order to use the unique_ptr for the singleton, we need to give
    // access to the destructor of this class to the default_delete so
    // the unique_ptr can destroy this object. Since the destructor is
    // private, we need the friend keyword here.
    friend struct std::default_delete<LPGPU2ppADBCommands>;

    LPGPU2ppADBCommands(QObject *vpParent = nullptr);
    ~LPGPU2ppADBCommands() override;    

// Attributes:
private:
    static std::unique_ptr<LPGPU2ppADBCommands> m_spMySingleInstance;

    QThread m_ADBCommandExecutorThread;
    LPGPU2ppADBCommandExecutor* m_pADBCommandExecutor = nullptr;

    bool m_bIsADBPathValid = false;
    bool m_bIsRAgentInstalled = false;
    bool m_bIsRAgentRunning = false;
    bool m_bIsADBRunning = false;
    osFilePath m_ADBPath;
    gtString m_ADBVersion;
    gtString m_ADBRevision;
    gtString m_ADBActiveDevice;
    gtVector<gtString> m_ADBDevices;
    CommandGroupId  m_lastActionsGroupId = 0;    
};
// clang-format on

#endif // LPGPU2_PP_ADB_COMMANDS_H_INCLUDE