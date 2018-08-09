//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMultiLinePlot.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTPowerProfiling/src/ppMultiLinePlot.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
//++CF:LPGPU2 (to determine from settings if target device is android)
#include <AMDTApplicationFramework/Include/afProjectManager.h>
//--CF:LPGPU2

// Local:
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppColors.h>
#include <AMDTPowerProfiling/src/ppGUIDefs.h>
#include <AMDTPowerProfiling/src/ppCountersSelectionDialog.h>
//++AT:LPGPU2
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
//--AT:LPGPU2

ppMultiLinePlot::ppMultiLinePlot(ppSessionController* pSessionController) :
    acMultiLinePlot(false, false)
{
    m_pSessionController = pSessionController;

    // Set xAxis visible / invisible:
    HideXAxisLabels(true);
}


//++AT:LPGPU2
/// @brief  Initialise a plot
/// @param[in] xAxisTitle Name of X axis
/// @param[in] yAxisTitle Name of Y axis
/// @param[in] graphCategory Category of the graph
/// @param[in] graphType Type of the graph
/// @param[in] valuesType Type of the values displayed in the graph
/// @param[in] unitStr Unit for the data displayed in the graph
/// @param[in] deviceId Parent ID of the counters in this graph
/// @note This method was copied from the AMD's existing one to maintain as
///       much retrocompatibility as possible
void ppMultiLinePlot::InitPlot(const QString& xAxisTitle,
    const QString& yAxisTitle, AMDTPwrCategory graphCategory,
    ppDataUtils::GraphViewCategoryType graphType, GraphValuesType valuesType,
    const QString& unitsStr, int deviceId)
{
    m_graphCategory = graphCategory;
    m_graphType = graphType;
    m_valuesType = valuesType;
    m_unitsStr = unitsStr;

    m_relevantCountersVec.clear();
    ppDataUtils::GetRelevantCounterIdsByGraphTypeAndDeviceId(
        m_relevantCountersVec, m_graphType, deviceId, m_pSessionController);

    m_isShown = false;

    GT_IF_WITH_ASSERT(m_pCustomPlot != nullptr)
    {
        m_pCustomPlot->xAxis->setBasePen(QPen(PP_ACTIVE_RANGE_AXES_COLOR,
              PP_TIMELINE_ACTIVE_RANGE_X_AXIS_WIDTH));
        m_pCustomPlot->xAxis->setTickLabelColor(PP_ACTIVE_RANGE_AXES_COLOR);
        m_pCustomPlot->xAxis->setTicks(false);
        m_pCustomPlot->yAxis->setBasePen(QPen(PP_ACTIVE_RANGE_AXES_COLOR,
              PP_TIMELINE_ACTIVE_RANGE_Y_AXIS_WIDTH));
        m_pCustomPlot->yAxis->setTickLabelColor(PP_ACTIVE_RANGE_AXES_COLOR);
    }

    // Initialize the plot axes titles:
    InitPlotAxesLabels(xAxisTitle, yAxisTitle);

    InitTrackingToolTip();
}
//--AT:LPGPU2

void ppMultiLinePlot::InitPlot(const QString& xAxisTitle, const QString& yAxisTitle,
                               AMDTPwrCategory graphCategory, ppDataUtils::GraphViewCategoryType graphType,
                               GraphValuesType valuesType, const QString& unitsStr)
{
    m_graphCategory = graphCategory;
    m_graphType = graphType;
    m_valuesType = valuesType;
    m_unitsStr = unitsStr;

    m_relevantCountersVec.clear();
    ppDataUtils::GetRelevantCounterIdsByGraphType(m_relevantCountersVec, m_graphType, m_pSessionController);

    m_isShown = false;

    GT_IF_WITH_ASSERT(m_pCustomPlot != nullptr)
    {
        m_pCustomPlot->xAxis->setBasePen(QPen(PP_ACTIVE_RANGE_AXES_COLOR, PP_TIMELINE_ACTIVE_RANGE_X_AXIS_WIDTH));
        m_pCustomPlot->xAxis->setTickLabelColor(PP_ACTIVE_RANGE_AXES_COLOR);
        m_pCustomPlot->xAxis->setTicks(false);
        m_pCustomPlot->yAxis->setBasePen(QPen(PP_ACTIVE_RANGE_AXES_COLOR, PP_TIMELINE_ACTIVE_RANGE_Y_AXIS_WIDTH));
        m_pCustomPlot->yAxis->setTickLabelColor(PP_ACTIVE_RANGE_AXES_COLOR);
    }

    // Initialize the plot axes titles:
    InitPlotAxesLabels(xAxisTitle, yAxisTitle);

    InitTrackingToolTip();
}


void ppMultiLinePlot::InitPlotWithSelectedCounters()
{
    QVector<acMultiLinePlotItemData*> graphData;

    PrepareTimeLineSelectedCountersInitData(graphData);

    //if there is new data to set into graph
    if (graphData.size() > 0)
    {
        // add empty graphs to plot
        bool allowReplot = false;
        InitPlotWithEmptyGraphs(graphData, allowReplot);

        // delete data
        for (int i = 0; i < graphData.count(); i++)
        {
            delete graphData[i];
        }

        // init legend
        InitPlotInfoTable();

//++AT:LPGPU2
        RemoveCounterSelectionFromPlotInfoTable();
//--AT:LPGPU2

        // connect the add/remove button to slot
        bool rc = connect(this, SIGNAL(AddRemoveActivated()), this, SLOT(OnAddRemoveCountersAccepted()));
        GT_ASSERT(rc);
    }
}

void ppMultiLinePlot::SetGraphInitializationData()
{
    // this function initializes the data to the existing empty graphs
    QVector<acMultiLinePlotItemData*> graphData;

    // get session time range fom DB
    SamplingTimeRange samplingRange(0, 0);
    m_pSessionController->GetSessionTimeRange(samplingRange);

    // get session counters from DB
    gtVector<int> counterIds;
    ppDataUtils::GetRelevantCounterIdsByGraphType(counterIds, m_graphType, m_pSessionController);

    // if there is counters selected in the session for getting their data
    if (counterIds.size() > 0)
    {
        // get data from dB
        PrepareTimelineDataFromDB(counterIds,
                                  samplingRange,
                                  graphData);
    }

    //if there is new data to set into graph
    if (graphData.size() > 0)
    {
        // add data series to graph
        bool allowReplot = false;
        InitPlotGraphs(graphData, allowReplot);

        // delete data
        for (int i = 0; i < graphData.count(); i++)
        {
            delete graphData[i];
        }

        ResetPlotInfoTable();

//++AT:LPGPU2
        RemoveCounterSelectionFromPlotInfoTable();
//--AT:LPGPU2

        ResetPlotInfoTableColumnWidth();
        SetInfoTableBySpecificTimePoint(samplingRange.m_toTime);
    }
    else
    {
        // hide graph and it's legend:
        m_isShown = false;
    }
}

void ppMultiLinePlot::PrepareTimeLineSelectedCountersInitData(QVector<acMultiLinePlotItemData*>& graphData)
{
    int count = m_relevantCountersVec.size();

    if (m_pSessionController != nullptr)
    {
        for (int i = 0; i < count; i++)
        {
            QString name = m_pSessionController->GetCounterNameById(m_relevantCountersVec[i]);
            QColor color = m_pSessionController->GetColorForCounter(m_relevantCountersVec[i]);
            QString description = m_pSessionController->GetCounterDescription(m_relevantCountersVec[i]);
//++AT:LPGPU2
            QString id;
            id.setNum(m_relevantCountersVec[i]);
//--AT:LPGPU2

            QVector<double> keyVec;
            QVector<double> valueVec;
//++AT:LPGPU2
            auto *graphItem = new acMultiLinePlotItemData(keyVec, valueVec,
                color, name, description, id);
//--AT:LPGPU2
            graphData << graphItem;
        }
    }
}


void ppMultiLinePlot::UpdateVisibility()
{
    if (nullptr != GetPlot())
    {
        GetPlot()->setVisible(m_isShown);
    }

    if (GetPlotInfoTable() != nullptr)
    {
        GetPlotInfoTable()->setVisible(m_isShown);
    }
}

void ppMultiLinePlot::UpdatePlotRange(bool isReplotNeeded, SamplingTimeRange samplingRange)
{
    QCPRange currentRange = GetGraphRange();

    if (currentRange.lower != samplingRange.m_fromTime ||
        currentRange.upper != samplingRange.m_toTime)
    {
        ChangeGraphRangeByBothPoints(samplingRange.m_fromTime, samplingRange.m_toTime);

        if (isReplotNeeded)
        {
            Replot();
        }
    }
}

void ppMultiLinePlot::PrepareTimelineDataFromDB(const gtVector<int>& counterIds,
                                                SamplingTimeRange& samplingTimeRange,
                                                QVector<acMultiLinePlotItemData*>& graphData)
{
    int id;
    int numOfValues;
    QString name;
    QString  description;
    acMultiLinePlotItemData* graphItem;
    QVector<double> keyVec;
    QVector<double> valueVec;
    QColor color;

    if (m_pSessionController != nullptr)
    {
        gtMap<int, gtVector<SampledValue>> sampledDataPerCounter;
        m_pSessionController->GetProfilerBL().GetSampledValuesByRange(counterIds, samplingTimeRange, sampledDataPerCounter);

        int countersNum = m_relevantCountersVec.size();

        for (int i = 0; i < countersNum; i++)
        {
            id = m_relevantCountersVec[i];

            GT_IF_WITH_ASSERT(sampledDataPerCounter.count(id) > 0)
            {
                name = m_pSessionController->GetCounterNameById(id);
                color = m_pSessionController->GetColorForCounter(id);
                description = m_pSessionController->GetCounterDescription(id);

                keyVec.clear();
                valueVec.clear();

                numOfValues = sampledDataPerCounter[id].size();
//++CF:LPGPU2
                for (int j = 0; j < numOfValues; j++)
                {
                    // Values added in either ms or ns, as specified above:
                    keyVec << (sampledDataPerCounter[id])[j].m_sampleTime;
//--CF:LPGPU2
                    valueVec << (sampledDataPerCounter[id])[j].m_sampleValue;
                }

                graphItem = new acMultiLinePlotItemData(keyVec, valueVec, color, name, description);
                graphData << graphItem;
            }
        }
    }
}

void ppMultiLinePlot::PrepareTimelineDataFromNewDataEvent(ppQtEventData pSampledDataPerCounter,
                                                          QVector<acMultiLinePlotItemData*>& graphData)
{
    GT_IF_WITH_ASSERT(nullptr != m_pSessionController)
    {
        int id;
        QString name, description;
        acMultiLinePlotItemData* graphItem;
        QColor color;
        QVector<double> keyVec;
        QVector<double> valueVec;

        int count = m_relevantCountersVec.size();

        for (int i = 0; i < count; i++)
        {
            id = m_relevantCountersVec[i];
            //find the relevant counter in the event data map
            gtMap<int, PPSampledValuesBatch>::const_iterator it = pSampledDataPerCounter->find(id);

            // if not fount in map - bug - the same selected counters should be in the map and in the plot
            GT_IF_WITH_ASSERT(it != pSampledDataPerCounter->end())
            {
                name = m_pSessionController->GetCounterNameById(id);
                color = m_pSessionController->GetColorForCounter(id);
                description = m_pSessionController->GetCounterDescription(id);

                keyVec.clear();
                valueVec.clear();

                keyVec << (*it).second.m_quantizedTime;
                valueVec << (*it).second.m_sampleValues[0];

                graphItem = new acMultiLinePlotItemData(keyVec, valueVec, color, name, description);
                graphData << graphItem;
            }
        }
    }
}

void ppMultiLinePlot::AddNewDataToExistingGraph(ppQtEventData pSampledDataPerCounter, SamplingTimeRange samplingRange, bool bShouldUpdateLastSample)
{
    QVector<double> valVec;

    // in specific event - all counters have the same key (time)
    gtMap<int, PPSampledValuesBatch>::const_iterator it = pSampledDataPerCounter->begin();
    double key = (*it).second.m_quantizedTime;

    if (m_relevantCountersVec.size() > 0)
    {
        GetValueVecFromEventData(pSampledDataPerCounter, valVec);

        // add data to graph
        bool removeOldSamplings = false;
        bool allowReplot = false;        

        AddDataToTimeLineGraph(key, valVec, samplingRange.m_fromTime, samplingRange.m_toTime, removeOldSamplings, allowReplot);

        // update lege0-11nd by last point
        if (HasLegend() && bShouldUpdateLastSample)
        {
            ResetPlotInfoTableColumnWidth();
            SetInfoTableBySpecificTimePoint(samplingRange.m_toTime);
        }
    }
}

void ppMultiLinePlot::GetValueVecFromEventData(ppQtEventData pSampledDataPerCounter, QVector<double>& valVec)
{
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        int id;
        int index;
        QString name;
        int countersNum = m_relevantCountersVec.size();

        for (int i = 0; i < countersNum; i++)
        {
            id = m_relevantCountersVec[i];

            // check if the id is in the event data map
//++AT:LPGPU2
            if(pSampledDataPerCounter->count(id) > 0)
//--AT:LPGPU2
            {
                // get graph index for this counter and set the vector in the index place to the counter value
//++AT:LPGPU2
// Comment this out and keep comment as a reference to what the code was like
// before. Now we identify lineplots in the multilineplot by the id of the
// counter they track.
                // name = m_pSessionController->GetCounterNameById(id);
                index = GetGraphIndexById(id);

                if(index > -1 && index < countersNum &&
                    !pSampledDataPerCounter->at(id).m_sampleValues.empty())
                {
                    valVec.push_back(pSampledDataPerCounter->at(id).m_sampleValues[0]);
//--AT:LPGPU2
                }
            }
        }
    }
}

void ppMultiLinePlot::OnAddRemoveCountersAccepted()
{
    ppCountersSelectionDialog::OpenCountersSelectionDialog(m_graphCategory);
}

void ppMultiLinePlot::Delete()
{
    // not in destructor because - this destructor called also when layout is deleted.
    // if the layout destructor is called first it will delete this object too, so when we get to this destructor the object will not be exist, and we cant delete it.
    m_pCustomPlot->clearGraphs();
    m_pGraphsVec.clear();
    m_pPlotInfoTable->deleteLater();
    m_pCustomPlot->deleteLater();
}

void ppMultiLinePlot::SetShown(bool isShown, bool shouldUpdateVisibility)
{
    m_isShown = isShown;

    if (shouldUpdateVisibility)
    {
        UpdateVisibility();
    }
}

void ppMultiLinePlot::HideSingleLineGraphFromCumulative(int selectedGraphIndex, bool hideGrpah)
{
    int prevGraphIndex = -1;
    acVectorLineGraph* pTmpGraph = qobject_cast<acVectorLineGraph*>(m_pCustomPlot->graph(selectedGraphIndex));
    GT_IF_WITH_ASSERT(nullptr != pTmpGraph)
    {
        acDataVector* pDataVec = pTmpGraph->VectorData();
        acDataVector* prevDataVec = nullptr;

        //step 1: get the index of the shown graph that comes right below the one we like to remove/add
        for (int i = selectedGraphIndex - 1; i >= 0; i--)
        {
            GT_IF_WITH_ASSERT(i < m_pGraphsVec.count())
            {
                if (!m_pGraphsVec[i]->IsHidden())
                {
                    prevGraphIndex = i;
                    break;
                }
            }
        }

        // step 2: if it is not the first graph shown (first on the bottom of the plot)
        // get the previous (the graphs below it) y values
        if (selectedGraphIndex > 0 && prevGraphIndex > -1)
        {
            pTmpGraph = qobject_cast<acVectorLineGraph*>(m_pCustomPlot->graph(prevGraphIndex));
            GT_IF_WITH_ASSERT(nullptr != pTmpGraph)
            {
                prevDataVec = pTmpGraph->VectorData();
            }
            else
            {
                prevGraphIndex = -1;
            }
        }

        // step 3: get the removed graph (single line graph) y values
        QVector<double> removedGraphVals;

        unsigned int vecSize = pDataVec->size();

        // if it is the first graph (on the bottom of the plot) - get its value
        if (selectedGraphIndex == 0 || prevGraphIndex == -1)
        {
            for (unsigned int i = 0; i < vecSize; i++)
            {
                removedGraphVals << ((*pDataVec)[i]).Value();
            }
        }
        else
        {
            // if not we need to reduce the graphs below it from its value to get its real value
            // (as in cumulative graph the values are also cumulative)
            // for showing (not hiding) the graph we the the oposide thing (add not reduce)
            GT_IF_WITH_ASSERT(nullptr != prevDataVec && nullptr != pDataVec)
            {
                // if the total graph - don't do nothing
                if (selectedGraphIndex < (m_pCustomPlot->graphCount() - 1))
                {
                    for (unsigned int i = 0; i < vecSize; i++)
                    {
                        // if not first - get its value minus/plus the value of shown graph below
                        if (hideGrpah)
                        {
                            removedGraphVals << ((*pDataVec)[i]).Value() - ((*prevDataVec)[i]).Value();
                            ((*pDataVec)[i]).SetValue(((*pDataVec)[i]).Value() - ((*prevDataVec)[i]).Value());
                        }
                        else // show graph
                        {
                            removedGraphVals << ((*pDataVec)[i]).Value();
                            ((*pDataVec)[i]).SetValue(((*pDataVec)[i]).Value() + ((*prevDataVec)[i]).Value());
                        }
                    }
                }
            }
        }

        // step 4: update all graphs data. update graphs values that above the removed/added graph
        // when the graph hidden - we need to reduce its y values from all graphs above it.
        // when the graph is added - we need to add its y values from all graphs above it.
        int numGraphs = m_pCustomPlot->graphCount();

        // only if the total counter exist - dont move it and dont reduce from it as that graph is static even if graphs below it are hidden
        if (m_isLastGraphTotal)
        {
            numGraphs -= 1;
        }

        for (int graphIndex = selectedGraphIndex + 1; graphIndex < numGraphs; graphIndex++)
        {
            // if the graph above not hidden
            if (!m_pGraphsVec[graphIndex]->IsHidden())
            {
                pTmpGraph = qobject_cast<acVectorLineGraph*>(m_pCustomPlot->graph(graphIndex));

                GT_IF_WITH_ASSERT(nullptr != pTmpGraph)
                {
                    pDataVec = pTmpGraph->VectorData();

                    // all graphs data vector is in the same size (same keys)
                    GT_IF_WITH_ASSERT(nullptr != pDataVec && vecSize == pDataVec->size())
                    {
                        for (unsigned int index = 0; index < vecSize; index++)
                        {
                            if (hideGrpah)
                            {
                                ((*pDataVec)[index]).SetValue(((*pDataVec)[index]).Value() - removedGraphVals[index]);
                            }
                            else
                            {
                                ((*pDataVec)[index]).SetValue(((*pDataVec)[index]).Value() + removedGraphVals[index]);
                            }
                        }
                    }
                }
            }
        }

        // step 5: order fill
        OrderGraphChannelFill();
    }
}

void ppMultiLinePlot::SetPowerGraphLegendCheckState(QVector<bool> hiddenStateVec)
{
    // get legend
    acListCtrl* pLegend = GetPlotInfoTable();

    GT_IF_WITH_ASSERT(pLegend != nullptr)
    {
        unsigned int count = hiddenStateVec.count();
        int row = 0;

        // for all graphs - set their check state by the inpout vector
        for (unsigned int graphIndex = 0; graphIndex < count; graphIndex++)
        {
            // get the graph row number in legend
            row = count - 1 - graphIndex;

            GT_IF_WITH_ASSERT(row < pLegend->rowCount())
            {
                // set row related row in legend checked/unchecked
                QTableWidgetItem* pItem = pLegend->item(row, LEGEND_CHECKBOX_COLUMN);

                GT_IF_WITH_ASSERT(pItem != nullptr)
                {
                    if (hiddenStateVec[graphIndex] == true)
                    {
                        pItem->setCheckState(Qt::Unchecked);
                    }
                    else
                    {
                        // set checked only if graph is not disabled
                        if (!IsGraphDisabledAtIndex(graphIndex))
                        {
                            pItem->setCheckState(Qt::Checked);
                        }
                    }
                }
            }
        }
    }
}

void ppMultiLinePlot::ResetPlotInfoTableColumnWidth()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPlotInfoTable != nullptr)
    {
        m_pPlotInfoTable->resizeColumnToContents(1);
    }
}

//++AT:LPGPU2
void ppMultiLinePlot::RemoveCounterSelectionFromPlotInfoTable()
{
  const auto& projSettings = afProjectManager::instance().currentProjectSettings();
  auto isAndroid = projSettings.isAndroidDeviceTarget();

  if (isAndroid)
  {
    auto items =
      m_pPlotInfoTable->findItems(AC_STR_CounterSelectionString,
          Qt::MatchExactly);
    m_addRemoveRowNum = m_pPlotInfoTable->rowCount();
    if (items.size() == 1)
    {
      auto row = m_pPlotInfoTable->row(items[0]);
      m_pPlotInfoTable->removeRow(row);
    }
    m_addRemoveRowNum = m_pPlotInfoTable->rowCount();

    // This is so that the last element does not activate the widget which
    // allows to set the counters directly from the timelineview, which we do
    // not support for the LPGPU2 mode.
    disconnect(m_pPlotInfoTable, SIGNAL(itemActivated(QTableWidgetItem*)),
        this, SLOT(OnLegendTableItemActivate(QTableWidgetItem*)));
  }
}
//--AT:LPGPU2
