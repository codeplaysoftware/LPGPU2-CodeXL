// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppSourceCodeView.h
///
/// @brief Separate tab view to display source code using QScintilla as the
///         the base component. Will be expanded, this is a proof of concept.
///
/// ppSourceCodeView declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __PPMULTISOURCECODEVIEW_H
#define __PPMULTISOURCECODEVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// Forward declarations
class acTabWidget;
class QComboBox;
class QPushButton;
class QLabel;
class ppSessionView;
class ppSessionController;
class osFilePath;
class gtString;
class LPGPU2ppInfoWidget;

namespace lpgpu2 {
  enum class PPFnStatus; // This is used as the return status
}

/// @brief    This class is used to contain and show multiple ppSourceCodeView
///           widgets.
/// @warning  This is a proof of concept.
/// @see      ppSourceCodeView
/// @see      ppMultiSourceCodeView
/// @date     11/08/2017.
/// @author   Alberto Taiuti
// clang-format off
class PP_API LPGPU2ppMultiSourceCodeView final : public QWidget
{
    Q_OBJECT

// Methods
public:
    LPGPU2ppMultiSourceCodeView(ppSessionView* pParent,
      ppSessionController* pSessionController);
  ~LPGPU2ppMultiSourceCodeView();

  // Resource handling
  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();

// Methods
public slots:
  void OpenAndDisplayShaderSlot(gtUInt32 frameNum, gtUInt32 drawNum);

// Methods
signals:
  void SourceSet();  

// Methods
private slots:
  void OnTabCloseRequestedSlot(int index);
  void OnShaderSelected(int vShaderIndex);
  void OnRunShaderFeedbackScript();

// Methods
private:
  lpgpu2::PPFnStatus OpenShaderWithASM(const gtString &shaderText,
      const osFilePath& shaderFileName,
      const gtString &asmText,
      const gtString &asmDescription,
      const gtVector<lpgpu2::db::ShaderAsmStat> &stats,
      const gtVector<lpgpu2::db::ShaderToAsm> &vShaderToAsm);

  lpgpu2::PPFnStatus OpenShaderNoASM(const gtString &shaderText,
      const osFilePath& shaderFileName);

  //lpgpu2::PPFnStatus OpenShaderBinary(const std::vector<gtUByte> &shaderBin,
  //    const osFilePath& shaderFileName);

  lpgpu2::PPFnStatus OpenASM(const gtString &asmText,
      const gtString &asmDescription,
      const gtVector<lpgpu2::db::ShaderAsmStat> &stats);

  lpgpu2::PPFnStatus InitViewLayout();
  lpgpu2::PPFnStatus InitShaders();
  lpgpu2::PPFnStatus InitCounters();
  lpgpu2::PPFnStatus InitFeedbackScripts();

private:
// Attributes  
  acTabWidget *m_pTabWidgdet                = nullptr;
  ppSessionController* m_pSessionController = nullptr;
  ppSessionView* m_pParentView              = nullptr;
  LPGPU2ppInfoWidget* m_pInfoWidget         = nullptr;
  
  QLabel *m_pShaderComboLabel     = nullptr;
  QComboBox *m_pShaderComboBox    = nullptr;

  QWidget* m_pShaderMatchOptionsWidget = nullptr; //!< Container widget for the shader match options.
  QComboBox *m_pScriptComboBox         = nullptr; //!< Combo box to allow the user to select a script for the shader_match feedback.
  QComboBox *m_pCounterComboBox        = nullptr; //!< Combo box to allow the user to select a counter for the shader_match feedback script.
  QPushButton *m_pRunScriptButton      = nullptr; //!< Button to execute the feedback script.

  QHash<QString, int> m_openTabsByName; // Used to lookup tabs by source file name

  bool  m_bIsInitialised = false;
};
// clang-format on

#endif // __PPMULTISOURCECODEVIEW_H
