// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGGUIProjectSettings interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_GUI_PROJECTSETTINGS_H_INCLUDE
#define LPGPU2_CCVG_GUI_PROJECTSETTINGS_H_INCLUDE

// STL:
#include <vector>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_ButtonDirFolder.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_WidgetStringConstants.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>

// Declarations:
class afBrowseAction;
class CCVGWGTLineEdit;
class CCVGWGTGroupBoxHideTooltipOverChild;
namespace ccvg { namespace fw { class CCVGFWTheCurrentState; } }

namespace ccvg {
namespace gui {

/// @brief    CCVGGUIProjectSettings is CCVG's GUI project settings extension 
///           pane to the application's project settings dialog. Derived from
///           afProjectSettingsExtension it implements it also handles project
///           data handling and some project handling (not behaviour this class
///           should do IMHO). Handling for load save project data and closing
///           of a project (which now found to be a event by another name - 
///           apEvent::APP_GLOBAL_VARIABLE_CHANGED + 
///           afGlobalVariableChangedEvent::CURRENT_PROJECT). From *this class
///           CCVG event notifications are sent out to other parts of CCVG.
/// @warning  This object is deleted by afNewProjectDialog after registration.
/// @see      CCVGFWTheCurrentState, CCVGFWTheCurrentStateProjectSettings.
/// @date     07/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGGUIProjectSettings final
: public afProjectSettingsExtension
, public CCVGClassCmnBase
{
  Q_OBJECT

// Classes:
public:
  /// @brief Class to inherited by classes that wish to be notified of when
  ///        current loaded project is closed.
  class NotifyOnProjectClose
  {
  // Methods:
  public:
    NotifyOnProjectClose();

  // Overrideable:
  public:
    virtual ~NotifyOnProjectClose();
    virtual status OnNotifyClose() = 0;
  };
  
// Methods:
public:
  CCVGGUIProjectSettings();
  status Initialise();
  status Shutdown();
  status RegisterForNotificationOnProjectClose(const NotifyOnProjectClose &vObj);

// Overridden:
public:
  // From afProjectSettingsExtension
  virtual ~CCVGGUIProjectSettings() override;
  virtual void      Initialize() override;
  virtual gtString  ExtensionXMLString() override;
  virtual gtString  ExtensionTreePathAsString() override;
  virtual bool      GetXMLSettingsString(gtString &vProjectAsXMLString) override;
  virtual bool      SetSettingsFromXMLString(const gtString &vProjectAsXMLString) override;
  virtual void      RestoreDefaultProjectSettings() override;
  virtual bool      AreSettingsValid(gtString &vInvalidMessageStr) override;
  virtual bool      AreSettingsValidSecondPass(gtString &vInvalidMessageStr) override;
  virtual bool      RestoreCurrentSettings() override;
  virtual bool      SaveCurrentSettings() override;
  virtual bool      CloseCurrentProject() override;
  //
  // From QWidget
  virtual void showEvent(QShowEvent *vpEvent) override;

// Typedef:
private:
  using VecNotifyOnProjectClose_t = std::vector<NotifyOnProjectClose *>;

  // Methods:
private:
  status InitialiseCCVGNotReadyToWork();
  status InitialiseCCVGReadyToWork();
  void   CopySettingsToWidgets(const fw::CCVGFWTheCurrentStateProjectSettings::Settings &vSettings);
  void   CopyWidgetsToSettings(fw::CCVGFWTheCurrentStateProjectSettings::Settings &vCurrentSettings);
  status ExecuteCmdProjDataSetNoDriverFilesReq(bool vbRequired);
  status NotifyProjectDataLoaded();
  status NotifyProjectDataSaved();
  status NotifyProjectClosed();

// Signal slot:
private slots:
  void OnClickBtnGetCCVGResultFolder();
  void OnClickBtnGetCCVGSourceFolder();
  void OnClickBtnGetCCVGDriverFolder();
  void OnClickGrpBxProjectEnabledFlag(bool vbChecked);
  void OnClickBtnGetCCVGExecuteableFolder();

// Attributes:
private:
  bool m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  bool m_bCCVGReadyToWork;  // True = yes all ok, false = one or more CCVG resources not available to useful work
  
  // *this reads and sets settings
  fw::CCVGFWTheCurrentState                *m_pGlobalSettings;
  fw::CCVGFWTheCurrentStateProjectSettings *m_pProjSettings;

  // GUI widgets
  QGroupBox                            *m_pCCVGProjectEnabledFlagGrpBx;
  CCVGWGTLineEdit                      *m_pCCVGResultsFolderPathEdit;          // Holds user defined parameter
  QLineEdit                            *m_pCCVGResultsFilesExtnEdit;           // Holds user defined parameter
  QString                               m_CCVGResultsFolderPathOrigEditStyle;  
  CCVGWGTButtonDirFolder<0>             m_CCVGBtnResultsFolder;
  CCVGWGTLineEdit                      *m_pCCVGSourceFolderPathEdit;           // Holds user defined parameter
  QLineEdit                            *m_pCCVGSourceFilesExtnEdit;            // Holds user defined parameter
  QString                               m_CCVGSourcePathOrigEditStyle;
  CCVGWGTButtonDirFolder<1>             m_CCVGBtnSourceFolder;
  CCVGWGTGroupBoxHideTooltipOverChild  *m_pCCVGDriverGrpBx;
  CCVGWGTLineEdit                      *m_pCCVGDriverFolderPathEdit;           // Holds user defined parameter
  QLineEdit                            *m_pCCVGDriverFilesExtnEdit;            // Holds user defined parameter
  QString                               m_CCVGDriverPathOrigEditStyle;
  CCVGWGTButtonDirFolder<2>             m_CCVGBtnDriverFolder;
  CCVGWGTLineEdit                      *m_pCCVGExecuteableFolderPathEdit;          // Holds user defined parameter
  QLineEdit                            *m_pCCVGExecuteableFilesExtnEdit;           // Holds user defined parameter
  QString                               m_CCVGExecuteableFolderPathOrigEditStyle;
  CCVGWGTButtonDirFolder<3>             m_CCVGBtnExecuteableFolder;

  // Event notifications
  bool                      m_bDoNotNotifyProjectClose; // True = No notification, false = send notification
  VecNotifyOnProjectClose_t m_vecNotifyOnProjectClose;
};
// clang-format on

} // namespace gui
} //namespace ccvg

#endif // LPGPU2_CCVG_GUI_PROJECTSETTINGS_H_INCLUDE