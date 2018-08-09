// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGGUITabPageSettings implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_TabPageSettings.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_Functions.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectSettings.h>

namespace ccvg {
namespace gui {

/// @brief    Class constructor. No work is done here by this class. Base class
///           carries out work
/// param[in] vpParent - Qt widget object to *this.
CCVGGUITabPageSettings::CCVGGUITabPageSettings()
: m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_bCCVGReadyToWork(false)
, m_pChkBxCCVGEnableForAllProjects(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGGUITabPageSettings::~CCVGGUITabPageSettings()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUITabPageSettings::Initialise()
{
  if (m_bBeenInitialised)
  {
    return success;
  }

  // Project settings are loaded into widgets in RestoreCurrentSettings()
  m_pCCVGStateSettings = &fw::CCVGFWTheCurrentState::Instance();
  m_bCCVGReadyToWork = m_pCCVGStateSettings->IsCCVGReadyToWork();
  
  m_bBeenInitialised = m_bCCVGReadyToWork ? InitialiseCCVGReadyToWork() : InitialiseCCVGNotReadyToWork();

  return m_bBeenInitialised;
}

/// @brief  Set up the CCVG project settings page of the new or current
///         project settings dialog.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUITabPageSettings::InitialiseCCVGNotReadyToWork()
{
  QVBoxLayout *pMainLayout = nullptr;
  QLabel *pLabelInfo = nullptr;
  QLabel *pLabelStretch = nullptr;
  QLabel *pLabelVersion = nullptr;
  try {
    pMainLayout = new QVBoxLayout;
    pLabelInfo = new QLabel;
    pLabelStretch = new QLabel;
    pLabelVersion = new QLabel;
  }
  catch (...)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_gui_ProjSettings_page_title));
  }

  const wchar_t *pInfo = CCVGRSRC(kIDS_CCVG_STR_gui_ProjectSettings_err_ccvg_not_readyToWork);
  pLabelInfo->setText(acGTStringToQString(pInfo));
  pMainLayout->addWidget(pLabelInfo);

  // Add the vertical expansion
  pLabelStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  pMainLayout->addWidget(pLabelStretch);

  // Add CCVG version information
  pLabelVersion->setText(acGTStringToQString(CCVGUtilGetVersionText()));
  pMainLayout->addWidget(pLabelVersion);

  // Add widgets to layout:
  setLayout(pMainLayout);

  return success;
}

/// @brief  Set up the CCVG global options page.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUITabPageSettings::InitialiseCCVGReadyToWork()
{
  QVBoxLayout *pMainLayout = nullptr;
  QLabel *pLabelStretch = nullptr;
  QLabel *pLabelVersion = nullptr;
  try {
    pMainLayout = new QVBoxLayout;
    pLabelStretch = new QLabel;
    m_pChkBxCCVGEnableForAllProjects = new QCheckBox;
    pLabelVersion = new QLabel;
  }
  catch (...)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_gui_TabPageSettings_err_fail_create_gui_elements));
  }

  // CCVG applicatiob wide settings
  const wchar_t *pText = CCVGRSRC(kIDS_CCVG_STR_gui_TabPageSettings_txtEnableForAllProjs);
  m_pChkBxCCVGEnableForAllProjects->setText(acGTStringToQString(pText));
  pMainLayout->addWidget(m_pChkBxCCVGEnableForAllProjects);

  // Add the vertical expansion
  pLabelStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  pMainLayout->addWidget(pLabelStretch);

  // Add CCVG version information
  pLabelVersion->setText(acGTStringToQString(CCVGUtilGetVersionText()));
  pMainLayout->addWidget(pLabelVersion);

  // Add widgets to layout:
  setLayout(pMainLayout);

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGGUITabPageSettings::Shutdown()
{
  if (m_bBeenShutdown)
  {
    return m_bBeenShutdown;
  }

  // Tidy up
  // Note Qt widgets added to a layout are owned and deleted by that layout

  m_bBeenShutdown = true;

  return m_bBeenShutdown;
}

/// @brief  Create the CCVG GUI tab view settings pane layout.
void CCVGGUITabPageSettings::initialize()
{
  // Do nothing - Initialise() replaces this function
}

/// @brief  User facing name or label for CCVG GUI tab view pane. It must 
///         unique and not match any other tab view titles.
/// @return gtString Name of the tab view.
gtString CCVGGUITabPageSettings::pageTitle()
{
  return CCVGRSRC(kIDS_CCVG_STR_gui_TabPageSettings_page_title);
}

/// @brief Returns the section title for this page in the global settings file.
///        Can be shared between different pages. The XML section name for 
///        these settings.
/// @return gtString Name of the XML section.
gtString CCVGGUITabPageSettings::xmlSectionTitle()
{
  return m_pCCVGStateSettings->GetXMLSectionTitle();
}

/// @brief Save the CCVG's settings to a settings string. Read the GUI to
///        extract data values to be inserting to the settings string.
///        Must have at least one data field otherwise the code aborts in the
///        TinyXML library.
///        See functions for example:
///          kaAnalyzeSettingsPage::getXMLSettingsString()
///          gdGlobalDebugSettingsPage::getXMLSettingsString()
/// @param[out] vProjectAsXMLString XML formatted settings string.
/// @return     bool True = success, false = failure.
bool CCVGGUITabPageSettings::getXMLSettingsString(gtString &vProjectAsXMLString)
{
  return m_pCCVGStateSettings->GetXMLGlobalSettings(vProjectAsXMLString);
}

/// @brief Reads the settings from an XML string. Restoring the data from the 
///        XML file. 
/// @param[out] vProjectAsXMLString XML formatted settings string.
/// @return     bool True = success, false = failure.
bool CCVGGUITabPageSettings::setSettingsFromXMLString(const gtString &vProjectAsXMLString)
{
  const bool bOk = m_pCCVGStateSettings->SetGlobaljSettings(vProjectAsXMLString);
  if (bOk)
  {
    loadCurrentSettings();
  }
  return bOk;
}

/// @brief Loads the current values into the settings page. This is not the
///        same as function setSettingsFromXMLString(). Load data from the 
///        application data.
void CCVGGUITabPageSettings::loadCurrentSettings()
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return;
  }

  const fw::CCVGFWTheCurrentState::Settings &rCurrentSettings(m_pCCVGStateSettings->GetSettingsWorking());
  CopySettingsToWidgets(rCurrentSettings);
}

/// @brief Restores the default values into this tab pane  when if the user
///        hits 'cancel' in the options dialog.
void CCVGGUITabPageSettings::restoreDefaultSettings()
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return;
  }

  const fw::CCVGFWTheCurrentState::Settings &rCurrentSettings(fw::CCVGFWTheCurrentState::GetSettingsDefault());
  CopySettingsToWidgets(rCurrentSettings);
}

/// @brief Save the data as it was changed in the widget to the specific
///        settings manager (when "Ok" is pressed). Applies the current 
///        settings to the data structures.
/// @return bool True = success, false = failure.
bool CCVGGUITabPageSettings::saveCurrentSettings()
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return true;
  }

  fw::CCVGFWTheCurrentState::Settings currentSettings;
  CopyWidgetsToSettings(currentSettings);
  m_pCCVGStateSettings->SetSettingsWorking(currentSettings);

  return true;
}

/// @breif Check if the page has valid data entered by the user before 
///        excepting it (the user clicked 'ok' button). This dialog will not
///        close on returning false here. On returning false you must give
///        the user information on which settings were incorrect.
/// @return bool True = data is ok, false = some part of the data needs
///         correcting.
bool CCVGGUITabPageSettings::IsPageDataValid()
{
  if (!m_bCCVGReadyToWork)
  {
    // Do nothing
    return true;
  }

  fw::CCVGFWTheCurrentState::Settings currentSettings;
  CopyWidgetsToSettings(currentSettings);

  gtString errMsg;
  if (!m_pCCVGStateSettings->ChkSettingsWorkingValid(currentSettings, errMsg))
  {
    return false;
  }

  return true;
}

/// @brief  Copy the current CCVG project settings to the appropriate widgets
///         which can then been changed by the user. 
/// @param[in] vSettings Data structure containing settings.
void CCVGGUITabPageSettings::CopySettingsToWidgets(const fw::CCVGFWTheCurrentState::Settings &vSettings)
{
  m_pChkBxCCVGEnableForAllProjects->setChecked(vSettings.GetCcvgAllProjectsEnabledFlag());
}

/// @brief  Copy the settings from the GUI widgets to the  CCVG project 
///         settings data structure. 
/// @param[in] vSettings Data structure containing settings.
void CCVGGUITabPageSettings::CopyWidgetsToSettings(fw::CCVGFWTheCurrentState::Settings &vCurrentSettings)
{
  vCurrentSettings.SetCcvgAllProjectsEnabledFlag(m_pChkBxCCVGEnableForAllProjects->isChecked());
}

} // namespace gui
}  //namespace ccvg
