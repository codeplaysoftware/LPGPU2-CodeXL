// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2RegionOfInterestView.h
///
/// @brief View widget to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// ppLPGPU2RegionOfInterestView declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __PPLPGPU2REGIONOFINTERESTVIEW_H
#define __PPLPGPU2REGIONOFINTERESTVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QWidget>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>

// Forward declarations
namespace lpgpu2 {
  enum class PPFnStatus;
}
class ppSessionController;
class LPGPU2ppFeedbackAnalysisView;

/// @brief    This is a widget that, similar to ppTimelineView, contains timeline
///           widgets, but specific to a particular Region of Interest.
/// @see      ppTimeLineView, ppLPGPU2MultiRegionOfInterestView
/// @date     04/10/17
/// @author   Callum Fare
class PP_API ppLPGPU2RegionOfInterestView final : public QWidget
{
    Q_OBJECT

// Methods
public:
  ppLPGPU2RegionOfInterestView(QWidget* pParent, ppSessionController* pSessionController, gtUInt32 regionOfInterestID);
  virtual ~ppLPGPU2RegionOfInterestView();

  // Resource handling
  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  void       ForceRangeUpdate();
  
  ppTimeLineView* GetTimeline();
  
signals:
  void OpenShaderSourceRequested(gtUInt32 vFrameNum, gtUInt32 vDrawNum);
  
// Methods
private:
  lpgpu2::PPFnStatus InitViewLayout();
  lpgpu2::PPFnStatus InitFeedbackAnalysisView();

private slots:
    void OnAnnotationSelected(gtUInt32 vAnnotationId);
    void OnShaderTraceSelected(const gtString& vShaderId);

// Attributes
private:
  ppTimeLineView*                       m_pTimelineView;
  LPGPU2ppFeedbackAnalysisView*         m_pFeedbackAnalysisView;
  QWidget*                              m_pParent;

  // Keep a pointer to the ppTimelineView in the parent session to read data etc:
  ppSessionController* m_pSessionController;
  
  gtUInt32                          m_regionOfInterestID;  
  lpgpu2::db::RegionOfInterest      m_regionOfInterest;
  gtVector<lpgpu2::db::Annotation>  m_Annotations;
  gtVector<lpgpu2::db::Annotation>  m_globalAnnotations;
  gtVector<lpgpu2::db::Annotation>  m_userAnnotations;
  gtVector<lpgpu2::db::ShaderTrace> m_ShaderTraces;
};

#endif // __PPLPGPU2REGIONOFINTERESTVIEW_H
