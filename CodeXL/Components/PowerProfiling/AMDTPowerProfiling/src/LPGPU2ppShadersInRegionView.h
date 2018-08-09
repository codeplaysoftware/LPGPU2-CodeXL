// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Shaders in Region view widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_SHADERS_IN_REGION_VIEW_H_INCLUDE
#define LPGPU2_PP_SHADERS_IN_REGION_VIEW_H_INCLUDE

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Declarations
namespace lpgpu2 { namespace db { struct ShaderTrace; } }
class QTableWidget;
class QTableWidgetItem;
class LPGPU2ppInfoWidget;

/// @brief   Represents the widget to display the shaders in region in order
///          to display lpgpu2::db::Shader traces objects. This widget is
///          basically a QTableWidget to display the shader traces.
/// @warning None.
/// @author  Thales Sabino.
/// @date    20/11/2017
// clang-format off
class PP_API LPGPU2ppShadersInRegionView : public QWidget
{
    Q_OBJECT

public:
    explicit LPGPU2ppShadersInRegionView(QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppShadersInRegionView();

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

    lpgpu2::PPFnStatus SetShaderTraces(const gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces) const;

signals:
    void ShaderSelected(const gtString& vShaderId);

private:
    lpgpu2::PPFnStatus InitialiseTableWidgetLayout() const;

private slots:
    void OnTableItemClicked(QTableWidgetItem *vpTableItem);

private:
    QTableWidget       *m_pTableWidget = nullptr;
    LPGPU2ppInfoWidget *m_pInfoWidget = nullptr;
};
// clang-format on

#endif // LPGPU2_PP_SHADERS_IN_REGION_VIEW_H_INCLUDE