// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2RegionOfInterestView.cpp
///
/// @brief Defines the Feedback Analysis view widget. Used to display
///		   all the widgets to visualise the feedback engine results.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// This needs to come first to avoid Qt internal warnings
// being treated as errors.
#include <qtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFeedbackAnalysisView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppCallsPerTypeView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppLongestRunningFunctionsView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppCallSuggestionsView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppShadersInRegionView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppShaderAnalysisView.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Qt:
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QStyleFactory>

// LPGPU2Database
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// Declarations
using lpgpu2::PPFnStatus;

/// @brief				  Set the group box font to bold. This will cause
///						  the group box title to be displayed as a bold text.
/// @param[in] vpGroupBox The group box to set the font as bold.
/// @warning			  If vpGroupBox is nullptr nothing will happen.
static void SetGroupBoxAsBold(QGroupBox *vpGroupBox)
{
    if (vpGroupBox != nullptr)
    {
        auto callsSummaryFont = vpGroupBox->font();
        callsSummaryFont.setBold(true);
        vpGroupBox->setFont(callsSummaryFont);
    }    
}

/// @brief	Class constructor. No work is performed here.
/// @param[in] vpParent The parent widget for this widget.
LPGPU2ppFeedbackAnalysisView::LPGPU2ppFeedbackAnalysisView(QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
{    
}

/// @brief Class destructor. Calls the Shutdown method to cleanup
///        resources used by this method.
LPGPU2ppFeedbackAnalysisView::~LPGPU2ppFeedbackAnalysisView()
{
    Shutdown();
}

/// @brief  Initialises the components and the layout of this widget.
/// @return PPFnStatus  success: If the components were initialised,
///                     failure: An error has occurred.
PPFnStatus LPGPU2ppFeedbackAnalysisView::Initialise()
{    
    QVBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);
    
    LPGPU2PPNewQtWidget(&m_pMainTabWidget, this);    
    m_pMainTabWidget->setTabsClosable(true);
    m_pMainTabWidget->setTabShape(QTabWidget::Rounded);
    m_pMainTabWidget->setMovable(false);

    pMainLayout->addWidget(m_pMainTabWidget);

    QWidget *pFeedbackComponentsWidget = nullptr;
    LPGPU2PPNewQtWidget(&pFeedbackComponentsWidget, this);

    m_pMainTabWidget->addTab(pFeedbackComponentsWidget, PP_STR_FeedbackAnalysisView_FeedbackTabTitle);
    m_pMainTabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();

    QVBoxLayout *pFeedbackComponentsLayout = nullptr;
    LPGPU2PPNewQtWidget(&pFeedbackComponentsLayout, pFeedbackComponentsWidget);

    QSplitter *pSplitter = nullptr;
    LPGPU2PPNewQtWidget(&pSplitter, pFeedbackComponentsWidget);
    pSplitter->setStyle(QStyleFactory::create(PP_STR_SplitterStyleName));
    pSplitter->setHandleWidth(6);
    pSplitter->setStyleSheet(PP_STR_SplitterStyleSheet);

    QGroupBox *pAPICallsSummaryGroupBox = nullptr;
    QHBoxLayout *pAPICallsSummaryGroupBoxLayout = nullptr;
    LPGPU2PPNewQtWidget(&pAPICallsSummaryGroupBox, PP_STR_FeedbackAnalysisView_APICallsSummaryGroupBoxTitle, pFeedbackComponentsWidget);
    LPGPU2PPNewQtWidget(&pAPICallsSummaryGroupBoxLayout, pAPICallsSummaryGroupBox);        

    LPGPU2PPNewQtLPGPU2Widget(&m_pCallsPerTypeView, pAPICallsSummaryGroupBox);
    LPGPU2PPNewQtLPGPU2Widget(&m_pLongestRunningAPICallsView, pAPICallsSummaryGroupBox);

    QTabWidget *pCallSummaryGroupBoxTab = nullptr;
    LPGPU2PPNewQtWidget(&pCallSummaryGroupBoxTab, pAPICallsSummaryGroupBox);
    
    pCallSummaryGroupBoxTab->addTab(m_pCallsPerTypeView,
        PP_STR_CallsPerTypeView_WindowTitle);
    pCallSummaryGroupBoxTab->addTab(m_pLongestRunningAPICallsView,
        PP_STR_LongestRunningFunctionsView_WindowTitle) ;
    auto groupBoxFont = pCallSummaryGroupBoxTab->font();
    groupBoxFont.setBold(true);
    pCallSummaryGroupBoxTab->setFont(groupBoxFont);

    pAPICallsSummaryGroupBoxLayout->addWidget(pCallSummaryGroupBoxTab);

    QGroupBox *pSuggestionsGroupBox = nullptr;
    QVBoxLayout *pSuggestionsGroupBoxLayout = nullptr;
    LPGPU2PPNewQtWidget(&pSuggestionsGroupBox, PP_STR_FeedbackAnalysisView_SuggestionGroupBoxTitle, pFeedbackComponentsWidget);
    LPGPU2PPNewQtWidget(&pSuggestionsGroupBoxLayout, pSuggestionsGroupBox);
    
    LPGPU2PPNewQtLPGPU2Widget(&m_pCallSuggestionsView, pSuggestionsGroupBox);
    LPGPU2PPNewQtLPGPU2Widget(&m_pShadersInRegionView, pSuggestionsGroupBox);

    QSplitter *pSuggestionsGroupBoxSplitter = nullptr;
    LPGPU2PPNewQtWidget(&pSuggestionsGroupBoxSplitter, Qt::Vertical, pSuggestionsGroupBox);
    pSuggestionsGroupBoxSplitter->setStyle(QStyleFactory::create(PP_STR_SplitterStyleName));
    pSuggestionsGroupBoxSplitter->setHandleWidth(6);
    pSuggestionsGroupBoxSplitter->setStyleSheet(PP_STR_SplitterStyleSheet);

    pSuggestionsGroupBoxSplitter->addWidget(m_pCallSuggestionsView);
    pSuggestionsGroupBoxSplitter->addWidget(m_pShadersInRegionView);

    pSuggestionsGroupBoxLayout->addWidget(pSuggestionsGroupBoxSplitter);

    pSplitter->addWidget(pAPICallsSummaryGroupBox);
    pSplitter->addWidget(pSuggestionsGroupBox);

    pFeedbackComponentsLayout->addWidget(pSplitter);       

    SetGroupBoxAsBold(pAPICallsSummaryGroupBox);
    SetGroupBoxAsBold(pSuggestionsGroupBox);

    connect(m_pCallSuggestionsView, &LPGPU2ppCallSuggestionsView::SuggestionSelected, this, &LPGPU2ppFeedbackAnalysisView::AnnotationSelected);
    connect(m_pShadersInRegionView, &LPGPU2ppShadersInRegionView::ShaderSelected, this, &LPGPU2ppFeedbackAnalysisView::ShaderTraceSeleted);
    connect(m_pMainTabWidget, &QTabWidget::tabCloseRequested, this, &LPGPU2ppFeedbackAnalysisView::OnTabCloseRequested);
   
    return PPFnStatus::success;
}

/// @brief Cleanup resources used by this class.
/// @return PPFnStatus always return success.
PPFnStatus LPGPU2ppFeedbackAnalysisView::Shutdown()
{
    if (m_pCallsPerTypeView != nullptr)
    {
        delete m_pCallsPerTypeView;
        m_pCallsPerTypeView = nullptr;
    }

    if (m_pCallSuggestionsView != nullptr)
    {
        delete m_pCallSuggestionsView;
        m_pCallSuggestionsView = nullptr;
    }

    if (m_pLongestRunningAPICallsView != nullptr)
    {
        delete m_pLongestRunningAPICallsView;
        m_pLongestRunningAPICallsView = nullptr;
    }

    if (m_pShadersInRegionView != nullptr)
    {
        delete m_pShadersInRegionView;
        m_pShadersInRegionView = nullptr;
    }

    if (m_pMainTabWidget != nullptr)
    {
        while (m_pMainTabWidget->count())
        {
            delete m_pMainTabWidget->widget(0);
        }

        delete m_pMainTabWidget;
        m_pMainTabWidget = nullptr;
    }   
    
    // Do not need to delete the pointers in the map. Already
    // taken care of above.
    m_shaderAnalysisTabsMap.clear();

    return PPFnStatus::success;
}

/// @brief					  Set the CallsPerType to be displayed in the underlying widget.
/// @param[in] vCallsPerType  The CallsPerType object to be displayed.
/// @return	   PPFnStatus	  success: if the CallsPerType was set and is ready to be displayed,
///						      failure: if m_pCallsPerTypeView is nullptr or an error occurred
///							  while setting the CallsPerType object
///	@see				      LPGPU2ppCallsPerTypeView::SetCallsPerType
PPFnStatus LPGPU2ppFeedbackAnalysisView::SetCallsPerType(const lpgpu2::db::CallsPerType &vCallsPerType) const
{
    auto bReturn = PPFnStatus::failure;    
    if (m_pCallsPerTypeView != nullptr)
    {
        bReturn = m_pCallsPerTypeView->SetCallsPerType(vCallsPerType);
    }
    return bReturn;
}

/// @brief						Set the CallSummary list to be displayed in the underlying widget as the
///								longest running functions.
/// @param[in] vAccumulatedData	The accumulated times of all the call summary. Can be retrieved using LPGPU2DatabaseAdapter.
/// @param[in] vCallSummaryData The list of CallSummary objects to be displayed.
/// @return	   PPFnStatus       success: If the call summary list was set and is ready to be displayed,
///								failure: if m_pLongestRunningAPICallsView is nullptr or an error occurred
///                             while setting the data.
/// @see                        LPGPU2DatabaseAdapter::GetAccumulatedCallSummaryTime().
/// @see                        LPGPU2ppLongestRunningFunctionsView::SetCallSummaryData().
PPFnStatus LPGPU2ppFeedbackAnalysisView::SetCallsSummaryData(const gtUInt64 vAccumulatedData, const gtVector<lpgpu2::db::CallSummary> &vCallSummaryData) const
{
    auto bReturn = PPFnStatus::failure;
    if (m_pLongestRunningAPICallsView != nullptr)
    {
        bReturn = m_pLongestRunningAPICallsView->SetCallSummaryData(vAccumulatedData, vCallSummaryData);
    }
    return bReturn;
}

/// @brief                   Set an annotation list to be displayed in the underlying widget.
/// @param[in] vAnnotations  The list of Annotation object to be displayed.
/// @return    PPFnStatus    success: if the list of annotations was set and are ready to be displayed,
///                          failure: if m_pCallSuggestionsView is nullptr or an error occurred while
///                          setting the data.
/// @see                     LPGPU2ppCallSuggestionsView::SetAnnotaions().
PPFnStatus LPGPU2ppFeedbackAnalysisView::SetAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations) const
{
    auto bReturn = PPFnStatus::failure;
    if (m_pCallSuggestionsView != nullptr)
    {
        bReturn = m_pCallSuggestionsView->SetAnnotations(vAnnotations);
    }
    return bReturn;
}

/// @brief                   Set the global annotation list to be displayed in
///                          the underlying widget.
/// @param[in] vAnnotations  The list of Annotation object to be displayed.
/// @return    PPFnStatus    success: if the list of annotations was set and
///                          is ready to be displayed,
///                          failure: if m_pCallSuggestionsView is nullptr or
///                          an error occurred while setting the data.
/// @see                     LPGPU2ppCallSuggestionsView::SetAnnotaions().
PPFnStatus LPGPU2ppFeedbackAnalysisView::SetGlobalAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations) const
{
    auto bReturn = PPFnStatus::failure;
    if (m_pCallSuggestionsView != nullptr)
    {
        bReturn = m_pCallSuggestionsView->SetGlobalAnnotations(vAnnotations);
    }
    return bReturn;
}

/// @brief                   Set the user annotation list to be displayed in
///                          the underlying widget.
/// @param[in] vAnnotations  The list of Annotation object to be displayed.
/// @return    PPFnStatus    success: if the list of annotations was set and
///                          is ready to be displayed,
///                          failure: if m_pCallSuggestionsView is nullptr or
///                          an error occurred while setting the data.
/// @see                     LPGPU2ppCallSuggestionsView::SetAnnotaions().
PPFnStatus LPGPU2ppFeedbackAnalysisView::SetUserAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations) const
{
    auto bReturn = PPFnStatus::failure;
    if (m_pCallSuggestionsView != nullptr)
    {
        bReturn = m_pCallSuggestionsView->SetUserAnnotations(vAnnotations);
    }
    return bReturn;
}

/// @brief                     Set the list of ShaderTrace objects to be displayed in the underlying widget.
/// @param[in] vShaderTraces   The list of ShaderTrace objects to be displayed.
/// @return    PPFnStatus      success: if the list of ShaderTraces was set and are ready to be displayed.
///                            failure: if m_pShadersInRegionView is nullptr or an error occurred while setting
///                            the data.
/// @see                       LPGPU2ppShadersInRegionView::SetShaderTraces().
PPFnStatus LPGPU2ppFeedbackAnalysisView::SetShaderTraces(const gtVector<lpgpu2::db::ShaderTrace>& vShaderTraces) const
{
    auto bReturn = PPFnStatus::failure;
    if (m_pShadersInRegionView != nullptr)
    {
        bReturn = m_pShadersInRegionView->SetShaderTraces(vShaderTraces);
    }
    return bReturn;
}

/// @brief                   Adds a LPGPU2ppShaderAnalysisView as a new tab for the user to inspect.
/// @param[in]  vShaderTrace The ShaderTrace object that will be used to build the new tab.
/// @return     PPFnStatus   success: if the tab was created, the data was set and ready to be displayed,
///                          failureL: an error has occurred while creating the new tab.
/// @see                     LPGPU2ppShaderAnalysisView
PPFnStatus LPGPU2ppFeedbackAnalysisView::AddShaderAnalysisTab(const lpgpu2::db::ShaderTrace &vShaderTrace)
{
    const auto tabTitle = QString{ "Shader (%0:%1)" }.arg(QString::number(vShaderTrace.m_frameNum),
                                                          QString::number(vShaderTrace.m_drawNum));

    if (m_shaderAnalysisTabsMap.contains(tabTitle))
    {
        auto *shaderAnalysisWidget = m_shaderAnalysisTabsMap[tabTitle];
        m_pMainTabWidget->setCurrentWidget(shaderAnalysisWidget);        
    }
    else
    {
        LPGPU2ppShaderAnalysisView *pShaderAnalysisView = nullptr;
        LPGPU2PPNewQtLPGPU2Widget(&pShaderAnalysisView, this, vShaderTrace.m_frameNum, vShaderTrace.m_drawNum);

        const auto newTabIndex = m_pMainTabWidget->addTab(pShaderAnalysisView, tabTitle);

        m_shaderAnalysisTabsMap[tabTitle] = pShaderAnalysisView;

        m_pMainTabWidget->setCurrentIndex(newTabIndex);
        
        connect(pShaderAnalysisView, &LPGPU2ppShaderAnalysisView::ShaderSourceOpenRequested, 
                this, &LPGPU2ppFeedbackAnalysisView::OpenShaderSourceRequested);
    }    

    return PPFnStatus::success;
}

/// @brief               Slot called when a tab (shader tab) was requested to be closed. 
///                      This gives the opportunity to deny the action of closing the tab
///                      since, in this particular case, the first tab cannot be closed.
/// @warning             This will delete the widget that was in the tab.
/// @param[in] vTabIndex The index of the tab to close.
void LPGPU2ppFeedbackAnalysisView::OnTabCloseRequested(const int vTabIndex)
{
    // Do not remove the first tab
    // Since we remove the close button from the first tab, vTabIndex should never be
    // 0 but we check here in case the event is fired programmatically.
    if (vTabIndex > 0)
    {
        auto *pShaderAnalysisWidget = qobject_cast<LPGPU2ppShaderAnalysisView*>(m_pMainTabWidget->widget(vTabIndex));
        if (pShaderAnalysisWidget != nullptr)
        {
            // Locate the key to remove from the map.
            const auto shaderID = m_shaderAnalysisTabsMap.key(pShaderAnalysisWidget);
            m_shaderAnalysisTabsMap.remove(shaderID);
            
            // Delete the widget since the tab widget does not take ownership.
            delete pShaderAnalysisWidget;
        }        
    }
}

