// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppSourceCodeView.h
///
/// @brief View widget to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// ppSourceCodeView declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __PPSOURCECODEVIEW_H
#define __PPSOURCECODEVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Forward declarations
namespace lpgpu2 {
  enum class PPFnStatus;
} // namespace lpgpu2

/// @brief    Source code display widget derived from afSourceCodeView. It acts
///           a wrapper around the base class so that custom functionalities
///           can be added.
/// @warning  This is a proof of concept.
/// @see      afSourceCodeView
/// @see      ppSourceCodeView
/// @date     11/08/2017.
/// @author   Alberto Taiuti.
class PP_API LPGPU2ppSourceCodeView final : public afSourceCodeView
{
    Q_OBJECT

// Methods
public:
  LPGPU2ppSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers = true,
    unsigned int contextMenuMask = (unsigned int)AC_ContextMenu_Edit);
  ~LPGPU2ppSourceCodeView();

  // Display a file given by a string instead of opening it from a file,
  // the file path is there only to determine the extension
  lpgpu2::PPFnStatus DisplayFile(const gtString &text, int selectedLineNumber,
      const osFilePath &filePath);
      
  lpgpu2::PPFnStatus AddAnnotation(const QString& annotation, int lineNumber);
  lpgpu2::PPFnStatus AnnotateAsmLines(int srcLine, int asmLineStart, int asmLineEnd);
  
  lpgpu2::PPFnStatus SelectLines(int lineStart, int lineEnd);

  // Resource handling
  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  
signals:
  void SourceLineClicked(int line);
  
private slots:
  void OnIndicatorClicked(int line, int index);

};

#endif // __PPSOURCECODEVIEW_H
