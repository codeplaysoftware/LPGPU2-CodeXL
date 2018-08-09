// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameDetailView.cpp
///
/// @brief Separate tab view to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// LPGPU2ppFrameDetailView definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppFrameDetailView.h>

// Framework:
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTPowerProfiling/src/LPGPU2ppGLTimelineItem.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFrameAPICallTable.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFrameCounterSummaryTable.h>
#include <AMDTPowerProfiling/src/LPGPU2ppGLTimelineItem.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFrameTimeline.h>

// Qt:
#include <QPushButton>
#include <QSplitter>
#include <QSizePolicy>
#include <QStyleFactory>

using lpgpu2::PPFnStatus;
using lpgpu2::db::EShaderTraceAPIId;

/// @brief Ctor. No work is done here
///
/// @param pParent            The QWidget which owns this one
/// @param pSessionController The session controller for the current session
LPGPU2ppFrameDetailView::LPGPU2ppFrameDetailView(QWidget* pParent, ppSessionController* pSessionController, gtUInt32 frameID)
: QWidget(pParent)
, m_pParent(pParent)
, m_pSessionController(pSessionController)
, m_frameID(frameID)
, m_timeline(nullptr)
{
  // The definition for this enum is in the database layer
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kGLES2]  = L"GLES2";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kGLES3]  = L"GLES3";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kEGL]    = L"EGL";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kGL]     = L"OpenGL";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kCL]     = L"OpenCL";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kVULKAN] = L"Vulkan";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kNEMA]   = L"NemaGFX";
  m_mapAPICategoryToAPIName[EShaderTraceAPIId::kALL]    = L"All";
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
///        we need to make sure that Shutdown was called anyway.
LPGPU2ppFrameDetailView::~LPGPU2ppFrameDetailView()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus LPGPU2ppFrameDetailView::Initialise()
{
  return InitViewLayout();
}

/// @brief Deinitialise the object
PPFnStatus LPGPU2ppFrameDetailView::Shutdown()
{
  if (m_pSessionController != nullptr)
  {
    m_pSessionController = nullptr;
  }

  return PPFnStatus::success;
}

/// @brief Initialise the layout of the widget by creating widget objects
PPFnStatus LPGPU2ppFrameDetailView::InitViewLayout()
{  
  setGeometry(m_pParent->geometry());
  QVBoxLayout *pMainLayout = nullptr;
  LPGPU2PPNewQtWidget(&pMainLayout, this);
  pMainLayout->setMargin(0);
  pMainLayout->setAlignment(Qt::AlignHCenter);
  // Create timeline for API calls
  acTimeline *pTraceView = new LPGPU2ppFrameTimeline(this);
  GT_ASSERT(pTraceView);
  
  for (auto branchPair : m_mapAPICategoryToAPIName)
  {
    auto branch = new (std::nothrow) acTimelineBranch();
    branch->setText(acGTStringToQString(branchPair.second));
    GT_ASSERT(branch);
    pTraceView->addBranch(branch);
    m_mapAPICategoryToBranch[branchPair.first] = branch;
  }
  
  m_timeline = LPGPU2ppRibbonTimelineWrapper(pTraceView);
  m_timeline.SetLabelText("API Trace");
  
  AddWidgetToLayout(&m_timeline, pMainLayout);
  
  // Bottom layout - API call table and counter overview text
  QWidget* pBottomWidget = new QWidget;
  QHBoxLayout* pBottomLayout = new QHBoxLayout;
  pBottomWidget->setLayout(pBottomLayout);
  
  QSplitter *pSplitter = nullptr;
  LPGPU2PPNewQtWidget(&pSplitter, Qt::Vertical, this);
  pSplitter->setStyle(QStyleFactory::create("Fusion"));
  pSplitter->setHandleWidth(6);
  pSplitter->setStyleSheet(PP_STR_SplitterStyleSheet);
  pSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  LPGPU2PPNewQtWidget(&m_pButtonContainer, this);
  QHBoxLayout* pButtonLayout = new QHBoxLayout;
  m_pButtonContainer->setLayout(pButtonLayout);
  m_pButtonContainer->setVisible(false);
  LPGPU2PPNewQtWidget(&m_pRegionButton, "");
  m_pRegionButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
  pButtonLayout->addWidget(m_pRegionButton);
  
  LPGPU2PPNewQtLPGPU2Widget(&m_pCallTable, pSplitter, m_frameID);
  pSplitter->addWidget(m_pCallTable);
  LoadAPICalls();

  LPGPU2PPNewQtLPGPU2Widget(&m_pCounterTable, pSplitter, m_pSessionController, m_frameID, m_frameEndTime);
  pSplitter->addWidget(m_pCounterTable);
  pMainLayout->addWidget(pSplitter);
  pMainLayout->addWidget(m_pButtonContainer);

  setVisible(true);
  ForceRangeUpdate();
  
  connect(m_pCallTable, &LPGPU2ppFrameAPICallTable::APICallSelected, this, &LPGPU2ppFrameDetailView::OnAPICallSelected);
  connect(m_pRegionButton, &QPushButton::clicked, this, &LPGPU2ppFrameDetailView::OnRegionButtonClicked);
  
  return PPFnStatus::success;
}

// Trigger a range updated event in ppTimelineView, used to force acTimelines to draw items correctly
void LPGPU2ppFrameDetailView::ForceRangeUpdate()
{
  m_timeline.GetWidget()->setGeometry(geometry());
}

// Adapted from method in ppTimelineView:
void LPGPU2ppFrameDetailView::AddWidgetToLayout(LPGPU2ppRibbonWidgetWrapper* wrapper, QLayout* layout)
{
    GT_IF_WITH_ASSERT(nullptr != wrapper)
    {
    QWidget*     pWrapWidget = new QWidget;
        QGridLayout* pLayout = new QGridLayout;

        // Faking the size of the info table that normally appears on the right of the plot.
        QWidget* pRightPaddingWidget = new QWidget;

        // Calculate the right padding fixed width, according to the current font metric:
        int rightPaddingWidth = 0;
        pRightPaddingWidget->setFixedWidth(rightPaddingWidth + 5); // +5 by trial and error

        // Using the wrapper's vertical label, set the fixed width padding on the left side of the ribbon widget
        int leftPaddingWidth = acScalePixelSizeToDisplayDPI(55); // 55 by trial and error
        wrapper->GetLabel()->setFixedWidth(leftPaddingWidth);

        // Adding the wrapped widget and padding widgets to the main bottom layout
        pLayout->addWidget(wrapper->GetLabel(), 0, 0);
        pLayout->addWidget(wrapper->GetWidget(), 0, 1, Qt::AlignVCenter);
        pLayout->addWidget(pRightPaddingWidget, 0, 2);
        pWrapWidget->setLayout(pLayout);
        layout->addWidget(pWrapWidget);
    }
}

PPFnStatus LPGPU2ppFrameDetailView::LoadAPICalls()
{
    VecTrace_t vecAllTraces;
    auto& profilerBL = m_pSessionController->GetProfilerBL();
    auto bOk = profilerBL.GetTraceRecords(vecAllTraces);

    if (!bOk)
    {
      return PPFnStatus::failure;
    }

    VecStackParams_t vecStackParams;
    bOk = profilerBL.GetStackTraceParameters(vecStackParams);

    auto pTimelineWrappedWidget = qobject_cast<acTimeline*>(m_timeline.GetWidget());
    GT_ASSERT(pTimelineWrappedWidget);


    gtUInt64 tEnd = 0;
    const gtUInt32 num_params = vecStackParams.size();

    m_vecTraces.clear();
    m_vecTraces.reserve(vecAllTraces.size());
    m_vecParams.clear();
    m_vecParams.reserve(num_params);

    size_t i = 0;
    size_t p = 0;
    // Get the first parameters entry with the frame we are interested in
    for (; p < vecStackParams.size(); ++p)
    {
      const auto &params = vecStackParams[p];
      if (m_frameID == params.m_frameNum)
      {
        break;
      }
    }

    // Find the trace entries for the frame we are interested in
    for (; i < vecAllTraces.size(); ++i)
    {
      auto &rec = vecAllTraces[i];      

      if (rec.m_frameNum == m_frameID)
      {
        const auto bInMilliSeconds = false;
        auto item = new LPGPU2ppGLTimelineItem{ rec.m_cpuStart, rec.m_cpuEnd,
          m_frameID, rec.m_drawNum, bInMilliSeconds };
        GT_ASSERT(item);

        auto branch = m_mapAPICategoryToBranch[static_cast<EShaderTraceAPIId>(rec.m_apiId)];

        if (tEnd == 0)
        {
          m_frameEndTime = tEnd = rec.m_cpuEnd;
        }

        item->setText(rec.m_callName.asUTF8CharArray());
        item->setBackgroundColor(PP_LPGPU2_API_TIMELINE_COLOR);
        branch->addTimelineItem(item);
        m_mapDrawNumtoItemPtr[rec.m_drawNum] = item;
        m_vecTraces.push_back(std::move(rec));

        if (!vecStackParams.empty() && p < vecStackParams.size())
        {
            const auto &params = vecStackParams[p];

            // Only get parameter entries with the same drawnum as the trace entry
            if ((p < num_params) && (params.m_drawNum == rec.m_drawNum))
            {
                m_vecParams.push_back(params);
                ++p;
            }
        }        
      }
    }
    
    // Hide branches with no items and update widget height:
    auto& branches = pTimelineWrappedWidget->GetBranches();
    gtUInt32 visibleBranchCount = branches.size();
    for (auto branch : branches)
    {
      if (branch->itemCount() == 0)
      {
        branch->setVisibility(false);
        visibleBranchCount--;
      }
    }
    // Adding the height of two branches provides enough room for grid and margins:
    pTimelineWrappedWidget->setMinimumHeight(branches[0]->height() * (visibleBranchCount + 2));

    m_pCallTable->PopulateTable(m_vecTraces, m_vecParams);

    connect(m_timeline.GetWidget(), SIGNAL(itemClicked(acTimelineItem*)), this, SLOT(OnAPITimelineItemClicked(acTimelineItem*)));
  
    // If frame is in a region of interest, display the button:
    gtVector<lpgpu2::db::RegionOfInterest> vecRegions;
    bool rc = profilerBL.GetRegionsOfInterest(vecRegions);
    if (!rc)
    {
      return PPFnStatus::failure;
    }
    
    for (auto region : vecRegions)
    {
      if (m_frameID >= region.m_frameStart && m_frameID <= region.m_frameEnd)
      {
        m_regionID = region.m_regionId;
        m_pRegionButton->setText(QString("Go to associated Region of Interest (%0)").arg(m_regionID));
        m_pButtonContainer->setVisible(true);
        break;
      }
    }

    return PPFnStatus::success;
}

/// @brief Selects and zooms in to a specified API call item on the timeline.
PPFnStatus LPGPU2ppFrameDetailView::ShowTimelineItem(quint64 drawNum)
{
  auto pTimelineWrappedWidget = qobject_cast<acTimeline*>(m_timeline.GetWidget());
  GT_ASSERT(pTimelineWrappedWidget);

  if (m_mapDrawNumtoItemPtr.find(drawNum) != m_mapDrawNumtoItemPtr.end())
  {
    pTimelineWrappedWidget->ZoomToItem(m_mapDrawNumtoItemPtr[drawNum], true);
    return PPFnStatus::success;
  }
  else
  {
    return PPFnStatus::failure;
  }
}

/// @brief Signal for when API call has been selected in the table.
void LPGPU2ppFrameDetailView::OnAPICallSelected(quint64 drawNum)
{
  ShowTimelineItem(drawNum);
}

/// @brief Signal for when API call item has been clicked in the timeline.
void LPGPU2ppFrameDetailView::OnAPITimelineItemClicked(acTimelineItem* item)
{
  LPGPU2ppGLTimelineItem* frameItem = qobject_cast<LPGPU2ppGLTimelineItem*>(item);
    GT_IF_WITH_ASSERT(frameItem != nullptr)
    {
        m_pCallTable->SelectRow(frameItem->drawId());
    }
}

/// @brief Signal for when the 'Open associated Region of Interest' view has 
///        been clicked.
void LPGPU2ppFrameDetailView::OnRegionButtonClicked()
{
  emit RegionButtonClicked(m_regionID);
}

/// @brief Trigger zooming of the timeline ribbon
/// @return Always PPFnStatus::Success
PPFnStatus LPGPU2ppFrameDetailView::TriggerZoom()
{
  auto pTimelineWrappedWidget = qobject_cast<acTimeline*>(m_timeline.GetWidget());
  GT_ASSERT(pTimelineWrappedWidget);

  pTimelineWrappedWidget->TriggerZoom();

  return PPFnStatus::success;
}
