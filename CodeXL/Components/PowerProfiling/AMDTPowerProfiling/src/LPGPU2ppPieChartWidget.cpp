// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines a Pie Chart Widget to be used within the Power Profiler context.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppPieChartWidget.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Qt:
#include <QPainter>
#include <QtMath>

using lpgpu2::PPFnStatus;

static const int g_MinSizeToDrawLabels = 100;
static const int g_LabelsFontSize = 12;
static const int g_LabelsPenLineWidth = 4;
static const int g_SelectionLineWidth = 3;

static constexpr qreal kRatioInternalWhiteCircle = 0.7;
static constexpr int kTotalLabelFontSize = 20;

/// @brief Class constructor. Enable mouse tracking so we can display tooltips
///        when appropriate and set the size policy to MinimumExpanding.
/// @param[in] vpParent The parent widget.
LPGPU2ppPieChartWidget::LPGPU2ppPieChartWidget(QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

/// @brief  Class destructor. Clear the data vectors.
LPGPU2ppPieChartWidget::~LPGPU2ppPieChartWidget()
{
    m_pieSectorAngles.clear();
    m_chartData.clear();
}

/// @brief  Adds a data point to the pie chart and triggers an update. This
///         method can be called while the chart is on the screen.
/// @param[in] vDataPoint The data point to insert into the pie chart.
/// @return    PPFnStatus success: The point was added to the chart,
///                       failure: An error has occurred while adding point.
PPFnStatus LPGPU2ppPieChartWidget::AddDataPoint(const DataPoint &vDataPoint)
{
    try
    {
        m_chartData.push_back(vDataPoint);
    }
    catch (...)
    {
        return PPFnStatus::failure;
    }    

    m_chartDataChanged = true;
    update();

    return PPFnStatus::success;
}

/// @brief Clear all the data points and redraws the chart.
/// @return PPFnStatus Always return success.
PPFnStatus LPGPU2ppPieChartWidget::ClearData()
{
    m_chartData.clear();
    update();
    return PPFnStatus::success;
}

/// @brief  Clear the select state of all data points.
/// @return PPFnStatus always return success.
PPFnStatus LPGPU2ppPieChartWidget::ClearSelection()
{
    for (auto& dataPoint : m_chartData)
    {
        dataPoint.m_bIsSelected = false;
    }

    update();

    return PPFnStatus::success;
}

/// @brief                 Set the color to be used as the selection highlight.
/// @param[in] vColor      The new selection color.
/// @return    PPFnStauts  always return success.
PPFnStatus LPGPU2ppPieChartWidget::SetSelectionColor(const QColor &vColor)
{
    m_selectionColor = vColor;
    update();

    return PPFnStatus::success;
}

/// @brief                Set a data point as selected. Multiple data points can be
///                       selected at the same time. The client code need to take care
///                       of the selection behaviour.
/// @param[in] vItemIndex The index of the data point to have the selection status changed.
/// @param[in] vbSelected The new selection state of the specified data point.
/// @return    PPFnStatus success: the data point was updated,
///                       failure: vItemIndex is not a valid data point index.
PPFnStatus LPGPU2ppPieChartWidget::SetPointSelection(const int vItemIndex, const bool vbSelected)
{
    auto bReturn = PPFnStatus::failure;

    if (vItemIndex >= 0 && vItemIndex < static_cast<int>(m_chartData.size()))
    {
        m_chartData[vItemIndex].m_bIsSelected = vbSelected;
        update();

        bReturn = PPFnStatus::success;
    }    

    return bReturn;
}

/// @brief  Draw the pie chart in the widget area.
/// @param[in]  vpPaintEvent The paint event to be handled.
void LPGPU2ppPieChartWidget::paintEvent(QPaintEvent *vpPaintEvent)
{
    QWidget::paintEvent(vpPaintEvent);

    if(vpPaintEvent != nullptr)
    {
        if (m_chartDataChanged)
        {
            // If an error occurred here, keep m_chartDataChanged to try again
            // in the next paintEvent.
            if(CalculatePieChartSectorAngles() == PPFnStatus::success)
            {                
                m_chartDataChanged = false;
            }            
        }

        // There must be exactly one sector more than data
        if (m_pieSectorAngles.empty() || m_chartData.size() + 1 != m_pieSectorAngles.size())
        {
            return;
        }

        QPainter painter{ this };
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
        const auto drawRect = GetPieWidgetRect();

        // Always draw the pies
        DrawPies(painter, drawRect);

        // Only draw the labels and values if there is enough space
        if (m_bShowLabelsAndValues)
        {
            DrawLabelsAndValues(painter, drawRect);
        }
    }    
}

/// @brief  Handles the mouse release event. Used to select a data point.
/// @param[in] vpMouseEvent The QMouseEvent to handle.
void LPGPU2ppPieChartWidget::mouseReleaseEvent(QMouseEvent *vpMouseEvent)
{
    if (vpMouseEvent != nullptr)
    {
        if (vpMouseEvent->button() == Qt::LeftButton)
        {            
            const auto itemIndex = GetItemIndexAtPoint(vpMouseEvent->pos());
            if (itemIndex != -1)
            {
                emit ItemClicked(itemIndex);
            }
        }        
    }

    // Let the base widget handle the event as well
    QWidget::mouseReleaseEvent(vpMouseEvent);
}

/// @brief  Handles the mouse move event. Used to display the correct
///         tooltip depending on the hovered data point.
/// @param[in] vpMouseEvent The mouse event to handle.
void LPGPU2ppPieChartWidget::mouseMoveEvent(QMouseEvent *vpMouseEvent)
{
    if (vpMouseEvent != nullptr)
    {
        const auto itemIndex = GetItemIndexAtPoint(vpMouseEvent->pos());        
        if (itemIndex >= 0 && static_cast<size_t>(itemIndex) < m_chartData.size())
        {
            // If the mouse is hovering a data point, display its tooltip
            const auto& dataPoint = m_chartData[itemIndex];
            setToolTip(acGTStringToQString(dataPoint.m_tooltip));
        }
        else
        {  
            // Otherwise clear the tooltip text
            setToolTip("");
        }
    }

    // Let the base widget handle the event as well
    QWidget::mouseMoveEvent(vpMouseEvent);
}

/// @brief  Handle the widget resize event. Used to verify if there
///         is enough space to draw the labels and values.
/// @param[in]  vpResizeEvent The QResizeEvent to handle
void LPGPU2ppPieChartWidget::resizeEvent(QResizeEvent *vpResizeEvent)
{
    if (vpResizeEvent != nullptr)
    {
        const auto newSize = vpResizeEvent->size();
        const auto minSize = qMin(newSize.width(), newSize.height());
        m_bShowLabelsAndValues = minSize > g_MinSizeToDrawLabels;
    }    

    // Let the base widget handle the event as well
    QWidget::resizeEvent(vpResizeEvent);
}

/// @brief  Returns the actual rectangle used to draw the widget.
/// @return QRect   The rectangle that represents the useful area
///                 used to draw the pie chart.
QRect LPGPU2ppPieChartWidget::GetPieWidgetRect() const
{
    const auto windowRect = rect();
    QRect drawRect;

    if (windowRect.width() > windowRect.height())
    {
        const auto delta = (windowRect.width() - windowRect.height()) / 2;
        drawRect.setCoords(delta, 0, delta + windowRect.height() - 1, windowRect.height() - 1);
    }
    else
    {
        const auto delta = (windowRect.height() - windowRect.width()) / 2;
        drawRect.setCoords(0, delta, windowRect.width() - 1, delta + windowRect.width() - 1);
    }

    return drawRect;
}

gtFloat32 LPGPU2ppPieChartWidget::GetPieChartRadius() const
{
    const auto pieChartRect = GetPieWidgetRect();
    return qMax(pieChartRect.width(), pieChartRect.height()) / 2.2f;
}

/// @brief            Returns the index of a data point under the position in pixels.
/// @param[in] vPoint The position, in pixels, to query for the data point index.
/// @return    int    The index of the data point of -1 if vPoint is not a valid
///                   position inside the pie chart.
int LPGPU2ppPieChartWidget::GetItemIndexAtPoint(const QPoint &vPoint) const
{
    if (!m_pieSectorAngles.empty())
    {
        const auto pieChartRect = GetPieWidgetRect();
        const auto pieChartRadius = GetPieChartRadius();

        const QLineF mouseLine{ pieChartRect.center(), vPoint };
        const auto mouseLineAngleDegrees = mouseLine.angle();

        for (decltype(m_pieSectorAngles)::size_type iSectorIndex = 0; iSectorIndex < m_pieSectorAngles.size() - 1; ++iSectorIndex)
        {
            const auto startAngleDegress = m_pieSectorAngles[iSectorIndex];
            const auto endAngleDegress = m_pieSectorAngles[iSectorIndex + 1];

            if (startAngleDegress <= mouseLineAngleDegrees && mouseLineAngleDegrees <= endAngleDegress)
            {
                if (mouseLine.length() <= pieChartRadius)
                {
                    return iSectorIndex;
                }
            }
        }
    }

    return -1;  
}

/// @brief             Calculates the angles of each pie sector in order to speedup
///                    the drawing. Stores the sectors in m_pieSectorAngles. 
/// @return PPFnStatus success: All angles were calculated and stored,
///                    failure: An error has occurred while trying to store the data.
PPFnStatus LPGPU2ppPieChartWidget::CalculatePieChartSectorAngles()
{
    // Get the sum of all data point values
    m_totalValue = std::accumulate(m_chartData.begin(), m_chartData.end(), 0.0f, [](const gtFloat32 value, const DataPoint &dataPoint)
    {
        return value + dataPoint.m_value;
    });    

    try
    {
        m_pieSectorAngles.clear();

        auto cumulativeAngle = 0.0f;

        m_pieSectorAngles.push_back(cumulativeAngle);

        for (const auto& dataPoint : m_chartData)
        {
            const auto pieAngle = dataPoint.m_value * 360 / m_totalValue;
            cumulativeAngle += pieAngle;
            m_pieSectorAngles.push_back(cumulativeAngle);
        }
    }
    catch(...)
    {
        return PPFnStatus::failure;
    }    

    return PPFnStatus::success;
}

/// @brief  Draw the pies of the pie chart.
/// @param[in] vPainter The painter that will be used to draw the pies.
/// @param[in] vDrawRect The rectangle where the pie chart should be drawn.
void LPGPU2ppPieChartWidget::DrawPies(QPainter &vPainter, const QRect &vDrawRect)
{
    vPainter.save();
      
    // Create a smaller rectangle by 30%
    auto smallerWidth = vDrawRect.width() * kRatioInternalWhiteCircle;
    auto smallerHeight = vDrawRect.height() * kRatioInternalWhiteCircle;
    auto smallX = static_cast<qreal>(
        vDrawRect.x() + ((vDrawRect.width() - smallerWidth) / 2.0));
    auto smallY = static_cast<qreal>(
      vDrawRect.y() + ((vDrawRect.height() - smallerHeight) / 2.0));


    for (decltype(m_chartData)::size_type iItemIndex = 0; iItemIndex < m_chartData.size(); ++iItemIndex)
    {
        const auto& dataPoint = m_chartData[iItemIndex];

        auto startAngleDegrees = m_pieSectorAngles[iItemIndex];
        auto endAngleDegress = m_pieSectorAngles[iItemIndex + 1];

        auto dataColor = dataPoint.m_colour;
        dataColor.setAlpha(200);

        const QBrush pieBrush{ dataColor };
        const auto originalPen = vPainter.pen();

        if (dataPoint.m_bIsSelected)
        {
            QPen piePen{ m_selectionColor };
            piePen.setWidth(g_SelectionLineWidth);
            vPainter.setPen(piePen);
        }
        else
        {
            vPainter.setPen(QPen{ Qt::lightGray });
        }

        vPainter.setBrush(pieBrush);

        // According to Qt documentation:
        // The startAngle and spanAngle must be specified in 1/16th of a degree, i.e. a full circle equals 5760 (16 * 360)
        // http://doc.qt.io/qt-4.8/qpainter.html#drawPie
        // so multiply by 16 here to get the right value
        startAngleDegrees *= 16.0f;
        endAngleDegress *= 16.0f;

        

        const auto spanAngleDegrees = endAngleDegress - startAngleDegrees;
        vPainter.drawPie(vDrawRect, startAngleDegrees, spanAngleDegrees);
        // Increase the size of the smaller circle so that its red border shows
        auto smallerRect = QRectF{smallX - 2.0,
          smallY - 2.0,
          smallerWidth + 4.0,
          smallerHeight + 4.0};
        vPainter.drawPie(smallerRect, startAngleDegrees, spanAngleDegrees);

        vPainter.setPen(originalPen);
    }

    // Draw the white central pie to cover the other ones
    if(!m_chartData.empty()) 
    {
      auto smallerRect = QRectF{smallX, smallY, smallerWidth, smallerHeight};
      const auto originalPen = vPainter.pen();
      vPainter.setPen(QPen{ Qt::white });
      const QBrush pieBrush{ Qt::white };
      vPainter.setBrush(pieBrush);
      // 5760 is a full circle in 16h of a degree, i.e. 16 * 360
      vPainter.drawPie(smallerRect, 0, 5760);
      vPainter.setPen(originalPen);
    }

    vPainter.restore();
}

/// @brief  Draw the labels and values inside the pie chart sectors.
/// @param[in] vPainter The painter that will be used to draw the labels and values.
/// @param[in] vDrawRect The rectangle where the pie chart should be drawn.
void LPGPU2ppPieChartWidget::DrawLabelsAndValues(QPainter &vPainter, const QRect &vDrawRect)
{
    vPainter.save();    

    QFont labelFont;
    labelFont.setPixelSize(g_LabelsFontSize);
    labelFont.setBold(true);

    auto pieChartCenter = vDrawRect.center();
    const auto pieChartRadius = GetPieChartRadius();

    const auto pieChartLabelRadius = pieChartRadius * 0.95f;

    for (decltype(m_chartData)::size_type iItemIndex = 0; iItemIndex < m_chartData.size(); ++iItemIndex)
    {
        const auto& dataPoint = m_chartData[iItemIndex];

        // Don't label data with a value of 0 as multiple empty segment labels may overlap:
        if (dataPoint.m_bIsSelected == false)
        {
          continue;
        }

        const auto startAngleDegrees = m_pieSectorAngles[iItemIndex];
        const auto endAngleDegress = m_pieSectorAngles[iItemIndex + 1];

        const auto angleToDrawLabelDegrees = -(startAngleDegrees + endAngleDegress) / 2.0f;
        const auto angleToDrawLabelRadians = qDegreesToRadians(angleToDrawLabelDegrees);

        const gtFloat32 pieChartLabelX = pieChartCenter.x() + pieChartLabelRadius * qCos(angleToDrawLabelRadians);
        const gtFloat32 pieChartLabelY = pieChartCenter.y() + pieChartLabelRadius * qSin(angleToDrawLabelRadians);

        const QPointF pointToDrawLabel{ pieChartLabelX, pieChartLabelY };        

        // Convert the value to the range [0,100]
        const auto relativeValue = static_cast<gtUInt64>(dataPoint.m_value / m_totalValue * 100);
        
        QPen textPen{ Qt::black };
        textPen.setWidth(1);
        
        vPainter.setPen(textPen);
        vPainter.setFont(labelFont);        

        vPainter.drawText(pointToDrawLabel.toPoint(), QString{ "%0 %1%" }.arg(acGTStringToQString(dataPoint.m_label)).arg(relativeValue));
    }

    if (!m_chartData.empty())
    {
        QPen textPen{ Qt::black };
        textPen.setWidth(1);
        
        vPainter.setPen(textPen);
        labelFont.setPixelSize(kTotalLabelFontSize);
        vPainter.setFont(labelFont);

        pieChartCenter.setX(pieChartCenter.x() - 30);
        vPainter.drawText(pieChartCenter, QString{ "Total" });
        pieChartCenter.setX(pieChartCenter.x() - 5);
        pieChartCenter.setY(pieChartCenter.y() + 20);
        vPainter.drawText(pieChartCenter, QString{ "Events" });
    }

    vPainter.restore();
}
