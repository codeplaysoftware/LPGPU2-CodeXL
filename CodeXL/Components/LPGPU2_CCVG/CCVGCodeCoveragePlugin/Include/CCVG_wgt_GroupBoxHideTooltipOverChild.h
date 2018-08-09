// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGWGTGroupBoxHideTooltipOverChild interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_WGT_GROUPBOX_H_INCLUDE
#define LPGPU2_CCVG_WGT_GROUPBOX_H_INCLUDE

// Qt:
#include <QtWidgets>
#include <QList>

/// @brief    CCVG widget derived Qt widget QGroupBox. It implements an event
///           filter to stop the tooltip for *this widget appearing when the 
///           mouse hovers a child widget (if the child had a tooltip this 
///           would not be a problem) within the group box. Widgets added as
///           children to *this group box have their parent set to *this
///           group box.
/// @warning  None.
/// @see      None.
/// @date     16/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGWGTGroupBoxHideTooltipOverChild
: public QGroupBox
{
  Q_OBJECT

// Methods:
public:
  explicit CCVGWGTGroupBoxHideTooltipOverChild(QWidget *vpParent = nullptr);
  explicit CCVGWGTGroupBoxHideTooltipOverChild(const QString &vTitle, QWidget *vpParent = nullptr);
  void     AddChild(QWidget &vpChild);
  
// Overridden:
public:
  // From QObject
  ~CCVGWGTGroupBoxHideTooltipOverChild() override;

// Overridden:
protected:
  // From QObject
  bool eventFilter(QObject *vpObj, QEvent *vpEvent) override;

// Methods:
private:
  bool HaveWgtChild(QObject &vChild) const;

// Attributes:
private:
  QList<QWidget *> m_listWgtChild;
};
// clang-format on

#endif // LPGPU2_CCVG_WGT_GROUPBOX_H_INCLUDE