// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  Defines the object responsible for executing ADB commands
///
/// LPGPU2ppADBCommandExecutor class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_ADB_COMMAND_EXECUTOR_H_INCLUDE
#define LPGPU2_PP_ADB_COMMAND_EXECUTOR_H_INCLUDE

// Qt:
#include <QObject>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

/// @brief  LPGPU2ppADBCommandExecutor is a class that abstracts commands
///         to be executed through the Android Debugger Bridge (ADB) command line
///         tool. This class is a QObject that is meant to be used in conjunction
///         with a QThread, since it will call blocking operations that can cause
///         the GUI to hang. This class is designed to emit signals reporting the
///         status of both ADB and the LPGPu2 remote agent running on the device.
///         This class works extremely well with class LPGPU2ppADBCommands.
/// @see    LPGPU2ppADBCommands.
/// @date   18/01/2018
/// @author Thales Sabino
// clang-format off
class LPGPU2ppADBCommandExecutor final : public QObject
{
    Q_OBJECT

// Methods:
public:
    explicit LPGPU2ppADBCommandExecutor(const osFilePath &vAdbPath, QObject *vpParent = nullptr);
    ~LPGPU2ppADBCommandExecutor() override;

    bool IsADBPathValid() const;
    const gtString& GetADBVersion() const;
    const gtString& GetADBRevision() const;
    const gtVector<gtString>& GetADBDevices() const;

    bool IsADBRunning() const;
    bool IsRAgentInstalled() const;
    bool IsRAgentRunning() const;

    void SetActiveDevice(const gtString& deviceName);

// Methods:
protected:

    // From QObject
    void timerEvent(QTimerEvent *vTimerEvent) override;

// Signals:
signals:    
    void ADBPathChanged();
    void ADBStatusChanged(bool vbIsAdbRunning);
    void ADBActiveDeviceChanged(const QString &vDeviceName);
    void RAgentInstalledStatusChanged(bool vbIsInstalled);
    void RAgentStatusChanged(bool vbIsRunning);
    void PreviousActionsCompleted(int vActionsGroupId);    
    void ADBDevicesChanged();

// Slots:
public slots:
    void OnSetADBPath(const QString &vAdbPath);
    void OnStartMonitoring();
    void OnStartADB();
    void OnStopADB();
    void OnStartRAgent();
    void OnStopRAgent();
    void OnRestartRAgent();
    void OnPortForward(unsigned short vOrigPort, unsigned short vDestPort);
    void OnEnableLPGPU2VulkanLayer();
    void OnInstallAPK(const QString &vApkFilePath);
    void OnSetADBDevice(const QString &vDeviceName);

// Methods:
private:   
    lpgpu2::PPFnStatus ExecADBCommand(const gtString &vArgs, gtString &vOutput, bool vNoDevice = false) const;
    bool GetADBRunningStatus() const;
    bool GetRAgentInstallationStatus() const;
    bool GetRAgentRunningStatus() const;
    bool IsDeviceAttached() const;
    void ValidateADBPath();
    gtVector<gtString> GetAttachedDevices() const;
    void QueryADBAndRAgentStatus();

    void SetADBRunning(bool vbIsRunning);
    void SetRAgentInstalledStatus(bool vbIsInstalled);
    void SetRAgentRunning(bool vbIsRunning);
    void SetADBDevices(gtVector<gtString> &&vNewDevices);

// Attributes:
private:
    bool m_bIsRAgentInstalled = false;  //!< Whether or not the LPGPU2 RAgent is installed in the device.
    bool m_bIsRAgentRunning = false;    //!< Whether or not the LPGPU2 RAgent is running in the device.
    bool m_bIsADBRunning = false;       //!< Whether or not the ADB is running as a server process in the local machine.

    int m_adbObserverTimerId = -1;  //!< The timer id used by this object to monitor the ADB status.

    bool m_bIsADBPathValid = false;
    osFilePath m_ADBPath;
    gtString m_ADBVersion;
    gtString m_ADBRevision;
    gtVector<gtString> m_ADBDevices;
    gtString m_ADBActiveDevice;
};
// clang-format on

#endif // LPGPU2_PP_ADB_COMMAND_EXECUTOR_H_INCLUDE