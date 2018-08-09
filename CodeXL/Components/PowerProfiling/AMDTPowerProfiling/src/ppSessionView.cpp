//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionView.cpp
///
//==================================================================================

//------------------------------ ppSessionView.cpp ------------------------------

#include <AMDTPowerProfiling/src/ppSessionView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTOSWrappers/Include/osFile.h>
//++AT:LPGPU2DEMO_V
#include <AMDTOSWrappers/Include/osDirectory.h>
//--AT:LPGPU2DEMO_V

// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppSummaryView.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>
//++AT:LPGPU2DEMO_V
#include <AMDTPowerProfiling/src/LPGPU2ppMultiSourceCodeView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
using lpgpu2::PPFnStatus;
//--AT:LPGPU2DEMO_V
//++CF:LPGPU2
#include <AMDTPowerProfiling/src/ppLPGPU2MultiRegionOfInterestView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppMultiFrameDetailView.h>
//--CF:LPGPU2
//++TLRS: LPGPU2:
#include <AMDTPowerProfiling/src/LPGPU2ppSessionComparisonView.h>
//--TLRS: LPGPU2:

// ---------------------------------------------------------------------------
ppSessionView::ppSessionView(QWidget* pParent, ppSessionController::SessionState state) :
    SharedSessionWindow(pParent),
    m_pMainLayout(nullptr), m_pTabWidget(nullptr), m_pTimelineView(nullptr),
    m_pSummaryView(nullptr)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_sessionController.SetState(state);

    if (state == ppSessionController::PP_SESSION_STATE_RUNNING)
    {
        ppAppController::instance().DisplayProcessRunningHTML();
    }
}

// ---------------------------------------------------------------------------
ppSessionView::~ppSessionView()
{

}

// ---------------------------------------------------------------------------
void ppSessionView::SetViewData(const osFilePath& sessionPath)
{
    gtString sessionName;
    gtString dbFullPath;
    UpdateSessionInfo(sessionPath, sessionName, dbFullPath);

    m_sessionController.OpenDB();

    // Create the tab view in a layout and add it.
    m_pMainLayout = new QVBoxLayout(this);

    m_pTabWidget = new acTabWidget;

    m_pTabWidget->setTabsClosable(false);

    m_pMainLayout->addWidget(m_pTabWidget);

    m_pTimelineView = new ppTimeLineView(this, &m_sessionController);
    m_pTabWidget->addTab(m_pTimelineView, acGTStringToQString(PP_STR_TreeNodeTimeline));

    m_pSummaryView = new ppSummaryView(this, &m_sessionController);
    m_pTabWidget->addTab(m_pSummaryView, acGTStringToQString(PP_STR_TreeNodeSummary));

    //++AT:LPGPU2
    m_pMultiSourceCodeView = new LPGPU2ppMultiSourceCodeView(this, &m_sessionController);

    // For the Shaders view, if the ppSessionView was created for a new session, this will listen to the
    // ProfileStopped signal in order to proper initialise the MultiSourceCodeView. In case this is an
    // old session, then we can call the Initialise method directly.
    if (ppAppController::instance().SessionIsOn())
    {
        connect(&ppAppController::instance(), &ppAppController::ProfileStopped, this, [=](const QString& sessionNameThatStopped)
        {
            if (acQStringToGTString(sessionNameThatStopped) == sessionName)
            {
                m_pMultiSourceCodeView->Initialise();
                m_pTabWidget->setCurrentIndex(0);
            }            
        });
    }
    else
    {
        m_pMultiSourceCodeView->Initialise();
        m_pTabWidget->setCurrentIndex(0);
    }

    m_pTabWidget->addTab(m_pMultiSourceCodeView,
    acGTStringToQString(PP_STR_TreeNodeSources));
    //--AT:LPGPU2

//++CF:LPGPU2
    m_pRegionsOfInterestView = new ppLPGPU2MultiRegionOfInterestView(this, &m_sessionController);
    auto status = m_pRegionsOfInterestView->Initialise();
    GT_ASSERT(status == PPFnStatus::success);
    m_pTabWidget->addTab(m_pRegionsOfInterestView, acGTStringToQString(PP_STR_TreeNodeFeedback));
    m_pTabWidget->addTab(m_pRegionsOfInterestView, acGTStringToQString(PP_STR_TreeNodeRegionsOfInterest));
    
    m_pFrameDetailView = new LPGPU2ppMultiFrameDetailView(this, &m_sessionController);
    status = m_pFrameDetailView->Initialise();
    GT_ASSERT(status == PPFnStatus::success);
    m_pTabWidget->addTab(m_pFrameDetailView, acGTStringToQString(PP_STR_TreeNodeFrameDetails));
//--CF:LPGPU2

    m_pMainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_pMainLayout);

    // Connect to profile start and stop signals:
    bool rc = QObject::connect(&ppAppController::instance(), SIGNAL(ProfileStopped(const QString&)), this, SLOT(OnProfileStopped(const QString&)));
    GT_ASSERT(rc);

    // TODO: Commented out to avoid build failing whilst signal/slot arguments don't match
//++AT:LPGPU2DEMO_V
    // // Connect the timelineview bubbles with the opening of source files
    // rc = connect(m_pTimelineView, &ppTimeLineView::OnBubbleClicked, 
    //   m_pMultiSourceCodeView, &ppMultiSourceCodeView::OpenAndDisplayFileSlot);
    // GT_ASSERT(rc);
    rc = connect(m_pMultiSourceCodeView, &LPGPU2ppMultiSourceCodeView::SourceSet,
      this, &ppSessionView::OnMultiSourceSourceSet);
    GT_ASSERT(rc);
//--AT:LPGPU2DEMO_V

    //++CF:LPGPU2
    // Connect the clicking of the timeline ROI bubbles with the opening of the
    // associated region of interest.
    rc = connect(m_pTimelineView, &ppTimeLineView::OnBubbleClicked, 
      m_pRegionsOfInterestView, &ppLPGPU2MultiRegionOfInterestView::DisplayRegionOfInterestSlot);
    GT_ASSERT(rc);
    rc = connect(m_pRegionsOfInterestView, &ppLPGPU2MultiRegionOfInterestView::RegionOfInterestOpened,
      this, &ppSessionView::OnRegionOfInterestOpened);
    GT_ASSERT(rc);
    // Connect the click of the API/Frame bubbles with the opening of a frame detail view:
    rc = connect(m_pTimelineView, &ppTimeLineView::OnFrameBubbleClicked, 
      m_pFrameDetailView, &LPGPU2ppMultiFrameDetailView::DisplayFrameDetailSlot);
    GT_ASSERT(rc);
    rc = connect(m_pFrameDetailView, &LPGPU2ppMultiFrameDetailView::FrameDetailOpened,
      this, &ppSessionView::OnFrameDetailOpened);
    GT_ASSERT(rc);
    // Connect the click of Open Region of Interest button in frame view to ROI view:
    rc = connect(m_pFrameDetailView, &LPGPU2ppMultiFrameDetailView::RegionButtonClicked,
      m_pRegionsOfInterestView, &ppLPGPU2MultiRegionOfInterestView::DisplayRegionOfInterestSlot);
    GT_ASSERT(rc);
    // Connect the request of a shader source open with the multi source view:
    rc = connect(m_pRegionsOfInterestView, &ppLPGPU2MultiRegionOfInterestView::ShaderSourceOpenRequested,
      m_pMultiSourceCodeView, &LPGPU2ppMultiSourceCodeView::OpenAndDisplayShaderSlot);
    GT_ASSERT(rc);
//--CF:LPGPU2

    //QMetaObject::invokeMethod(m_pTimelineView, "OnBubbleClicked", Q_ARG(QString, "test_source_file.cpp"));
    //QMetaObject::invokeMethod(m_pTimelineView, "OnBubbleClicked", Q_ARG(QString, "test_source_file.cpp"));
    //--CF:LPGPU2

    //++TLRS: LPGPU2: Adding the new Session Comparison view
    m_pSessionComparisonView = new (std::nothrow) LPGPU2ppSessionComparisonView{ &m_sessionController, this };
    if (m_pSessionComparisonView != nullptr)
    {
        const auto tabIndex = m_pTabWidget->addTab(m_pSessionComparisonView, acGTStringToQString(PP_STR_TreeNodeCompareToSession));
        m_pTabWidget->setTabEnabled(tabIndex, m_pSessionComparisonView->Initialise() == PPFnStatus::success);
    }
    //--TLRS: LPGPU2: Adding the new Session Comparison view
}

// ---------------------------------------------------------------------------
void ppSessionView::UpdateSessionInfo(const osFilePath& sessionPath, gtString& sessionName, gtString& dbFullPath)
{
    ppAppController& appController = ppAppController::instance();

    osFilePath sessionDBPath = sessionPath;

    // Set the file path in the controller:
    m_sessionController.SetDBFilePath(sessionDBPath);

    if (sessionName.isEmpty())
    {
        appController.SetStandaloneSessionDbPath(sessionPath);
    }

    dbFullPath = sessionDBPath.asString();

    // Get the session name from the sessionPath:
    osDirectory sessionDir;
    sessionPath.getFileDirectory(sessionDir);
    QString sessionNameAsQString = appController.GetProjectNameFromSessionDir(sessionDir);
    sessionName = acQStringToGTString(sessionNameAsQString);
}

// ---------------------------------------------------------------------------
void ppSessionView::DisplayTab(int tabIndex)
{
    GT_IF_WITH_ASSERT(tabIndex >= 0 && tabIndex < m_pTabWidget->count())
    {
        m_pTabWidget->setCurrentIndex(tabIndex);
    }
}

void ppSessionView::StartDBConnection()
{
    m_sessionController.OpenDB();
}

void ppSessionView::StopDBConnection()
{
    m_sessionController.CloseDB();
}

void ppSessionView::ActivateSession()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTimelineView != nullptr) && (m_pSummaryView != nullptr))
    {
        // Set the session state:
        m_sessionController.SetState(ppSessionController::PP_SESSION_STATE_RUNNING);

        // Open the database:
        StartDBConnection();

        // Open the view connection to profile events:
        m_pTimelineView->UpdateProfileState();

        // Open the view connection to profile events:
        m_pSummaryView->UpdateSessionInformation();

        ppAppController::instance().DisplayProcessRunningHTML();
    }
}

void ppSessionView::SetSessionFilePath(const osFilePath& sessionPath)
{
    // Set the file path in the controller:
    m_sessionController.SetDBFilePath(sessionPath);
}

void ppSessionView::OnProfileStopped(const QString& sessionName)
{
    GT_UNREFERENCED_PARAMETER(sessionName);

    if (m_sessionController.GetSessionState() == ppSessionController::PP_SESSION_STATE_RUNNING)
    {
        // Change the profile state to complete:
        m_sessionController.SetState(ppSessionController::PP_SESSION_STATE_COMPLETED);

        // Only after the state is updated, make the inner views respond to stop profile:
        GT_IF_WITH_ASSERT(m_pTimelineView != nullptr)
        {
            m_pTimelineView->OnProfileStopped(sessionName);
        }

        GT_IF_WITH_ASSERT(m_pSummaryView != nullptr)
        {
            m_pSummaryView->OnProfileStopped(sessionName);
        }
    }
}

void ppSessionView::OnCloseMdiWidget()
{
    osFilePath filePath = m_sessionController.DBFilePath();
    osDirectory fileDirectory;
    filePath.getFileDirectory(fileDirectory);

    // If the MDI window being closed is the one the contains the session that is currently executing, then stop the session execution.
    if (ppAppController::instance().GetExecutedSessionName() == ppAppController::instance().GetProjectNameFromSessionDir(fileDirectory))
    {
        SharedProfileManager::instance().stopCurrentRun();
    }
}


//++AT:LPGPU2DEMO_V
void ppSessionView::OnMultiSourceSourceSet()
{
//++CF:LPGPU2
  GT_IF_WITH_ASSERT(m_pTabWidget->count() > 2)
  {
    m_pTabWidget->setCurrentIndex(2);
  }
//--CF:LPGPU2
}
//--AT:LPGPU2DEMO_V

//++CF:LPGPU2
void ppSessionView::OnRegionOfInterestOpened(ppTimeLineView* pTimeline)
{
  // Connect frame open event:
  bool rc = connect(pTimeline, &ppTimeLineView::OnFrameBubbleClicked, 
    m_pFrameDetailView, &LPGPU2ppMultiFrameDetailView::DisplayFrameDetailSlot);
  GT_ASSERT(rc);
  
  GT_IF_WITH_ASSERT(m_pTabWidget->count() > 3)
  {
    m_pTabWidget->setCurrentIndex(3);
  }
}

void ppSessionView::OnFrameDetailOpened()
{
  GT_IF_WITH_ASSERT(m_pTabWidget->count() > 4)
  {
    m_pTabWidget->setCurrentIndex(4);
  }
}
//--CF:LPGPU2
