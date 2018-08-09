// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2RegionOfInterestView.cpp
///
/// @brief Defines the Feedback Analysis view widget. Used to display
///		   all the widgets to visualise the feedback engine results.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_FEEDBACK_ANALYSIS_VIEW_H_INCLUDE
#define LPGPU2_PP_FEEDBACK_ANALYSIS_VIEW_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

// Declarations
class QTabWidget;
class LPGPU2ppCallsPerTypeView;
class LPGPU2ppLongestRunningFunctionsView;
class LPGPU2ppCallSuggestionsView;
class LPGPU2ppShadersInRegionView;
class LPGPU2ppShaderAnalysisView;

namespace lpgpu2 {
enum class PPFnStatus;

namespace db {

struct CallsPerType;
struct CallSummary;
struct Annotation;
struct ShaderTrace;
enum class EShaderTraceAPIId;

} // namespace db
} // namespace lpgpu2


/// @brief   This is the widget used to display all the visualisations that
///		     uses the data produced by the Feedback Engine. This widget can
///		     be seen as a container for all other LPGPU2 widgets developed
///		     to display visualisations from data of a power profiling session.
/// @warning None.
/// @author  Thales Sabino.
/// @date    20/11/2017
// clang-format off
class LPGPU2ppFeedbackAnalysisView final : public QWidget
{
    Q_OBJECT
// Methods
public:
    explicit LPGPU2ppFeedbackAnalysisView(QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppFeedbackAnalysisView();

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

    lpgpu2::PPFnStatus SetCallsPerType(const lpgpu2::db::CallsPerType &vCallsPerType) const;
    lpgpu2::PPFnStatus SetCallsSummaryData(gtUInt64 vAccumulatedData, const gtVector<lpgpu2::db::CallSummary> &vCallSummaryList) const;
    lpgpu2::PPFnStatus SetAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations) const;
    lpgpu2::PPFnStatus SetGlobalAnnotations(
        const gtVector<lpgpu2::db::Annotation> &vAnnotations) const;
    lpgpu2::PPFnStatus SetUserAnnotations(
        const gtVector<lpgpu2::db::Annotation> &vAnnotations) const;
    lpgpu2::PPFnStatus SetShaderTraces(const gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces) const;

    lpgpu2::PPFnStatus AddShaderAnalysisTab(const lpgpu2::db::ShaderTrace &vShaderTrace);

// Signals
signals:
    void AnnotationSelected(gtUInt32 vAnnotationId);
    void ShaderTraceSeleted(const gtString& vShaderTraceId);
    void OpenShaderSourceRequested(gtUInt32 vFrameNum, gtUInt32 vDrawNum);

// Slots
private slots:
    void OnTabCloseRequested(int vTabIndex);

// Attributes
private:
    QMap<QString, LPGPU2ppShaderAnalysisView*> m_shaderAnalysisTabsMap;
    acTabWidget*                         m_pMainTabWidget = nullptr;
    LPGPU2ppCallsPerTypeView*            m_pCallsPerTypeView = nullptr;
    LPGPU2ppLongestRunningFunctionsView* m_pLongestRunningAPICallsView = nullptr;
    LPGPU2ppCallSuggestionsView*         m_pCallSuggestionsView = nullptr;
    LPGPU2ppShadersInRegionView*         m_pShadersInRegionView = nullptr;
};
// clang-format on

#endif // LPGPU2_PP_FEEDBACK_ANALYSIS_VIEW_H_INCLUDE
