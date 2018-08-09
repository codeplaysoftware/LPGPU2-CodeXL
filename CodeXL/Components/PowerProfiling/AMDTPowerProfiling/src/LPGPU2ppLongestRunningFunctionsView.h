// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Longest running functions widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_LONGESTRUNNINGFUNCTIONSVIEW_H_INCLUDE
#define LPGPU2_PP_LONGESTRUNNINGFUNCTIONSVIEW_H_INCLUDE

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

namespace lpgpu2 { namespace db { struct CallSummary; } }
class QGroupBox;
class QTableWidget;

/// @brief      Represents the widget to display the longest running functions from
///             a list of lpgpu2::db::CallSummary's. This widget is basically a group
///             box with a table inside it.
/// @warning    None.
/// @author     Thales Sabino.
/// @date       20/11/2017
// clang-format off
class PP_API LPGPU2ppLongestRunningFunctionsView final : public QWidget
{
    Q_OBJECT

// Methods
public:
    explicit LPGPU2ppLongestRunningFunctionsView(QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppLongestRunningFunctionsView();

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

    lpgpu2::PPFnStatus SetCallSummaryData(const gtUInt64 vAccumulatedTime, const gtVector<lpgpu2::db::CallSummary> &vCallSummaryList) const;

// Methods
private:
    lpgpu2::PPFnStatus InitialiseTableWidgetLayout() const;

// Attributes
private:    
    QGroupBox    *m_pGroupBox = nullptr;
    QTableWidget *m_pTableWidget = nullptr;
};
// clang-format on

#endif