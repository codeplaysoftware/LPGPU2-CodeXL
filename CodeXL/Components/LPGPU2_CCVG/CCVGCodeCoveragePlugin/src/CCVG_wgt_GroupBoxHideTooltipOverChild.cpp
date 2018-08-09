// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGWGTGroupBoxHideTooltipOverChild implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_GroupBoxHideTooltipOverChild.h>

/// @brief Class constructor. 
/// @param[in] vpParent This widget's parent.
CCVGWGTGroupBoxHideTooltipOverChild::CCVGWGTGroupBoxHideTooltipOverChild(QWidget *vpParent /* = nullptr */)
: QGroupBox(vpParent)
{
}

/// @brief Class constructor. 
/// @param[in] vpParent This widget's parent.
/// @param[in] vTitle Group box's caption.
CCVGWGTGroupBoxHideTooltipOverChild::CCVGWGTGroupBoxHideTooltipOverChild(const QString &vTitle, QWidget *vpParent /* = nullptr */)
: QGroupBox(vTitle, vpParent)
{
}

/// @brief  Overridden. Class destructor. Tidy up release resources used by 
///        *this instance.
CCVGWGTGroupBoxHideTooltipOverChild::~CCVGWGTGroupBoxHideTooltipOverChild()
{
}

/// @brief  Overridden. Intercept and filter events that match objects that 
///         are children pf *this groupbox. Matching objects trying to have
///         the tooltip shown will not be. This is to stop the groupbox's
///         tooltip being displayed when moust hover a groupbox child widget.
/// @param[in] vpObj Intercepted object.
/// @param[in] vpEvent The event associated with the intercepted object.
/// @return    bool True = stop event being processed further, 
///            false = allow further processing. 
bool CCVGWGTGroupBoxHideTooltipOverChild::eventFilter(QObject *vpObj, QEvent *vpEvent)
{
  if (HaveWgtChild(*vpObj) && (vpEvent->type() == QEvent::ToolTip))
  {
    QToolTip::hideText(); // Hides of parent's tooltip shown
    return true;          // Filter tooltip out of processing
  }

  return QGroupBox::eventFilter(vpObj, vpEvent);
}

/// @brief  Specify widgets to be child widgets of *this group box.
/// @param[in] vChild A widget.
void CCVGWGTGroupBoxHideTooltipOverChild::AddChild(QWidget &vChild)
{
  m_listWgtChild.push_back(&vChild);
  vChild.setParent(this);
  vChild.installEventFilter(this);
}

/// @brief  Is the widget a child widget of *this group box widget.
/// @param[in] vChild A widget.
/// @return    bool True = yes a child, false = not a child.
bool CCVGWGTGroupBoxHideTooltipOverChild::HaveWgtChild(QObject &vChild) const
{
  for(const auto &pWgt : m_listWgtChild)
  {
    if (pWgt == &vChild)
    {
      return true;
    }
  }

  return false;
}
