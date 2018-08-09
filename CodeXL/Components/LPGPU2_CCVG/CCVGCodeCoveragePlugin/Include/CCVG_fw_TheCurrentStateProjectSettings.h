// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateProjectSettings interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTSETTINGS_H_INCLUDE
#define LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTSETTINGS_H_INCLUDE

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

// Declarations:
class TiXmlNode;

namespace ccvg {
namespace fw {

/// @brief    CCVGFWTheCurrentStateProjectSettings provides information on the 
///           current project settings of the CCVG project in use. It does not
///           handle the project's data.
///           A project's settings are loaded from XML persistent data reseting
///           any previous existing project's settings (as expected) however
///           loading the same project again also resets that same project 
///           again.
///           CCVGFWTheCurrentStateProjectSettings is not a singleton however
///           it lives as long as CCVGFWTheCurrentState singleton. There can 
///           only be one project loaded at a time. The settings data is reset
///           and reloaded for each new or same project loaded. via 
///           CCVGGUIProjectSettings.
/// @warning  File names extension (and so file path length) limited by 
///           OS_MAX_PATH which is 260 characters. Windows10 can have longer
///           paths.
///           CCVG's project settings can be lost should a project with CCVG
///           settings be loaded into CodeXL where the CCVG plugin is not
///           built in to it (enabled). Settings are written back out to the 
///           project files from all present enabled plugins. Since CCVG is
///           not there to write its bit then the CCVG settings are not 
///           recorded. Loading the project into CodeXL with CCVG enabled
///           will show a project where CCVG settings are set to default.
///           This behaviour is the same for any plugin which is not built in
///           at the time.
/// @see      CCVGGUIProjectSettings.
/// @date     08/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGFWTheCurrentStateProjectSettings final
: public CCVGClassCmnBase
{
// Data structures:
public:
/// @brief The data structure containing all CCVG operational parameters' XML
///        tag IDs. They each must be unique and not be empty.
struct SettingsXMLTagIDs
{
  friend CCVGFWTheCurrentStateProjectSettings;

// Methods:
public:
  SettingsXMLTagIDs();

  const gtString& GetCcvgProjectEnabledFlag() const { return ccvgProjectEnabledFlag;  }
  const gtString& GetCcvgFolderDirResult() const { return ccvgFolderDirResult; }
  const gtString& GetCcvgFolderDirSource() const { return ccvgFolderDirSource; }
  const gtString& GetCcvgFolderDirDriver() const { return ccvgFolderDirDriver; }
  const gtString& GetCcvgFolderDirExecuteable() const { return ccvgFolderDirExecuteable; }
  const gtString& GetCcvgFilesExtnResult() const { return ccvgFilesExtnResult; }
  const gtString& GetCcvgFilesExtnSource() const { return ccvgFilesExtnSource; }
  const gtString& GetCcvgFilesExtnDriver() const { return ccvgFilesExtnDriver; }
  const gtString& GetCcvgDriverFilesMandatoryFlag() const { return ccvgDriverFilesMandatoryFlag; }
  const gtString& GetCcvgFilesExtnExecuteable() const { return ccvgFilesExtnExecuteable; }
  const gtString& GetPreviousPCId() const { return previousPCId; }
  const gtString& GetPreviousUserId() const { return previousUserId; }

// Attributes:
private:
  gtString ccvgProjectEnabledFlag;
  gtString ccvgFolderDirResult;
  gtString ccvgFolderDirSource;
  gtString ccvgFolderDirDriver;
  gtString ccvgFolderDirExecuteable;
  gtString ccvgFilesExtnResult;
  gtString ccvgFilesExtnSource;
  gtString ccvgFilesExtnDriver;
  gtString ccvgDriverFilesMandatoryFlag;
  gtString ccvgFilesExtnExecuteable;
  gtString previousPCId;
  gtString previousUserId;
};

/// @brief The data structure containing all CCVG operational parameters 
///        settings.
struct Settings
{
  friend CCVGFWTheCurrentStateProjectSettings;

// Methods:
public:
  Settings();
  Settings(bool vbProjectEnabledFlag,
           wchar_t *vpFolderDirResult,
           wchar_t *vpFolderSource,
           wchar_t *vpFolderDirDriver,
           wchar_t *vpFolderDirExecuteable,
           wchar_t *vpFolderExtnResult,
           wchar_t *vpFolderExtnSource,
           wchar_t *vpFolderExtnDriver,
           bool vbDriverFilesMandatory, 
           wchar_t *vpFolderExtnExecuteable,
           gtString vPreviousPCId,
           gtString vPreviousUserId);

  const bool      GetCcvgProjectEnabledFlag() const { return ccvgProjectEnabledFlag; }
  const gtString& GetCcvgFolderDirResult() const { return ccvgFolderDirResult; }
  const gtString& GetCcvgFolderDirSource() const { return ccvgFolderDirSource; }
  const gtString& GetCcvgFolderDirDriver() const { return ccvgFolderDirDriver; }
  const gtString& GetCcvgFolderDirExecuteable() const { return ccvgFolderDirExecuteable; }
  const gtString& GetCcvgFilesExtnResult() const { return ccvgFilesExtnResult; }
  const gtString& GetCcvgFilesExtnSource() const { return ccvgFilesExtnSource; }
  const gtString& GetCcvgFilesExtnDriver() const { return ccvgFilesExtnDriver; }
  const bool      GetCcvgDriverFilesMandatoryFlag() const { return ccvgDriverFilesMandatoryFlag; }
  const gtString& GetCcvgFilesExtnExecuteable() const { return ccvgFilesExtnExecuteable; }
  const gtString& GetPreviousPCId() const { return previousPCId; }
  const gtString& GetPreviousUserId() const { return previousUserId; }

// Overrideable:
public:
  virtual status SetCcvgProjectEnabledFlag(bool vbFlag) { ccvgProjectEnabledFlag = vbFlag; return success; }
  virtual status SetCcvgFolderDirResult(const gtString &vData) { ccvgFolderDirResult = vData; return success; }
  virtual status SetCcvgFolderDirSource(const gtString &vData) { ccvgFolderDirSource = vData; return success; }
  virtual status SetCcvgFolderDirDriver(const gtString &vData) { ccvgFolderDirDriver = vData; return success; }
  virtual status SetCcvgFolderDirExecuteable(const gtString &vData) { ccvgFolderDirExecuteable = vData; return success; }
  virtual status SetCcvgFilesExtnResult(const gtString &vData) { ccvgFilesExtnResult = vData; return success; }
  virtual status SetCcvgFilesExtnSource(const gtString &vData) { ccvgFilesExtnSource = vData; return success; }
  virtual status SetCcvgFilesExtnDriver(const gtString &vData) { ccvgFilesExtnDriver = vData; return success; }
  virtual status SetCcvgDriverFilesMandatoryFlag(bool vbData) { ccvgDriverFilesMandatoryFlag = vbData; return success; }
  virtual status SetCcvgFilesExtnExecuteable(const gtString &vData) { ccvgFilesExtnExecuteable = vData; return success; }
  virtual status SetPreviousPCId(const gtString& vrPreviousPCId) { previousPCId = vrPreviousPCId; return success; }
  virtual status SetPreviousUserId(const gtString& vrPreviousUserId) { previousUserId = vrPreviousUserId; return success; }

// Attributes:
protected:
  bool     ccvgProjectEnabledFlag;
  gtString ccvgFolderDirResult;
  gtString ccvgFolderDirSource;
  gtString ccvgFolderDirDriver;
  gtString ccvgFolderDirExecuteable;
  gtString ccvgFilesExtnResult;
  gtString ccvgFilesExtnSource;
  gtString ccvgFilesExtnDriver;
  gtString ccvgFilesExtnExecuteable;
  bool     ccvgDriverFilesMandatoryFlag;
  gtString previousPCId;
  gtString previousUserId;
};

/// @brief The data structure containing all CCVG operational parameters 
///        settings.
struct SettingsConst
: public Settings
{
  friend CCVGFWTheCurrentStateProjectSettings;

  // Methods:
public:
  SettingsConst();
  SettingsConst(bool vbProjectEnabledFlag,
                wchar_t *vpFolderDirResult,
                wchar_t *vpFolderSource,
                wchar_t *vpFolderDirDriver,
                wchar_t *vpFolderDirExecuteable,
                wchar_t *vpFolderExtnResult,
                wchar_t *vpFolderExtnSource,
                wchar_t *vpFolderExtnDriver,
                bool vbDriverFilesMandatory,
                wchar_t *vpFolderExtnExecuteable,
                gtString vPreviousPCId,
                gtString vPreviousUserId);

// Overriden:
private:
  // From Settings
  status SetCcvgProjectEnabledFlag(bool vbFlag) override { ccvgDriverFilesMandatoryFlag = vbFlag; return success; }
  status SetCcvgFolderDirResult(const gtString &vData) override { ccvgFolderDirResult = vData; return success; }
  status SetCcvgFolderDirSource(const gtString &vData) override { ccvgFolderDirSource = vData; return success; }
  status SetCcvgFolderDirDriver(const gtString &vData) override { ccvgFolderDirDriver = vData; return success; }
  status SetCcvgFolderDirExecuteable(const gtString &vData) override { ccvgFolderDirExecuteable = vData; return success; }
  status SetCcvgFilesExtnResult(const gtString &vData) override { ccvgFilesExtnResult = vData; return success; }
  status SetCcvgFilesExtnSource(const gtString &vData) override { ccvgFilesExtnSource = vData; return success; }
  status SetCcvgFilesExtnDriver(const gtString &vData) override { ccvgFilesExtnDriver = vData; return success; }
  status SetCcvgDriverFilesMandatoryFlag(bool vbData) override { ccvgDriverFilesMandatoryFlag = vbData; return success; }
  status SetCcvgFilesExtnExecuteable(const gtString &vData) override { ccvgFilesExtnExecuteable = vData; return success; }
  status SetPreviousPCId(const gtString& vrPreviousPCId) override { previousPCId = vrPreviousPCId; return success; }
  status SetPreviousUserId(const gtString& vrPreviousUserId) override { previousUserId = vrPreviousUserId; return success; }
};

// Statics:
public:
  static const gtString&          GetExtnName();
  static const SettingsConst&     GetSettingsDefault();
  static const SettingsXMLTagIDs& GetSettingsXMLTagIDs();
  static const gtString&          GetExtnNameValidChars();

// Methods:
public:
  CCVGFWTheCurrentStateProjectSettings();
  ~CCVGFWTheCurrentStateProjectSettings();
  const Settings& GetSettingsWorking() const;
  status          SetSettingsWorking(const Settings &vUpdateSettings);
  void            SettingsWorkingResetToDefault();
  bool            ChkSettingsWorkingValid(const Settings &vSettings, gtString &vInvalidMessageStr);
  //
  // Load save project data and settings
  status GetXMLDataString(gtString &vProjectAsXMLString);
  status SetDataFromXMLString(const gtString &vProjectAsXMLString, const TiXmlNode &vMainNode);
  //
  // Handle user and computer IDs 
  bool IsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject() const;
  void SetUserAndPCTheSameAsPreviousUseOfTheCodeXLProject();
  void SetUserEditedProjectSettingsOnThisComputer(bool vbYes);
  bool GetUserEditedProjectSettingsOnThisComputer() const;

// Methods:
private:
  bool  IsValidFileExtn(const gtString &vExtn) const;
  bool  IsValidDirPath(const gtString &vPath) const;
  //
  // Handle user and computer IDs
  const gtString& GetComputerIDPrevious() const;
  const gtString& GetUserIDPrevious() const;
  void            DetermineProjectMovedPCOrUser();
  bool            IsNewDifferentProject();

// Attributes:
private:
  static gtString          ms_xmlProjExtnName;
  static gtString          ms_xmlProjSettingsName;
  static SettingsConst     ms_settingsDefault;              // The starting or reset data values for CCVG operational parameters
  static SettingsXMLTagIDs ms_xmlSettingTagIDs;
  static gtString          ms_ccvgFilesExtnsValidCharsNonAlpha;
  //
  Settings m_settingsWorking; // User entered data values for CCVG operational parameters.
  //
  // Handle user and computer IDs 
  bool m_bIsUserAndPCTheSameAsPreviousUseOfTheCodeXLProject; // True = yes same user and PC, false = different.
  bool m_bUserEnabledProjForThisComputer;                    // True = user edited CCVG project settings, false = not yet
  //
  // Project information
  gtString m_projectNameCurrent;
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_FW_THECURRENTSTATEPROJECTSETTINGS_H_INCLUDE