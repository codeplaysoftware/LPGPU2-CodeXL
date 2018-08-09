// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2RegionOfInterestView.cpp
///
/// @brief Separate tab view to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// ppLPGPU2RegionOfInterestView definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/ppLPGPU2RegionOfInterestView.h>

// Framework:
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFeedbackAnalysisView.h>

// Qt:
#include <QSplitter>
#include <QStyleFactory>
#include <chrono>

using lpgpu2::PPFnStatus;

/// @brief Ctor. No work is done here
///
/// @param pParent            The QWidget which owns this one
/// @param pSessionController The session controller for the current session
ppLPGPU2RegionOfInterestView::ppLPGPU2RegionOfInterestView(QWidget* pParent, ppSessionController* pSessionController, gtUInt32 regionOfInterestID)
: QWidget(pParent)
, m_pParent(pParent)
, m_pSessionController(pSessionController)
, m_regionOfInterestID(regionOfInterestID)
{
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
///        we need to make sure that Shutdown was called anyway.
ppLPGPU2RegionOfInterestView::~ppLPGPU2RegionOfInterestView()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus ppLPGPU2RegionOfInterestView::Initialise()
{
    PPFnStatus bInitViewLayoutReturn = InitViewLayout();
    PPFnStatus bInitFeedbackAnalysisViewReturn = InitFeedbackAnalysisView();
    
    if ((bInitViewLayoutReturn == PPFnStatus::success) && (bInitFeedbackAnalysisViewReturn == PPFnStatus::success))
    {
        return PPFnStatus::success;
    }
    else
    {
        return PPFnStatus::failure;
    }
}

/// @brief Deinitialise the object
PPFnStatus ppLPGPU2RegionOfInterestView::Shutdown()
{
  if (m_pTimelineView != nullptr)
  {
    delete m_pTimelineView;
    m_pTimelineView = nullptr;
  }

  if (m_pFeedbackAnalysisView != nullptr)
  {
    delete m_pFeedbackAnalysisView;
    m_pFeedbackAnalysisView = nullptr;
  }

  if (m_pSessionController != nullptr)
  {
    m_pSessionController = nullptr;
  }

  return PPFnStatus::success;
}

/// @brief Initialise the layout of the widget by creating widget objects
PPFnStatus ppLPGPU2RegionOfInterestView::InitViewLayout()
{  
  QVBoxLayout *pMainLayout = nullptr;
  LPGPU2PPNewQtWidget(&pMainLayout, this);

  QScrollArea *pBottomScrollArea = nullptr;
  LPGPU2PPNewQtWidget(&pBottomScrollArea, this);

  LPGPU2PPNewQtLPGPU2Widget(&m_pFeedbackAnalysisView, this);
  m_pFeedbackAnalysisView->setStyleSheet(AF_STR_WhiteBG);

  const bool bIsRegionOfInterestTimeline = true;
  LPGPU2PPNewQtWidget(&m_pTimelineView, this, m_pSessionController, bIsRegionOfInterestTimeline, m_regionOfInterestID);

  pBottomScrollArea->setGeometry(m_pFeedbackAnalysisView->geometry());
  pBottomScrollArea->setWidget(m_pFeedbackAnalysisView);
  pBottomScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  pBottomScrollArea->setWidgetResizable(true);
  pBottomScrollArea->setContentsMargins(0, 0, 0, 0);
  pBottomScrollArea->setFrameShape(QFrame::NoFrame);
  
  QSplitter *pVerticalSpliter = nullptr;
  LPGPU2PPNewQtWidget(&pVerticalSpliter, Qt::Vertical, this);  
  pVerticalSpliter->setStyle(QStyleFactory::create("Fusion"));
  pVerticalSpliter->setStyleSheet(PP_STR_SplitterStyleSheet);
  pVerticalSpliter->setHandleWidth(6);

  pVerticalSpliter->addWidget(m_pTimelineView);
  pVerticalSpliter->addWidget(pBottomScrollArea);

  pMainLayout->setMargin(0);
  pMainLayout->addWidget(pVerticalSpliter);

  connect(m_pFeedbackAnalysisView, &LPGPU2ppFeedbackAnalysisView::AnnotationSelected, this, &ppLPGPU2RegionOfInterestView::OnAnnotationSelected);
  connect(m_pFeedbackAnalysisView, &LPGPU2ppFeedbackAnalysisView::ShaderTraceSeleted, this, &ppLPGPU2RegionOfInterestView::OnShaderTraceSelected);
  connect(m_pFeedbackAnalysisView, &LPGPU2ppFeedbackAnalysisView::OpenShaderSourceRequested, 
          this, &ppLPGPU2RegionOfInterestView::OpenShaderSourceRequested);
  
  setGeometry(m_pParent->geometry());
  m_pTimelineView->setGeometry(geometry());
  m_pTimelineView->ForceRangeUpdate();
  
  setVisible(true);
  
  return PPFnStatus::success;
}

PPFnStatus ppLPGPU2RegionOfInterestView::InitFeedbackAnalysisView()
{
    auto& profilerBL = m_pSessionController->GetProfilerBL();

    lpgpu2::db::CallsPerType callsPerType;
    gtVector<lpgpu2::db::CallSummary> vecCallSummary;
    gtUInt64 accumulatedCallSummaryTime; // TODO this is not being set correctly

    auto bDataIsOk = true;

    if(profilerBL.GetRegionOfInterestById(m_regionOfInterestID, m_regionOfInterest))
    {
        // Calculate duration of this region:
        gtUInt64 frameStartStart, frameStartEnd, frameEndStart, frameEndEnd;
        profilerBL.GetFrameTimeRange(m_regionOfInterest.m_frameStart, frameStartStart, frameStartEnd);
        profilerBL.GetFrameTimeRange(m_regionOfInterest.m_frameEnd, frameEndStart, frameEndEnd);
        const auto regionDuration = frameEndEnd - frameStartStart;

        if (profilerBL.GetCallsPerTypeByRegionId(m_regionOfInterestID, callsPerType))
        {
            m_pFeedbackAnalysisView->SetCallsPerType(callsPerType);
        }

        if (profilerBL.GetAccumulatedCallSummaryTime(accumulatedCallSummaryTime) &&
            profilerBL.GetCallSummaryForRegionId(m_regionOfInterestID, vecCallSummary))
        {
            m_pFeedbackAnalysisView->SetCallsSummaryData(regionDuration, vecCallSummary);
        }

        if(profilerBL.GetAnnotationsInFrameRange(m_regionOfInterest.m_frameStart, m_regionOfInterest.m_frameEnd, m_Annotations))
        {
            m_pFeedbackAnalysisView->SetAnnotations(m_Annotations);
        }

        if(profilerBL.GetGlobalAnnotations(m_globalAnnotations))
        {
            m_pFeedbackAnalysisView->SetGlobalAnnotations(m_globalAnnotations);
        }

        if(profilerBL.GetUserAnnotations(m_userAnnotations))
        {
            m_pFeedbackAnalysisView->SetUserAnnotations(m_userAnnotations);
        }

        if(profilerBL.GetShaderTracesInFrameRange(m_regionOfInterest.m_frameStart, m_regionOfInterest.m_frameEnd, m_ShaderTraces))
        {
            m_pFeedbackAnalysisView->SetShaderTraces(m_ShaderTraces);
        }            
    } 
    else
    {
        bDataIsOk = false;
    }

    return bDataIsOk ? PPFnStatus::success : PPFnStatus::failure;
}

void ppLPGPU2RegionOfInterestView::OnAnnotationSelected(gtUInt32 vAnnotationId)
{
    m_pTimelineView->ClearHighlightRegionsOnTimes();

    auto annotationIt = std::find_if(m_Annotations.begin(), m_Annotations.end(), [&](const lpgpu2::db::Annotation &annotation)
    {
        return annotation.m_annotationId == vAnnotationId;
    });

    if (annotationIt != m_Annotations.end())
    {
        const auto& annotation = *annotationIt;

        std::chrono::nanoseconds cpuStartNs{ annotation.m_cpuStartTime };
        std::chrono::nanoseconds cpuEndNs{ annotation.m_cpuEndTime };        

        // When there is no range available for the annotation, use the frame number range
        // to highlight the timeline and draw the user's attention.
        if (cpuStartNs == cpuEndNs)
        {
            gtUInt64 startNs = 0;
            gtUInt64 endNs = 0;
            if (m_pSessionController->GetProfilerBL().GetFrameTimeRange(annotation.m_frameNum, startNs, endNs))
            {
                cpuStartNs = std::chrono::nanoseconds{ startNs };
                cpuEndNs = std::chrono::nanoseconds{ endNs };
            }
        }

        const auto cpuStartMs = std::chrono::duration_cast<std::chrono::milliseconds>(cpuStartNs);
        const auto cpuEndMs = std::chrono::duration_cast<std::chrono::milliseconds>(cpuEndNs);

        m_pTimelineView->AddHighlightRegionOnTimelines(m_regionOfInterestID, cpuStartMs.count(), cpuEndMs.count(), QColor{ Qt::yellow });
    }
}

void ppLPGPU2RegionOfInterestView::OnShaderTraceSelected(const gtString& vShaderId)
{
    m_pTimelineView->ClearHighlightRegionsOnTimes();

    // TODO - rather than using a string with two numbers for an ID, it might be nicer to
    //        add a primary key to the database or use the hash of the shader text.
    auto shaderTraceIt = std::find_if(m_ShaderTraces.begin(), m_ShaderTraces.end(), [&](const lpgpu2::db::ShaderTrace &shaderTrace)
    {
      gtUInt32 frameID, drawID;
      try
      {
        QStringList components = acGTStringToQString(vShaderId).split(":");
        frameID = components[0].toULong();
        drawID = components[1].toULong();
      }
      catch (...)
      {
        return false;
      }
      return (static_cast<gtUInt32>(shaderTrace.m_frameNum) == frameID) &&
             (static_cast<gtUInt32>(shaderTrace.m_drawNum) == drawID);        
    });

    if (shaderTraceIt != m_ShaderTraces.end())
    {
        const auto& shaderTrace = *shaderTraceIt;

        for (const auto& annotation : m_Annotations)
        {
            if (annotation.m_cpuStartTime <= shaderTrace.m_cpuTime && shaderTrace.m_cpuTime <= annotation.m_cpuEndTime)
            {
                m_pTimelineView->AddHighlightRegionOnTimelines(m_regionOfInterestID, annotation.m_cpuStartTime, annotation.m_cpuEndTime, QColor{ Qt::red });
            }
        }        

        m_pFeedbackAnalysisView->AddShaderAnalysisTab(shaderTrace);
    }
}

// Trigger a range updated event in ppTimelineView, used to force acTimelines to draw items correctly
void ppLPGPU2RegionOfInterestView::ForceRangeUpdate()
{
  m_pTimelineView->ForceRangeUpdate();
}

ppTimeLineView* ppLPGPU2RegionOfInterestView::GetTimeline()
{
  return m_pTimelineView;
}
