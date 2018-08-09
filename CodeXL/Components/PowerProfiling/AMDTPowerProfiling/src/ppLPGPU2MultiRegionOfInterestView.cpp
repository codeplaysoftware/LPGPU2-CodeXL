// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2MultiRegionOfInterestView.cpp
///
/// @brief 
///
/// MultiRegionOfInterestView implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/ppLPGPU2MultiRegionOfInterestView.h>

// Local:
#include <AMDTPowerProfiling/src/ppSessionController.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/src/ppLPGPU2RegionOfInterestView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>

// Framework:
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

using lpgpu2::PPFnStatus;
/// @brief Ctor. No work is done here
/// @param pParent The session view which owns this one
/// @param pSessionController The PP session controller
ppLPGPU2MultiRegionOfInterestView::ppLPGPU2MultiRegionOfInterestView(ppSessionView* pParent,
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
ppLPGPU2MultiRegionOfInterestView::~ppLPGPU2MultiRegionOfInterestView()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus ppLPGPU2MultiRegionOfInterestView::Initialise()
{
  return InitViewLayout();
}

/// @brief Deinitialise the object
PPFnStatus ppLPGPU2MultiRegionOfInterestView::Shutdown()
{
  // Qt takes care of deleting the Qt objects on the heap so we do not need to
  // to delete them ourselves

  return PPFnStatus::success;
}

/// @brief Initialise the layout of the widget by creating widget objects
PPFnStatus ppLPGPU2MultiRegionOfInterestView::InitViewLayout()
{
  PPFnStatus ret_status = PPFnStatus::success;

  // Create and set the layout
  LPGPU2PPNewQtWidget(&m_pMainLayout, this);
  m_pMainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pMainLayout);
  setVisible(false);
  
  // Create the info widget:
  LPGPU2PPNewQtLPGPU2Widget(&m_pInfoWidget, PP_STR_LPGPU2InfoNoRegionViewTitle, PP_STR_LPGPU2InfoNoRegionViewMsg, this);
  m_pMainLayout->addWidget(m_pInfoWidget);

  // Create tab widget used to hold all the ROI views
  LPGPU2PPNewQtWidget(&m_pTabWidgdet, this);
  m_pTabWidgdet->setTabsClosable(true);
  m_pTabWidgdet->setVisible(false);
  m_pMainLayout->addWidget(m_pTabWidgdet);

  // Connect the tab close event
  bool rc = connect(m_pTabWidgdet, &acTabWidget::tabCloseRequested, 
      this, &ppLPGPU2MultiRegionOfInterestView::OnTabCloseRequestedSlot);

  if (rc == false)
  {
    ret_status = PPFnStatus::failure;
  }
  
  return ret_status;
}

PPFnStatus ppLPGPU2MultiRegionOfInterestView::DisplayRegionOfInterest(quint64 roiID)
{
    // Switch focus to the ROI if it has already been opened:
    auto it = m_openTabsByID.find(roiID);

    if (it != m_openTabsByID.end())
    {
        m_pTabWidgdet->setCurrentWidget(it.value());
        return PPFnStatus::success;
    }
    
  // Hide the info label and show the MDI widget:
  m_pInfoWidget->setVisible(false);
  m_pTabWidgdet->setVisible(true);

  // Create a new ROI view
  ppLPGPU2RegionOfInterestView* pROIView = nullptr;
  LPGPU2PPNewQtLPGPU2Widget(&pROIView, this, m_pSessionController, roiID);
  
  pROIView->setGeometry(geometry());
  pROIView->ForceRangeUpdate();
  
  // Add the ROI view to the tab widget
  const int newIndex = m_pTabWidgdet->addTab(pROIView, QString{ "ROI #%0" }.arg(roiID));
  m_pTabWidgdet->setCurrentIndex(newIndex);
  
  m_openTabsByID[roiID] = pROIView;
  
  connect(pROIView, &ppLPGPU2RegionOfInterestView::OpenShaderSourceRequested, 
          this, &ppLPGPU2MultiRegionOfInterestView::ShaderSourceOpenRequested);
  
  return PPFnStatus::success;
}

/// @brief Called by the tab widget when the user tries to close a tab
/// @see   This function does not use the PPFnStatus return value idiom
///        because it needs to interface with Qt
/// @param index The index of the tab in the tab widget
void ppLPGPU2MultiRegionOfInterestView::OnTabCloseRequestedSlot(int index)
{ 
    
  auto *pROIView = qobject_cast<ppLPGPU2RegionOfInterestView*>(m_pTabWidgdet->widget(index));
  if (pROIView != nullptr)
  {
      const quint64 roiID = m_openTabsByID.key(pROIView);      
      m_openTabsByID.remove(roiID);

      delete pROIView;
      pROIView = nullptr;
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
void ppLPGPU2MultiRegionOfInterestView::DisplayRegionOfInterestSlot(quint64 roiID)
{
  PPFnStatus status = DisplayRegionOfInterest(roiID);
  GT_ASSERT(status == PPFnStatus::success);

  // Inform listeners that the tab was opened successfully
  emit RegionOfInterestOpened(m_openTabsByID[roiID]->GetTimeline());
}
