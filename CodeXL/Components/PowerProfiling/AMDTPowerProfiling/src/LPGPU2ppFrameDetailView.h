// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameDetailView.h
///
/// LPGPU2ppFrameDetailView declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPFRAMEDETAILVIEW_H
#define __LPGPU2PPFRAMEDETAILVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QWidget>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>

// Forward declarations
namespace lpgpu2 { enum class PPFnStatus; }
class ppSessionController;
class LPGPU2ppFrameAPICallTable;
class LPGPU2ppFrameCounterSummaryTable;

/// @brief    This is a widget that contains timeline views of API calls
///           specific to a given frame.
/// @see      ppTimeLineView, LPGPU2ppMultiFrameDetailView
/// @date     02/11/17
/// @author   Callum Fare
class PP_API LPGPU2ppFrameDetailView final : public QWidget
{
    Q_OBJECT

// Typedefs
using VecTrace_t = gtVector<lpgpu2::db::ProfileTraceInfo>;
using VecStackParams_t = gtVector<lpgpu2::db::StackTraceParameter>;

// Methods
public:
  LPGPU2ppFrameDetailView(QWidget* pParent, ppSessionController* pSessionController, gtUInt32 frameID);
  virtual ~LPGPU2ppFrameDetailView();

  // Resource handling
  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  void       ForceRangeUpdate();
  void       AddWidgetToLayout(LPGPU2ppRibbonWidgetWrapper* wrapper, QLayout* layout);
  lpgpu2::PPFnStatus LoadAPICalls();
  lpgpu2::PPFnStatus ShowTimelineItem(quint64 drawNum);
  lpgpu2::PPFnStatus TriggerZoom();
  
// Methods
private:
  lpgpu2::PPFnStatus InitViewLayout();

private slots:
    void OnAPICallSelected(quint64 drawNum);
    void OnAPITimelineItemClicked(acTimelineItem* item);
    void OnRegionButtonClicked();
    
signals:
    void RegionButtonClicked(quint64 regionNum);

// Attributes
private:
  QWidget*                              m_pParent = nullptr;
  QWidget*                              m_pBottomWidget = nullptr;
  QWidget*                              m_pButtonContainer = nullptr;
  QPushButton*                          m_pRegionButton = nullptr;
  LPGPU2ppFrameAPICallTable*            m_pCallTable = nullptr;
  LPGPU2ppFrameCounterSummaryTable*     m_pCounterTable = nullptr;

  // Keep a pointer to the ppTimelineView in the parent session to read data etc:
  ppSessionController* m_pSessionController;
  
  gtUInt32                      m_frameID;
  gtUInt64                      m_regionID;
  gtUInt64                      m_frameEndTime;
  VecTrace_t                    m_vecTraces;
  VecStackParams_t              m_vecParams;
  LPGPU2ppRibbonTimelineWrapper m_timeline;
  
  std::map<quint64, acTimelineItem*>                            m_mapDrawNumtoItemPtr;
  std::map<lpgpu2::db::EShaderTraceAPIId, acTimelineBranch*> m_mapAPICategoryToBranch;
  
  std::map<lpgpu2::db::EShaderTraceAPIId, gtString> m_mapAPICategoryToAPIName;
}; //LPGPU2ppFrameDetailView

#endif // __LPGPU2PPFRAMEDETAILVIEW_H
