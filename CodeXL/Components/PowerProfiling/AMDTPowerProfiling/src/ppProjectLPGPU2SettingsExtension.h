// Local.
// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppProjectLPGPU2SettingsExtension.h
///
/// @brief New project GUI extension class that allows to inject the LPGPU2
///        Power Profiling GUI into the existing framework
/// @warning When this class was first implemented, it was copied from the
///          existing AMD power profiling one and hence some comments on
///          methods are the old ones.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PPPROJECTSETTINGSEXTENSION_H
#define LPGPU2_PPPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QtWidgets>

#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>

// AMDTRemoteClient:
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppLPGPU2CounterList.h>
#include <AMDTPowerProfiling/src/ppLPGPU2CollectionDefs.h>
#include <AMDTPowerProfiling/src/LPGPU2ppADBCommands.h>

#include <AMDTPowerProfilingMidTier/include/ppLPGPU2TargetCharacteristics.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppTargetDefinition.h>

// Power models config manager
// #include "lpgpuPowerModelsConfigManager.h"
#include <LPGPU2PowerModeling/LPGPU2PowerModelsBase/LPGPU2PowerModelsConfigManager.h>


// STL
#include <memory>
#include <functional>

// Forward declarations
class TiXmlElement;
namespace lpgpu2 {
  enum class PPFnStatus;
  class FrameTerminatorSelWidget;
  class AppsList;
}
class QShowEvent;

// clang-format on
/// @brief Class representing the LPGPU2 extension of the Power Profiling
///        New Project GUI.
class PP_API ppProjectLPGPU2SettingsExtension final
: public afProjectSettingsExtension
{
  Q_OBJECT

// Methods
public:
  ppProjectLPGPU2SettingsExtension();
  ~ppProjectLPGPU2SettingsExtension() override;

  /// Initialize the widget:
  void Initialize() override;

  /// Return the extension name:
  gtString ExtensionXMLString() override;

  /// Return the extension page title:
  gtString ExtensionTreePathAsString() override;

  /// Load / Save the project settings into a string:
  bool GetXMLSettingsString(gtString& projectAsXMLString) override;
  bool SetSettingsFromXMLString(const gtString& projectAsXMLString) override;
  void RestoreDefaultProjectSettings() override;
  bool AreSettingsValid(gtString& invalidMessageStr) override;
  bool AreSettingsValidSecondPass(gtString &vInvalidMessageStr) override;
  bool RestoreCurrentSettings() override;
  bool NotifyHostChange(lpgpu2::HostType hostType) override;

  /// Get the data from the widget:
  bool SaveCurrentSettings() override;

  //++IOR: LPGPU2 Notify extensions the project is closed
  // Notification from the afProjectManager that the project is now closed.
  /// \return bool True = success, false = failure.
  bool CloseCurrentProject() override { return true; }
  //--IOR: LPGPU2 Notify extensions the project is closed

// Attributes
private:
  // Project settings widgets:
  QSpinBox* m_pSamplingIntervalSpinBox           = nullptr;
  afBrowseAction* m_pAdbBrowseAction             = nullptr;
  QLineEdit* m_pAdbTextBox                       = nullptr;
  QLineEdit* m_pIpTextBox                        = nullptr;
  QLineEdit* m_pPortTextBox                      = nullptr;
  QLineEdit* m_pDeviceNameTextBox                = nullptr;
  lpgpu2::AppsList* m_pAppList                   = nullptr;
  lpgpu2::ppCounterList* m_pCounterList          = nullptr;
  QCheckBox* m_pTimelineCheckBox                 = nullptr;
  QCheckBox* m_pGLCheckBox                       = nullptr;
  QCheckBox* m_pEGLCheckBox                      = nullptr;
  QCheckBox* m_pCLCheckBox                       = nullptr;
  QCheckBox* m_pVKCheckBox                       = nullptr;
  //QCheckBox* m_pNemaGFXCheckBox                = nullptr;
  QCheckBox* m_pParamCaptureCheckBox             = nullptr;
  QCheckBox* m_pCaptureScreenshotsCheckBox       = nullptr;
  QCheckBox* m_pGpuTimingCheckBox                = nullptr;
  QComboBox* m_pGpuTimingModeComboBox            = nullptr;
  QCheckBox* m_pCallStacksCheckBox               = nullptr;
  QCheckBox* m_pKHRDebugCheckBox                 = nullptr;
  QCheckBox* m_pShaderSrcCheckBox                = nullptr;
  QSpinBox* m_pStartDrawSpinBox                  = nullptr;
  QSpinBox* m_pStartFrameSpinBox                 = nullptr;
  QSpinBox* m_pStopDrawSpinBox                   = nullptr;
  QSpinBox* m_pStopFrameSpinBox                  = nullptr;
  QRadioButton *m_pManualRadioBtn                = nullptr;
  QRadioButton *m_pTimerRadioBtn                 = nullptr;
  QRadioButton *m_pDurationRadioBtn              = nullptr;
  QRadioButton *m_pExplicitControlRadioBtn       = nullptr;
  QSpinBox* m_pTimerDurationSpinBox              = nullptr;
  QListWidget *m_pPwrModelsNamesList             = nullptr;
  QListWidget *m_pPwrModelsDescList              = nullptr;
  gtString m_validDeviceAddress                  = nullptr;
  QLabel *m_hostModeLabel                        = nullptr;
  QPushButton *m_pGetAppsButton                  = nullptr;
  QLabel *m_pStartDrawLabel                      = nullptr;
  QLabel *m_pStartFrameLabel                     = nullptr;
  QLabel *m_pStopDrawLabel                       = nullptr;
  QLabel *m_pStopFrameLabel                      = nullptr;
  QLabel *m_pTimerDurationLabel                  = nullptr;
  QLabel *m_pADBStatusLabel                      = nullptr;
  QLabel *m_pRAgentStatusLabel                   = nullptr;
  QCheckBox* m_pAlwaysRestartRAgentCheckBox              = nullptr;
  QLabel *m_pApiSelectionLabel                   = nullptr;
  QCheckBox *m_pADBPortForwardCheckBox           = nullptr;
  QPushButton *m_pStartADBButton                 = nullptr;
  QPushButton *m_pInstallRAgentButton            = nullptr;  
  QPushButton *m_pRestartRAgentButton            = nullptr;
  QComboBox* m_pTerminatorsComboBox              = nullptr;
  lpgpu2::FrameTerminatorSelWidget *m_termWidget = nullptr;
  QLabel *m_pADBVersionLabel = nullptr;
  QLabel *m_pADBRevisionLabel = nullptr;
  QComboBox *m_pADBDeviceComboBox = nullptr;


  TargetCharacteristics m_targetCharacteristics;
  lpgpu2::TargetDefinition m_targetDefinition;
  CollectionDefs m_collectionDefs;
  gtString m_appListStr;
  gtString m_selectedAppRowAsStr;
  gtString m_adbPath;
  gtVector<lpgpuPowerModelDescriptor> m_powerModelDescriptors;

// Methods
private:
  lpgpu2::PPFnStatus GetProfilableApplications();
  lpgpu2::PPFnStatus GetTargetDefinition();
  lpgpu2::PPFnStatus GetTargetCharacteristics();
  lpgpu2::PPFnStatus DumpXMLIntoProjectSettings(
    const gtString &fileName, const gtString &fileExt,
    const gtString &xmlTag, gtString& projectAsXMLString);
  lpgpu2::PPFnStatus PopulateCounterList();
  lpgpu2::PPFnStatus PopulatePowerModelsLists();
  lpgpu2::PPFnStatus RestoreAppList();
  lpgpu2::PPFnStatus RestoreCounterList();
  lpgpu2::PPFnStatus RestoreTimelineCheckboxes();
  gtString GetDeviceAddress();
  gtUInt16 GetDevicePortNumber();
  lpgpu2::PPFnStatus GetCXLDaemonClient(CXLDaemonClient **p_DmCli);
  lpgpu2::PPFnStatus GenerateCollectionDefinitionsFromXMLElem(
      TiXmlElement *baseElement);
  bool CreateDaemonConnection(CXLDaemonClient* pDmnClient);
  bool DaemonCommunicate(CXLDaemonClient* pDmnClient,
      std::function<bool()>& func);
  bool GenerateCollectionDefinitions(osFilePath& defFilePath);
  lpgpu2::PPFnStatus SetupAPICheckBoxesFromTargetCharacteristics();

  void StartGetDeviceInfo();
  void GetDeviceInfo();
  void EndGetDeviceInfo();

signals:
  void GetSupportedAppsRequested();

// Methods
private slots:
  void OnmPwrModelsNamesListItemActivated(QListWidgetItem*);
  void OnGetAppsClicked();
  void SetApiSelectionEnabled(int state);
  void OnAppsListItemActivated(QListWidgetItem*);  
  void OnADBPathBrowse();
  void OnADBPathTextChanged(const QString &vNewPath);
  void OnADBStatusChanged(bool vbIsADBRunning);  
  void OnRAgentInstalledStatusChanged(bool vbIsRAgentInstalled);
  void OnRAgentStatusChanged(bool vbIsRAgentRunning);
  void OnStartADB();
  void OnInstallRAgentAPK();
  void OnRestartRAgent();
  void OnADBPreviousActionsCompleted(int vActionsGroupId);
  void OnPortForwardCheckboxStateChanged(int vCheckState);  
  void OnADBDeviceSelectionChanged(const QString &vSelectedDevice);  

  void ResetADBInfoLabels();
  void ResetADBStatusLabels();
  void ResetDeviceInfo();

private:
    LPGPU2ppADBCommands::CommandGroupId m_getDeviceInfoGroupId = LPGPU2ppADBCommands::InvalidCommandGroupId;
    LPGPU2ppADBCommands::CommandGroupId m_installRAgentAPKInfoGroupId = LPGPU2ppADBCommands::InvalidCommandGroupId;
    LPGPU2ppADBCommands::CommandGroupId m_restartRAgentGroupId = LPGPU2ppADBCommands::InvalidCommandGroupId;
    LPGPU2ppADBCommands::CommandGroupId m_startADBGroupId = LPGPU2ppADBCommands::InvalidCommandGroupId;

// Overridden methods
private:
    void showEvent(QShowEvent *event) override;

}; // class ppProjectLPGPU2SettingsExtension
// clang-format on

#endif //LPGPU2_PPPROJECTSETTINGSEXTENSION_H
