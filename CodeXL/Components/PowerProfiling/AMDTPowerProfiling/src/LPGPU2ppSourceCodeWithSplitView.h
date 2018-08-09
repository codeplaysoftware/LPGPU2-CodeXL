// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppSourceCodeWithSplitView.h
///
/// @brief Widget displaying two source code listings side by side: one is the
///        high-level shader, the other one the assembly version
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_SOURCECODEWITHSPLITVIEW_H
#define LPGPU2_PP_SOURCECODEWITHSPLITVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// Forward declarations
namespace lpgpu2 {
  enum class PPFnStatus;
} // namespace lpgpu2

class LPGPU2ppSourceCodeView;

namespace lpgpu2 {

/// @brief   Widget which displays two source code views side by side
/// @date    19/02/2018
/// @author  Alberto Taiuti
// clang-format off
class PP_API ppSrcCodeWithSplitView final : public QWidget
{
  Q_OBJECT

// Methods
public:
  ppSrcCodeWithSplitView(QWidget *parent);
  ~ppSrcCodeWithSplitView();

  PPFnStatus DisplayMainTextAsFile(const gtString &text,
      const osFilePath &filePath);
  PPFnStatus DisplayASMText(const gtString &text, const gtString &description,
      const gtVector<db::ShaderAsmStat> &stats, const gtVector<db::ShaderToAsm> &vShaderToAsm);
      
  const gtString &GetMainTextFileName() const { return m_mainFileName; }
  PPFnStatus HighlightLines(int start, int end);
  // Resource handling
  PPFnStatus Initialise();
  PPFnStatus Shutdown();

// Attributes
private:
  QHBoxLayout* m_pMainLayout= nullptr;
  QSplitter *m_pSplitter = nullptr;
  gtString m_mainFileName;
  LPGPU2ppSourceCodeView *m_pSourceCodeView = nullptr;
  QTableView *m_pTableView = nullptr;

// Methods
private:
  PPFnStatus AddViewToSplitter(QWidget* pView, const QString &caption);

// Methods
private slots:
  void OnSourceLineClicked(int line);

private:
    gtVector<lpgpu2::db::ShaderToAsm> m_shaderToAsmMappings;

};
// clang-format on

} // namespace lpgpu2

#endif // LPGPU2_PP_SOURCECODEWITHSPLITVIEW_H
