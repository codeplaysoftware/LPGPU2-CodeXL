// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGGUITabPageSettings interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_GUI_TABPAGESETTINGS_H_INCLUDE
#define LPGPU2_CCVG_GUI_TABPAGESETTINGS_H_INCLUDE

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>

namespace ccvg {
namespace gui {

/// @brief    CCVGGUITabPageSettings derived from AMDT's class 
///           afGlobalSettingsPage (global tool options dialog page).
///           CCVGGUITabPageSettings is CodeXL settings extension. The 
///           extension populates a Tab Page view pane in the application's
///           options dialog. It displays CCVG's various options or settings.
///           The settings are available from the application's main tool bar
///           under Tools-->Options.
/// @warning  None.
/// @see      CCVGFWTheCurrentState.
/// @date     02/09/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGGUITabPageSettings final
: public afGlobalSettingsPage
, public CCVGClassCmnBase
{
  Q_OBJECT

// Methods:
public:
  CCVGGUITabPageSettings();
  status Initialise();
  status Shutdown();

// Overridden:
public:
  // From afGlobalSettingsPage
  virtual ~CCVGGUITabPageSettings() override;
  virtual void     initialize() override;
  virtual gtString pageTitle() override;
  virtual gtString xmlSectionTitle() override;
  virtual bool     getXMLSettingsString(gtString &vProjectAsXMLString) override;
  virtual bool     setSettingsFromXMLString(const gtString &vProjectAsXMLString) override;
  virtual void     loadCurrentSettings() override;
  virtual void     restoreDefaultSettings() override;
  virtual bool     saveCurrentSettings() override; 
  virtual bool     IsPageDataValid() override;

// Methods:
private:
  status InitialiseCCVGNotReadyToWork();
  status InitialiseCCVGReadyToWork();
  void   CopySettingsToWidgets(const fw::CCVGFWTheCurrentState::Settings &vSettings);
  void   CopyWidgetsToSettings(fw::CCVGFWTheCurrentState::Settings &vCurrentSettings);

// Attributes:
private:
  bool m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  bool m_bCCVGReadyToWork;  // True = yes all ok, false = one or more CCVG resources not available to useful work
  //
  // *this reads and sets settings
  fw::CCVGFWTheCurrentState *m_pCCVGStateSettings;
  //
  // GUI widgets
  QCheckBox  *m_pChkBxCCVGEnableForAllProjects;
};
// clang-format on

} // namespace gui
}  //namespace ccvg

#endif // LPGPU2_CCVG_GUI_TABPAGESETTINGS_H_INCLUDE