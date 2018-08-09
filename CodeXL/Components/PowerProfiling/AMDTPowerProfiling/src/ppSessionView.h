//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionView.h
///
//==================================================================================

//------------------------------ ppSessionView.h ------------------------------

#ifndef __PPSESSIONVIEW_H
#define __PPSESSIONVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppSessionController.h>

class ppSummaryView;
class ppTimeLineView;
//++AT:LPGPU2DEMO_V
class LPGPU2ppMultiSourceCodeView;
//--AT:LPGPU2DEMO_V
//++CF:LPGPU2
class ppLPGPU2MultiRegionOfInterestView;
class LPGPU2ppMultiFrameDetailView;
//--CF:LPGPU2
//++TLRS: LPGPU2
class LPGPU2ppSessionComparisonView;
//--TLRS: LPGPU2

enum ppSessionViewsTabIndex
{
    PP_TAB_TIMELINE_INDEX = 0,
    PP_TAB_SUMMARY_INDEX
};


class PP_API ppSessionView : public SharedSessionWindow
{
    Q_OBJECT

public:
    ppSessionView(QWidget* pParent, ppSessionController::SessionState state);
    virtual ~ppSessionView();

    /// Display a specific tab in the view
    void DisplayTab(int tabIndex);

    /// Set the view data that will enable displaying the tab views:
    /// \param sessionPath the session path
    void SetViewData(const osFilePath& sessionPath);

    /// Starts the database connection:
    void StartDBConnection();

    /// Stops the database connection:
    void StopDBConnection();

    /// Set the session path. Is called when a session is renamed:
    /// \param sessionPath the session path (the offpp file)
    void SetSessionFilePath(const osFilePath& sessionPath);

    /// Activate the current session (start listening to profile events):
    void ActivateSession();

    /// Get a reference to the session controller:
    ppSessionController& SessionController() { return m_sessionController; };

    /// handle closing of view from VS
    void OnCloseMdiWidget();

public slots:

    /// Is handling profile stopped signal:
    void OnProfileStopped(const QString& sessionName);

//++AT:LPGPU2DEMO_V
    /// Called when a source was openend or focussed.
    /// Switches to the multi source code view tab
    void OnMultiSourceSourceSet();
//--AT:LPGPU2DEMO_V

//++CF:LPGPU2
    /// Called when a region of interest was opened or focussed.
    /// Switches focus to the multi region of interest tab.
    /// Also connects frame view event for ROI timeline.
    void OnRegionOfInterestOpened(ppTimeLineView* pTimeline);
    
    /// Called when a frame detail view was opened or focussed.
    /// Switches focus to the multi region of interest tab.
    void OnFrameDetailOpened();
//--CF:LPGPU2

private:
    // Get Session info based on the sessionPath:
    void UpdateSessionInfo(const osFilePath& sessionPath, gtString& sessionName, gtString& dbFullPath);

    /// Main view layout:
    QLayout* m_pMainLayout;

    /// Holds all the internal views:
    acTabWidget* m_pTabWidget;

    /// Holds the timeline view:
    ppTimeLineView* m_pTimelineView;

    /// Holds the summary view:
    ppSummaryView* m_pSummaryView;

//++AT:LPGPU2DEMO_V
    /// Holds the source code pages view:
    LPGPU2ppMultiSourceCodeView* m_pMultiSourceCodeView;
//--AT:LPGPU2DEMO_V

//++CF:LPGPU2
    /// Holds the Regions of Interest view:
    ppLPGPU2MultiRegionOfInterestView* m_pRegionsOfInterestView;
    
    /// Holds the Frame Detail view:
    LPGPU2ppMultiFrameDetailView* m_pFrameDetailView;
//--CF:LPGPU2

    //++TLRS: LPPGU2
    LPGPU2ppSessionComparisonView *m_pSessionComparisonView;
    //--TLRS: LPGPU2

    /// The object used for session data control:
    ppSessionController m_sessionController;

};

#endif // __PPSESSIONVIEW_H
