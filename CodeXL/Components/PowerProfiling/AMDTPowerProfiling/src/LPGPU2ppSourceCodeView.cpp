// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppSourceCodeView.h
///
/// @brief Separate tab view to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// ppSourceCodeView definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeView.h>

// Framework
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTApplicationComponents/Include/acSourceCodeLanguageHighlighter.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>

// QScintilla
#include <Qsci/qscistyle.h>
#include <Qsci/qscistyledtext.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
using lpgpu2::PPFnStatus;

// Constants for formatting annotations of the source:
const QString kQStringAsmAnnotationSingle = QString("▲ Asm Line %0");
const QString kQStringAsmAnnotationMulti = QString("▲ Asm Lines %0-%1");

/// @brief Ctor. No work is done here
///
/// @param pParent The QWidget which owns this one
/// @param shouldShowLineNumbers Whether to show the line numbers or not
/// @param contextMenuMask A bitmaks indicating what context menus to enable
LPGPU2ppSourceCodeView::LPGPU2ppSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers,
  unsigned int contextMenuMask) :
    afSourceCodeView(pParent, shouldShowLineNumbers, contextMenuMask)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    setReadOnly(true);
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
//         we need to make sure that Shutdown was called anyway.
LPGPU2ppSourceCodeView::~LPGPU2ppSourceCodeView()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus LPGPU2ppSourceCodeView::Initialise()
{
  m_enableBreakpoints = false;

  return PPFnStatus::success;
}

/// @brief Deinitialise the object
PPFnStatus LPGPU2ppSourceCodeView::Shutdown()
{
  return PPFnStatus::success;
}

PPFnStatus LPGPU2ppSourceCodeView::DisplayFile(const gtString &text,
      int selectedLineNumber, const osFilePath &filePath)
{
  GT_IF_WITH_ASSERT(text.length() > 0)
  {
    _ignoreTextChanged = true;

    // Clear the text:
    clear();

    // Set the file path:
    _filePath = filePath;

    // Get the file extension:
    gtString fileExtension;
    filePath.getFileExtension(fileExtension);
    fileExtension = fileExtension.toLowerCase();
    QString strExt = acGTStringToQString(fileExtension);
    int posOfValidExt = strExt.indexOf(QRegExp(AC_STR_FILE_EXTENSIONS_QREGEXP), 0);

    // if file extension is not a kernel/shader extension set it to .cl to enable syntax highlight
    if (-1 == posOfValidExt)
    {
        fileExtension = L"cl";
    }

    // Find the lexer according to the file type:
    QsciLexer* pLexer = acSourceCodeLanguageHighlighter::instance().getLexerByExtension(fileExtension);

    if (pLexer != NULL)
    {
        // Set my lexer:
        setLexer(pLexer);

        //setMarginsForegroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
        //setMarginsBackgroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_BG_COLOR);
    }

    // This function's signature return type is void
    setText(acGTStringToQString(text));

    // Set the cursor at the requested line number:
    if (selectedLineNumber >= 0)
    {
      // Get the selected line length:
      int selectedLineLength = lineLength(selectedLineNumber - 1);

      // Set the line selection:
      setSelection(selectedLineNumber - 1, 0, selectedLineNumber - 1, selectedLineLength);
    }

    showLineNumbers(_shouldShowLineNumbers);

    _ignoreTextChanged = false;
  }

  return PPFnStatus::success;
}

/// @brief Add an inline annotation to a given line number.
PPFnStatus LPGPU2ppSourceCodeView::AddAnnotation(const QString& annotation, int lineNumber)
{
  setAnnotationDisplay(QsciScintilla::AnnotationBoxed);
  // QsciStyle annotationStyle;
  // annotationStyle.setHotspot(true);
  annotate(lineNumber, QsciStyledText{annotation, 0});

  indicatorDefine(QsciScintilla::HiddenIndicator, 1);
  // setIndicatorHoverStyle(QsciScintilla::FullBoxIndicator);
  fillIndicatorRange(lineNumber, 0, lineNumber + 1, 0, 1);
  connect(this, &LPGPU2ppSourceCodeView::indicatorReleased, this,
          &LPGPU2ppSourceCodeView::OnIndicatorClicked);

  return PPFnStatus::success;
}

/// @brief Add an annotation to the source describing what assembly lines map
///        to a particular line.
lpgpu2::PPFnStatus LPGPU2ppSourceCodeView::AnnotateAsmLines(int srcLine, int asmLineStart,
                                                          int asmLineEnd)
{
  if (asmLineStart == asmLineEnd)
  {
    AddAnnotation(kQStringAsmAnnotationSingle.arg(asmLineStart), srcLine - 1);
  }
  else
  {
    AddAnnotation(kQStringAsmAnnotationMulti.arg(asmLineStart).arg(asmLineEnd), srcLine - 1);
  }

  return PPFnStatus::success;
}

/// @brief Highlight and scroll to a range of lines.
PPFnStatus LPGPU2ppSourceCodeView::SelectLines(int lineStart, int lineEnd)
{
  setSelection(lineStart, 0, lineEnd + 1, 0);
  setFirstVisibleLine(lineStart);

  return PPFnStatus::success;
}

/// @brief Slot to be called when an indicator is clicked, we don't care
///        about the column index so just the line number is emitted.
void LPGPU2ppSourceCodeView::OnIndicatorClicked(int line, int index)
{
  (void) index;
  emit SourceLineClicked(line + 1);
}
