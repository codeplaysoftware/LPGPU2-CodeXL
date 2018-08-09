//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppTimelineView.cpp
///
//==================================================================================

//------------------------------ ppTimelineView.cpp ------------------------------Sheet("background-color: red");

// must be first after Qt to handle include of qcustomplot correctly
#include <AMDTPowerProfiling/src/ppTimelineView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
//++CF:LPGPU2 (to determine from settings if target device is android)
#include <AMDTApplicationFramework/Include/afProjectManager.h>
//--CF:LPGPU2

// Powerprofiler midtier classes
#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>

// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/Include/ppAppWrapper.h>
#include <AMDTPowerProfiling/src/ppColors.h>
#include <AMDTPowerProfiling/src/ppGUIDefs.h>
#include <AMDTPowerProfiling/src/ppSessionNavigationChart.h>
#include <AMDTPowerProfiling/src/ppCountersSelectionDialog.h>
#include <AMDTPowerProfiling/src/ppMultiLinePowerStackedPlot.h>
#include <AMDTPowerProfiling/src/ppMultiLinePowerNonStackedPlot.h>
//++SRB: LPGPU2
#include <AMDTPowerProfiling/src/ppTimeline.h>
#include <AMDTPowerProfiling/src/LPGPU2ppGLTimelineItem.h>
#include <AMDTPowerProfiling/src/LPGPU2ppMultiLinePlot.h>
//--SRB: LPGPU2

//++TLRS: LPGPU2
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTPowerProfiling/src/LPGPU2ppRunFeedbackEngineShortcutFrame.h>
#include <chrono>
//--TLRS: LPGPU2
//++AT:LPGPU2
#include <AMDTPowerProfiling/src/LPGPU2ppTimerTimingModeFrame.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <cmath>
//--AT:LPGPU2


#define PP_TIMELINE_NAVIGATION_CHART_RIGHT_MARGIN 60
#define PP_TIMELINE_PLOT_MARGINS QMargins(60, 10, 5, 10)
#define PP_TIMELINE_NAVIGATION_CHART_MARGINS QMargins(60, 20, PP_TIMELINE_NAVIGATION_CHART_RIGHT_MARGIN, 40)

const int NO_CATALYST_REFRESH_RATE = 1000;
const int TRACK_LINE_WIDTH = 1;

// Counters information table size constants:
#define PP_INFO_TABLE_ICON_COL_WIDTH 20
#define PP_INFO_TABLE_X_MARGIN 50

#define MILLISECONDS_IN_SECOND 1000
#define PP_INVALID_COUNTER_VALUE -1.0
#define PP_COUNTER_VALUE_FP_PRECISION 2

/// Ribbon buttons:
#define PP_RIBBON_BUTTON_DIMENSION 16
#define PP_RIBBON_BUTTON_PADDING 4
#define PP_RIBBON_BUTTON_MARGIN 5


//++AT:LPGPU2
static const char *kHelpTimelineLabel = "ROI";
//--AT:LPGPU2

//++CF:LPGPU2
static constexpr gtUInt32 gs_interpolationPointsPerSec = 250;
//--CF:LPGPU2

// static members initialization:
int ppRibbonButton::m_sRibbonsCount = 0;


ppRibbonButton::ppRibbonButton(QWidget* pParent) : QPushButton(pParent)
{
    // Make the font gray:
    setStyleSheet(PP_StrTimelineRibbonButtonStyle);

    m_plotIndex = -1;
    m_buttonType = PP_RIBBON_BUTTON_UP;
    setEnabled(false);

    m_isMouseIn = false;
}

ppRibbonButton::~ppRibbonButton()
{

}

void ppRibbonButton::enterEvent(QEvent* pEvent)
{
    // Call the base class implementation:
    QPushButton::enterEvent(pEvent);

    // Check if the button should be enabled (if we can go up / down according
    // to the ribbons count and plot index):
    bool isButtonEnabled = false;

    if (m_buttonType == PP_RIBBON_BUTTON_DOWN)
    {
        isButtonEnabled = m_plotIndex > 0;
    }

    else
    {
        isButtonEnabled = m_plotIndex < (m_sRibbonsCount - 1);
    }


    // Enable the button:
    setEnabled(isButtonEnabled);

    // Make sure that the signal is fired only once:
    if (!m_isMouseIn)
    {
        // Emit a signal for button area enter:
        emit ButtonEnterLeave(m_plotIndex, m_buttonType, true);
    }

    m_isMouseIn = true;
}

void ppRibbonButton::leaveEvent(QEvent* pEvent)
{
    // Call the base class implementation:
    QPushButton::leaveEvent(pEvent);

    // Disable the button:
    setEnabled(false);

    // Make sure that the signal is fired only once:
    if (m_isMouseIn)
    {
        // Emit a signal for button area leave:
        emit ButtonEnterLeave(m_plotIndex, m_buttonType, false);
    }

    m_isMouseIn = false;

}

//++KB: LPGPU2
LPGPU2ppRibbonWidgetWrapper::LPGPU2ppRibbonWidgetWrapper(QWidget* pWidget) :
    m_pWrappedWidget(pWidget),
    m_pLabel(new LPGPU2ppVerticalLabel)
{

}

/// @brief  Retrieves the wrapped widget.
/// @return QWidget* Pointer to the internal QWidget.
//++AT:LPGPU2
// Had to rm constness from method because it is used in code where
// the expected returned ptr is non-const but method doesn't make sense
// as const as we return a non-const ptr.
//--AT:LPGPU2
QWidget* LPGPU2ppRibbonWidgetWrapper::GetWidget()
{
    return m_pWrappedWidget;
}

/// @brief      Sets the text that is displayed as a vertical label.
/// @param[in]  label The string to be displayed
void LPGPU2ppRibbonWidgetWrapper::SetLabelText(const QString& label)
{
    m_pLabel->setText(label);
}

/// @brief  Gets the vertical label widget.
/// @return ppVerticalLabel* Pointer to the vertical label widget.
//++AT:LPGPU2
// Had to rm constness from method because it is used in code where
// the expected returned ptr is non-const but method doesn't make sense
// as const as we return a non-const ptr.
//--AT:LPGPU2
LPGPU2ppVerticalLabel* LPGPU2ppRibbonWidgetWrapper::GetLabel()
{
    return m_pLabel;
}

/// @brief  Calculates the y-coordinate that corresponds to the bottom of this
///         wrapped widget and vertical label, using the Qt widget geometry of
///         whichever of the two is the lowest.
/// @return int Y-coordinate representing the bottom of this wrapper.
int LPGPU2ppRibbonWidgetWrapper::GetRibbonBottom() const
{
    int widgetBottom = m_pWrappedWidget->geometry().bottom();
    int labelBottom = m_pLabel->geometry().bottom();
    return widgetBottom > labelBottom ? widgetBottom : labelBottom;
}

LPGPU2ppRibbonTimelineWrapper::LPGPU2ppRibbonTimelineWrapper(acTimeline* pTimeline) :
    LPGPU2ppRibbonWidgetWrapper(pTimeline)
{

}
/// @brief Sets the visible range of the internal ppTimeline widget.
void LPGPU2ppRibbonTimelineWrapper::UpdateTimeRange(SamplingTimeRange newRange)
{
    GT_IF_WITH_ASSERT(m_pWrappedWidget)
    {
        auto pTimelineWrappedWidget =
          qobject_cast<acTimeline*>(m_pWrappedWidget);
        GT_IF_WITH_ASSERT(pTimelineWrappedWidget)
        {
            using std::chrono::milliseconds;
            using std::chrono::nanoseconds;
            using std::chrono::duration_cast;

            const auto startNs =
              duration_cast<nanoseconds>(milliseconds{newRange.m_fromTime});
            const auto endNs =
              duration_cast<nanoseconds>(milliseconds{newRange.m_toTime});
            const auto durationNs = endNs - startNs;
            pTimelineWrappedWidget->SetVisibleRange(startNs.count(),
                durationNs.count());
        }
    }
}

//++AT:LPGPU2
/// @brief Update the m_nFullRange of the widget, so that when the
///        other update method, UpdateTimeRange, is called, it will
///        not trigger the assertion related to the m_nFullRange being
///        smaller than visibleRangeTime.
/// @param newRange The new range to set
/// @note  We do not return a code to conform with the existing codebase
///        style
/// @note  We add 1 to the original time so that the comparison does not
///        trigger an assertion.
void LPGPU2ppRibbonTimelineWrapper::UpdateFullTimeRange(
    SamplingTimeRange newRange)
{
  GT_IF_WITH_ASSERT(m_pWrappedWidget)
  {
    auto pTimelineWrappedWidget = qobject_cast<acTimeline*>(m_pWrappedWidget);
    GT_IF_WITH_ASSERT(pTimelineWrappedWidget)
    {
      using std::chrono::milliseconds;
      using std::chrono::nanoseconds;
      using std::chrono::duration_cast;

      const auto startNs =
        duration_cast<nanoseconds>(milliseconds{newRange.m_fromTime});
      pTimelineWrappedWidget->setFullRange(startNs.count() + 1);
    }
  }
}
//--AT:LPGPU2
//--KB: LPGPU2

//++AT:LPGPU2
void LPGPU2ppRibbonWidgetWrapper::SetVisible(bool isVisible)
{
  GT_ASSERT(m_pWrappedWidget);
  GT_ASSERT(m_pLabel);

  auto pTimelineWrappedWidget = qobject_cast<acTimeline*>(m_pWrappedWidget);
  GT_IF_WITH_ASSERT(pTimelineWrappedWidget)
  {
    pTimelineWrappedWidget->setVisible(isVisible);
  }
  m_pLabel->setVisible(isVisible);
}
//--AT:LPGPU2

//++CF:LPGPU2 (changed pParentSession to QWidget so timeline can be used anywhere)
//            (added isRegionOfInterestTimeline param)
ppTimeLineView::ppTimeLineView(QWidget* pParentSession, ppSessionController* pSessionController, bool isRegionOfInterestTimeline /*=false*/,
                               gtUInt32 regionOfInterestID /*=0*/) :
//--CF:LPGPU2
    QWidget(nullptr),
    m_pMainLayout(nullptr),
    m_pBottomScrollArea(nullptr),
    m_pBottomVLayout(nullptr),
    m_pGridStretchWidget(nullptr),
//++AT:LPGPU2
    m_APITimelineWidget{},
    m_HelpTimelineWidget{},
//--AT:LPGPU2
    m_pPowerPlot(nullptr),
    m_pPowerStackedPlot(nullptr),
    m_samplingTimeRange(0, 0),
    m_pSessionController(pSessionController),
    m_pSessionNavigationChart(nullptr),
    m_timeLineGraphInterval(0),
    m_navigationCounterID(-1),
    m_pParentSession(pParentSession),
    m_pTrackingLine(nullptr),
    m_pBoundingBoxLeftLine(nullptr),
    m_pBoundingBoxRightLine(nullptr),
    m_pBoundingBoxBottomLine(nullptr),
    m_pNavigationCounterSelectionLabel(nullptr),
    m_pNavigationCounterSelectionComboBox(nullptr),
    m_shouldDrawTrackingLine(false),
    m_trackingLineXCoordinate(-1),
    m_pTimeLabel(nullptr),
    m_trackLineKey(PP_INVALID_COUNTER_VALUE),
    m_isRangeChanged(false),
    m_wasRangeBoundingLinePainted(false),
    m_isVisibilityUpdated(false),
    m_lastReplotTime(-9999),
    m_isInShowRibbonButtons(false),
//++CF:LPGPU2
    m_isRegionOfInterestTimeline(isRegionOfInterestTimeline),
    m_regionOfInterestID(regionOfInterestID)
//--CF:LPGPU2
{
    InitViewLayout();   
}

ppTimeLineView::~ppTimeLineView()
{
    RemoveAllRibbons();

    delete m_pPowerPlot;
    delete m_pPowerStackedPlot;

    // Delete the widgets that are not added to the view's layout:
    delete m_pTrackingLine;
    delete m_pBoundingBoxLeftLine;
    delete m_pBoundingBoxRightLine;
    delete m_pBoundingBoxBottomLine;
    delete m_pTimeLabel;

    // A map plot -> ribbon buttons:
    m_ribbonButtonsMap.clear();

    //++TLRS: LPGPU2
    if(m_pRunFeedbackEngineShortcutFrame != nullptr)
    {
        delete m_pRunFeedbackEngineShortcutFrame;
        m_pRunFeedbackEngineShortcutFrame = nullptr;
    }
    //++TLRS: LPGPU2
}

//++TLRS:LPGPU2: Adding method to highlight the graphs that support the operation
void ppTimeLineView::AddHighlightRegionOnTimelines(gtUInt32 vRegionId, gtUInt64 vStart, gtUInt64 vEnd, const QColor &vColor)
{
    for (auto *pPlot : m_allRibbonsVec)
    {
        auto *pTimelinePlot = qobject_cast<LPGPU2ppMultiLinePlot*>(pPlot);
        if (pTimelinePlot != nullptr)
        {
            pTimelinePlot->AddRangeHighlight(vRegionId, vStart, vEnd, vColor);            
        }
    }
}

void ppTimeLineView::ClearHighlightRegionsOnTimes()
{
    for (auto *pPlot : m_allRibbonsVec)
    {
        auto *pTimelinePlot = qobject_cast<LPGPU2ppMultiLinePlot*>(pPlot);
        if (pTimelinePlot != nullptr)
        {
            pTimelinePlot->ClearHighlights();
        }
    }
}

//--TLRS:LPGPU2: Adding method to highlight the graphs that support the operation

void ppTimeLineView::InitViewLayout()
{
    // Create the tab view in a layout and add it.
    m_pMainLayout = new QVBoxLayout;

    m_pMainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_pMainLayout);

    QHBoxLayout* pTopHLayout = new QHBoxLayout;
    m_pBottomVLayout = new QVBoxLayout;
    m_pBottomVLayout->setContentsMargins(0, 0, 0, 0);
    
//++CF:LPGPU2
    m_pTimelineContainer = new (std::nothrow) QWidget;
    m_pTimelineLayout = new (std::nothrow) QVBoxLayout(m_pTimelineContainer);
    m_pTimelineLayout->setContentsMargins(0, 0, 0, 0);
//--CF:LPGPU2

    QString style = QString(PP_STR_CSS_TimelineViewTooltipStyle).arg(PP_TOOLTIP_BORDER_COLOR).arg(PP_TOOLTIP_BACKGROUND_COLOR);

    // Set the style for the timeline plots:
    acMultiLinePlot::SetTooltipHTMLStyle(style);

    InitGraphs();

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pParentSession != nullptr) && (m_pSessionController != nullptr))
    {
        //++CF:LPGPU2
                // Create the navigation chart, forcing all data passed in to be shown by default:
        const auto bForceFullRangeDisplay = true;
        m_pSessionNavigationChart = new ppSessionNavigationChart{ this, m_pSessionController, PP_STR_NavChartYAxisLabel, bForceFullRangeDisplay };        
        //--CF:LPGPU2

        QMargins margins(acScalePixelSizeToDisplayDPI(PP_TIMELINE_NAVIGATION_CHART_MARGINS.left()), acScalePixelSizeToDisplayDPI(PP_TIMELINE_NAVIGATION_CHART_MARGINS.top()),
                         acScalePixelSizeToDisplayDPI(PP_TIMELINE_NAVIGATION_CHART_MARGINS.right()), acScalePixelSizeToDisplayDPI(PP_TIMELINE_NAVIGATION_CHART_MARGINS.bottom()));

        // Set the margins to the navigation chart:
        m_pSessionNavigationChart->yAxis->axisRect()->setAutoMargins(QCP::msNone);
        m_pSessionNavigationChart->xAxis->axisRect()->setAutoMargins(QCP::msNone);
        m_pSessionNavigationChart->yAxis->axisRect()->setMargins(margins);
        m_pSessionNavigationChart->xAxis->axisRect()->setMargins(margins);

        m_pSessionNavigationChart->setMinimumHeight(acScalePixelSizeToDisplayDPI(PP_RIBBON_MIN_HEIGHT));
        m_pSessionNavigationChart->setMaximumHeight(acScalePixelSizeToDisplayDPI(PP_RIBBON_MIN_HEIGHT));

        // Connect to navigation chart range change signal:
        bool rc = connect(m_pSessionNavigationChart, SIGNAL(RangeChangedByUser(const QPointF&)), this, SLOT(OnRangeChangedByUser(const QPointF&)));
        GT_ASSERT(rc);

        m_pMainLayout->addLayout(pTopHLayout);
//++CF:LPGPU2
        m_pMainLayout->addWidget(m_pTimelineContainer);
//--CF:LPGPU2

        QWidget* pBottomWidget = new QWidget;
        m_pBottomScrollArea = new QScrollArea;
        m_pBottomScrollArea->setGeometry(pBottomWidget->geometry());
        m_pBottomScrollArea->setWidget(pBottomWidget);
        m_pBottomScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_pBottomScrollArea->setWidgetResizable(true);
        m_pBottomScrollArea->setContentsMargins(0, 0, 0, 0);
        pBottomWidget->setLayout(m_pBottomVLayout);
        m_pMainLayout->addWidget(m_pBottomScrollArea);
        m_pBottomScrollArea->setFrameShape(QFrame::NoFrame);
        pBottomWidget->setStyleSheet(AF_STR_WhiteBG);

        AddStretchItemToGrid();

//++AT:LPGPU2
    m_pTimerTimingModeFrame =
      new (std::nothrow) lpgpu2::ppTimerTimingModeFrame;
    if (m_pTimerTimingModeFrame &&
        m_pTimerTimingModeFrame->Initialise(
          ppAppController::instance().GetTimingModeTimerQTimer())
            == lpgpu2::PPFnStatus::success)
    {
      m_pMainLayout->addWidget(m_pTimerTimingModeFrame);

      m_pTimerTimingModeFrame->setVisible(false);
    }
//--AT:LPGPU2

        //++TLRS: LPGPU: Init the hint frame
        m_pRunFeedbackEngineShortcutFrame = new (std::nothrow) LPGPU2ppRunFeedbackEngineShortcutFrame{ this };
        if (m_pRunFeedbackEngineShortcutFrame != nullptr)
        {
            if(m_pRunFeedbackEngineShortcutFrame->Initialise() == lpgpu2::PPFnStatus::success)
            {
                // Connect the button click to the action of running the feedback engine
                connect(m_pRunFeedbackEngineShortcutFrame, &LPGPU2ppRunFeedbackEngineShortcutFrame::RunFeedbackButtonClicked, this, [=]
                {
                    // m_pSessionController->DBFilePath();
                    const auto sessionId = ProfileApplicationTreeHandler::instance()->GetCurrentDisplaySessionId();

                    if (sessionId == SESSION_ID_ERROR)
                    {
                        ppAppController::instance().RunLPGPU2FeedbackEngine(m_pSessionController->DBFilePath());
                    }
                    else
                    {
                        ppAppController::instance().RunLPGPU2FeedbackEngine(sessionId);
                    }
                    
                });                
            }

            m_pMainLayout->addWidget(m_pRunFeedbackEngineShortcutFrame);
            m_pRunFeedbackEngineShortcutFrame->setVisible(false);                        
        }

        // Connect the signal of Feedback Engine executed to refresh the view and display the regions of interest.
        connect(&ppAppController::instance(), &ppAppController::FeedbackEngineExecuted, this, [=]
        {
            InitHelpTimelineFromDB();
        });
        //--TLRS: LPGPU2: Init the hint frame

        // Add the navigation chart to the top layout:
        pTopHLayout->setContentsMargins(0, 0, 0, 0);
        m_pSessionNavigationChart->setContentsMargins(0, 0, 0, 0);

        QWidget* pContainingWidget = new QWidget;

        //++AT:LPGPU2
        // Sanity check:
        GT_IF_WITH_ASSERT((m_pSessionController != nullptr) && (m_pSessionNavigationChart != nullptr))
        {
            // Get the current session state:
            auto currentState = m_pSessionController->GetSessionState();

            // in session complete state
            if (currentState == ppSessionController::PP_SESSION_STATE_COMPLETED)
            {
                InitTimelinesFromDB();
            }
        }
        //--AT:LPGPU2

        // Add combo box and label for session navigation counter:
        AddNavigationCounterSelectionWidgets(pContainingWidget);

        QWidget* pDummyWidget3 = new QWidget;

        int widget2W = CalculateInfoTableFixedWidth() - acScalePixelSizeToDisplayDPI(PP_TIMELINE_NAVIGATION_CHART_RIGHT_MARGIN);
        pContainingWidget->setFixedWidth(widget2W);

        // Get the Qt scroll width:
        int scrollW = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        pDummyWidget3->setFixedWidth(scrollW);

        pTopHLayout->addWidget(m_pSessionNavigationChart, TIMELINE_GRAPH_TO_LEGEND_WIDTH_PROPORTION, 0);
        pTopHLayout->addWidget(pContainingWidget, 0, Qt::AlignRight);
        pTopHLayout->addWidget(pDummyWidget3, 0, Qt::AlignRight);
        pContainingWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
        pDummyWidget3->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);


        m_pTimeLabel = new QLabel(this);
        m_pTimeLabel->setTextFormat(Qt::RichText);
        m_pTimeLabel->setStyleSheet(style);
        m_pTimeLabel->setVisible(false);

        m_pTrackingLine = new QWidget(this);
        m_pTrackingLine->setGeometry(0, 0, 1, height());
        QString style2 = QString(AF_STR_BGWithParam).arg(PP_TOOLTIP_BORDER_COLOR);
        m_pTrackingLine->setStyleSheet(style2);
        m_pTrackingLine->setVisible(false);
        m_pTrackingLine->setAttribute(Qt::WA_TransparentForMouseEvents);

        QString style3 = QString(AF_STR_BGWithParam).arg(acQAMD_ORANGE_PRIMARY_COLOUR.name());

        m_pBoundingBoxLeftLine = new QWidget(this);
        m_pBoundingBoxLeftLine->setGeometry(0, 0, 1, height());
        m_pBoundingBoxLeftLine->setStyleSheet(style3);
        m_pBoundingBoxLeftLine->setVisible(false);

        m_pBoundingBoxRightLine = new QWidget(this);
        m_pBoundingBoxRightLine->setGeometry(0, 0, 1, height());
        m_pBoundingBoxRightLine->setStyleSheet(style3);
        m_pBoundingBoxRightLine->setVisible(false);

        m_pBoundingBoxBottomLine = new QWidget(this);
        m_pBoundingBoxBottomLine->setGeometry(0, 0, 1, height());
        m_pBoundingBoxBottomLine->setStyleSheet(style3);
        m_pBoundingBoxBottomLine->setVisible(false);

        // Connect / Disconnect from profile events:
        UpdateProfileState();

        rc = connect(&(ppAppController::instance()), SIGNAL(CountersSelectionModified()), this, SLOT(OnSelectedCountersChanged()));
        GT_ASSERT(rc);

        AddStretchItemToGrid();

        ppAppController& appController = ppAppController::instance();

        if (m_pSessionController->GetSessionState() != ppSessionController::PP_SESSION_STATE_COMPLETED)
        {
            rc = connect(&appController, SIGNAL(NewPowerProfileData(ppQtEventData)), this, SLOT(OnNewProfileData(ppQtEventData)));
            GT_ASSERT(rc);
//--AT:LPGPU2
            rc = connect(&appController,
                SIGNAL(PowerProfilerCoreAboutToStop()), this,
                SLOT(OnPowerProfilerCoreAboutToStop()));
            GT_ASSERT(rc);
//--AT:LPGPU2
        }

        rc = connect(&acCustomPlotDropManager::Instance(), SIGNAL(PlotDropped(acCustomPlot*, acCustomPlot*)), this, SLOT(OnPlotDropped(acCustomPlot*, acCustomPlot*)));
        GT_ASSERT(rc);
    }
}

//TODO CF:LPGPU2 - this needs testing with live data now ranges have been updated
void ppTimeLineView::UpdateProfileState()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionController != nullptr) && (m_pSessionNavigationChart != nullptr))
    {
        // Get the current session state:
        ppSessionController::SessionState currentState = m_pSessionController->GetSessionState();

        // Set the sampling interval for the navigation chart:
        m_pSessionNavigationChart->SetInterval(m_pSessionController->GetSamplingTimeInterval());

        // in session complete state
        if (currentState == ppSessionController::PP_SESSION_STATE_COMPLETED)
        {
            disconnect(&(ppAppController::instance()), SIGNAL(NewPowerProfileData(ppQtEventData)), this, SLOT(OnNewProfileData(ppQtEventData)));
//++AT:LPGPU2
            disconnect(&(ppAppController::instance()),
                SIGNAL(PowerProfilerCoreAboutToStop()), this,
                SLOT(OnPowerProfilerCoreAboutToStop()));
//--AT:LPGPU2

            // Off-line sessions - get time range from DB
            SamplingTimeRange samplingTimeRange{ 0, 0 };
//++CF:LPGPU2 Time range stored in ms but retrieved in ns
            m_pSessionController->GetSessionTimeRange(samplingTimeRange);
//--CF:LPGPU2

            // if session is more than 10 seconds - show last 10 seconds. else - show all session from start to end
//++CF:LPGPU2
            // if (samplingTimeRange.m_toTime - samplingTimeRange.m_fromTime > PP_TIMELINE_GRAPHS_DEFAULT_INITIAL_RANGE)
            // {
            //     samplingTimeRange.m_fromTime = samplingTimeRange.m_toTime - PP_TIMELINE_GRAPHS_DEFAULT_INITIAL_RANGE;
            // }
//--CF:LPGPU2

            InitTimelinesFromDB();

            SetGraphsInitializationData();
//++AT:LPGPU2
            if (!m_isRegionOfInterestTimeline)
            {
              UpdateRibbonsTimeRangeFromDB(samplingTimeRange, false);
            }
//--AT:LPGPU2
        }
        else
        {
            // on running mode - disable clicking on legend add/remove counters row
            if (currentState == ppSessionController::PP_SESSION_STATE_RUNNING)
            {
                if (ppAppController::instance().GetDurationMode() ==
                    lpgpu2::DurationMode::Timer)
                {
                  m_pTimerTimingModeFrame->StartedSession();
                }

                EnableRibbonsInfoTabelAndRemoveCounters(false);
            }
        }
    }

    // Update the navigation counter:
    UpdateNavigationCounter();

    AddStretchItemToGrid();
}

// ----------------------------  session events ------------------------------
void ppTimeLineView::OnNewProfileData(ppQtEventData pSampledDataPerCounter)
{
    SamplingTimeRange range(0, 0);
    eRangeState rangeState = RANGE_STATE_DEFAULT;

    if (pSampledDataPerCounter != nullptr && pSampledDataPerCounter->size() > 0)
    {
        //update from time only at start
        gtMap<int, PPSampledValuesBatch>::const_iterator it = pSampledDataPerCounter->begin();
        int sampleTime = (*it).second.m_quantizedTime;

        // Fill the duration string and set the progress wrapper bar text
        gtString durationStr = acDurationAsString(sampleTime / MILLISECONDS_IN_SECOND);
        durationStr.prepend(PP_StrTimelineProgressBarWrapperLabel);
        afProgressBarWrapper::instance().setProgressText(durationStr);

        // if catalyst is not installed refresh only every NO_CATALYST_REFRESH_RATE ms
        bool shouldReplot = true;

        if (!(afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() | AF_AMD_CATALYST_COMPONENT))
        {
            if (sampleTime - m_lastReplotTime < NO_CATALYST_REFRESH_RATE)
            {
                shouldReplot = false;
            }
        }

        // on new data from event - update range directly from navigation chart (AddNewDataToSessionNavigationChart)
        SamplingTimeRange samplingRange(0, 0);
        samplingRange = AddNewDataToSessionNavigationChart(pSampledDataPerCounter, shouldReplot);
//++AT:LPGPU2
        rangeState = m_pSessionNavigationChart->GetRangeState();

        const bool bShouldUpdateLastSample =
          ((RANGE_STATE_START_END == rangeState) ||
           (RANGE_STATE_MID_END == rangeState) ||
           (RANGE_STATE_DEFAULT == rangeState));

        
        AddNewDataToAllExistingGraphs(pSampledDataPerCounter, bShouldUpdateLastSample);

        UpdateRibbonsTimeRangeOnNewProfileData(samplingRange, false);

        // if there was a range change
        if (m_isRangeChanged)
        {
//--AT:LPGPU2
            // if tracking line is visible - update the displayed time
            if (m_pTimeLabel->isVisible() && m_shouldDrawTrackingLine)
            {
                // check ribbons vector before referring to first element
                if (!m_visibleRibbonsVec.isEmpty())
                {
                    ppMultiLinePlot* pPlot = m_visibleRibbonsVec[0];

                    if (pPlot != nullptr)
                    {
                        m_trackLineKey =
                          pPlot->UpdateTrackingTime(m_trackLineKey);
                    }

                    OnTrackingXAxis(m_trackLineKey, m_trackingLineXCoordinate);
                }
            }
        }

        if (shouldReplot)
        {
            ReplotVisibleGraphs();
            m_lastReplotTime = sampleTime;
        }

//++AT:LPGPU2
        FixLabelPaddingDiffs();
//--AT:LPGPU2
    }
}
void ppTimeLineView::OnProfileStopped(const QString& sessionName)
{
    GT_UNREFERENCED_PARAMETER(sessionName);

    ppAppController& appController = ppAppController::instance();

    disconnect(&appController, SIGNAL(NewPowerProfileData(ppQtEventData)), this, SLOT(OnNewProfileData(ppQtEventData)));


//++AT:LPGPU2
    disconnect(&(ppAppController::instance()),
        SIGNAL(PowerProfilerCoreAboutToStop()), this,
        SLOT(OnPowerProfilerCoreAboutToStop()));
    RemoveAllRibbons();
    InitGraphs();
    UpdateProfileState();
    m_pTimerTimingModeFrame->StoppedSession();
//--AT:LPGPU2

    //EnableRibbonsInfoTabelAndRemoveCounters(true);

    //ReplotAllGraphs();
}

// ----------------------- All graphs initialization functions -------------------------------

void ppTimeLineView::InitGraphs()
{
    QVector<ppDataUtils::GraphViewCategoryType> graphTypes;

//++AT:LPGPU2
    graphTypes << ppDataUtils::TIMELINE_POWER;
//--AT:LPGPU2
    graphTypes << ppDataUtils::TIMELINE_POWER_DGPU;
    graphTypes << ppDataUtils::TIMELINE_FREQUENCY;
    graphTypes << ppDataUtils::TIMELINE_TEMPERATURE;
    graphTypes << ppDataUtils::TIMELINE_VOLTAGE;
    graphTypes << ppDataUtils::TIMELINE_CURRENT;
    graphTypes << ppDataUtils::TIMELINE_CPU_CORE_PSTATE;
    graphTypes << ppDataUtils::TIMELINE_CPU_CORE_CSTATE;

    // mgb:ss adding support for counter types
    graphTypes << ppDataUtils::TIMELINE_COUNTER;    

    // delete all graphs and start over
    RemoveAllRibbons();

    // Add the power graphs:
    gtVector<int> ids;
//++AT:LPGPU2
// TODO Commented out because it is never used due to InitPowerGraph being
// commented out by the original developers
    //ppDataUtils::GetRelevantCounterIdsByGraphType(ids, ppDataUtils::TIMELINE_POWER, m_pSessionController);
//--AT:LPGPU2
//++CF:LPGPU2 - set the offset before any graphs are drawn.
    UpdateTraceStartOffset();
//--CF:LPGPU2

    if (ids.size() > 0)
    {
        //SRB: This seems to create problems, fix this
        //InitPowerGraph();
    }

    ids.clear();

//++AT:LPGPU2
    const auto devicesFlattened =
      m_pSessionController->GetSystemDevicesAsFlatVector();
    for (const auto &dev : devicesFlattened)
    {
      for (const auto &gt : graphTypes)
      {
          ppDataUtils::GetRelevantCounterIdsByGraphTypeAndDeviceId(ids, gt,
              dev.m_deviceId,
              m_pSessionController);

          if (ids.size() > 0)
          {
              // Initialize the current graph:
              InitGraphByCategoryAndDeviceId(gt, dev.m_deviceId, dev.m_deviceName);
          }
      }
    }
//--AT:LPGPU2

  //++KB: LPGPU2
//++AT:LPGPU2
  InitAPITimeline();
//--AT:LPGPU2
  //--KB: LPGPU2
//++CF:LPGPU2
  if (!m_isRegionOfInterestTimeline)
  {
//++AT:LPGPU2
    InitHelpTimeline();
//--AT:LPGPU2
  }
//--CF:LPGPU2

    // All ribbons are initialized in m_visibleRibbonsVec and m_allRibbonsVec. Now the ribbons should be added to the grid:
    AddRibbonsToGrid();
    AddTimelineRibbonsToGrid();

    ReplotVisibleGraphs();
}

//++KB: LPGPU2
/// @brief Initialise the GL timeline ribbon by creating it and sets the
///        label's text
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
void ppTimeLineView::InitAPITimeline()
{
  InitGenericTimeline(m_APITimelineWidget);
  m_APITimelineWidget->SetLabelText("Frames");
  m_APITimelineWidget->SetVisible(false);
//--AT:LPGPU2
}

//++AT:LPGPU2
/// @brief Initialise the timeline ribbons by reading their data from the DB
/// @note It assumes that the timeline ribbons have been created
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
void ppTimeLineView::InitTimelinesFromDB()
{
  InitAPITimelineFromDB();
  if (!m_isRegionOfInterestTimeline)
  {
    InitHelpTimelineFromDB();
  }
}

/// @brief Add the timeline ribbon widgets to the main vertical grid
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
void ppTimeLineView::AddTimelineRibbonsToGrid()
{
  // Make them visible by adding them to the grid
  GT_ASSERT(m_APITimelineWidget);
  AddWidgetToGrid(m_APITimelineWidget.get());
  if (!m_isRegionOfInterestTimeline)
  {
    GT_ASSERT(m_HelpTimelineWidget);
    AddWidgetToGrid(m_HelpTimelineWidget.get());
  }
}

/// @brief Create a timeline widget by resetting the unique ptr
/// @param timelineWidget The unique ptr representing the widget to initialise
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
/// @note Used by the GL and Help timeline ribbon widgets creation code
void ppTimeLineView::InitGenericTimeline(ppTimeLineView::RWWUP &timelineWidget)
{
  ppTimeline *pTraceView = new ppTimeline(this);
  GT_ASSERT(pTraceView);

  pTraceView->setTitleSectionLeftSpace(0);
  pTraceView->setTitleSectionRightSpace(0);

  pTraceView->grid()->setGridLabelSpace(0);
  pTraceView->grid()->hide();

  auto rc = connect(pTraceView, SIGNAL(mouseMove(int)), this, SLOT(OnTrackingTraceXAxis(int)));
  GT_ASSERT(rc);

  auto branch = new acTimelineBranch();
  GT_ASSERT(branch);
  pTraceView->addBranch(branch);
  pTraceView->setPivotMouseTracking(false);
  pTraceView->setPivot(-1); //-1 means don't display

  pTraceView->setMinimumHeight(branch->height());

  timelineWidget.reset(new LPGPU2ppRibbonTimelineWrapper(pTraceView));
}

/// @brief Set the data of the API Call Timeline by reading from the database
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
void ppTimeLineView::InitAPITimelineFromDB()
{
    GT_ASSERT(static_cast<bool>(m_APITimelineWidget));

    auto pTimelineWrappedWidget = qobject_cast<ppTimeline*>(m_APITimelineWidget->GetWidget());
    GT_ASSERT(pTimelineWrappedWidget);
    auto branches = pTimelineWrappedWidget->GetBranches();
    auto branch = branches[0];


    // Insert a dummy frame at the start of the frame timeline, this is needed to synchronise
    // the start of the frames with the start of the counter data. Its height is set to a small
    // value so it is not visible:
    const quint64 dummyFrameStart = 0;
    const quint64 dummyFrameEnd = 1;
    const float dummyFrameHeight = 0.01f;
    auto *item = new (std::nothrow) LPGPU2ppTimelineItem{ dummyFrameStart, dummyFrameEnd, false };
    GT_ASSERT(item);
    item->setBackgroundColor(PP_LPGPU2_FRAME_TIMELINE_COLOR);
    item->setFractionalHeight(dummyFrameHeight);
    branch->addTimelineItem(item);

    gtVector<lpgpu2::db::ProfileTraceInfo> records;
    if (m_pSessionController->GetProfilerBL().GetTraceRecords(records))
    {
        if (!records.empty())
        {
            // Filter the records vector to get the unique frame numbers.
            auto lastUniqueFrameNumRecord = std::unique(records.begin(), records.end(), [](const lpgpu2::db::ProfileTraceInfo &traceA, const lpgpu2::db::ProfileTraceInfo &traceB)
            {
                return traceA.m_frameNum == traceB.m_frameNum;
            });
            records.erase(lastUniqueFrameNumRecord, records.end());

            for (const auto& trace : records)
            {
                gtUInt64 frameStartNs;
                gtUInt64 frameEndNs;

                GT_IF_WITH_ASSERT(m_pSessionController->GetProfilerBL().GetFrameTimeRange(trace.m_frameNum, frameStartNs, frameEndNs))
                {
                    const auto bInMilliSeconds = false;
                    auto *item = new (std::nothrow) LPGPU2ppGLTimelineItem{ frameStartNs, frameEndNs, trace.m_frameNum, 0, bInMilliSeconds };

                    GT_ASSERT(item);

                    item->setText(QString::number(trace.m_frameNum));
                    item->setBackgroundColor(PP_LPGPU2_FRAME_TIMELINE_COLOR);
                    branch->addTimelineItem(item);
                }
            }
        }
    }           

    GT_ASSERT(connect(pTimelineWrappedWidget, SIGNAL(itemClicked(acTimelineItem*)), this, SLOT(OnFrameItemClicked(acTimelineItem*))));

    const auto isLPGPU2Session =
      m_pSessionController->GetProfilerBL().IsLPGPU2Session();

    // Only show the timeline widgets if the session is of LPGPU2 type
    m_APITimelineWidget->SetVisible(isLPGPU2Session);

    if (m_isRegionOfInterestTimeline)
    {
//++AT:LPGPU2
        UpdateRibbonsTimeRangeFromDB(m_samplingTimeRange, false);
//--AT:LPGPU2
    }
    else
    {
        //++CF:LPGPU2
        // Get the start of the session range to correctly offset the timeline:
        SamplingTimeRange samplingRange{ 0, 0 };
        m_pSessionController->GetSessionTimeRange(samplingRange);

        using std::chrono::milliseconds;
        using std::chrono::nanoseconds;
        using std::chrono::duration_cast;

        const auto startNs =
          duration_cast<nanoseconds>(milliseconds{samplingRange.m_fromTime});
        const auto endNs =
          duration_cast<nanoseconds>(milliseconds{samplingRange.m_toTime});

        branch->setStartTime(startNs.count());
        branch->setEndTime(endNs.count());
        //--CF:LPGPU2    
    }    
}

/// @brief Set the data of the Help Timeline by reading from the database
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
void ppTimeLineView::InitHelpTimelineFromDB()
{
    // Do not use assert here since this is a valid case. This function is called when the feedback engine
    // executed but sometimes the m_HelpTimelineWidget can be null if this is not the target session of
    // the feedback engine.
    if (m_HelpTimelineWidget != nullptr)
    {
        auto pTimelineWrappedWidget = qobject_cast<ppTimeline*>(m_HelpTimelineWidget->GetWidget());
        GT_IF_WITH_ASSERT(pTimelineWrappedWidget != nullptr)
        {
            auto branches = pTimelineWrappedWidget->GetBranches();
            auto branch = branches[0];

            gtVector<lpgpu2::db::RegionOfInterest> regionsOfInterest;
            m_pSessionController->GetProfilerBL().GetRegionsOfInterest(regionsOfInterest);

            // Get the start of the session range to correctly offset the timeline:
            SamplingTimeRange samplingRange(0, 0);
            m_pSessionController->GetSessionTimeRange(samplingRange);

            if (m_pRunFeedbackEngineShortcutFrame != nullptr)
            {
                // Display the feedback engine shortcut in case there are no regions of interest
                const auto bIsLPGPU2Session = m_pSessionController->GetProfilerBL().IsLPGPU2Session();
                const auto bHasRegionsOfInterest = !regionsOfInterest.empty();
                m_pRunFeedbackEngineShortcutFrame->setVisible(bIsLPGPU2Session && !bHasRegionsOfInterest);
            }

            if (!regionsOfInterest.empty())
            {
                for (auto roi : regionsOfInterest)
                {
                    gtUInt64 frameStartStartTime;
                    gtUInt64 frameStartEndTime;
                    gtUInt64 frameEndStartTime;
                    gtUInt64 frameEndEndTime;

                    m_pSessionController->GetProfilerBL().GetFrameTimeRange(roi.m_frameStart, frameStartStartTime, frameStartEndTime);
                    m_pSessionController->GetProfilerBL().GetFrameTimeRange(roi.m_frameEnd, frameEndStartTime, frameEndEndTime);

                    const std::chrono::nanoseconds beginTimeNs{ frameStartStartTime };
                    const std::chrono::nanoseconds endTimeNs{ frameEndEndTime };

                    const auto bInMilliSeconds = false;
                    auto item = new (std::nothrow) LPGPU2ppHelpTimelineItem{
                      static_cast<quint64>(beginTimeNs.count()),
                      static_cast<quint64>(endTimeNs.count()),
                      roi.m_regionId,
                      QString::number(roi.m_regionId),
                      roi.m_degreeOfInterest,
                      bInMilliSeconds };
                    GT_ASSERT(item);

                    // If Degree of interest is out of range, round it down so the colour is still valid:
                    if (roi.m_degreeOfInterest > 1)
                    {
                        roi.m_degreeOfInterest = 1;
                    }

                    item->setText(QString::number(roi.m_regionId));
                    item->setBackgroundColor(QColor::fromRgbF(static_cast<double>(roi.m_degreeOfInterest),
                                                              static_cast<double>((1 - roi.m_degreeOfInterest)),
                                                              0));
                    branch->addTimelineItem(item);

                    bool rc = connect(pTimelineWrappedWidget, SIGNAL(itemClicked(acTimelineItem*)), this, SLOT(OnHelpItemClicked(acTimelineItem*)));
                    GT_ASSERT(rc);
                }
            }

            //++CF:LPGPU2
            using std::chrono::milliseconds;
            using std::chrono::nanoseconds;
            using std::chrono::duration_cast;

            const auto startNs =
              duration_cast<nanoseconds>(
                  milliseconds{samplingRange.m_fromTime});
            const auto endNs =
              duration_cast<nanoseconds>(milliseconds{samplingRange.m_toTime});

            branch->setStartTime(startNs.count());
            branch->setEndTime(endNs.count());
            //branch->setStartTime(samplingRange.m_fromTime);
            //branch->setEndTime(samplingRange.m_toTime);
            //--CF:LPGPU2

            //++AT:LPGPU2
            // Only show the timeline widgets if the session is of LPGPU2 type
            const auto isLPGPU2Session =
                m_pSessionController->GetProfilerBL().IsLPGPU2Session();

            m_HelpTimelineWidget->SetVisible(isLPGPU2Session);
            //--AT:LPGPU2
        }       
    }    
}

/// @brief Initialise the Help timeline ribbon by creating it and sets the
///        label's text
/// @note We return void because even though it is against the LPGPU2
///       coding convention because this conforms with the CXL existing
///       codebase
void ppTimeLineView::InitHelpTimeline()
{
  InitGenericTimeline(m_HelpTimelineWidget);
  m_HelpTimelineWidget->SetLabelText(kHelpTimelineLabel);
  m_HelpTimelineWidget->SetVisible(false);
}
//--AT:LPGPU2

void ppTimeLineView::OnHelpItemClicked(acTimelineItem* item)
{
    LPGPU2ppHelpTimelineItem* helpItem = qobject_cast<LPGPU2ppHelpTimelineItem*>(item);
    GT_IF_WITH_ASSERT(helpItem != nullptr)
    {
        emit OnBubbleClicked(helpItem->helpId());
    }
}

//++CF:LPGPU2
void ppTimeLineView::OnFrameItemClicked(acTimelineItem* item)
{
    LPGPU2ppGLTimelineItem* frameItem = qobject_cast<LPGPU2ppGLTimelineItem*>(item);
    GT_IF_WITH_ASSERT(frameItem != nullptr)
    {
        emit OnFrameBubbleClicked(frameItem->frameId());
    }
}
//--CF:LPGPU2

//--KB: LPGPU2DEMO_V

void ppTimeLineView::SetGraphsInitializationData()
{
    // Go through the existing ribbons and update its data
    foreach (ppMultiLinePlot* pPlot, m_allRibbonsVec)
    {
        GT_IF_WITH_ASSERT(pPlot != nullptr)

        {
            pPlot->SetGraphInitializationData();
        }
    }

    // replot the shown graphs
    ReplotVisibleGraphs();
}

void ppTimeLineView::AddNewDataToAllExistingGraphs(ppQtEventData pSampledDataPerCounter, bool bShouldUpdateLastSample)
{
    // Go through the existing ribbons and add the new data:
    foreach (ppMultiLinePlot* pPlot, m_allRibbonsVec)
    {
        GT_IF_WITH_ASSERT(pPlot != nullptr)
        {
            pPlot->AddNewDataToExistingGraph(pSampledDataPerCounter, m_samplingTimeRange, bShouldUpdateLastSample);
        }
    }
}


void ppTimeLineView::ReplotVisibleGraphs()
{
    // Go through the visible ribbons and update its data
    foreach (ppMultiLinePlot* pPlot, m_visibleRibbonsVec)
    {
        if (pPlot != nullptr)
        {
            // need to replot even if the region of the plot is not shown (might be because of not shown because of scroll bar region but still need to be updated)
            if (pPlot->IsShown())
            {
                pPlot->Replot();
            }
        }
    }
}

void ppTimeLineView::ReplotAllGraphs()
{
    foreach (ppMultiLinePlot* pPlot, m_allRibbonsVec)
    {
        if (pPlot != nullptr)
        {
            pPlot->Replot();
        }
    }
}

// -------------------------- Power Graph----------------------------------------
//++CF:LPGPU2
  // This method is never actually used, all uses of it have been commented out
  // by the original developers.
//--CF:LPGPU2
// void ppTimeLineView::InitPowerGraph()
// {
//     // Create the power plot:
//     m_pPowerPlot = new ppMultiLinePowerNonStackedPlot(m_pSessionController);
// 
//     QString graphName = PP_StrTimelineAPUPowerGraphName;
// 
//     // Initialize the power plot properties:
//     QString xAxisTitle;
//     QString yAxisTitle = QString(AF_STR_QStringAppend).arg(graphName).arg(PP_STR_UnitsPostfixWatt);
//     m_pPowerPlot->InitPlot(xAxisTitle, yAxisTitle, AMDT_PWR_CATEGORY_POWER, ppDataUtils::TIMELINE_POWER, acMultiLinePlot::GRAPHVALUESTYPE_DOUBLE, PP_STR_UnitsPostfixWatt);
// 
//     m_allRibbonsVec << m_pPowerPlot;
// 
//     m_pPowerPlot->InitPlotWithSelectedCounters();
//     m_pPowerPlot->SetShown(false, false);
//     m_pPowerPlot->GetPlot()->EnableDragAndDrop(false);
// 
//     m_pPowerPlot->GetPlot()->setContextMenuPolicy(Qt::CustomContextMenu);
//     bool rc = connect(m_pPowerPlot->GetPlot(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuRequest(const QPoint&)));
//     GT_ASSERT(rc);
// 
//     m_pPowerStackedPlot = new ppMultiLnePowerStackedPlot(m_pSessionController);
// 
//     // Initialize the power stacked plot properties:
//     yAxisTitle = QString(AF_STR_QStringAppend).arg(graphName).arg(PP_STR_UnitsPostfixWatt);
//     m_pPowerStackedPlot->InitPlot(xAxisTitle, yAxisTitle, AMDT_PWR_CATEGORY_POWER, ppDataUtils::TIMELINE_POWER, acMultiLinePlot::GRAPHVALUESTYPE_DOUBLE, PP_STR_UnitsPostfixWatt);
// 
//     m_pPowerStackedPlot->InitPlotWithSelectedCounters();
//     m_pPowerStackedPlot->SetShown(true, false);
//     m_pPowerStackedPlot->GetPlot()->EnableDragAndDrop(false);
//     m_visibleRibbonsVec << m_pPowerStackedPlot;
//     m_allRibbonsVec << m_pPowerStackedPlot;
// 
//     // connect the context menu
//     m_pPowerStackedPlot->GetPlot()->setContextMenuPolicy(Qt::CustomContextMenu);
//     rc = connect(m_pPowerStackedPlot->GetPlot(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuRequest(const QPoint&)));
//     GT_ASSERT(rc);
// 
// }

void ppTimeLineView::OnContextMenuRequest(const QPoint& pos)
{
    QMenu* pMenu = new QMenu(this);
    pMenu->setAttribute(Qt::WA_DeleteOnClose);

    GT_IF_WITH_ASSERT((m_pPowerPlot != nullptr) && (m_pPowerStackedPlot != nullptr))
    {
        // Find the plot to show and hide:
        bool isNonStackedGraphShown = m_pPowerPlot->IsShown();

        // Add the actions to the context menu:
        QAction* pSelectStackedAction = pMenu->addAction(PP_STR_TimelineContextMenuShowStackedGraph, this, SLOT(OnChangePowerGraphDisplayType()));
        pSelectStackedAction->setObjectName(PP_STR_TimelineContextMenuShowStackedGraph);

        QAction* pSelectNonStackedAction = pMenu->addAction(PP_STR_TimelineContextMenuShowNonStackedGraph, this, SLOT(OnChangePowerGraphDisplayType()));
        pSelectNonStackedAction->setObjectName(PP_STR_TimelineContextMenuShowNonStackedGraph);

        GT_IF_WITH_ASSERT(nullptr != pSelectStackedAction && nullptr != pSelectNonStackedAction)
        {
            // set action checked
            if (isNonStackedGraphShown)
            {
                pSelectNonStackedAction->setCheckable(true);
                pSelectNonStackedAction->setChecked(true);
            }
            else
            {
                pSelectStackedAction->setCheckable(true);
                pSelectStackedAction->setChecked(true);
            }

            ppMultiLinePlot* pShownPlot = isNonStackedGraphShown ? m_pPowerPlot : m_pPowerStackedPlot;
            ppMultiLinePlot* pNonShownPlot = isNonStackedGraphShown ? m_pPowerStackedPlot : m_pPowerPlot ;

            // popup context menu:
            pMenu->popup(acMapToGlobal(pShownPlot->GetPlot(), pos));

            // when switching between power stacked graph and non-stacked graph -
            // set the legend check states according to the previous showed legend check states
            QVector<bool> hiddenStateVec;
            pShownPlot->GetAllGraphsHiddenState(hiddenStateVec);
            pNonShownPlot->SetPowerGraphLegendCheckState(hiddenStateVec);
        }
    }
}

void ppTimeLineView::AddPlotToGrid(ppMultiLinePlot* pPlot)
{
    GT_IF_WITH_ASSERT(nullptr != pPlot && nullptr != pPlot->GetPlot())
    {
        // Get the custom plot item:
        acCustomPlot* pCustomPlot = pPlot->GetPlot();

        // get legend
        acListCtrl* pInfoTable = pPlot->GetPlotInfoTable();
        GT_IF_WITH_ASSERT((pInfoTable != nullptr) && (pCustomPlot != nullptr))
        {
            // Calculate the table fixed width, according to the current font metric:
            int tableFixedWidth = CalculateInfoTableFixedWidth();
            pInfoTable->setFixedWidth(tableFixedWidth);

            // Set the table min / max height. The table height should be the plot height minus the vertical margins:
            int tableH = acScalePixelSizeToDisplayDPI(PP_RIBBON_MIN_HEIGHT) - PP_TIMELINE_PLOT_MARGINS.bottom() - PP_TIMELINE_PLOT_MARGINS.top();
            pInfoTable->setMinimumHeight(tableH);
            pInfoTable->setMaximumHeight(tableH);

            pCustomPlot->setMinimumWidth(tableFixedWidth);

            // Create new ribbon, add the plot into the ribbon, and add the new ribbon to ribbons vector:
            QGridLayout* pLayout = new QGridLayout;

            // Initialize the plot buttons and add it to the layout:
            InitRibbonButtons(pPlot);

            // Create a dummy widget to hold the left side of the layout. Give it the same height as the buttons,
            // to make sure that it reserves the height needed:
            int buttonDim = (int)acScalePixelSizeToDisplayDPI(PP_RIBBON_BUTTON_DIMENSION);
            QWidget* pDummyWidget = new QWidget;
            pDummyWidget->setMinimumHeight(buttonDim);
            pDummyWidget->setMinimumWidth(tableFixedWidth);

            pLayout->addWidget(pCustomPlot, 0, 0);
            pLayout->addWidget(pInfoTable, 0, 1);

            m_pBottomVLayout->addLayout(pLayout, 0);

            QMargins margins(acScalePixelSizeToDisplayDPI(PP_TIMELINE_PLOT_MARGINS.left()), acScalePixelSizeToDisplayDPI(PP_TIMELINE_PLOT_MARGINS.top()),
                             acScalePixelSizeToDisplayDPI(PP_TIMELINE_PLOT_MARGINS.right()), acScalePixelSizeToDisplayDPI(PP_TIMELINE_PLOT_MARGINS.bottom()));

            pCustomPlot->xAxis->axisRect()->setAutoMargins(QCP::msNone);
            pCustomPlot->xAxis->axisRect()->setMargins(margins);

//++AT:LPGPU2
            pCustomPlot->yAxis->axisRect()->setAutoMargins(QCP::msLeft);
//--AT:LPGPU2
            pCustomPlot->yAxis->axisRect()->setMargins(margins);

            pCustomPlot->setMinimumHeight(acScalePixelSizeToDisplayDPI(PP_RIBBON_MIN_HEIGHT));
            pCustomPlot->setMaximumHeight(acScalePixelSizeToDisplayDPI(PP_RIBBON_MIN_HEIGHT));

            QColor color = Qt::black;
            color.setAlpha(7);

            pCustomPlot->xAxis->axisRect()->setBackground(color);

            bool rc = connect(pPlot, SIGNAL(TrackingXAxis(double, int)), this, SLOT(OnTrackingXAxis(double, int)));
            GT_ASSERT(rc);

            rc = connect(pPlot->GetPlot(), SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(OnPlotMouseMove(QMouseEvent*)));
            GT_ASSERT(rc);

            rc = connect(pPlot->GetPlot(), SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(OnPlotMouseWheel(QWheelEvent*)));
            GT_ASSERT(rc);

            rc = connect(pPlot->GetPlot(), SIGNAL(PlotEntered(acCustomPlot*)), this, SLOT(OnPlotEntered(acCustomPlot*)));
            GT_ASSERT(rc);

            rc = connect(pPlot->GetPlot(), SIGNAL(PlotLeave(acCustomPlot*)), this, SLOT(OnPlotLeave(acCustomPlot*)));
            GT_ASSERT(rc);
        }

    }
}

//++KB: LPGPU2
void ppTimeLineView::AddWidgetToGrid(LPGPU2ppRibbonWidgetWrapper* wrapper)
{
    GT_IF_WITH_ASSERT(nullptr != wrapper)
    {
        QGridLayout* pLayout = new QGridLayout;

        // Faking the size of the info table that normally appears on the right of the plot.
        QWidget* pRightPaddingWidget = new QWidget;

        // Calculate the right padding fixed width, according to the current font metric:
        int rightPaddingWidth = CalculateInfoTableFixedWidth();
        pRightPaddingWidget->setFixedWidth(rightPaddingWidth + PP_TIMELINE_PLOT_MARGINS.right());

        // Using the wrapper's vertical label, set the fixed width padding on the left side of the ribbon widget
        int leftPaddingWidth = acScalePixelSizeToDisplayDPI(PP_TIMELINE_PLOT_MARGINS.left());
        wrapper->GetLabel()->setFixedWidth(leftPaddingWidth);

        // Adding the wrapped widget and padding widgets to the main bottom layout
        pLayout->addWidget(wrapper->GetLabel(), 0, 0);
        pLayout->addWidget(wrapper->GetWidget(), 0, 1, Qt::AlignVCenter);
        pLayout->addWidget(pRightPaddingWidget, 0, 2);
        //++CF:LPGPU2
        m_pTimelineLayout->addLayout(pLayout, 0);
        //--CF:LPGPU2
    }
}
//--KB: LPGPU2

//++AT:LPGPU2
void ppTimeLineView::UpdateRibbonsTimeRangeFromDB(
    const SamplingTimeRange& newRange, bool isReplotNeeded)
{
  //++CF:LPGPU2
  UpdateRibbonsTimeRange(newRange, isReplotNeeded);


  SamplingTimeRange adjustedRange { std::abs(m_samplingTimeRange.m_fromTime -
      m_traceStartOffsetMs), std::abs(m_samplingTimeRange.m_toTime -
        m_traceStartOffsetMs) };

  if (adjustedRange.m_fromTime != 0 || adjustedRange.m_toTime != 0)
  {
    m_APITimelineWidget->UpdateTimeRange(adjustedRange);
    if (!m_isRegionOfInterestTimeline)
    {
      m_HelpTimelineWidget->UpdateTimeRange(m_samplingTimeRange);
    }
  }
//--CF:LPGPU2

}

void ppTimeLineView::UpdateRibbonsTimeRangeOnNewProfileData(
    const SamplingTimeRange& newRange, bool isReplotNeeded)
{
  UpdateRibbonsTimeRange(newRange, isReplotNeeded);
}

void ppTimeLineView::UpdateRibbonsTimeRange(const SamplingTimeRange& newRange,
    bool isReplotNeeded)
{
    // on range change event from controller
    m_samplingTimeRange.m_fromTime = newRange.m_fromTime;
    m_samplingTimeRange.m_toTime = newRange.m_toTime;

    foreach (ppMultiLinePlot* pPlot, m_allRibbonsVec)
    {
        if (pPlot != nullptr)
        {
            pPlot->UpdatePlotRange(isReplotNeeded, m_samplingTimeRange);
        }
    }
}
//--AT:LPGPU2

//++CF:LPGPU2
// Fetch the trace offset from the database. This is a separate method so it can
// be called once at the start rather than querying the DB every time the range
// changes.
void ppTimeLineView::UpdateTraceStartOffset()
{
  if (m_isRegionOfInterestTimeline)
  {
    // The below line used to generate an offset to fix the position of timeline items,
    // but this is no longer used:
    // m_pSessionController->GetProfilerBL().GetTraceStartTimeOffsetMs(m_traceStartOffsetMs);
    m_traceStartOffsetMs = 0;
  }
  else
  {
    m_traceStartOffsetMs = 0;
  }
}
//--CF:LPGPU2

void ppTimeLineView::OnSelectedCountersChanged()
{
    // Get the current session state:
    ppSessionController::SessionState currentState = m_pSessionController->GetSessionState();

    // draw the new selected counters graphs only if in new session mode. don't change the view in running or completed mode
    if (currentState == ppSessionController::PP_SESSION_STATE_NEW)
    {
        InitGraphs();
    }
}

bool ppTimeLineView::IsRibbonTypeShown(ppDataUtils::GraphViewCategoryType type)
{
    bool retVal = false;

    foreach (ppMultiLinePlot* pPlot, m_visibleRibbonsVec)
    {
        if (pPlot != nullptr)
        {
            if (pPlot->GraphType() == type)
            {
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}

void ppTimeLineView::RemoveAllRibbons()
{
    foreach(ppMultiLinePlot* pPlot, m_visibleRibbonsVec)
    {
        if (pPlot != nullptr)
        {
            m_pBottomVLayout->removeWidget(pPlot->GetPlotInfoTable());
            m_pBottomVLayout->removeWidget(pPlot->GetPlot());

            pPlot->Delete();
            pPlot->deleteLater();
        }
    }

    m_visibleRibbonsVec.clear();
    m_allRibbonsVec.clear();

    //++AT:LPGPU2
        // Rm the gl and help timeline   ribbons
    if (m_APITimelineWidget)
    {
        m_pBottomVLayout->removeWidget(m_APITimelineWidget->GetWidget());
    }
    if (m_HelpTimelineWidget)
    {
        m_pBottomVLayout->removeWidget(m_HelpTimelineWidget->GetWidget());
    }
    //--AT:LPGPU2
}

void ppTimeLineView::OnChangePowerGraphDisplayType()
{
    QObject* pSender = sender();

    if (nullptr != pSender)
    {
        // sanity check
        GT_IF_WITH_ASSERT((m_pPowerPlot != nullptr) && (m_pPowerStackedPlot != nullptr))
        {
            // if first shown (non-cumulative power graph) - hide it and show second
            bool isNonStackedGraphShown = m_pPowerPlot->IsShown();
            QString senderName = pSender->objectName();

            // Find the plot to show and hide:
            ppMultiLinePlot* pPlotToHide = isNonStackedGraphShown ? m_pPowerPlot : m_pPowerStackedPlot;
            ppMultiLinePlot* pPlotToShow = isNonStackedGraphShown ? m_pPowerStackedPlot : m_pPowerPlot;


            // check if same graph that selected in the context menu already shown - do nothing
            if ((senderName == PP_STR_TimelineContextMenuShowStackedGraph && isNonStackedGraphShown) ||
                (senderName == PP_STR_TimelineContextMenuShowNonStackedGraph && !isNonStackedGraphShown))
            {
                // Replace the item in ribbons vector, and rebuild the layout:
                int index = m_visibleRibbonsVec.indexOf(pPlotToHide);
                GT_IF_WITH_ASSERT((index >= 0) && (index < m_visibleRibbonsVec.size()))
                {
                    m_visibleRibbonsVec[index] = pPlotToShow;

                    // Re-build the layout:
                    AddRibbonsToGrid();

                    // Set ribbon visibility and update the display:
                    pPlotToShow->SetShown(true, true);
                    pPlotToHide->SetShown(false, true);

                    // Replot the new shown graph:
                    pPlotToShow->Replot();
                }
            }
        }
    }

}

void ppTimeLineView::OnRangeChangedByUser(const QPointF& range)
{
//++AT:LPGPU2
  // Get the current session state:
  ppSessionController::SessionState currentState
    = m_pSessionController->GetSessionState();

  // Set the sampling interval for the navigation chart:
  m_pSessionNavigationChart->SetInterval(m_pSessionController->GetSamplingTimeInterval());
  
  //++AT:LPGPU2
  // Necessary because when a new session is started, the old one will read the
  // range from the DB and it could be negative
    auto newRange = range;
    newRange.setX(qMax(0.0, newRange.x()));
    newRange.setY(qMax(0.0, newRange.y()));
  //--AT:LPGPU2
  
    // in session complete state
    if (currentState == ppSessionController::PP_SESSION_STATE_COMPLETED)
    {
      // in power profiling the range is assumed to be acceptable in integers
      UpdateRibbonsTimeRangeFromDB(
          SamplingTimeRange(static_cast<gtUInt64>(newRange.x()), static_cast<gtUInt64>(newRange.y())), true);
    }
    else
    {
      // in power profiling the range is assumed to be acceptable in integers
      UpdateRibbonsTimeRangeOnNewProfileData(
          SamplingTimeRange(static_cast<gtUInt64>(newRange.x()), static_cast<gtUInt64>(newRange.y())), true);
    }
//--AT:LPGPU2
}
//----------------------------- chart navigation -----------------------------------------

// add sample to the session navigation chart
SamplingTimeRange ppTimeLineView::AddNewDataToSessionNavigationChart(ppQtEventData pSampledDataPerCounter, bool shouldReplot)
{
    QCPRange returnRange(0, 0);
    SamplingTimeRange samplingRange(0, 0);

    QString name;
    double dValue;

    gtMap<int, PPSampledValuesBatch>::iterator it = pSampledDataPerCounter->begin();

    // in specific event - all counters have the same key (time)
//++AT:LPGPU2
    double dKey = static_cast<double>((*it).second.m_quantizedTime);
//--AT:LPGPU2

    it = pSampledDataPerCounter->find(m_navigationCounterID);

    if (it != pSampledDataPerCounter->end())
    {
        dValue = (*it).second.m_sampleValues[0];
    }
    else
    {
//++AT:LPGPU2
        m_isRangeChanged = false;
        return SamplingTimeRange{0, 0};
//--AT:LPGPU2
    }

    m_isRangeChanged = m_pSessionNavigationChart->AddNewData(dKey, dValue, returnRange, shouldReplot);
    samplingRange.m_fromTime = returnRange.lower;
    samplingRange.m_toTime = returnRange.upper;

    // Calculate the bottom ribbons Y coordinate. This coordinate is used for the geometry of the tooltip and bounding box:
    CalculateCurrentBottomRibbonsYCoord();

    // If the range bounding box was not painted yet, paint it:
    // The range bounding box is only painted first when the navigation chart is first displaying it:
//++AT:LPGPU2
    //if (!m_wasRangeBoundingLinePainted)
    //{
        //DrawBoundingBox();
    //}
//--AT:LPGPU2

    return samplingRange;
}

void ppTimeLineView::resizeEvent(QResizeEvent* pResizeEvent)
{
    if (pResizeEvent != nullptr)
    {
        if (m_visibleRibbonsVec.size() > 0)
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(m_visibleRibbonsVec[0] != nullptr)
            {
                QCustomPlot* pPlot = m_visibleRibbonsVec[0]->GetPlot();
                GT_IF_WITH_ASSERT(pPlot != nullptr)
                {
                    // Prints for debug. Comment before submit:
                    // DebugPrintPlotsGeomeries();

                    // Align all labels in the plots, regarding the number of digits in the tick labels:
                    FixLabelPaddingDiffs();

                    // Fix the right margin of the navigation chart manually, so that its margins are aligned with the
                    // ribbons plot margins:
                    FixNavigationChartRightMarginDiff();

                    // Prints for debug. Comment before submit:
                    // DebugPrintPlotsGeomeries();

                    // Calculate the bottom ribbons Y coordinate. This coordinate is used for the geometry of the tooltip and bounding box:
                    CalculateCurrentBottomRibbonsYCoord();

                    // Draw the range bounding box:
//++AT:LPGPU2
                    //DrawBoundingBox();
//--AT:LPGPU2
                }
            }
        }
    }

    QWidget::resizeEvent(pResizeEvent);
//++CF:LPGPU2
    ForceRangeUpdate();
//--CF:LPGPU2
}

void ppTimeLineView::SetOfflineSessionData()
{
    GT_IF_WITH_ASSERT(-1 != m_navigationCounterID)
    {
        SamplingTimeRange samplingRange{ 0, 0 };
        std::chrono::milliseconds startTimeMs;
        std::chrono::milliseconds endTimeMs;

        if (!m_isRegionOfInterestTimeline)
        {
            m_pSessionController->GetSessionTimeRange(samplingRange);
        }
        else
        {
            lpgpu2::db::RegionOfInterest regionOfInterest;
            const auto status = m_pSessionController->GetProfilerBL().GetRegionOfInterestById(m_regionOfInterestID, regionOfInterest);
            GT_IF_WITH_ASSERT(status)
            {
                gtUInt64 frameStartStartTime;
                gtUInt64 frameStartEndTime;
                gtUInt64 frameEndStartTime;
                gtUInt64 frameEndEndTime;

                m_pSessionController->GetProfilerBL().GetFrameTimeRange(regionOfInterest.m_frameStart, frameStartStartTime, frameStartEndTime);
                m_pSessionController->GetProfilerBL().GetFrameTimeRange(regionOfInterest.m_frameEnd, frameEndStartTime, frameEndEndTime);

                const std::chrono::nanoseconds startTimeNs{ frameStartStartTime };
                const std::chrono::nanoseconds endTimeNs{ frameEndEndTime };

                startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(startTimeNs);
                endTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeNs);
                
                // Look up counter samples before and after the frame start and end, and use these to determine the actual
                // region sampling range. This is needed to ensure the lack of counter data in the frame-determined range
                // doesn't cause the range to round down and not include all the frames.
                SampledValue sampleBefore;
                SampledValue sampleAfter;
                m_pSessionController->GetProfilerBL().GetNearestSampleValueBeforeTime(startTimeMs.count(), m_navigationCounterID, sampleBefore);
                m_pSessionController->GetProfilerBL().GetNearestSampleValueAfterTime(endTimeMs.count(), m_navigationCounterID, sampleAfter);

                // No sample after was found, which sometimes happens at the last frame of a session.
                // In this case, it is safe to just set some arbitrarily big range:
                if (sampleAfter.m_sampleTime == 0)
                {
                    sampleAfter.m_sampleTime = std::numeric_limits<gtInt32>::max();
                }

//++AT:LPGPU2
                samplingRange = SamplingTimeRange{ static_cast<gtInt64>(sampleBefore.m_sampleTime), static_cast<gtInt64>(sampleAfter.m_sampleTime)};
                // samplingRange = SamplingTimeRange{ static_cast<gtInt64>(nearestBeforeTime), static_cast<gtInt64>(nearestAfterTime) };
//--AT:LPGPU2
            }
        }

        QVector<double> keyVec, valueVec;
        gtMap<int, gtVector<SampledValue>> sampledDataPerCounter;

        const gtVector<int> counterIds = { m_navigationCounterID };
        m_pSessionController->GetProfilerBL().GetSampledValuesByRange(counterIds, samplingRange, sampledDataPerCounter);

        if (!sampledDataPerCounter.empty())
        {
            GT_IF_WITH_ASSERT(sampledDataPerCounter.count(m_navigationCounterID) > 0)
            {
                const auto& vecSampledValue = sampledDataPerCounter[m_navigationCounterID];

                for (const auto &it : vecSampledValue)
                {
                    keyVec << it.m_sampleTime;
                    valueVec << it.m_sampleValue;
                }

                // Perform interpolation of the counter data to gain higher navigation granularity:
                const bool bInterpolateNavigationData = true;
                double counterSampleRate;
                m_pSessionController->GetProfilerBL().GetEstimatedCounterFrequency(m_navigationCounterID, counterSampleRate);
                const unsigned int interpolationFactor = gs_interpolationPointsPerSec / static_cast<gtInt32>(counterSampleRate < 1.0 ? 1.0 : counterSampleRate);
                QVector<double> xDataUpdated;
                QVector<double> yDataUpdated;

                if (bInterpolateNavigationData)
                {
                    for (int i = 0; i < keyVec.size() - 1; i++)
                    {
                        double x1 = keyVec.at(i);
                        double x2 = keyVec.at(i+1);
                        double y1 = valueVec.at(i);
                        double y2 = valueVec.at(i+1);

                        for (unsigned int j = 0; j < interpolationFactor; j++)
                        {
                          double ix = ((1-(j/static_cast<double>(interpolationFactor))) * x1) + (j/static_cast<double>(interpolationFactor) * x2);
                          double iy = ((1-(j/static_cast<double>(interpolationFactor))) * y1) + (j/static_cast<double>(interpolationFactor) * y2);
                          if (!m_isRegionOfInterestTimeline || (ix > startTimeMs.count() && ix < endTimeMs.count()))
                          {
                            xDataUpdated.append(ix);
                            yDataUpdated.append(iy);
                          }
                        }
                    }
                }
                else
                {
                    xDataUpdated = keyVec;
                    yDataUpdated = valueVec;
                }
                // ------------------------------------------
                m_pSessionNavigationChart->SetOfflineData(xDataUpdated, yDataUpdated);
                
                // Handle the offline data changing the selected session range:
                const auto& currentRange = m_pSessionNavigationChart->GetCurrentRange();
                
                // in power profiling the range is assumed to be acceptable in integers
                const auto bReplotNeeded = true;
//++AT:LPGPU2
                UpdateRibbonsTimeRangeFromDB(SamplingTimeRange{ static_cast<qint64>(currentRange.x()), static_cast<qint64>(currentRange.y()) }, bReplotNeeded);
//--AT:LPGPU2
            }
        }
    }
}

void ppTimeLineView::OnTrackingTraceXAxis(int trackingLineXCoordinate)
{
    //++KB: LPGPU2
    // trackingLineXCoordinate is relative to the ribbon widget's dimensions, which has a margin offset
    int trackingLineXCoordinateWithLeftMarginOffset = trackingLineXCoordinate + m_trackingLineOffset;

    // Normalize the widget's x-coordinate wrt its width, then use the normalized value to get
    // the appropriate sample time
    int widgetWidth = m_APITimelineWidget->GetWidget()->rect().width();
    double normalizedX = (double) trackingLineXCoordinate / (double) widgetWidth;
    double sampleRangeWidth = m_samplingTimeRange.m_toTime - m_samplingTimeRange.m_fromTime;
    double sampleRangeValue = m_samplingTimeRange.m_fromTime + (sampleRangeWidth * normalizedX);
    OnTrackingXAxis(sampleRangeValue, trackingLineXCoordinateWithLeftMarginOffset);
    //--KB: LPGPU2
}

void ppTimeLineView::OnTrackingXAxis(double key, int trackingLineXCoordinate)
{
    GT_IF_WITH_ASSERT(nullptr != m_pSessionNavigationChart && nullptr != m_pSessionController && nullptr != m_pTrackingLine && nullptr != m_pTimeLabel)
    {
        eRangeState rangeState = m_pSessionNavigationChart->GetRangeState();

        //show tracking line and counters tool tips only if range is not being changed
        if ((m_pSessionController->GetSessionState() == ppSessionController::PP_SESSION_STATE_COMPLETED) ||
            (rangeState == RANGE_STATE_MID_MID) ||
            (rangeState == RANGE_STATE_START_MID))
        {
            if ((key < m_samplingTimeRange.m_fromTime) || (key > m_samplingTimeRange.m_toTime))// if the key is out of current active range set it to -1
            {
                key = -1;
            }

            m_shouldDrawTrackingLine = (key != -1);
            m_trackingLineXCoordinate = trackingLineXCoordinate;

            for (auto pPlot : m_visibleRibbonsVec)
            {
                GT_IF_WITH_ASSERT(nullptr != pPlot)
                {
                    // By default the tooltip is empty:
                    QString currentPlotTooltip;

                    if (key >= 0)
                    {
                        m_trackLineKey = key;
                        pPlot->SetInfoTableBySpecificTimePoint(key);

                        // Check if the current tooltip should be displayed:
                        bool shouldTooltipBeDisplayed = true;

                        RibbonButtons buttons = m_ribbonButtonsMap[pPlot->GetPlot()];

                        if (buttons.m_pDownButton != nullptr)
                        {
                            if (buttons.m_pDownButton->isVisible())
                            {
                                // If the buttons are visible, hide the tooltip near its X coordinate:
                                if ((trackingLineXCoordinate > buttons.m_pDownButton->geometry().left() - PP_RIBBON_BUTTON_MARGIN * 2) &&
                                    (trackingLineXCoordinate < buttons.m_pDownButton->geometry().right() + PP_RIBBON_BUTTON_MARGIN * 2))
                                {
                                    shouldTooltipBeDisplayed = false;
                                }
                            }
                        }

                        if (shouldTooltipBeDisplayed)
                        {
                            QString valueStr;
                            pPlot->GetInfoTableSelectedGraphValue(currentPlotTooltip, valueStr);

                            if (!valueStr.isEmpty())
                            {
                                // Build the tooltip:
                                currentPlotTooltip.prepend(AF_STR_HtmlBoldTagStartA);
                                currentPlotTooltip.append(AF_STR_HtmlBoldTagEndA);
                                currentPlotTooltip += AF_STR_Colon;
                                currentPlotTooltip += AF_STR_HtmlNBSP;
                                currentPlotTooltip += valueStr;
                            }
                        }
                    }

                    // Set the tooltip:
                    pPlot->SetPlotToolTip(currentPlotTooltip, trackingLineXCoordinate);
                }
            }

            UpdateTrackingLine();

        }
        else
        {
            m_shouldDrawTrackingLine = false;

            for (auto pPlot : m_visibleRibbonsVec)
            {
                GT_IF_WITH_ASSERT(nullptr != pPlot)
                {
                    // passing empty string hides tooltip:
                    pPlot->SetPlotToolTip(QString(), trackingLineXCoordinate);
                }
            }

            UpdateTrackingLine();
        }
    }
}

void ppTimeLineView::AddStretchItemToGrid()
{
    if (m_pGridStretchWidget != nullptr)
    {
        // Remove the stretch widget if it exists:
        m_pBottomVLayout->removeWidget(m_pGridStretchWidget);
    }
    else
    {
        m_pGridStretchWidget = new QWidget;
    }

    // Add the widget in the last row of the grid layout:
    m_pBottomVLayout->addStretch();
}

void ppTimeLineView::UpdateTrackingLine()
{
    if (m_shouldDrawTrackingLine)
    {
        int navigationChartBottom = m_pSessionNavigationChart->GetActiveRangeXAxis()->axisRect()->bottom() + abs(2 * PP_NAV_TICK_LABELS_PADDING);
        int trackingLineHeight = height() - navigationChartBottom;

        // If the bottom ribbons Y coordinate is calculated, cut the tracking line at this point:
        trackingLineHeight = (m_bottomRibbonYCoord > 0) ? (m_bottomRibbonYCoord - navigationChartBottom) : height() - navigationChartBottom;

        m_pTrackingLine->setGeometry(m_trackingLineXCoordinate, navigationChartBottom, TRACK_LINE_WIDTH, trackingLineHeight);
        m_pTrackingLine->setVisible(true);
        QString labelText(PP_StrTimelineTimeLabelPrefix);
        labelText.prepend(AF_STR_HtmlBoldTagStartA);
        labelText.append(AF_STR_HtmlBoldTagEndA);
        labelText += MsecToTimeString(m_trackLineKey, true);
        m_pTimeLabel->setText(labelText);
        m_pTimeLabel->adjustSize();
        int nTimeLabelX = m_trackingLineXCoordinate;
        int timeLabelWidth = m_pTimeLabel->width();
        int timeLabelHeight = m_pTimeLabel->height();

        if ((m_trackingLineXCoordinate + timeLabelWidth) > m_pSessionNavigationChart->rect().right())
        {
            nTimeLabelX = m_trackingLineXCoordinate - timeLabelWidth + 1;
        }

        m_pTimeLabel->setGeometry(nTimeLabelX, navigationChartBottom, timeLabelWidth, timeLabelHeight);
        m_pTimeLabel->setVisible(true);
    }
    else
    {
        m_pTrackingLine->setVisible(false);
        m_pTimeLabel->setVisible(false);
    }

    m_pTrackingLine->update();
    m_pTimeLabel->update();
}

void ppTimeLineView::EnableRibbonsInfoTabelAndRemoveCounters(bool enable)
{
    foreach (ppMultiLinePlot* pPlot, m_allRibbonsVec)
    {
        if (pPlot != nullptr)
        {
            pPlot->EnableInfoTableAddRemoveCounters(enable);
        }
    }
}

int ppTimeLineView::CalculateInfoTableFixedWidth()
{
    static int sFixedWidth = 0;
    static bool sWasWidthInitialized = false;

    if (!sWasWidthInitialized)
    {
        // Add the width for the widest counter name:
        sFixedWidth = QFontMetrics(font()).boundingRect(PP_STR_LongestCounterName).width();

        // Add the icon column size:
        sFixedWidth += PP_INFO_TABLE_ICON_COL_WIDTH;

        // Add table margins space:
        sFixedWidth += PP_INFO_TABLE_X_MARGIN;

        int messageW = QFontMetrics(font()).boundingRect(AC_STR_CounterSelectionString).width();
        messageW += PP_INFO_TABLE_X_MARGIN;

        sFixedWidth = qMax(messageW, sFixedWidth);
    }

    return sFixedWidth;
}

void ppTimeLineView::FixLabelPaddingDiffs()
{
    // The width of the widest label found, used to calculate offset:
    int maxLabelWidth = 0;
    // The final adjusted left position of any/all of the graph axes:
    int axesLeftPosition = 0;

    foreach (ppMultiLinePlot* pPlotPtr, m_visibleRibbonsVec)
    {
        if (pPlotPtr != nullptr)
        {
            QCustomPlot* pPlot = pPlotPtr->GetPlot();
            GT_IF_WITH_ASSERT(pPlot != nullptr)
            {
                if (pPlotPtr->IsShown())
                {
                    int precision = pPlot->yAxis->numberPrecision();
                    // Also include the width of the y label (which is actually its height as it is rotated 90 deg)
                    QString yLabel = pPlot->yAxis->label();
                    QFont font = pPlot->yAxis->labelFont();
                    int yLabelWidth = QFontMetrics(font).boundingRect(QRect{ 0,0,0,0 }, 0, yLabel).height();
                    double lower = pPlot->yAxis->range().lower;
                    double upper = pPlot->yAxis->range().upper;
                    QString rangeStr = QString::number(lower, 'g', precision);
                    int tickLabelWidthLower = QFontMetrics(font).boundingRect(rangeStr).width() + yLabelWidth;
                    rangeStr = QString::number(upper, 'g', precision);
                    int tickLabelWidthUpper = QFontMetrics(font).boundingRect(rangeStr).width() + yLabelWidth;
                    maxLabelWidth = qMax(tickLabelWidthLower, maxLabelWidth);
                    maxLabelWidth = qMax(tickLabelWidthUpper, maxLabelWidth);
                }
            }
        }
    }

    foreach (ppMultiLinePlot* pPlotPtr, m_visibleRibbonsVec)
    {
        if (pPlotPtr != nullptr)
        {
            QCustomPlot* pPlot = pPlotPtr->GetPlot();
            GT_IF_WITH_ASSERT(pPlot != nullptr)
            {
                if (pPlotPtr->IsShown())
                {
                    int precision = pPlot->yAxis->numberPrecision();
                    QFont font = pPlot->yAxis->labelFont();
                    double lower = pPlot->yAxis->range().lower;
                    double upper = pPlot->yAxis->range().upper;
                    QString yLabel = pPlot->yAxis->label();
                    int yLabelWidth = QFontMetrics(font).boundingRect(QRect{ 0, 0, 0, 0 }, 0, yLabel).height();
                    QString rangeStr = QString::number(lower, 'g', precision);
                    int tickLabelWidthLower = QFontMetrics(font).boundingRect(rangeStr).width();
                    rangeStr = QString::number(upper, 'g', precision);
                    int tickLabelWidthUpper = QFontMetrics(font).boundingRect(rangeStr).width();
                    int currentLabelMaxWidth = qMax(tickLabelWidthLower, tickLabelWidthUpper);

                    // 'Fix' the location of the tick labels, using padding:
                    if (currentLabelMaxWidth < maxLabelWidth)
                    {
                        pPlot->yAxis->setLabelPadding(maxLabelWidth - currentLabelMaxWidth - yLabelWidth);
                    }
                    axesLeftPosition = pPlot->axisRect(0)->rect().left();
                }
            }
        }
    }

    // Apply the calculated padding to the timeline widgets, and store it to use to calculate
    // the tracking line offset:
    int leftPaddingWidth = axesLeftPosition - PP_TIMELINE_PLOT_MARGINS.right();
    m_trackingLineOffset = axesLeftPosition;

    if (m_HelpTimelineWidget != nullptr)
    {
        m_HelpTimelineWidget->GetLabel()->setFixedWidth(leftPaddingWidth);
    }
    m_APITimelineWidget->GetLabel()->setFixedWidth(leftPaddingWidth);

}

void ppTimeLineView::FixNavigationChartRightMarginDiff()
{
    if ((m_visibleRibbonsVec.size() > 1) && (m_visibleRibbonsVec[0] != nullptr) && (m_visibleRibbonsVec[1] != nullptr) && (m_pSessionNavigationChart != nullptr))
    {
        QCustomPlot* pPlot = m_visibleRibbonsVec[0]->GetPlot();

        if (!m_visibleRibbonsVec[0]->IsShown())
        {
            pPlot = m_visibleRibbonsVec[1]->GetPlot();
        }

        GT_IF_WITH_ASSERT(pPlot != nullptr)
        {
            // Find the length of the plot:
            int plotLength = pPlot->xAxis->axisRect()->right() - pPlot->xAxis->axisRect()->left();
            int navLength = m_pSessionNavigationChart->xAxis->axisRect()->right() - pPlot->xAxis->axisRect()->left();

            // If there is a difference in both axes length, fix it by changing the margin for the navigation chart:
            if (navLength != plotLength)
            {
                // Handle only small length diffs (other diffs occur before the windows if fully sized):
                int lengthDiff = plotLength - navLength;

                if (qAbs(lengthDiff) < 50)
                {
                    int rightMargin = m_pSessionNavigationChart->xAxis->axisRect()->margins().right();
                    rightMargin -= (plotLength - navLength);

                    QMargins margins = m_pSessionNavigationChart->xAxis->axisRect()->margins();
                    margins.setRight(acScalePixelSizeToDisplayDPI(rightMargin));
                    m_pSessionNavigationChart->xAxis->axisRect()->setMargins(margins);
                    m_pSessionNavigationChart->yAxis->axisRect()->setMargins(margins);
                }
            }
        }
    }
}

void ppTimeLineView::DebugPrintPlotsGeomeries()
{
    if (m_visibleRibbonsVec.size() > 0)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(m_visibleRibbonsVec[0] != nullptr)
        {
            QCustomPlot* pPlot = m_visibleRibbonsVec[0]->GetPlot();
            GT_IF_WITH_ASSERT(pPlot != nullptr)
            {
                int plotleft = pPlot->xAxis->axisRect()->left();
                int plottop = pPlot->xAxis->axisRect()->top();
                int plotright = pPlot->xAxis->axisRect()->right();
                int plotbottom = pPlot->xAxis->axisRect()->bottom();

                int plotleftM = pPlot->xAxis->axisRect()->margins().left();
                int plottopM = pPlot->xAxis->axisRect()->margins().top();
                int plotrightM = pPlot->xAxis->axisRect()->margins().right();
                int plotbottomM = pPlot->xAxis->axisRect()->margins().bottom();

                QString infoString = QString("Plot XaxisRect: (%1, %2, %3, %4). Margins: (%5, %6, %7, %8)\n").arg(plotleft).arg(plottop).arg(plotright).arg(plotbottom).arg(plotleftM).arg(plottopM).arg(plotrightM).arg(plotbottomM);
                afApplicationCommands::instance()->AddStringToInformationView(infoString);

                plotleft = pPlot->yAxis->axisRect()->left();
                plottop = pPlot->yAxis->axisRect()->top();
                plotright = pPlot->yAxis->axisRect()->right();
                plotbottom = pPlot->yAxis->axisRect()->bottom();

                plotleftM = pPlot->yAxis->axisRect()->margins().left();
                plottopM = pPlot->yAxis->axisRect()->margins().top();
                plotrightM = pPlot->yAxis->axisRect()->margins().right();
                plotbottomM = pPlot->yAxis->axisRect()->margins().bottom();

                infoString = QString("Plot YaxisRect: (%1, %2, %3, %4). Margins: (%5, %6, %7, %8)\n").arg(plotleft).arg(plottop).arg(plotright).arg(plotbottom).arg(plotleftM).arg(plottopM).arg(plotrightM).arg(plotbottomM);
                afApplicationCommands::instance()->AddStringToInformationView(infoString);

                plotleft = pPlot->geometry().left();
                plottop = pPlot->geometry().top();
                plotright = pPlot->geometry().right();
                plotbottom = pPlot->geometry().bottom();

                infoString = QString("Plot geometry: (%1, %2, %3, %4). \n").arg(plotleft).arg(plottop).arg(plotright).arg(plotbottom);
                afApplicationCommands::instance()->AddStringToInformationView(infoString);

                int left = m_pSessionNavigationChart->xAxis->axisRect()->left();
                int top = m_pSessionNavigationChart->xAxis->axisRect()->top();
                int right = m_pSessionNavigationChart->xAxis->axisRect()->right();
                int bottom = m_pSessionNavigationChart->xAxis->axisRect()->bottom();

                int leftM = m_pSessionNavigationChart->xAxis->axisRect()->margins().left();
                int topM = m_pSessionNavigationChart->xAxis->axisRect()->margins().top();
                int rightM = m_pSessionNavigationChart->xAxis->axisRect()->margins().right();
                int bottomM = m_pSessionNavigationChart->xAxis->axisRect()->margins().bottom();

                infoString = QString("Nav XaxisRect: (%1, %2, %3, %4). Margins: (%5, %6, %7, %8)\n").arg(left).arg(top).arg(right).arg(bottom).arg(leftM).arg(topM).arg(rightM).arg(bottomM);
                afApplicationCommands::instance()->AddStringToInformationView(infoString);


                left = m_pSessionNavigationChart->yAxis->axisRect()->left();
                top = m_pSessionNavigationChart->yAxis->axisRect()->top();
                right = m_pSessionNavigationChart->yAxis->axisRect()->right();
                bottom = m_pSessionNavigationChart->yAxis->axisRect()->bottom();

                leftM = m_pSessionNavigationChart->yAxis->axisRect()->margins().left();
                topM = m_pSessionNavigationChart->yAxis->axisRect()->margins().top();
                rightM = m_pSessionNavigationChart->yAxis->axisRect()->margins().right();
                bottomM = m_pSessionNavigationChart->yAxis->axisRect()->margins().bottom();

                infoString = QString("Nav YaxisRect: (%1, %2, %3, %4). Margins: (%5, %6, %7, %8)\n").arg(left).arg(top).arg(right).arg(bottom).arg(leftM).arg(topM).arg(rightM).arg(bottomM);
                afApplicationCommands::instance()->AddStringToInformationView(infoString);

                left = m_pSessionNavigationChart->geometry().left();
                top = m_pSessionNavigationChart->geometry().top();
                right = m_pSessionNavigationChart->geometry().right();
                bottom = m_pSessionNavigationChart->geometry().bottom();

                infoString = QString("Nav geometry: (%1, %2, %3, %4). \n").arg(left).arg(top).arg(right).arg(bottom);
                afApplicationCommands::instance()->AddStringToInformationView(infoString);
            }
        }
    }
}

void ppTimeLineView::DrawBoundingBox()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pBoundingBoxRightLine != nullptr) && (m_pBoundingBoxLeftLine != nullptr) && (m_pSessionNavigationChart != nullptr))
    {
        if (m_pSessionNavigationChart->ShouldDrawBoundingLine())
        {
            // Find the left & right positions of the lines:
            int navLeft = m_pSessionNavigationChart->GetActiveRangeXAxis()->axisRect()->left();
            int navRight = m_pSessionNavigationChart->GetActiveRangeXAxis()->axisRect()->right();

            // Find the top position of the lines (bottom of navigation chart:
            int navBottom = m_pSessionNavigationChart->GetActiveRangeXAxis()->axisRect()->bottom() + abs(2 * PP_NAV_TICK_LABELS_PADDING);

            // Build points for the geometry of the lines:
            QPoint leftTop(navLeft - 1, navBottom);
            QPoint leftBottom(navLeft, m_bottomRibbonYCoord);
            QPoint rightTop(navRight, navBottom);
            QPoint rightBottom(navRight + 1, m_bottomRibbonYCoord + 1);

            m_pBoundingBoxLeftLine->setGeometry(QRect(leftTop, leftBottom));
            m_pBoundingBoxLeftLine->setVisible(true);


            m_pBoundingBoxRightLine->setGeometry(QRect(rightTop, rightBottom));
            m_pBoundingBoxRightLine->setVisible(true);

            if (m_bottomRibbonYCoord > 0)
            {
                m_pBoundingBoxBottomLine->setGeometry(QRect(leftBottom, rightBottom));
                m_pBoundingBoxBottomLine->setVisible(true);
            }

            m_wasRangeBoundingLinePainted = true;
        }
    }
}

void ppTimeLineView::showEvent(QShowEvent* pEvent)
{
    // Call the base class implementation:
    QWidget::showEvent(pEvent);

    if ((pEvent != nullptr) && (!m_isVisibilityUpdated))
    {
        foreach (ppMultiLinePlot* pPlot, m_visibleRibbonsVec)
        {
            if (pPlot != nullptr)
            {
                pPlot->UpdateVisibility();
            }
        }

        // Set the flag indicating that visibility was updated:
        m_isVisibilityUpdated = true;
    }

    // Calculate the bottom ribbons Y coordinate. This coordinate is used for the geometry of the tooltip and bounding box:
    CalculateCurrentBottomRibbonsYCoord();

    // Draw the range bounding box:
//++AT:LPGPU2
    ForceRangeUpdate();
    //DrawBoundingBox();
//--AT:LPGPU2
}

void ppTimeLineView::CalculateCurrentBottomRibbonsYCoord()
{
    // Reset the coordinate:
    m_bottomRibbonYCoord = -1;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionNavigationChart != nullptr)
    {
        // Find the top position of the lines (bottom of navigation chart:
//++CF:LPGPU2 - add timeline container height
        int navBottom = m_pSessionNavigationChart->GetActiveRangeXAxis()->axisRect()->bottom()
                        + abs(2 * PP_NAV_TICK_LABELS_PADDING) + m_pTimelineContainer->height();
//--CF:LPGPU2

        // Add bottom padding if there is a bounding box:
        int bottomPadding = (m_pSessionNavigationChart->ShouldDrawBoundingLine()) ? abs(2 * PP_NAV_TICK_LABELS_PADDING) : 6;

        foreach (ppMultiLinePlot* pPlot, m_visibleRibbonsVec)
        {
            if (pPlot != nullptr)
            {
                QRect currentRibbonRect = pPlot->GetPlot()->geometry();
                int currentBottom = currentRibbonRect.bottom() + navBottom + bottomPadding;
                m_bottomRibbonYCoord = std::max(m_bottomRibbonYCoord, currentBottom);
            }
        }

//++KB: LPGPU2
//++AT:LPGPU2
      if (m_APITimelineWidget != nullptr)
      {
          int currentRibbonBottom =  m_APITimelineWidget->GetRibbonBottom() + navBottom + bottomPadding;
          m_bottomRibbonYCoord = std::max(m_bottomRibbonYCoord, currentRibbonBottom);
      }
      if (m_HelpTimelineWidget!= nullptr)
      {
          int currentRibbonBottom =  m_HelpTimelineWidget->GetRibbonBottom() + navBottom + bottomPadding;
          m_bottomRibbonYCoord = std::max(m_bottomRibbonYCoord, currentRibbonBottom);
      }
//--AT:LPGPU2
//--KB: LPGPU2
    }
}

void ppTimeLineView::OnPlotDropped(acCustomPlot* pDroppedIntoPlot, acCustomPlot* pDraggedPlot)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pDroppedIntoPlot != nullptr) && (pDraggedPlot != nullptr) && (m_pBottomVLayout != nullptr))
    {
        QWidget* pWidget1 = pDraggedPlot;
        QWidget* pWidget2 = pDroppedIntoPlot;

        QString graphName1 = pDraggedPlot->yAxis->label();
        QString graphName2 = pDroppedIntoPlot->yAxis->label();

        // The indices of the dragged and dropped plots in the ribbons vector:
        int widgetIndex1 = -1;
        int widgetIndex2 = -1;

        // Look for both plots in the vector of ribbons:
        for (int i = 0; i < m_visibleRibbonsVec.size(); i++)
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(m_visibleRibbonsVec[i] != nullptr)
            {
                if (m_visibleRibbonsVec[i]->GetPlot() == pWidget1)
                {
                    widgetIndex1 = i;
                }

                if (m_visibleRibbonsVec[i]->GetPlot() == pWidget2)
                {
                    widgetIndex2 = i;
                }
            }
        }

        // Drag plot from index 1 into index 2:
        DragPlotIntoOther(widgetIndex1, widgetIndex2);

    }
}

void ppTimeLineView::InitGraphByCategory(ppDataUtils::GraphViewCategoryType graphCategory)
{
    // Find the category, graph name and units according to the graph category:
    AMDTPwrCategory category = AMDT_PWR_CATEGORY_CNT;
    QString graphName;
    QString unitsStr;
    acMultiLinePlot::GraphValuesType valueType = acMultiLinePlot::GRAPHVALUESTYPE_DOUBLE;

    switch (graphCategory)
    {
        case ppDataUtils::TIMELINE_POWER_DGPU:
        {
            category = AMDT_PWR_CATEGORY_POWER;
            graphName = PP_STR_Counter_Power_DGPU;
            unitsStr = PP_STR_UnitsPostfixWatt;
            break;
        }

        case ppDataUtils::TIMELINE_FREQUENCY:
        {
            category = AMDT_PWR_CATEGORY_FREQUENCY;
            graphName = PP_STR_FrequencyCategoryName;
            unitsStr = PP_STR_UnitsPostfixMHz;
            valueType = acMultiLinePlot::GRAPHVALUESTYPE_INT;
            break;
        }

        case ppDataUtils::TIMELINE_TEMPERATURE:
        {
            category = AMDT_PWR_CATEGORY_TEMPERATURE;
            graphName = PP_StrTimelineScaledTemperatureGraphName;
            unitsStr = PP_STR_UnitsPostfixCelsius;
            break;
        }

        case ppDataUtils::TIMELINE_VOLTAGE:
        {
            category = AMDT_PWR_CATEGORY_VOLTAGE;
            graphName = PP_StrTimelineVoltageGraphName;
            unitsStr = PP_STR_UnitsPostfixVolt;
            break;
        }

        case ppDataUtils::TIMELINE_CURRENT:
        {
            category = AMDT_PWR_CATEGORY_CURRENT;
            graphName = PP_StrTimelineCurrentGraphName;
            unitsStr = PP_STR_UnitsPostfixmA;
            break;
        }

        case ppDataUtils::TIMELINE_CPU_CORE_PSTATE:
        {
            category = AMDT_PWR_CATEGORY_DVFS;
            graphName = PP_StrTimelineCPUCoreStateGraphName;
            unitsStr = AF_STR_EmptyA;
            valueType = acMultiLinePlot::GRAPHVALUESTYPE_INT;
            break;
        }

        case ppDataUtils::TIMELINE_CPU_CORE_CSTATE:
        {
            category = AMDT_PWR_CATEGORY_DVFS;
            graphName = PP_StrTimelineCStateGraphName;
            unitsStr = PP_STR_UnitsPostfixPercentage;
            break;
        }

        // mgb:ss add counter support
        case ppDataUtils::TIMELINE_COUNTER:
        {
//++AT:LPGPU2
            category = AMDT_PWR_CATEGORY_COUNT;
//--AT:LPGPU2
            graphName = PP_StrTimelineCounterGraphName;
            unitsStr = AF_STR_EmptyA;
//++AT:LPGPU2
            valueType = acMultiLinePlot::GRAPHVALUESTYPE_DOUBLE;
//--AT:LPGPU2
            break;
        }

//++AT:LPGPU2
        case ppDataUtils::TIMELINE_POWER:
        {
          graphName = PP_STR_Counter_Power_DGPU;
          category = AMDT_PWR_CATEGORY_POWER;
          unitsStr = PP_STR_UnitsPostfixWatt;
          break;
        }
//--AT:LPGPU2

        default:
        {
            GT_ASSERT_EX(false, L"Should not get here");
            break;
        }
    }

    // Create the new plot:
    auto *pNewPlot = new (std::nothrow) LPGPU2ppMultiLinePlot(m_pSessionController);
    pNewPlot->Initialise();

    QString yAxisTitle = QString(AF_STR_QStringAppend).arg(graphName).arg(unitsStr);
    pNewPlot->InitPlot(AF_STR_EmptyA, yAxisTitle, category, graphCategory, valueType, unitsStr);

    // Enable drag & drop:
    pNewPlot->GetPlot()->EnableDragAndDrop(false);

    // Initialize the plot with the currently selected counters:
    pNewPlot->InitPlotWithSelectedCounters();

    // Set the ribbon shown, and do not update the visibility (we're still creating the view, the first update is in 'onShowEvent'):
    pNewPlot->SetShown(true, false);

    // Add the plot to the ribbons vector:
    m_visibleRibbonsVec << pNewPlot;
    m_allRibbonsVec << pNewPlot;
}

//++AT:LPGPU2
/// @brief Initialise graph by category and device ID
/// @param[in] graphCategory Category of graph to initialise
/// @param[in] deviceId Parent device ID of the counters to show in this graph
/// @param[in] deviceName Parent device name of the counters to show in this
///            graph
/// @note This method was copied from the existing AMD method to maintain
///       as much retrocompatibility as possible
void ppTimeLineView::InitGraphByCategoryAndDeviceId(
    ppDataUtils::GraphViewCategoryType graphCategory, int deviceId,
    gtString deviceName)
{
  // Find the category, graph name and units according to the graph category:
  AMDTPwrCategory category = AMDT_PWR_CATEGORY_CNT;
  QString graphName;
  QString unitsStr;
  acMultiLinePlot::GraphValuesType valueType =
    acMultiLinePlot::GRAPHVALUESTYPE_DOUBLE;

  switch (graphCategory)
  {
    case ppDataUtils::TIMELINE_POWER_DGPU:
    {
      category = AMDT_PWR_CATEGORY_POWER;
      graphName = PP_STR_Counter_Power_DGPU;
      unitsStr = PP_STR_UnitsPostfixWatt;
      break;
    }

    case ppDataUtils::TIMELINE_FREQUENCY:
    {
      category = AMDT_PWR_CATEGORY_FREQUENCY;
      graphName = PP_STR_FrequencyCategoryName;
      unitsStr = PP_STR_UnitsPostfixMHz;
      valueType = acMultiLinePlot::GRAPHVALUESTYPE_INT;
      break;
    }

    case ppDataUtils::TIMELINE_TEMPERATURE:
    {
        category = AMDT_PWR_CATEGORY_TEMPERATURE;
        graphName = PP_StrTimelineScaledTemperatureGraphName;
        unitsStr = PP_STR_UnitsPostfixCelsius;
        break;
    }

    case ppDataUtils::TIMELINE_VOLTAGE:
    {
      category = AMDT_PWR_CATEGORY_VOLTAGE;
      graphName = PP_StrTimelineVoltageGraphName;
      unitsStr = PP_STR_UnitsPostfixVolt;
      break;
    }

    case ppDataUtils::TIMELINE_CURRENT:
    {
        category = AMDT_PWR_CATEGORY_CURRENT;
        graphName = PP_StrTimelineCurrentGraphName;
        unitsStr = PP_STR_UnitsPostfixmA;
        break;
    }

    case ppDataUtils::TIMELINE_CPU_CORE_PSTATE:
    {
      category = AMDT_PWR_CATEGORY_DVFS;
      graphName = PP_StrTimelineCPUCoreStateGraphName;
      unitsStr = AF_STR_EmptyA;
      valueType = acMultiLinePlot::GRAPHVALUESTYPE_INT;
      break;
    }

    case ppDataUtils::TIMELINE_CPU_CORE_CSTATE:
    {
      category = AMDT_PWR_CATEGORY_DVFS;
      graphName = PP_StrTimelineCStateGraphName;
      unitsStr = PP_STR_UnitsPostfixPercentage;
      break;
    }

    // mgb:ss add counter support
    case ppDataUtils::TIMELINE_COUNTER:
    {
      category = AMDT_PWR_CATEGORY_CNT;
      graphName = PP_StrTimelineCounterGraphName;
      unitsStr = AF_STR_EmptyA;
      valueType = acMultiLinePlot::GRAPHVALUESTYPE_INT;
      break;
    }

    case ppDataUtils::TIMELINE_POWER:
    {
      graphName = PP_STR_Counter_Power_LPGPU2;
      category = AMDT_PWR_CATEGORY_POWER;
      unitsStr = PP_STR_UnitsPostfixWatt;
      break;
    }

    default:
    {
      GT_ASSERT_EX(false, L"Should not get here");
      break;
    }
  }

  auto yAxisTitle =
    QString( PP_STR_FormatGraphYaxis )
      .arg(acGTStringToQString(deviceName))
      .arg(graphName)
      .arg(unitsStr);

  // Create the new plot:
  auto *pNewPlot =
    new (std::nothrow) LPGPU2ppMultiLinePlot(m_pSessionController);
  pNewPlot->Initialise();

  pNewPlot->InitPlot(AF_STR_EmptyA, yAxisTitle, category, graphCategory,
      valueType, unitsStr, deviceId);

  // Enable drag & drop:
  pNewPlot->GetPlot()->EnableDragAndDrop(false);

  // Initialize the plot with the currently selected counters:
  pNewPlot->InitPlotWithSelectedCounters();

  // Set the ribbon shown, and do not update the visibility
  // (we're still creating the view, the first update is in 'onShowEvent'):
  pNewPlot->SetShown(true, false);

  // Add the plot to the ribbons vector:
  m_visibleRibbonsVec << pNewPlot;
  m_allRibbonsVec << pNewPlot;
}
//--AT:LPGPU2

void ppTimeLineView::AddRibbonsToGrid()
{
    // Clear the bottom layout:
    GT_IF_WITH_ASSERT(m_pBottomVLayout != nullptr)
    {
        // Iterate count() times, and takeAt each time (clear the layout):
        for (int i = m_pBottomVLayout->count(); i >= 0; i--)
        {
            if (m_pBottomVLayout->takeAt(0) == nullptr)
            {
                break;
            }
        }

        // Make sure that the layout is empty:
        GT_ASSERT(m_pBottomVLayout->children().isEmpty());

        // Re-build the layout:
        foreach (ppMultiLinePlot* pPlot, m_visibleRibbonsVec)
        {
            AddPlotToGrid(pPlot);
        }

        m_pBottomVLayout->addStretch();
        m_pBottomVLayout->update();

        ReplotAllGraphs();

        // Initialize the ribbons count:
        ppRibbonButton::SetRibbonCount(m_visibleRibbonsVec.size());
    }

}

void ppTimeLineView::AddNavigationCounterSelectionWidgets(QWidget* pContainingWidget)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionController != nullptr) && (pContainingWidget != nullptr))
    {
        QVBoxLayout* pVLayout = new QVBoxLayout;
        m_pNavigationCounterSelectionComboBox = new QComboBox;
        m_pNavigationCounterSelectionComboBox->setEditable(false);

        // Create the navigation label:
        m_pNavigationCounterSelectionLabel = new QLabel;
        m_pNavigationCounterSelectionLabel->setWordWrap(true);

        // Add the combo and label to the layout:
        pVLayout->addWidget(m_pNavigationCounterSelectionLabel, 0, Qt::AlignTop);
        pVLayout->addWidget(m_pNavigationCounterSelectionComboBox, 0, Qt::AlignTop);

        // Connect the combo and labels to slots:
        bool rc = connect(m_pNavigationCounterSelectionComboBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(OnNavigationCounterSelectionChange(const QString&)));
        GT_ASSERT(rc);

        rc = connect(m_pNavigationCounterSelectionLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OnNavigationCounterLinkClicked(const QString&)));
        GT_ASSERT(rc);

        m_pNavigationCounterSelectionComboBox->setVisible(false);

        pContainingWidget->setLayout(pVLayout);

        // Get the APU counter id (will be the default counter, if it exists in the session counters list):
        int apuCounterId = m_pSessionController->GetAPUCounterID();

        // Build a string list for the combo box, and find the default selected counter (APU ID if exists):
        QStringList countersStringsList;
        QString selectedCounterText;

        // Get the list of counter IDs for this session:
        gtVector<int> sessionCounterIds;

        for (int i = (int)ppDataUtils::TIMELINE_POWER; i <= (int)ppDataUtils::TIMELINE_COUNTER; i++)
        {
            // Add the counter IDs for this type:
            ppDataUtils::GetRelevantCounterIdsByGraphType(sessionCounterIds, (ppDataUtils::GraphViewCategoryType)i, m_pSessionController);

            for (int j = 0; j < (int)sessionCounterIds.size(); j++)
            {
                // Get the current counter name:
                QString counterName = m_pSessionController->GetCounterNameById(sessionCounterIds[j]);

                if (sessionCounterIds[j] == apuCounterId)
                {
                    selectedCounterText = counterName;
                }

//++AT:LPGPU2
                // Add the current counter to the list:
                m_pNavigationCounterSelectionComboBox->addItem(counterName, QVariant::fromValue(sessionCounterIds[j]));
                countersStringsList.push_back(counterName);
            }
        }
//--AT:LPGPU2

        // Set the current text on the combo:
        if (selectedCounterText.isEmpty() && !countersStringsList.isEmpty())
        {
            selectedCounterText = countersStringsList[0];            
        }

        m_pNavigationCounterSelectionComboBox->setCurrentText(selectedCounterText);
        OnNavigationCounterSelectionChange(selectedCounterText);
    }
}

void ppTimeLineView::OnNavigationCounterSelectionChange(const QString& counterName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pNavigationCounterSelectionLabel != nullptr) && (m_pNavigationCounterSelectionComboBox != nullptr))
    {
//++AT:LPGPU2
        bool checkCastFromVariant = false;
        auto counterId = m_pNavigationCounterSelectionComboBox->currentData().toInt(&checkCastFromVariant);
        GT_ASSERT(checkCastFromVariant);
//--AT:LPGPU2

        QString labelText = QString(PP_StrTimelineNavigationCounterSelectionLabel).arg(counterName);
        m_pNavigationCounterSelectionLabel->setText(labelText);

        // Get the navigation counter ID:
//++AT:LPGPU2
        m_navigationCounterID = counterId;
//--AT:LPGPU2
        GT_ASSERT(m_navigationCounterID >= 0);

        // Update the navigation chart counter:
        UpdateNavigationCounter();

        // Hide the navigation combo box:
        m_pNavigationCounterSelectionComboBox->hide();
    }
}

void ppTimeLineView::OnNavigationCounterLinkClicked(const QString& link)
{
    GT_UNREFERENCED_PARAMETER(link);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pNavigationCounterSelectionComboBox != nullptr)
    {
        // Show the counter selection combo box:
        m_pNavigationCounterSelectionComboBox->setVisible(true);
        m_pNavigationCounterSelectionComboBox->setFocus();
    }
}

void ppTimeLineView::UpdateNavigationCounter()
{
    // Sanity check (navigation counter should be initialized at this point):
    GT_IF_WITH_ASSERT((m_navigationCounterID != -1) && (m_pSessionController != nullptr) && (m_pSessionNavigationChart != nullptr))
    {
        int navigationChartYmin = 0;
        int navigationChartYmax = 41;


        const AMDTPwrCounterDesc* pNavigationCounterDesc = m_pSessionController->GetCounterDescriptor(m_navigationCounterID);
        GT_IF_WITH_ASSERT(pNavigationCounterDesc != nullptr)
        {
            navigationChartYmin = navigationChartYmax = -1000;
            navigationChartYmin = pNavigationCounterDesc->m_minValue;
            navigationChartYmax = pNavigationCounterDesc->m_maxValue;

            // Set the Y axis value (range + label):
            m_pSessionNavigationChart->SetYAxisValues(navigationChartYmin, navigationChartYmax, pNavigationCounterDesc->m_name);

            if (m_pSessionController->GetSessionState() == ppSessionController::PP_SESSION_STATE_COMPLETED)
            {
                // Set the navigation chart offline data:
                SetOfflineSessionData();
            }
//++AT:LPGPU2
            else
            {
                m_pSessionNavigationChart->ResetPlot();
            }
//--AT:LPGPU2

            // Replot the chart:
            m_pSessionNavigationChart->replot();

        }
    }
}

void ppTimeLineView::OnRibbonUp()
{
    // Down cast the sender to a ppRibbonButton:
    ppRibbonButton* pPushButton = qobject_cast<ppRibbonButton*>(sender());

    // Sanity check:
    GT_IF_WITH_ASSERT(pPushButton != nullptr)
    {
        // Get the ribbon index:
        int ribbonIndex = pPushButton->PlotIndex();

        // Perform the drag operation:
        DragPlotIntoOther(ribbonIndex, ribbonIndex - 1);

        // Make sure that no plot is highlighted (highlight is done when the user hovers the up / down arrows):
        ClearPlotsBackgrounds();
    }
}

void ppTimeLineView::OnRibbonDown()
{
    // Down cast the sender to a ppRibbonButton:
    ppRibbonButton* pPushButton = qobject_cast<ppRibbonButton*>(sender());

    // Sanity check:
    GT_IF_WITH_ASSERT(pPushButton != nullptr)
    {
        // Get the ribbon index:
        int ribbonIndex = pPushButton->PlotIndex();

        // Perform the drag operation:
        DragPlotIntoOther(ribbonIndex, ribbonIndex + 1);

        // Make sure that no plot is highlighted (highlight is done when the user hovers the up / down arrows):
        ClearPlotsBackgrounds();
    }
}

void ppTimeLineView::DragPlotIntoOther(int plotIndex1, int plotIndex2)
{
    if ((plotIndex1 >= 0) && (plotIndex1 < m_visibleRibbonsVec.size()) &&
        (plotIndex2 >= 0) && (plotIndex2 < m_visibleRibbonsVec.size()) && (plotIndex1 != plotIndex2))
    {
        ppMultiLinePlot* pPlot1 = m_visibleRibbonsVec[plotIndex1];
        ppMultiLinePlot* pPlot2 = m_visibleRibbonsVec[plotIndex2];

        if (plotIndex1 > plotIndex2)
        {
            // Dragging from top to bottom (in vector. From bottom to top in visual):
            for (int i = plotIndex2 + 1; i <= plotIndex1; i++)
            {
                m_visibleRibbonsVec[i] = m_visibleRibbonsVec[i - 1];
            }
        }
        else
        {
            // Dragging from bottom to top (in vector. From top to bottom in visual):
            for (int i = plotIndex1; i < plotIndex2; i++)
            {
                m_visibleRibbonsVec[i] = m_visibleRibbonsVec[i + 1];
            }
        }

        // Put plot1 in plotIndex2 position:
        m_visibleRibbonsVec[plotIndex2] = pPlot1;

        pPlot1->GetPlot()->setBackground(Qt::white);
        pPlot2->GetPlot()->setBackground(Qt::white);


        // Add the ribbons from scratch:
        AddRibbonsToGrid();
    }
}

void ppTimeLineView::ShowRibbonButtons(acCustomPlot* pPlot, bool shouldShow)
{
    if (!m_isInShowRibbonButtons)
    {
        m_isInShowRibbonButtons = true;

        // Sanity check:
        GT_IF_WITH_ASSERT(pPlot != nullptr)
        {
            // Find the plot buttons:
            RibbonButtons buttons = m_ribbonButtonsMap[pPlot];
            GT_IF_WITH_ASSERT((buttons.m_pDownButton != nullptr) && (buttons.m_pUpButton != nullptr))
            {
                // Set the buttons geometry (the buttons should be located in the top right corner of the plot axis rect):
                int buttonSize = PP_RIBBON_BUTTON_DIMENSION + PP_RIBBON_BUTTON_PADDING;

                int left1 = pPlot->xAxis->axisRect()->right() - buttonSize - PP_RIBBON_BUTTON_MARGIN;
                int left2 = left1 - buttonSize - PP_RIBBON_BUTTON_MARGIN;

                buttons.m_pUpButton->setGeometry(left2, PP_TIMELINE_PLOT_MARGINS.top() + PP_RIBBON_BUTTON_MARGIN, buttonSize, buttonSize);
                buttons.m_pDownButton->setGeometry(left1, PP_TIMELINE_PLOT_MARGINS.top() + PP_RIBBON_BUTTON_MARGIN, buttonSize, buttonSize);

                buttons.m_pDownButton->setVisible(shouldShow);
                buttons.m_pUpButton->setVisible(shouldShow);

                buttons.m_pDownButton->update();
                buttons.m_pUpButton->update();
            }

            // Hide all other buttons:
            foreach (ppMultiLinePlot* pOtherPlot, m_visibleRibbonsVec)
            {
                GT_IF_WITH_ASSERT(pOtherPlot != nullptr)
                {
                    // If the current plot is visible, and other then the original one:
                    if (pOtherPlot->GetPlot() != pPlot)
                    {

                        // Find the plot buttons:
                        RibbonButtons ribButtons = m_ribbonButtonsMap[pOtherPlot->GetPlot()];
                        GT_IF_WITH_ASSERT((ribButtons.m_pDownButton != nullptr) && (ribButtons.m_pUpButton != nullptr))
                        {
                            ribButtons.m_pDownButton->setVisible(false);
                            ribButtons.m_pUpButton->setVisible(false);

                            ribButtons.m_pDownButton->update();
                            ribButtons.m_pUpButton->update();
                        }
                    }
                }
            }
        }

        m_isInShowRibbonButtons = false;
    }
}

void ppTimeLineView::InitRibbonButtons(ppMultiLinePlot* pPlot)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pPlot != nullptr)
    {
        // Set the size for the buttons:
        int buttonDim = (int)acScalePixelSizeToDisplayDPI(PP_RIBBON_BUTTON_DIMENSION);
        QSize buttonSize(buttonDim + PP_RIBBON_BUTTON_PADDING, buttonDim + PP_RIBBON_BUTTON_PADDING);

        // Create a struct that will contain the buttons:
        RibbonButtons currentRibbonButtons;

        QPixmap downPixmap, upPixmap;
        bool rc = acSetIconInPixmap(downPixmap, AC_ICON_FIND_DOWN, AC_16x16_ICON);
        GT_ASSERT(rc);

        rc = acSetIconInPixmap(upPixmap, AC_ICON_FIND_UP, AC_16x16_ICON);
        GT_ASSERT(rc);

        QString graphName = pPlot->GetPlot()->yAxis->label().replace("\"", "");
        QString tooltipDown = QString(PP_StrTimelineRibbonButtonDownTooltip).arg(graphName);
        QString tooltipUp = QString(PP_StrTimelineRibbonButtonUpTooltip).arg(graphName);

        currentRibbonButtons.m_pUpButton = new ppRibbonButton(pPlot->GetPlot());
        currentRibbonButtons.m_pUpButton->setIcon(upPixmap);
        currentRibbonButtons.m_pUpButton->setContentsMargins(0, 0, 0, 0);
        currentRibbonButtons.m_pUpButton->setToolTip(tooltipUp);
        currentRibbonButtons.m_pUpButton->SetButtonType(ppRibbonButton::PP_RIBBON_BUTTON_UP);

        currentRibbonButtons.m_pDownButton = new ppRibbonButton(pPlot->GetPlot());
        currentRibbonButtons.m_pDownButton->setIcon(downPixmap);
        currentRibbonButtons.m_pDownButton->setContentsMargins(0, 0, 0, 0);
        currentRibbonButtons.m_pDownButton->setToolTip(tooltipDown);
        currentRibbonButtons.m_pUpButton->SetButtonType(ppRibbonButton::PP_RIBBON_BUTTON_DOWN);

        // Set the size for the buttons:
        currentRibbonButtons.m_pUpButton->setMaximumSize(buttonSize);
        currentRibbonButtons.m_pUpButton->setMinimumSize(buttonSize);
        currentRibbonButtons.m_pDownButton->setMaximumSize(buttonSize);
        currentRibbonButtons.m_pDownButton->setMinimumSize(buttonSize);

        // Set the ribbon index for each of the buttons. The index will be set as a property, and will be used
        // in the buttons slot to extract the specific ribbon on which the action should be performed on:
        int currentRibbonIndex = m_visibleRibbonsVec.indexOf(pPlot);
        currentRibbonButtons.m_pUpButton->SetPlotIndex(currentRibbonIndex);
        currentRibbonButtons.m_pDownButton->SetPlotIndex(currentRibbonIndex);

        // Connect the click buttons signals:
        rc = connect(currentRibbonButtons.m_pUpButton, SIGNAL(clicked()), this, SLOT(OnRibbonUp()));
        GT_ASSERT(rc);

        rc = connect(currentRibbonButtons.m_pDownButton, SIGNAL(clicked()), this, SLOT(OnRibbonDown()));
        GT_ASSERT(rc);

        // Connec to the buttons enter + leave events:
        rc = connect(currentRibbonButtons.m_pUpButton, SIGNAL(ButtonEnterLeave(int, ppRibbonButton::ButtonType, bool)), this, SLOT(OnRibbonButtonEnterLeave(int, ppRibbonButton::ButtonType, bool)));
        GT_ASSERT(rc);

        rc = connect(currentRibbonButtons.m_pDownButton, SIGNAL(ButtonEnterLeave(int, ppRibbonButton::ButtonType, bool)), this, SLOT(OnRibbonButtonEnterLeave(int, ppRibbonButton::ButtonType, bool)));
        GT_ASSERT(rc);

        // Connect the mouse enter

        // Add the set of buttons to the map and hide it (the buttons are only visible when the mouse enters the plot):
        m_ribbonButtonsMap[pPlot->GetPlot()] = currentRibbonButtons;


        // Hide the buttons by default:
        currentRibbonButtons.m_pDownButton->setVisible(false);
        currentRibbonButtons.m_pUpButton->setVisible(false);
    }
}

void ppTimeLineView::OnPlotEntered(acCustomPlot* pPlot)
{
    // Show the buttons for the requested plot:
    ShowRibbonButtons(pPlot, true);
//++AT:LPGPU2
// Add a call to this method here to correctly calculate the length of the red
// line which shows where the cursor with respect to time in the graph. If not
// called here, the length of the cursor would be wrong
    CalculateCurrentBottomRibbonsYCoord();
//--AT:LPGPU2
}

void ppTimeLineView::OnPlotLeave(acCustomPlot* pPlot)
{
    // Hide the buttons for the requested plot:
    ShowRibbonButtons(pPlot, false);
}

void ppTimeLineView::OnRibbonButtonEnterLeave(int buttonPlotIndex, ppRibbonButton::ButtonType buttonType, bool wasEntered)
{
    // Find the plot which is about to be dropped to when the button is clicked:
    int highlightedPlotIndex = (buttonType == ppRibbonButton::PP_RIBBON_BUTTON_UP) ? buttonPlotIndex + 1 : buttonPlotIndex - 1;

    // Clear all the backgrounds:
    ClearPlotsBackgrounds();

    // Check if the target plot exists:
    if ((highlightedPlotIndex < m_visibleRibbonsVec.size()) && (highlightedPlotIndex >= 0)
        && (buttonPlotIndex < m_visibleRibbonsVec.size()) && (buttonPlotIndex >= 0))
    {
        // Get the plot which is about to be overridden:
        ppMultiLinePlot* pHightlightPlot = m_visibleRibbonsVec[highlightedPlotIndex];
        GT_IF_WITH_ASSERT((pHightlightPlot != nullptr) && (pHightlightPlot->GetPlot() != nullptr))
        {
            pHightlightPlot->GetPlot()->SetHighlighted(wasEntered);
        }
    }
}

void ppTimeLineView::OnPlotMouseMove(QMouseEvent* pMouseEvent)
{
    // Get the acCustomPlot from sender:
    acCustomPlot* pCustomPlot = qobject_cast<acCustomPlot*>(sender());
    GT_IF_WITH_ASSERT((pCustomPlot != nullptr) && (m_pSessionNavigationChart != nullptr))
    {
        if (pCustomPlot->Dragging())
        {
            // Get the plot mouse point x coordinate:
            int plotMouseXPosition = pMouseEvent->pos().x();

            // Find the requested X position relative location on the plot X Axis:
            double xLeft = pCustomPlot->xAxis->axisRect()->left();
            double xRight = pCustomPlot->xAxis->axisRect()->right();
            double currentPos = xRight - (double)plotMouseXPosition;
            double xRelativePosition = currentPos / (xRight - xLeft);

            // Calculate the respecting X position on the session navigation chart:
            double xLeftNav = m_pSessionNavigationChart->xAxis->axisRect()->left();
            double xRightNav = m_pSessionNavigationChart->xAxis->axisRect()->right();
            double navXPosition = (xRightNav - xLeftNav) * xRelativePosition;

            m_pSessionNavigationChart->DragRangeTo((int)navXPosition);
            m_pSessionNavigationChart->HighlightActiveRange();

            // Handle the session navigation range changed:
            const QPointF& currentRange = m_pSessionNavigationChart->GetCurrentRange();
            OnRangeChangedByUser(currentRange);
        }
    }
}

void ppTimeLineView::OnPlotMouseWheel(QWheelEvent* pMouseEvent)
{
    // Get the acCustomPlot from sender:
    acCustomPlot* pCustomPlot = qobject_cast<acCustomPlot*>(sender());
    GT_IF_WITH_ASSERT((pCustomPlot != nullptr) && (m_pSessionNavigationChart != nullptr) && (m_pBottomScrollArea != nullptr))
    {
        // Check if the control key is clicked:
        bool isCtrlClicked = QApplication::keyboardModifiers() & Qt::ControlModifier;

        // If the control key is clicked, then zoom should be implemented:
        bool isWheelUp = (pMouseEvent->angleDelta().y() > 0);

        if (isCtrlClicked)
        {
            // Calculate the range after zoom:
            QCPRange rangeAfterZoom;
            m_pSessionNavigationChart->CalculateRangeAfterZoom(isWheelUp, rangeAfterZoom);

            // Apply the calculated range:
            m_pSessionNavigationChart->ApplyRangeAfterZoom(rangeAfterZoom);
        }
        else
        {
            // NOTICE: Since the custom plot is stealing the scroll wheel events from the scroll area,
            // we should implement it manually here (when control key is not pressed, scroll up / down):
            // Get the scroll step:
            int scrollStep = m_pBottomScrollArea->verticalScrollBar()->singleStep();

            // Get the current scroll value:
            int scrollValue = m_pBottomScrollArea->verticalScrollBar()->value();

            // Get the new value:
            scrollValue = isWheelUp ? (scrollValue - scrollStep) : (scrollValue + scrollStep);

            // Set the new value:
            m_pBottomScrollArea->verticalScrollBar()->setValue(scrollValue);
        }
    }
}

void ppTimeLineView::ClearPlotsBackgrounds()
{
    // Check if the target plot exists:
    for (int i = 0; i < m_visibleRibbonsVec.size(); i++)
    {
        // Get the plot which is about to be overridden:
        ppMultiLinePlot* pHightlightPlot = m_visibleRibbonsVec[i];
        GT_IF_WITH_ASSERT((pHightlightPlot != nullptr) && (pHightlightPlot->GetPlot() != nullptr))
        {
            pHightlightPlot->GetPlot()->SetHighlighted(false);
        }
    }
}

//++CF:LPGPU2
// Trigger a range updated event, used to force acTimelines to draw items correctly
void ppTimeLineView::ForceRangeUpdate()
{
  const QPointF currentRange = m_pSessionNavigationChart->GetCurrentRange();
  OnRangeChangedByUser(currentRange);
}
//--CF:LPGPU2

void ppTimeLineView::OnPowerProfilerCoreAboutToStop()
{
}
