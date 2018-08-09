// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGToolBarMainWindow interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_GUI_TOOLBARMAINWINDOW_H_INCLUDE
#define LPGPU2_CCVG_GUI_TOOLBARMAINWINDOW_H_INCLUDE

// Infra:
#include <AMDTApplicationComponents/Include/acToolBar.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>

// Declarations:
class QLabel;

namespace ccvg {
namespace gui {

// Declarations:

/// @brief    CCVGToolBarMainWnd derived from AMDT's acToolBar in turn
///           derived from QToolbar. acToolBar is a base class for all Qt 
///           toolbars used in the application, controls common style and 
///           functionality. CCVGToolBarMainWnd handles the set up and
///           events to change the tool bar features dynamically to match
///           operational context at the time.
/// @warning  None.
/// @see      None.
/// @date     12/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGToolBarMainWnd
: public acToolBar
, public CCVGClassCmnBase
{
  Q_OBJECT

// Methods:
public:
  CCVGToolBarMainWnd(QWidget *vpParent);
  status Initialise();
  status Shutdown();

// Overridden:
public:
  // From acToolBar
  virtual ~CCVGToolBarMainWnd();

// Attributes:
private:
  bool    m_bBeenInitialised;  // True = yes this has been initialised, false = not yet
  bool    m_bBeenShutdown;     // True = yes this has been shutdown already, false = not yet
  QLabel* m_pLineLabel;
};
// clang-format on

} // namespace gui
}  //namespace ccvg

#endif // LPGPU2_CCVG_GUI_TOOLBARMAINWINDOW_H_INCLUDE