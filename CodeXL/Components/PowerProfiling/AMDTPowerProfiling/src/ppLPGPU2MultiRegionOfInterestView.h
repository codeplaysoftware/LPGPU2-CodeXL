// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2MultiRegionOfInterestView.h
///
/// @brief 
///
/// ppLPGPU2MultiRegionOfInterestView class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __PPLPGPU2MULTIREGIONOFINTERESTVIEW_H
#define __PPLPGPU2MULTIREGIONOFINTERESTVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/LPGPU2ppInfoWidget.h>

// Forward declarations
class acTabWidget;
class ppSessionView;
class ppSessionController;
class ppLPGPU2RegionOfInterestView;
class ppTimeLineView;
class LPGPU2ppInfoWidget;
namespace lpgpu2 {
  enum class PPFnStatus;
}

/// @brief ...
class PP_API ppLPGPU2MultiRegionOfInterestView final : public QWidget
{
    Q_OBJECT

// Methods
public:
  ppLPGPU2MultiRegionOfInterestView(ppSessionView* pParent,
      ppSessionController* pSessionController);
  ~ppLPGPU2MultiRegionOfInterestView();
  // Resource handling
  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  // 
  lpgpu2::PPFnStatus DisplayRegionOfInterest(quint64 roiID);
  
// Methods
public slots:
  void DisplayRegionOfInterestSlot(quint64 roiID);
  
// Methods
signals:
  void RegionOfInterestOpened(ppTimeLineView* pTimeline);
  void ShaderSourceOpenRequested(gtUInt32 frameNum, gtUInt32 drawNum);
  
// Methods
private:
  lpgpu2::PPFnStatus InitViewLayout();
  
// Methods
private slots:
  void OnTabCloseRequestedSlot(int index);

// Attributes
private:
  QHBoxLayout*          m_pMainLayout;
  acTabWidget*          m_pTabWidgdet;
  ppSessionController*  m_pSessionController;
  ppSessionView*        m_pParentView;
  LPGPU2ppInfoWidget*   m_pInfoWidget;
  QHash<quint64, ppLPGPU2RegionOfInterestView*> m_openTabsByID;
  
};

#endif //__PPLPGPU2MULTIREGIONOFINTERESTVIEW_H
