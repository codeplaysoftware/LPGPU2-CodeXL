//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionNavigationChart.h
///
//==================================================================================

//------------------------------ kaExportBinariesDialog.h ------------------------------
#ifndef __SESSIONNAVIGATIONCHART_H
#define __SESSIONNAVIGATIONCHART_H

#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <qobject.h>
#include <qcustomplot.h>

// Local:
#include <AMDTPowerProfiling/src/ppSessionView.h>

// AMDTApplicationComponents
#include <AMDTApplicationComponents/Include/LPGPU2acNavigationChart.h>


class ppSessionNavigationChart : public LPGPU2acNavigationChart
{
    Q_OBJECT

public:
//++CF:LPGPU2 (Adding forceFullRangeDisplay which has been added to acNavigationChart)
    ///constructor
    ppSessionNavigationChart(QWidget* pParent, ppSessionController* pSessionController, const QString& dataLabel, const bool bForceFullRangeDisplay);
//--CF:LPGPU2

    virtual ~ppSessionNavigationChart() {}
//++AT:LPGPU2
    void ResetPlot();
//--AT:LPGPU2

private:
    /// Pointer to session controller:
    ppSessionController* m_pSessionController = nullptr;
};



#endif // __SESSIONNAVIGATIONCHART_H


