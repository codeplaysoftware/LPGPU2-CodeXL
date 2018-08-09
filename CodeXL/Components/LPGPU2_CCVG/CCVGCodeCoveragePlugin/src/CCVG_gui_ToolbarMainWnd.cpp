// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGToolBarMainWnd implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <QLabel>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_ToolbarMainWnd.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_gui_WidgetStringConstants.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace gui {

/// @brief    Class constructor. No work is done here by this class. Base class
///           carries out work
/// param[in] vpParent - Qt widget object to *this.
CCVGToolBarMainWnd::CCVGToolBarMainWnd(QWidget *vpParent)
: acToolBar(vpParent, LPGPU2_CCVG_STR_ID_TOOLBAR_MAINWND)
, m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_pLineLabel(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGToolBarMainWnd::~CCVGToolBarMainWnd()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGToolBarMainWnd::Initialise()
{
  if (m_bBeenInitialised) 
  {
    return success;
  }

  const QString toolBarQtName(LPGPU2_CCVG_STR_ID_TOOLBAR_MAINWND);
  setObjectName(toolBarQtName);
  setStyleSheet(QString("QToolBar{spacing:5px;}"));

  m_pLineLabel = new (std::nothrow) QLabel(CCVGRSRCA(kIDS_CCVG_STR_gui_toolbar_mainWnd_label_info), this);
  GT_IF_WITH_ASSERT(m_pLineLabel != nullptr)
  {
    addWidget(m_pLineLabel);
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGToolBarMainWnd::Shutdown()
{
  if (m_bBeenShutdown)
  {
    return m_bBeenShutdown;
  }

  if (m_pLineLabel != nullptr)
  {
    delete m_pLineLabel;
    m_pLineLabel = nullptr;
  }

  m_bBeenShutdown = true;

  return m_bBeenShutdown;
}


} // namespace gui
}  //namespace ccvg
