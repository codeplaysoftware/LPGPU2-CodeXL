    
#include <AMDTPowerProfiling/src/LPGPU2ppMultiLinePlot.h>

#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
using lpgpu2::PPFnStatus;

LPGPU2ppMultiLinePlot::LPGPU2ppMultiLinePlot(ppSessionController *vpSessionController)
: ppMultiLinePlot(vpSessionController)
{
}

LPGPU2ppMultiLinePlot::~LPGPU2ppMultiLinePlot()
{
}

PPFnStatus LPGPU2ppMultiLinePlot::Initialise()
{
    PPFnStatus bReturn = PPFnStatus::failure;

    GT_IF_WITH_ASSERT(m_pCustomPlot != nullptr)
    {
        // Enable Regions to be clicked
        const bool bEnableSelectItems = true;
        m_pCustomPlot->setInteraction(QCP::iSelectItems, bEnableSelectItems);

        bReturn = PPFnStatus::success;
    }

    return bReturn;
}

PPFnStatus LPGPU2ppMultiLinePlot::AddRangeHighlight(gtUInt32 vRegionId, double vStartPoint, double vEndPoint, const QColor &vColor)
{
    PPFnStatus bReturn = PPFnStatus::failure;

    GT_IF_WITH_ASSERT(m_pCustomPlot != nullptr)
    {
        std::unique_ptr<QCPItemRect> pRangeHighlightItem{ new (std::nothrow) QCPItemRect{ m_pCustomPlot } };
        if(pRangeHighlightItem != nullptr)
        {
            const double topLeftXCoord = vStartPoint;
            const double topLeftYCoord = m_pCustomPlot->yAxis->range().upper;

            const double bottomRightXCoord = vEndPoint;
            const double bottomRightYCoord = m_pCustomPlot->yAxis->range().lower;            
            
            pRangeHighlightItem->topLeft->setCoords(topLeftXCoord, topLeftYCoord);
            pRangeHighlightItem->bottomRight->setCoords(bottomRightXCoord, bottomRightYCoord);

            QColor rectFillColor = vColor;
            rectFillColor.setAlpha(pp_defaultAlphaValue);
            
            QColor rectContourColor = rectFillColor.darker(pp_defaultDarkerFactor);
            
            pRangeHighlightItem->setBrush(QBrush{ rectFillColor });
            pRangeHighlightItem->setPen(QPen{ rectContourColor });

            try
            {
                m_hightlightRegionToIdMap[pRangeHighlightItem.get()] = vRegionId;                
                bReturn = PPFnStatus::success;
            }
            catch (...)
            {                
                bReturn = PPFnStatus::failure;
            }            

            if (bReturn == PPFnStatus::success)
            {               
                connect(pRangeHighlightItem.get(), &QCPItemRect::selectionChanged, this, &LPGPU2ppMultiLinePlot::OnRegionSelected);
                connect(m_pCustomPlot->yAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(OnYAxisRangeChanged(const QCPRange&)));

                pRangeHighlightItem.release();
            }   

            m_pCustomPlot->replot();
        }
    }   

    return bReturn;
}

PPFnStatus LPGPU2ppMultiLinePlot::ClearHighlights()
{
    for (auto highlightRegion : m_hightlightRegionToIdMap.keys())
    {
        delete highlightRegion;
    }

    m_hightlightRegionToIdMap.clear();
    m_pCustomPlot->replot();

    return PPFnStatus::success;
}

void LPGPU2ppMultiLinePlot::OnRegionSelected(bool vbSelected)
{
    if (vbSelected)
    {
        auto highlightRegion = qobject_cast<QCPItemRect*>(QObject::sender());
        GT_IF_WITH_ASSERT(highlightRegion)
        {
            // Disable the selection, we are only interested in the click event
            highlightRegion->setSelected(false);

            GT_IF_WITH_ASSERT(m_hightlightRegionToIdMap.contains(highlightRegion))
            {
                auto regionId = m_hightlightRegionToIdMap[highlightRegion];

                emit RegionClicked(regionId);
            }
        }
    }    
}

void LPGPU2ppMultiLinePlot::OnYAxisRangeChanged(const QCPRange &newRange)
{
    for (auto highlightRegion :  m_hightlightRegionToIdMap.keys())
    {
        QPointF topLeftCoords = highlightRegion->topLeft->coords();
        QPointF bottomRightCoords = highlightRegion->bottomRight->coords();
        
        topLeftCoords.setY(newRange.upper);
        bottomRightCoords.setY(newRange.lower);

        highlightRegion->topLeft->setCoords(topLeftCoords);
        highlightRegion->bottomRight->setCoords(bottomRightCoords);
    }
}