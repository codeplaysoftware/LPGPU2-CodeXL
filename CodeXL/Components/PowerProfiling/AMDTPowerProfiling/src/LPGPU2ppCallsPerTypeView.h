// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines Calls Per Type View widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_CALLS_PER_TYPE_VIEW_H_INCLUDE
#define LPGPU2_PP_CALLS_PER_TYPE_VIEW_H_INCLUDE

// Qt:
#include <QWidget>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Declarations
namespace lpgpu2 { namespace db { struct CallsPerType; } }
class LPGPU2ppPieChartWidget;
class QGroupBox;
class QTableWidget;
class QSplitter;

/// @brief   Widget to display a lpgpu2::db::CallsPerType object. The Calls Per Type
///          will be displayed as a pie chart widget inside a QGroupBox.
/// @warning None.
/// @see     LPGPU2ppPieChartWidget
/// @author  Thales Sabino
/// @date    20/11/2017
// clang-format off
class LPGPU2ppCallsPerTypeView final : public QWidget
{
    Q_OBJECT

// Methods
public:
    explicit LPGPU2ppCallsPerTypeView(QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppCallsPerTypeView();

    lpgpu2::PPFnStatus Initialise();
    lpgpu2::PPFnStatus Shutdown();

    lpgpu2::PPFnStatus SetCallsPerType(const lpgpu2::db::CallsPerType &vCallsPerType) const;

// Slots
private slots:
    void OnPieChartItemClicked(const int vItemIndex) const;
    void OnTableWidgetCellChanged(int row, int column);

// Attributes
private:
    QGroupBox              *m_pGroupBox       = nullptr;
    LPGPU2ppPieChartWidget *m_pPieChartWidget = nullptr;
    QTableWidget           *m_pTableWidget    = nullptr;
    QSplitter              *m_pSplitter       = nullptr;
    bool                    m_isInitialised   = false;

// Methods
private:
    lpgpu2::PPFnStatus InitialiseTableWidgetLayout();
    void showEvent(QShowEvent *event) override;
};
// clang-format on

#endif // LPGPU2_PP_CALLS_PER_TYPE_VIEW_H_INCLUDE
