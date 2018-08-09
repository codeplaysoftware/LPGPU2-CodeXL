// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentState interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_THECURRENTSTATE_H_INCLUDE
#define LPGPU2_CCVG_FW_THECURRENTSTATE_H_INCLUDE

// Std:
#include <vector>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_SingletonBase.h>

// Declarations:
class TiXmlNode;

namespace ccvg {
namespace fw {

// Declarations:
class CCVGFWTheCurrentStateProjectSettings;
class CCVGFWTheCurrentStateProjectData;

/// @brief    CCVGFWTheCurrentState provides information on the application 
///           wide global options current state and passes through from 
///           CCVGFWTheCurrentStateProjectSettings and 
///           CCVGFWTheCurrentStateProjectData project state and project data
///           to the rest of the CCVG plugin codebase. This class also saves
///           and loads that global state and projec state from persistent
///           application XML file. CCVG_gui_ProjectSettings and 
///           CCVG_gui_TabPageSettings (global tool options dialog page)
///           call the *this on load and save of data to XML file.
///           Information on the CodeXL framework and state of the CCVG 
///           execution mode.
///           This information can be used to by the GUI with such things like
///           menu items being checked unchecked enabled or greyed out. 
///           Commands can also use this information to carry out tasks.
/// @warning  None.
/// @see      CCVGFWTheCurrentStateSetIsBusy, 
///           CCVGFWTheCurrentStateProjectSettings,
///           CCVGFWTheCurrentStateProjectData
/// @date     02/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTheCurrentState final
: public CCVGClassCmnBase
, public ISingleton<CCVGFWTheCurrentState>
{
  friend ISingleton<CCVGFWTheCurrentState>;
  
// Data structures:
public:
  /// @brief The data structure containing all CCVG operational parameters' XML
  ///        tag IDs. They each must be unique and not be empty.
  struct SettingsXMLTagIDs
  {
    friend CCVGFWTheCurrentState;

  // Methods:
  public:
    SettingsXMLTagIDs();
    //
    const gtString& GetCcvgAllProjectsEnabledFlag() const { return ccvgAllProjectsEnabledFlag; }

  // Attributes:
  private:
    gtString ccvgAllProjectsEnabledFlag;
  };

  /// @brief The data structure containing all CCVG operational parameters 
  ///        settings. Global settings (main menu Tools --> options).
  struct Settings
  {
    friend CCVGFWTheCurrentState;

  // Methods:
  public:
    Settings();
    Settings(bool vbAllProjectsEnabledFlag);
    //
    const bool GetCcvgAllProjectsEnabledFlag() const { return bCcvgAllProjectsEnabledFlag; }

  // Overrideable:
  public:
    virtual status SetCcvgAllProjectsEnabledFlag(bool vbFlag) { bCcvgAllProjectsEnabledFlag = vbFlag; return success; }

  // Attributes:
  protected:
    bool bCcvgAllProjectsEnabledFlag; // True = CCVG Exec mode fn'ality is enabled, false = disabled
  };

  /// @brief The data structure containing all CCVG operational parameters 
  ///        settings.
  struct SettingsConst
  : public Settings
  {
    friend CCVGFWTheCurrentState;

  // Methods:
  public:
    SettingsConst();
    SettingsConst(bool vbAllProjectsEnabledFlag);

  // Overriden:
  private:
    // From Settings
    virtual status SetCcvgAllProjectsEnabledFlag(bool vbFlag) override { bCcvgAllProjectsEnabledFlag = vbFlag; return success; }
  };

// Classes:
public:
  /// @brief Class to inherited by classes that wish to be notified of when
  ///        project data has been changed.
  class NotifyOnSettingChangedCcvgAllProjectsEnabledFlag
  {
  // Methods:
  public:
    NotifyOnSettingChangedCcvgAllProjectsEnabledFlag();

  // Overrideable:
  public:
    virtual ~NotifyOnSettingChangedCcvgAllProjectsEnabledFlag();
    virtual status OnNotifySettingChangedCcvgAllProjectsEnabledFlag() = 0;
  };

// Statics:
public:
  static const SettingsConst&     GetSettingsDefault();
  static const SettingsXMLTagIDs& GetSettingsXMLTagIDs();
  static const gtString&          GetXMLSectionTitle();

// Methods:
public:
  CCVGFWTheCurrentState();
  ~CCVGFWTheCurrentState();
  //
  status Initialise();
  status Shutdown();
  //
  // Event notifications
  status RegisterForNotificationOnSettingChangedCcvgAllProjectsEnabledFlag(const NotifyOnSettingChangedCcvgAllProjectsEnabledFlag &vObj);
  //
  // CCVG global settings tool wide
  bool            IsInCCVGExecutionMode();
  bool            IsCCVGReadyToWork();
  bool            IsCCVGBusy();
  bool            IsProjectOpen();
  void            SetCCVGIsBusy(bool vbYesBusy);
  void            SetProjectIsOpen(bool vbYesOpen);
  const Settings& GetSettingsWorking() const;
  status          SetSettingsWorking(const Settings &vUpdateSettings);
  void            SettingsWorkingResetToDefault();
  bool            ChkSettingsWorkingValid(const Settings &vSettings, gtString &vInvalidMessageStr);
  const gtString& GetComputerIDCurrent() const;
  const gtString& GetUserIDCurrent() const;
  void            SetComputerIDCurrent(const gtString &vCurrentPCId);
  void            SetUserIDCurrent(const gtString &vUserPCId);
  //
  // CCVG Project Settings extension
  CCVGFWTheCurrentStateProjectSettings& GetProjectSetting();
  CCVGFWTheCurrentStateProjectData&     GetProjectData();
  //
  // Load save project data and settings
  status GetXMLProjSettingsAndProjDataString(gtString &vProjectAsXMLString);
  status SetProjSettingsAndProjDataFromXMLString(const gtString &vProjectAsXMLString);
  //
  // Load save global tool wide settings
  status GetXMLGlobalSettings(gtString &vParamsAsXMLString);
  status SetGlobaljSettings(const gtString &vParamsAsXMLString);

// Typedef:
private:
  using VecNotifySettingChangedCcvgAllProjectsEnabledFlag_t = std::vector<NotifyOnSettingChangedCcvgAllProjectsEnabledFlag *>;

// Methods:
private:
  status FindXMLNode(const gtString &vXMLString, const gtString &vNodeName, TiXmlNode *&vpNodeDeleteMe);
  status NotifySettingChangedCcvgAllProjectsEnabledFlag();

// Attributes:
private:
  static bool                                   ms_bIsCCVGBusy; // True = yes doing some task, false = ready to do work
  static bool                                   ms_bProjOpen;   // True = yes currenty have CodeXL project/CCVG project open ATM, false = project closed no project
  static CCVGFWTheCurrentStateProjectSettings  *ms_pProjSetting;
  static CCVGFWTheCurrentStateProjectData      *ms_pProjData;
  static gtString                               ms_xmlGlobalStateName;
  static SettingsConst                          ms_settingsDefault;   // The starting or reset data values for CCVG operational parameters
  static SettingsXMLTagIDs                      ms_xmlSettingTagIDs;
  //
  Settings m_settingsWorking; // User entered data values for CCVG operational parameters.
  //
  // Handle user and computer IDs 
  gtString m_computerIDCurrent;
  gtString m_userIDCurrent;
  //
  // Event notification
  bool                                                m_bDoNotNotifySettingChangedCcvgAllProjectsEnabledFlag; // True = No notification, false = send notification
  VecNotifySettingChangedCcvgAllProjectsEnabledFlag_t m_vecNotifySettingChangedCcvgAllProjectsEnabledFlag;
};

/// @brief    Auto scope to call CCVGFWTheCurrentState::SetCCVGIsBusy() to set 
///           and unset the CCVG is busy flag.
/// @warning  None.
/// @see      CCVGFWTheCurrentState.
/// @date     29/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTheCurrentStateSetIsBusy final
{
// Methods:
public:
  CCVGFWTheCurrentStateSetIsBusy();
  ~CCVGFWTheCurrentStateSetIsBusy();
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif //LPGPU2_CCVG_FW_THECURRENTSTATE_H_INCLUDE