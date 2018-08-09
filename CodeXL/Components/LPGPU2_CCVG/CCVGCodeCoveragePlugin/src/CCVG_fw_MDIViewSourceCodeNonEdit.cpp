// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWMDIViewSourceCodeNonEdit implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <qwidget.h>
#include <Qsci/qscistyle.h>
#include <Qsci/qscistyledtext.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewSourceCodeNonEdit.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateProjectData.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_MDIViewId.h>


namespace ccvg {
namespace fw {

/// @brief Overridden. Retrieve the Qt QWidget pointer for *this view.
/// @return QWidget 
::QWidget* CCVGFWMDIViewSourceCodeNonEdit::GetWidget()
{
  ::QsciScintilla *pQSci = static_cast<::QsciScintilla *>(this);
  ::QWidget *pWidget = static_cast<::QWidget *>(pQSci);

  return pWidget;
}

/// @brief  Class constructor. No work is done here by this class.
CCVGFWMDIViewSourceCodeNonEdit::CCVGFWMDIViewSourceCodeNonEdit():
  acSourceCodeView(nullptr, 
                   true, // Show line numbers
                   AC_ContextMenu_Copy | AC_ContextMenu_SelectAll) // r/o menu
{
  m_viewName = L"CCVGFWMDIViewSourceCodeNonEdit";
  m_eViewType = kCCVGMDIViewTypeType_SourceCodeNonEdit;
  m_pSelfCreatorFn = &CCVGFWMDIViewSourceCodeNonEdit::CreateSelf;
  
  setReadOnly(true);
  SetMonospace();
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGFWMDIViewSourceCodeNonEdit::~CCVGFWMDIViewSourceCodeNonEdit()
{
}

/// @brief  Static. Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGFWMDIViewBase* CCVGFWMDIViewSourceCodeNonEdit::CreateSelf()
{
  return new (std::nothrow) CCVGFWMDIViewSourceCodeNonEdit();
}

/// @brief Update the source view text with the file contents of the CCVG Item
///        specified by the ViewId.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewSourceCodeNonEdit::ReloadContent()
{
  ccvg::fw::CCVGFWTheCurrentStateProjectData &rProjData = ccvg::fw::CCVGFWTheCurrentState::Instance().GetProjectData();
  ccvg::fw::CCVGFWTheCurrentStateProjectDataCcvg *pData = nullptr;
  CCVGMDIViewId rViewIdObj(GetViewId());
  bool bIsSourceView = (rViewIdObj.GetDataTypeEnum() == cmd::CCVGCmdFileRead::kFileDataType_Source);
  status status = rProjData.ItemsDataGet(rViewIdObj.GetCCVGItemId(), pData);
  
  // Retrieve the correct CCVG item text data and display it
  switch (rViewIdObj.GetDataTypeEnum())
  {
    case cmd::CCVGCmdFileRead::kFileDataType_Source:
      // Insert blank line to attach the Highlight Key annotation to:
      setText(QString("\n") + QString(pData->GetDataSource().GetFileContent()));
      break;
    case cmd::CCVGCmdFileRead::kFileDataType_Driver:
      setText(QString(pData->GetDataDrivers().GetFileContent()));
      break;
    case cmd::CCVGCmdFileRead::kFileDataType_Results:
      setText(QString(pData->GetDataResults().GetFileContent()));
      break;
    case cmd::CCVGCmdFileRead::kFileDataType_Executeable:
      setText(QString(pData->GetDataExecuteable().GetFileContent()));
      break;
    case cmd::CCVGCmdFileRead::kFileDataType_None:
    default:
      // No content so nothing to display
      break;
  }

  // If this is not the source view, there are no highlights to display, done.
  if (!bIsSourceView)
  {
    return success;
  }
  
  if (status == success)
  {
    // Apply all highlights to the text
    for (auto highlight : pData->GetDataSource().GetHighlights())
    {
      ApplyHighlight(highlight);
    }
    setIndicatorDrawUnder(true);
  } 
  else
  {
    const gtString &rErrMsg(rProjData.ErrorGetDescription());
    ErrorSet(rErrMsg);
  }
  
  // Build highlighting key at top of document:
  QsciStyle annotationStyle = QsciStyle();
  QFont font = annotationStyle.font();
  font.setBold(true);
  annotationStyle.setFont(font);
  annotationStyle.setPaper(QColor(255,255,255));
  QList<QsciStyledText> listKeyText;
  listKeyText.append(QsciStyledText(QString::fromWCharArray(CCVGRSRC(kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyHeader)), annotationStyle));
  QsciStyle executedStyle = QsciStyle();
  executedStyle.setPaper(CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_Covered));
  listKeyText.append(QsciStyledText(QString::fromWCharArray(CCVGRSRC(kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyExecuted)), executedStyle));
  
  QsciStyle notExecutedStyle = QsciStyle();
  notExecutedStyle.setPaper(CCVGRSRCCOL(kIDS_CCVG_COL_Highlight_Uncovered));
  listKeyText.append(QsciStyledText(QString::fromWCharArray(CCVGRSRC(kIDS_CCVG_STR_cmn_MDISourceView_HighlightKeyNotExecuted)), notExecutedStyle));
  
  listKeyText.append(QsciStyledText(QString("\n"), annotationStyle));
  annotate(0, listKeyText);

  // Create function summary annotations
  QsciStyle inlineStyle = QsciStyle();
  QFont fontInline = inlineStyle.font();
  fontInline.setBold(true);
  inlineStyle.setFont(font);
  inlineStyle.setPaper(QColor(230,230,230));
  setAnnotationDisplay(QsciScintilla::AnnotationStandard);

  for (auto pairLineSummaries : pData->GetDataSource().GetMapLineToVecSummaries())
  {
    QString allSummaries;
    bool bFirstSummary = true;
    for (auto summary : pairLineSummaries.second)
    {
      if (bFirstSummary)
      {
        bFirstSummary = false;
      }
      else
      {
        allSummaries += "\n";
      }
      unsigned short percentage = ((float)summary.GetNonZeroCounterCount() / (float)summary.GetCounterCount() * 100);
      allSummaries += QString::fromWCharArray(CCVGRSRC(kIDS_CCVG_STR_cmn_MDISourceView_SummaryText))
                      .arg(QString(summary.GetName().asASCIICharArray()),
                      QString::number(summary.GetNonZeroCounterCount()),
                      QString::number(summary.GetCounterCount()),
                      QString::number(percentage));
    }
    annotate(pairLineSummaries.first, allSummaries, inlineStyle);
  }

  // Line numbers are incorrect, so manually construct a line number margin:
  showLineNumbers(false);
  setMarginType(1, QsciScintilla::TextMarginRightJustified);
  setMarginWidth(2, 16);

  unsigned long lineCount = SendScintilla(QsciScintillaBase::SCI_GETLINECOUNT);
  for (unsigned long i = 1; i < lineCount; i++)
  {
    SendScintilla(QsciScintillaBase::SCI_MARGINSETTEXT, i, QString::number(i).toStdString().c_str());
    SendScintilla(QsciScintillaBase::SCI_MARGINSETSTYLE, i, QsciScintillaBase::STYLE_LINENUMBER);  
  }  
  // --
  return status;
}

/// @brief  Overridden. Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewSourceCodeNonEdit::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Overridden. Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewSourceCodeNonEdit::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  m_bBeenShutdown = true;

  return success;
}

/// @brief Set the source view to use a monosace font.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewSourceCodeNonEdit::SetMonospace()
{
  QFont monoSpaceFont(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY);
  monoSpaceFont.setPointSize(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);
  monoSpaceFont.setStyleHint(QFont::Monospace);
  setFont(monoSpaceFont);
  
  return success;
}

/// @brief Display a CCVGHighlight in the editor
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGFWMDIViewSourceCodeNonEdit::ApplyHighlight(const CCVGFWCCVGHighlight &vrHighlight)
{
  // One indicator for each highlight type:
  const int indicator = static_cast<int>(vrHighlight.GetHighlightType());
  
  SendScintilla(QsciScintillaBase::SCI_SETINDICATORCURRENT, indicator);
  SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA, indicator,  255);
  SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE, indicator, INDIC_STRAIGHTBOX);
  SendScintilla(QsciScintillaBase::SCI_INDICSETFORE, indicator, vrHighlight.GetHighlightColor());
  SendScintilla(QsciScintillaBase::SCI_INDICATORFILLRANGE,
                                     vrHighlight.GetStartPos(),
                                     vrHighlight.GetLength());
  return success;
}


} // namespace fw 
} // namespace ccvg 
