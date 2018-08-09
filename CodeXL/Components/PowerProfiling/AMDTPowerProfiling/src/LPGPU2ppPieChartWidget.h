// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines a Pie Chart Widget to be used within the Power Profiler context.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_PIECHARTWIDGET_H_INCLUDE
#define LPGPU2_PP_PIECHARTWIDGET_H_INCLUDE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Qt:
#include <QWidget>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

/// @brief   Defines a Pie Chart Widget capable of displaying an arbitrary number
///          of data points with colours and allows selection of individual sections.
///          Clients of this class can connect to the ItemClicked signal to know when
///          an item was clicked
/// @warning None.
/// @author  Thales Sabino
/// @date    15/11/2017
// clang-format off
class PP_API LPGPU2ppPieChartWidget : public QWidget
{
    Q_OBJECT

// Declarations
public:

    /// @brief A struct to define a data point that can be
    ///        displayed in this widget.
    struct DataPoint
    {
        gtFloat32 m_value;
        QColor    m_colour;
        bool      m_bIsSelected = false;
        gtString  m_tooltip;
        gtString  m_label;
    };

// Methods
public:
    explicit LPGPU2ppPieChartWidget(QWidget *vpParent = nullptr);
    virtual ~LPGPU2ppPieChartWidget();

    lpgpu2::PPFnStatus AddDataPoint(const DataPoint &vDataPoint);

    lpgpu2::PPFnStatus SetSelectionColor(const QColor &vColor);
    lpgpu2::PPFnStatus SetPointSelection(int vItemIndex, bool vbSelected);

// Slots
public slots:
    lpgpu2::PPFnStatus ClearData();
    lpgpu2::PPFnStatus ClearSelection();

// Signals
signals:
    void ItemClicked(int vItemIndex);

// Methods
protected:    

    // From QWidget
    void paintEvent(QPaintEvent *vpPaintEvent) override;    
    void mouseMoveEvent(QMouseEvent *vpMouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *vpMouseEvent) override;
    void resizeEvent(QResizeEvent *vpResizeEvent) override;

// Methods:
private:       
    QRect GetPieWidgetRect() const;    
    gtFloat32 GetPieChartRadius() const;
    int GetItemIndexAtPoint(const QPoint &vPoint) const;
    lpgpu2::PPFnStatus CalculatePieChartSectorAngles();
    void DrawPies(QPainter &vPainter, const QRect &vDrawRect);
    void DrawLabelsAndValues(QPainter &vPainter, const QRect &vDrawRect);

// Attributes
private:
    gtVector<gtFloat32> m_pieSectorAngles;        
    gtVector<DataPoint> m_chartData;
    QColor              m_selectionColor       = QColor{ Qt::red };
    bool                m_chartDataChanged     = false;
    gtFloat32           m_totalValue           = 0.0f;    
    bool                m_bShowLabelsAndValues = true;    
};
// clang-format on

#endif // LPGPU2_PP_PIECHARTWIDGET_H_INCLUDE
