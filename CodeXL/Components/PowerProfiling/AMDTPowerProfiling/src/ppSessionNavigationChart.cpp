//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionNavigationChart.cpp
///
//==================================================================================

//------------------------------ SessionNavigationChart.cpp ------------------------------

// Local:
#include <AMDTPowerProfiling/src/ppSessionNavigationChart.h>
#include <AMDTPowerProfiling/src/ppColors.h>

#include <chrono>

ppSessionNavigationChart::ppSessionNavigationChart(QWidget* pParent,
                                                   ppSessionController* pSessionController,
                                                   const QString& dataLabel,
                                                   const bool bForceFullRangeDisplay) 
    : LPGPU2acNavigationChart(pParent, dataLabel, nullptr, nullptr, bForceFullRangeDisplay)
    , m_pSessionController(pSessionController)
{
    // initial sampling interval
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        SetInterval(pSessionController->GetSamplingTimeInterval());
        const std::chrono::duration<double> frameDuration{ 1.0 / 60.0 }; // Set the minimum range to be the frame time of a 60FPS application
        const auto frameTimeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count();
        SetMinimumRange(frameTimeInMs); 
        
        SetNavigationUnitsX(eNavigationMilliseconds);
        SetNavigationUnitsY(eNavigationSingleUnits);
        SetUnitsDisplayType(eNavigationDisplayAll);
    }
}

//++AT:LPGPU2
/// @brief Reset the plot so that new fresh data, coming from another counter,
///        can be added to the graph
void ppSessionNavigationChart::ResetPlot()
{
  m_pAllSessionGraph->clearData();
}
//--AT:LPGPU2
