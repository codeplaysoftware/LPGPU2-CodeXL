// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Shader analysis view widget
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppShaderAnalysisView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// STL:
// Introduces unique_ptr
#include <memory>

// Qt:
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSplitter>
#include <QStyleFactory>
#include <QPushButton>

// Declarations
using lpgpu2::PPFnStatus;

/// @brief  Sets the style of a table widget.
/// @param[in] vpTableWidget A pointer to the QTableWidget to be configured.
/// @param[in] vTableTitle   The title to be set on the table header item.
/// @return    PPFnStatus    success: The table style, behaviour and header items were configured,
///                          failure: vpTableWidget is nullptr or the header item cannot be created.
static PPFnStatus SetTableWidgetStyleAndBehavior(QTableWidget *vpTableWidget, const QString &vTableTitle)
{
    auto bReturn = PPFnStatus::failure;

    if (vpTableWidget != nullptr)
    {
        // Sets the table style and behaviour
        vpTableWidget->setDragDropOverwriteMode(true);
        vpTableWidget->setAlternatingRowColors(true);
        vpTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        vpTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        vpTableWidget->setSortingEnabled(true);
        vpTableWidget->setCornerButtonEnabled(false);
        vpTableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
        vpTableWidget->horizontalHeader()->setSortIndicatorShown(true);
        vpTableWidget->horizontalHeader()->setStretchLastSection(true);
        vpTableWidget->verticalHeader()->setVisible(false);
        vpTableWidget->setStyleSheet(PP_STR_FeedbackViews_TableWidgetStyleSheet);

        auto headerItem = std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };
        if (headerItem != nullptr)
        {
            headerItem->setText(vTableTitle);
            auto headerFont = headerItem->font();
            headerFont.setBold(true);
            headerItem->setFont(headerFont);

            vpTableWidget->setColumnCount(1);
            vpTableWidget->setHorizontalHeaderItem(0, headerItem.release());
            bReturn = PPFnStatus::success;
        }
    }    

    return bReturn;
}

/// @brief                Class constructor. No work is done here.
/// @param[in] vpParent   The parent of this widget.
/// @param[in] vFrameNum  The frame number this shader appears at.
/// @param[in] vDrawNum   The draw number this shader appears at.
LPGPU2ppShaderAnalysisView::LPGPU2ppShaderAnalysisView(QWidget *vpParent, gtUInt32 vFrameNum, gtUInt32 vDrawNum)
    : QWidget(vpParent)
    , m_frameID(vFrameNum)
    , m_drawID(vDrawNum)
{
}

/// @brief  Class destructor. Calls the Shutdown() method to cleanup resources.
LPGPU2ppShaderAnalysisView::~LPGPU2ppShaderAnalysisView()
{
    Shutdown();
}

/// @brief  Initialises the widget components and layout.
/// @return PPFnStatus  success: All components are initialised and ready to use,
///                     failure: An error has occurred while creating the components or
///                              setting their style.
PPFnStatus LPGPU2ppShaderAnalysisView::Initialise()
{
    QVBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);

    QSplitter *pSplitter = nullptr;
    LPGPU2PPNewQtWidget(&pSplitter, this);
    pSplitter->setStyle(QStyleFactory::create(PP_STR_SplitterStyleName));
    pSplitter->setStyleSheet(PP_STR_SplitterStyleSheet);
    pSplitter->setHandleWidth(6);

    pMainLayout->addWidget(pSplitter);

    LPGPU2PPNewQtWidget(&m_pCompilerAnalysisTableWidget, pSplitter);
    LPGPU2PPNewQtWidget(&m_pSuggestionsTableWidget, pSplitter);    

    if (PPFnStatus::failure == SetTableWidgetStyleAndBehavior(m_pSuggestionsTableWidget, "Compiler Analysis"))
    {
        return PPFnStatus::failure;
    }
    if (PPFnStatus::failure == SetTableWidgetStyleAndBehavior(m_pCompilerAnalysisTableWidget, "Suggestions"))
    {
        return PPFnStatus::failure;
    }
    
    LPGPU2PPNewQtWidget(&m_pButtonContainer, this);
    QHBoxLayout* pButtonLayout = new QHBoxLayout;
    m_pButtonContainer->setLayout(pButtonLayout);
    LPGPU2PPNewQtWidget(&m_pOpenSourceButton, QString(PP_STR_LPGPU2ViewSourceForShader).arg(QString::number(m_frameID),
                                                                                            QString::number(m_drawID)));
    m_pOpenSourceButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    pButtonLayout->addWidget(m_pOpenSourceButton);
    m_pButtonContainer->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
    pMainLayout->addWidget(m_pButtonContainer);
    
    bool rc = connect(m_pOpenSourceButton, &QPushButton::clicked, this, 
      &LPGPU2ppShaderAnalysisView::OnOpenSourceButtonClicked);
      
    if (!rc)
    {
      return PPFnStatus::failure;
    }

    return PPFnStatus::success;
}

/// @brief  Delete the resources used by this class.
/// @return PPFnStatus  Always return success.
PPFnStatus LPGPU2ppShaderAnalysisView::Shutdown()
{
    if (m_pCompilerAnalysisTableWidget != nullptr)
    {
        delete m_pCompilerAnalysisTableWidget;
        m_pCompilerAnalysisTableWidget = nullptr;
    }

    if (m_pSuggestionsTableWidget != nullptr)
    {
        delete m_pSuggestionsTableWidget;
        m_pSuggestionsTableWidget = nullptr;
    }

    return PPFnStatus::success;
}

void LPGPU2ppShaderAnalysisView::OnOpenSourceButtonClicked()
{
  emit ShaderSourceOpenRequested(m_frameID, m_drawID);
}