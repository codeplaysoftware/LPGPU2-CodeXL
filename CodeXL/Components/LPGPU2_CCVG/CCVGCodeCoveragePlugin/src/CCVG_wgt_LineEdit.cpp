// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGWGTLineEdit implementation
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_wgt_LineEdit.h>

// Instantiations:
const QString CCVGWGTLineEdit::ms_lineEditFontStyleForTxtTip = "color: grey; font-style: italic";
QString CCVGWGTLineEdit::ms_lineEditFontStyleOriginal;

/// @brief Class constructor. 
/// @param[in] vpParent This widget's parent.
CCVGWGTLineEdit::CCVGWGTLineEdit(QWidget *vpParent /* = nullptr */)
: QLineEdit(vpParent)
, m_bDisplayTooltip(true)
{
  ms_lineEditFontStyleOriginal = styleSheet();
}

/// @brief Class constructor. 
/// @param[in] vpParent This widget's parent.
/// @param[in] vUserFaceText Text to display in the edit box.
CCVGWGTLineEdit::CCVGWGTLineEdit(const QString &vUserFaceText, QWidget *vpParent /* = nullptr */)
: QLineEdit(vUserFaceText, vpParent)
{
}

/// @brief Overridden. Class destructor. Tidy up release resources used by 
///        *this instance.
CCVGWGTLineEdit::~CCVGWGTLineEdit()
{
}

/// @brief Overridden. Intercept the focus in event to emit a signal Focussed
///        with value bool true. 
/// @param[in] vpE QFocusEvent object.
void CCVGWGTLineEdit::focusInEvent(QFocusEvent *vpE)
{
  QLineEdit::focusInEvent(vpE);
  emit(Focussed(true));
  OnFocussed(true);
}

/// @brief Overridden. Intercept the focus out event to emit a signal Focussed
///        with value bool false. 
/// @param[in] vpE QFocusEvent object.
void CCVGWGTLineEdit::focusOutEvent(QFocusEvent *vpE)
{
  QLineEdit::focusOutEvent(vpE);
  emit(Focussed(false));
  OnFocussed(false);
}

/// @brief Overridden. Intercept the key press event so that this object can 
///        update the true text data value being entered or changed. 
/// @param[in] vpE QKeyEvent object.
void CCVGWGTLineEdit::keyPressEvent(QKeyEvent *vpE)
{
  QLineEdit::keyPressEvent(vpE);
  OnEdit();
}

/// @brief Helper function. Change the text in the widget to the real data
///        value. 
void CCVGWGTLineEdit::DislayRealTextEntry()
{
  m_bDisplayTooltip = false;
  const bool bBlkSig = blockSignals(true);
  setStyleSheet(ms_lineEditFontStyleOriginal);
  setText(m_realTextEntry);
  blockSignals(bBlkSig);
}

/// @brief Helper function. Change the text in the widget to that of the 
///        tooltip text data value. 
void CCVGWGTLineEdit::DisplayToolTip()
{
  m_bDisplayTooltip = true;
  const bool bBlkSig = blockSignals(true);
  setStyleSheet(ms_lineEditFontStyleForTxtTip);
  setText(m_editTooltipText);
  blockSignals(bBlkSig);
}

/// @brief  Public accessor. Retrieve the real text entered by the user.
/// @return QString& The data text. 
const QString& CCVGWGTLineEdit::GetRealTextEntry() const
{
  return m_realTextEntry;
}

/// @brief  Public accessor. Set the text that will be displayed in the
///         widget. This is not the tooltip text. Use SetDisplayToolTip() 
///         to show this text.
/// @param[in] vData The text data to be editted. 
void CCVGWGTLineEdit::SetRealTextEntry(const QString &vData)
{
  m_realTextEntry = vData;
  if (!m_bDisplayTooltip)
  {
    DislayRealTextEntry();
  }
}

/// @brief  Public accessor. Set the text that will be displayed as the 
///         tooltup text. Use SetDisplayToolTip() to show this text. This is
///         not the tooltip text.
/// @param[in] vData The text data to be editted. 
void CCVGWGTLineEdit::SetToolTip(const QString &vText)
{
  m_editTooltipText = vText;
  if (m_bDisplayTooltip)
  {
    DisplayToolTip();
  }
}

/// @brief  Public accessor. Swap between displaying the tooltip text and the
///         edit text.
/// @param[in] vbYes True = display tooltip, false = display edit text. 
void CCVGWGTLineEdit::SetDisplayToolTip(bool vbYes)
{
  if (vbYes)
  {
    DisplayToolTip();
  }
  else
  {
    DislayRealTextEntry();
  }
}

/// @brief  Visited on the in or out of mouse or keyboard focus *this widget
///         swaps the text displayed in the edit box to that of the tooltip or
///         the edit text. This function is only enabled when the real data
///         value is empty.
/// @param[in] vbInFocus True = edit text, false = tooltip. 
void CCVGWGTLineEdit::OnFocussed(bool vbInFocus)
{
  if (m_realTextEntry.isEmpty())
  {
    if (vbInFocus)
    {
      DislayRealTextEntry();
    }
    else
    {
      DisplayToolTip();
    }
  }
}

/// @brief  Visited on keyboard press event of *this widget. The text has been
///         changed. Take the text in the edit box and determine if 
///         pre-existing tooltip text or data text. If tooltip text then remove
///         all but the first key character entered, then switch to non tooltip
///         mode. If the text is cleared by the user *this widget switch to
///         tooltip mode and displays the tooltip text.
/// @param[in] vbInFocus True = edit text, false = tooltip. 
void CCVGWGTLineEdit::OnEdit()
{
  const QString txt(text());
  if (!txt.isEmpty())
  {
    QString txtEntry(txt);
    if (m_bDisplayTooltip)
    {
      if (m_editTooltipText.size() < txt.size())
      {
        const int cp = cursorPosition() - 1;
        const QChar c = txtEntry.at(cp);
        txtEntry = c;
      }
      else
      {
        txtEntry.clear();
      }
      setStyleSheet(ms_lineEditFontStyleOriginal);
      const bool bBlkSig = blockSignals(true);
      setText(txtEntry);
      blockSignals(bBlkSig);
      m_bDisplayTooltip = false;
    }
    
    m_realTextEntry = txtEntry;
  }
  else
  {
    m_realTextEntry.clear();
    DisplayToolTip();
  }
}


