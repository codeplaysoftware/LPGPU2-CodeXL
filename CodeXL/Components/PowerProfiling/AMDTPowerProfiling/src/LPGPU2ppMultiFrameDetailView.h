// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppMultiFrameDetailView.h
///
/// @brief 
///
/// LPGPU2ppMultiFrameDetailView class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPMULTIFRAMEDETAILVIEW_H
#define __LPGPU2PPMULTIFRAMEDETAILVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Forward declarations
namespace lpgpu2 { enum class PPFnStatus; }
class acTabWidget;
class ppSessionView;
class ppSessionController;
class LPGPU2ppFrameDetailView;
class LPGPU2ppInfoWidget;

/// @brief MDI for containing multiple Frame Detail Views.
class PP_API LPGPU2ppMultiFrameDetailView final : public QWidget
{
    Q_OBJECT

// Methods
public:
  LPGPU2ppMultiFrameDetailView(ppSessionView* pParent,
      ppSessionController* pSessionController);
  ~LPGPU2ppMultiFrameDetailView();
  // Resource handling
  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  // 
  lpgpu2::PPFnStatus DisplayFrameDetail(quint64 frameID);
  
// Methods
public slots:
  void DisplayFrameDetailSlot(quint64 frameID);
  void OnRegionButtonClicked(quint64 regionID);
  
// Methods
signals:
  void FrameDetailOpened();
  void RegionButtonClicked(quint64 regionID);
  
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
  QHash<quint64, LPGPU2ppFrameDetailView*> m_openTabsByID;
  
};

#endif //__LPGPU2PPMULTIFRAMEDETAILVIEW_H