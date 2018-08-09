// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppSourceCodeWithSplitView.cpp
///
/// @brief Widget displaying two source code listings side by side: one is the
///        high-level shader, the other one the assembly version
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeWithSplitView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeTreeModel.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>

#include <QTableView>

namespace lpgpu2 {

/// @brief Add a view to the widget's splitter
/// @param pView The view to add
/// @param caption The caption to put on top of the view
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus ppSrcCodeWithSplitView::AddViewToSplitter(QWidget* pView,
  const QString &caption)
{
  if (nullptr == m_pSplitter || nullptr == pView)
  {
    return PPFnStatus::failure;
  }

  // Create the objects:
  QWidget* pWidget = nullptr;
  LPGPU2PPNewQtWidget(&pWidget, nullptr);

  QVBoxLayout* pVLayout = nullptr;
  LPGPU2PPNewQtWidget(&pVLayout, nullptr);

  pVLayout->setContentsMargins(0, 0, 0, 0);

  QLabel* pText = nullptr;
  LPGPU2PPNewQtWidget(&pText, caption);

  // Organize everything:
  pVLayout->addWidget(pText);

  // second parameter stretches the widget
  pVLayout->addWidget(pView, 1, 0);

  pWidget->setLayout(pVLayout);
  m_pSplitter->addWidget(pWidget);

  return PPFnStatus::success;
}

/// @brief Ctor; no work is done here
/// @param parent Ptr to the parent widget
ppSrcCodeWithSplitView::ppSrcCodeWithSplitView(QWidget *parent)
: QWidget{parent}, m_pSplitter{nullptr}, m_mainFileName{}, m_pTableView{nullptr}
{
}

/// @brief Dtor; calls the Shutdown() method
ppSrcCodeWithSplitView::~ppSrcCodeWithSplitView()
{
  Shutdown();
}

/// @brief Display the main source code listing (i.e. a high-level shader code
///        listing)
/// @param text The text of the listing
/// @param filePath The file path to the listing; this is not used to load the
///        listing, but instead to give it a description
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus ppSrcCodeWithSplitView::DisplayMainTextAsFile(const gtString &text,
  const osFilePath &filePath)
{
  // Create the source code view and attempt opening the file  
  LPGPU2PPNewQtLPGPU2Widget(&m_pSourceCodeView, this);
  // We pass 1 because for this usage we don't need it and
  // hence we don't need to pass a specific line number.
  auto status = m_pSourceCodeView->DisplayFile(text, 1, filePath);

  if (status == PPFnStatus::failure)
  {
    return PPFnStatus::failure;
  }

  AddViewToSplitter(m_pSourceCodeView, acGTStringToQString(filePath.asString()));

  m_mainFileName = filePath.asString();

  connect(m_pSourceCodeView, &LPGPU2ppSourceCodeView::SourceLineClicked,
          this, &ppSrcCodeWithSplitView::OnSourceLineClicked);

  return PPFnStatus::success;
}

/// @brief Display assembly shader code in a new split, along with its
///        statistics
/// @param text The text version of the assembly code
/// @param description Label to display above the listing
/// @param stats The counter statistics related to this assembly code listing
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus ppSrcCodeWithSplitView::DisplayASMText(const gtString &text,
    const gtString &description, 
    const gtVector<db::ShaderAsmStat> &stats, 
    const gtVector<db::ShaderToAsm> &vShaderToAsm)
{
  if (m_pTableView == nullptr)
  {
    // Create the source code view and attempt opening the file
    LPGPU2PPNewQtWidget(&m_pTableView, this);

    ppSourceCodeTreeModel *pSourceCodeModel = nullptr;
    LPGPU2PPNewQtWidget(&pSourceCodeModel, this);

    m_pTableView->setModel(pSourceCodeModel);

    acTablePercentItemDelegate *percentDelegate = nullptr;
    LPGPU2PPNewQtWidget(&percentDelegate, this);
    m_pTableView->setItemDelegateForColumn(SrcViewColumns::percentage,
        percentDelegate);

    AddViewToSplitter(m_pTableView, acGTStringToQString(description));
  }

  auto *model = m_pTableView->model();
  if (model == nullptr)
  {
    return PPFnStatus::failure;
  }

  auto *sourceModel = qobject_cast<ppSourceCodeTreeModel *>(model);
  if (sourceModel == nullptr)
  {
    return PPFnStatus::failure;
  }

  sourceModel->BuildDisassemblyTree(text, stats);
  sourceModel->UpdateModel();
  m_pTableView->setSelectionMode(QAbstractItemView::MultiSelection);
  m_pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_pTableView->resizeColumnsToContents();
  m_pTableView->resizeRowsToContents();

  if (m_pSourceCodeView != nullptr)
  {
      m_shaderToAsmMappings = vShaderToAsm;

      for (const auto &shaderToAsm : vShaderToAsm)
      {
          m_pSourceCodeView->AnnotateAsmLines(shaderToAsm.m_shaderLine, shaderToAsm.m_asmStartLine, shaderToAsm.m_asmEndLine);
      }
  }

  return PPFnStatus::success;
}

/// @brief Initialise the object
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus ppSrcCodeWithSplitView::Initialise()
{
  // Create and set the layout
  LPGPU2PPNewQtWidget(&m_pMainLayout, this);
  m_pMainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pMainLayout);

  // Create splitter widget used to hold all the source code views
  LPGPU2PPNewQtWidget(&m_pSplitter, this);
  m_pMainLayout->addWidget(m_pSplitter);

  return PPFnStatus::success;
}

/// @brief Highlight a range of assembly code lines
/// @param start The starting line of the range
/// @param end The ending line of the range
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus ppSrcCodeWithSplitView::HighlightLines(int start, int end)
{
  if (m_pTableView == nullptr)
  {
    return PPFnStatus::failure;
  }

  m_pTableView->clearSelection();

  for (int i = start; i <= end; ++i) {
    m_pTableView->selectRow(i);    
  }

  m_pTableView->setFocus();

  return PPFnStatus::success;
}

/// @brief Deinitialise the object
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus ppSrcCodeWithSplitView::Shutdown()
{
  if (m_pMainLayout)
  {
    delete m_pMainLayout;
    m_pMainLayout = nullptr;
  }

  if (m_pSplitter)
  {
    delete m_pSplitter;
    m_pSplitter = nullptr;
  }

  return PPFnStatus::success;
}

/// @brief Slot called when the user clicks a line in the shader source.
void ppSrcCodeWithSplitView::OnSourceLineClicked(int line)
{
  // TODO:
  // * Read matching assembly lines from db
  // * Call function/emit signal to highlight asm lines in tree view
  
    const auto shaderToAsmIt = std::find_if(m_shaderToAsmMappings.begin(), m_shaderToAsmMappings.end(), [=](const lpgpu2::db::ShaderToAsm &shaderToAsm)
    {
        return shaderToAsm.m_shaderLine == line;
    });

    if (shaderToAsmIt != m_shaderToAsmMappings.end())
    {
        HighlightLines(shaderToAsmIt->m_asmStartLine - 1, shaderToAsmIt->m_asmEndLine - 1);
    }    
}

} // namespace lpgpu2
