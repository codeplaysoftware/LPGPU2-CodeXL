// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGWGTLineEdit interface
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_WGT_LINEEDIT_H_INCLUDE
#define LPGPU2_CCVG_WGT_LINEEDIT_H_INCLUDE

// Qt:
#include <QtWidgets>

/// @brief    CCVG widget derived Qt widget QLineEdit. It implements the signal
///           focus gained and focus lost. It also has the ability to display
///           in the edit box a tooltip text to instruct the use. The tooltip
///           is seperate from the entry made by the user. To set or retrieve
///           real text data (not the tooltip) use accessor functions 
///           GetRealTextEntry() and SetRealTextEntry(). SetDisplayToolTip()
///           changes between the text and the tooltip text.
/// @warning  None.
/// @see      None.
/// @date     14/12/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGWGTLineEdit
: public QLineEdit
{
  Q_OBJECT

// Methods:
public:
  explicit CCVGWGTLineEdit(QWidget *vpParent = nullptr);
  explicit CCVGWGTLineEdit(const QString &vUserFaceText, QWidget *vpParent = nullptr);
  //
  const QString& GetRealTextEntry() const;
  void           SetRealTextEntry(const QString &vData);
  void           SetToolTip(const QString &vText);
  void           SetDisplayToolTip(bool vbYes);

// Overridden:
public:
   // From QObject
  ~CCVGWGTLineEdit() override;

// Signal slot:
signals:
  void Focussed(bool vbHasFocus);

// Methods:
protected:
  void OnFocussed(bool vbInFocus);
  void OnEdit();
  void DislayRealTextEntry();
  void DisplayToolTip();

// Overridden:
protected:
  // From QLineEdit
  void focusInEvent(QFocusEvent *vpE) override;
  void focusOutEvent(QFocusEvent *vpE) override;
  void keyPressEvent(QKeyEvent *vpE) override;

// Attributes:
protected:
  QString m_realTextEntry; 
  QString m_editTooltipText;
  bool    m_bDisplayTooltip;
  //
  static const QString ms_lineEditFontStyleForTxtTip;
  static QString       ms_lineEditFontStyleOriginal;
};
// clang-format on

#endif // LPGPU2_CCVG_WGT_LINEEDIT_H_INCLUDE