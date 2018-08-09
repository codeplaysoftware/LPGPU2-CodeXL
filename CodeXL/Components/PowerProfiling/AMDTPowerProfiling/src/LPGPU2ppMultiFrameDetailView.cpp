// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppMultiFrameDetailView.cpp
///
/// @brief 
///
/// LPGPU2ppMultiFrameDetailView implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppMultiFrameDetailView.h>

// Local:
#include <AMDTPowerProfiling/src/ppSessionController.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFrameDetailView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppInfoWidget.h>

// Framework:
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

using lpgpu2::PPFnStatus;

/// @brief Ctor. No work is done here
/// @param pParent The session view which owns this one
/// @param pSessionController The PP session controller
LPGPU2ppMultiFrameDetailView::LPGPU2ppMultiFrameDetailView(ppSessionView* pParent,
  ppSessionController* pSessionController) :
    QWidget(nullptr)
    ,m_pMainLayout(nullptr)
    ,m_pTabWidgdet(nullptr)
    ,m_pSessionController(pSessionController)
    ,m_pParentView(pParent)
{
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
//         we need to make sure that Shutdown was called anyway.
LPGPU2ppMultiFrameDetailView::~LPGPU2ppMultiFrameDetailView()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus LPGPU2ppMultiFrameDetailView::Initialise()
{
  return InitViewLayout();
}

/// @brief Deinitialise the object
PPFnStatus LPGPU2ppMultiFrameDetailView::Shutdown()
{
  // Qt takes care of deleting the Qt objects on the heap so we do not need to
  // to delete them ourselves

  return PPFnStatus::success;
}

/// @brief Initialise the layout of the widget by creating widget objects
PPFnStatus LPGPU2ppMultiFrameDetailView::InitViewLayout()
{
  PPFnStatus ret_status = PPFnStatus::success;

  // Create and set the layout
  LPGPU2PPNewQtWidget(&m_pMainLayout, this);
  m_pMainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pMainLayout);
  setGeometry(m_pParentView->geometry());
  setVisible(false);

  // Create tab widget used to hold all the frame views
  LPGPU2PPNewQtWidget(&m_pTabWidgdet, this);
  m_pTabWidgdet->setTabsClosable(true);
  m_pTabWidgdet->setVisible(false);
  m_pMainLayout->addWidget(m_pTabWidgdet);
  
  LPGPU2PPNewQtLPGPU2Widget(&m_pInfoWidget, QString(PP_STR_LPGPU2InfoNoFrameViewTitle), QString(PP_STR_LPGPU2InfoNoFrameViewMsg), this);
  m_pMainLayout->addWidget(m_pInfoWidget);

  // Connect the tab close event
  bool rc = connect(m_pTabWidgdet, &acTabWidget::tabCloseRequested, 
      this, &LPGPU2ppMultiFrameDetailView::OnTabCloseRequestedSlot);

  if (rc == false)
  {
    ret_status = PPFnStatus::failure;
  }
  
  return ret_status;
}

PPFnStatus LPGPU2ppMultiFrameDetailView::DisplayFrameDetail(quint64 frameID)
{
  // Display the MDI if hidden, and hide the info widget:
  m_pTabWidgdet->setVisible(true);
  m_pInfoWidget->setVisible(false);
  
  // Switch focus to the frame if it has already been opened:
  auto it = m_openTabsByID.find(frameID);

  if (it != m_openTabsByID.end())
  {
      m_pTabWidgdet->setCurrentWidget(it.value());
      return PPFnStatus::success;
  }

  // Create a new frame view
  LPGPU2ppFrameDetailView* pFrameView = nullptr;
  LPGPU2PPNewQtLPGPU2Widget(&pFrameView, this, m_pSessionController, frameID);
  
  // Add the frame view to the tab widget
  const int newIndex = m_pTabWidgdet->addTab(pFrameView, QString{ "Frame #%0" }.arg(frameID));
  m_pTabWidgdet->setCurrentIndex(newIndex);
  
  m_openTabsByID[frameID] = pFrameView;
  
  // Update geometry to fix acTimelineItem display:
  setGeometry(m_pParentView->geometry());
  m_pTabWidgdet->setGeometry(geometry());
  pFrameView->setGeometry(geometry());    
  pFrameView->ForceRangeUpdate();
 
  // Trigger zooming in so that the ribbon shows a view of all of its contents
  pFrameView->TriggerZoom();
  
  // Connect the Region of Interest Button event
  bool rc =  connect(pFrameView, &LPGPU2ppFrameDetailView::RegionButtonClicked,
                     this, &LPGPU2ppMultiFrameDetailView::OnRegionButtonClicked);
  if (!rc)
  {
    return PPFnStatus::failure;
  }
  
  return PPFnStatus::success;
}

/// @brief Called by the tab widget when the user tries to close a tab
/// @see   This function does not use the PPFnStatus return value idiom
///        because it needs to interface with Qt
/// @param index The index of the tab in the tab widget
void LPGPU2ppMultiFrameDetailView::OnTabCloseRequestedSlot(int index)
{ 
    
  auto *pFrameView = qobject_cast<LPGPU2ppFrameDetailView*>(m_pTabWidgdet->widget(index));
  if (pFrameView != nullptr)
  {
      const quint64 frameID = m_openTabsByID.key(pFrameView);      
      m_openTabsByID.remove(frameID);

      delete pFrameView;
      pFrameView = nullptr;
  }  
}

/// @brief Function called externally by other widgets to request the
///        display of a particular Region of Interest.
///        Open a file using the entry in the database
///        (using the session controller) and show it or focus the relative
///        tab
/// @see   This function does not use the PPFnStatus return value idiom
///        because it needs to interface with Qt
/// @param[in] roiID The id within the databases's region of interest table.
void LPGPU2ppMultiFrameDetailView::DisplayFrameDetailSlot(quint64 frameID)
{
  PPFnStatus status = DisplayFrameDetail(frameID);
  GT_ASSERT(status == PPFnStatus::success);

  // Inform listeners that the tab was opened successfully
  emit FrameDetailOpened();
}

/// @brief Called when a contained FrameDetailView requests a region of interest
///        to be opened. Connected by the session view.
void LPGPU2ppMultiFrameDetailView::OnRegionButtonClicked(quint64 regionID)
{
  emit RegionButtonClicked(regionID);
}
