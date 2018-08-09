// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Session Comparison View widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// This is necessary because QCustomPlot will raise a warning that will
// be treated as an error. acMultiLinePlot.cpp does the same.
#include <qtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppSessionComparisonView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppSessionController.h>

// Infra:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acTableWidgetItem.h>
#include <AMDTApplicationComponents/Include/acCustomPlot.h>

// Qt:
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHeaderView>
#include <QSplitter>
#include <QPen>

using lpgpu2::PPFnStatus;

// These are old style enums because the intention is to use them
// to name the columns of the table this class represents. In order
// to avoid casting this to int we are making this enum like this.
// Moreover, this is a private enum used only by this file.
enum EnumStatsTableColumn
{
    kEnumStatsTableColumn_CounterId = 0,
    kEnumStatsTableColumn_CounterName = 1,
    kEnumStatsTableColumn_Average1 = 2,
    kEnumStatsTableColumn_Average2 = 3,
    kEnumStatsTableColumn_AverageRatio = 4,
    kEnumStatsTableColumn_StdDev1 = 5,
    kEnumStatsTableColumn_StdDev2 = 6,
    kEnumStatsTableColumn_StdDevRatio = 7,

    kEnumStatsTableColumn_NumberOfColumns
};

/// @brief      Compute a simple average of a list of sampled values.
/// @param[in]  vSampledValues  A list of sampled values to calculate the average.
/// @return     double          The average of the sample values or 0.0 if the list
///                             if empty.
static double GetAverageSampleValue(const gtVector<SampledValue> &vSampledValues)
{
    auto average = 0.0;    

    if (!vSampledValues.empty())
    {
        // Compute the total sum of all the sampled values
        const double sum = std::accumulate(vSampledValues.begin(), vSampledValues.end(), 0.0, [](double currentValue, const SampledValue &vSampledValue)
        {
            return currentValue + vSampledValue.m_sampleValue;
        });

        // Divide the sum by the amount of samples.
        average = sum / vSampledValues.size();
    }
    
    return average;
}

/// @brief                      Computes the standard deviation from a list of samples.
/// @param[in]  vSampledValues  A list of sampled values to calculate the standard deviation.
/// @param[in]  vAverageValue   The previous calculated average. To avoid doing the average calculation
///                             again we take the value here.
/// @return     double          The standard deviation from the average from the list of samples or 0.0 if
///                             the list of samples contains less them two elements.
/// @see     https://en.wikipedia.org/wiki/Standard_deviation
/// @see     GetAverageSampleValue
/// @warning The list of samples must have at least two elements.
static double GetStdDevSampleValue(const gtVector<SampledValue> &vSampledValues, const double vAverageValue)
{
    auto stdDev = 0.0;

    if (vSampledValues.size() > 1)
    {
        const double accum = std::accumulate(vSampledValues.begin(), vSampledValues.end(), 0.0, [=](double currentValue, const SampledValue &vSampledValue)
        {
            // Compute the distance between this sample and the average
            const auto diff = vSampledValue.m_sampleValue - vAverageValue;

            // Accumulate with the square value of the difference
            return currentValue + diff * diff;
        });

        // Finally takes the square root to calculate the std dev
        stdDev = qSqrt(accum / (vSampledValues.size() - 1));
    }    

    return stdDev;
}

/// @brief                      Utility function to get a list of all the 
///                             keys from a generic gtMap.
/// @param[in] vMap             The map to extract the keys from.
/// @return gtVector<ValueType> A list of all the keys from the map.
template<typename KeyType, typename ValueType>
static gtVector<KeyType> GetKeysFromMap(const gtMap<KeyType, ValueType> &vMap)
{
    gtVector<KeyType> keys;
    keys.reserve(vMap.size());
    std::transform(vMap.begin(), vMap.end(), std::back_inserter(keys), [](const std::pair<KeyType, ValueType> &pair)
    {
        return pair.first;
    });
    return keys;
}

/// @brief Class constructor. No work is performed here.
/// @param[in] vpSessionController A pointer to the session controller of the source section
///                                to be used in the comparison.
/// @param[in] vpParent            The parent widget where this will be displayed.
LPGPU2ppSessionComparisonView::LPGPU2ppSessionComparisonView(ppSessionController *vpSessionController, QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
    , m_pSessionController(vpSessionController)    
{
    GT_ASSERT(m_pSessionController != nullptr);
}

/// @brief  Class destructor. Call the Shutdown method to cleanup
///         resources used by this class.
LPGPU2ppSessionComparisonView::~LPGPU2ppSessionComparisonView()
{
    Shutdown();
}

/// @brief  Initialises this class components and layout.
/// @return PPFnStatus  success: All components were initialised and are
///                     ready to use,
///                     failure: An error has occurred while initialising
///                     this widget.
PPFnStatus LPGPU2ppSessionComparisonView::Initialise()
{
    auto bReturn = PPFnStatus::failure;

    auto *treeHandler = ProfileApplicationTreeHandler::instance();
    if (treeHandler != nullptr && m_pSessionController != nullptr)
    {
        const auto& sessionPath = m_pSessionController->DBFilePath();
        m_pSessionNodeData = treeHandler->FindSessionDataByProfileFilePath(sessionPath);

        if (m_pSessionNodeData != nullptr)
        {
            QVBoxLayout *pMainLayout = nullptr;
            LPGPU2PPNewQtWidget(&pMainLayout, this);

            QHBoxLayout *pHeaderLayout = nullptr;
            LPGPU2PPNewQtWidget(&pHeaderLayout, nullptr);

            QLabel *pHeaderLabel = nullptr;
            LPGPU2PPNewQtWidget(&pHeaderLabel, PP_STR_CompareToSession_SelectSessionToCompareLabel, this);

            LPGPU2PPNewQtWidget(&m_pSessionComboBox, this);
            LPGPU2PPNewQtWidget(&m_pComparePushButton, PP_STR_CompareToSession_CompareButton, this);

            m_pSessionComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

            QSpacerItem *pHorizontalSpacer = nullptr;
            LPGPU2PPNewQtWidget(&pHorizontalSpacer, 20, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);

            pHeaderLayout->addWidget(pHeaderLabel);
            pHeaderLayout->addWidget(m_pSessionComboBox);
            pHeaderLayout->addWidget(m_pComparePushButton);
            pHeaderLayout->addItem(pHorizontalSpacer);            

            QWidget *pMainWidget = nullptr;
            LPGPU2PPNewQtWidget(&pMainWidget, this);
            pMainWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

            QVBoxLayout *pMainWidgetLayout = nullptr;
            LPGPU2PPNewQtWidget(&pMainWidgetLayout, pMainWidget);

            QSplitter *pMainWidgetSplitter = nullptr;
            LPGPU2PPNewQtWidget(&pMainWidgetSplitter, Qt::Vertical, pMainWidget);
            pMainWidgetSplitter->setStyle(QStyleFactory::create(PP_STR_SplitterStyleName));
            pMainWidgetSplitter->setHandleWidth(6);
            pMainWidgetSplitter->setStyleSheet(PP_STR_SplitterStyleSheet);

            LPGPU2PPNewQtWidget(&m_pStatisticsTable, pMainWidgetSplitter);

            QStringList tableHeaders;
            tableHeaders << PP_STR_CompareToSession_CounterIdColumn
                << PP_STR_CompareToSession_CounterNameColumn
                << PP_STR_CompareToSession_Average1Column
                << PP_STR_CompareToSession_Average2Column
                << PP_STR_CompareToSession_AvgRatioColumn
                << PP_STR_CompareToSession_StdDev1Column
                << PP_STR_CompareToSession_StdDev2Column
                << PP_STR_CompareToSession_StdDevRatioColumn;

            GT_ASSERT(tableHeaders.size() == kEnumStatsTableColumn_NumberOfColumns);
            
            m_pStatisticsTable->setColumnCount(kEnumStatsTableColumn_NumberOfColumns);
            m_pStatisticsTable->setHorizontalHeaderLabels(tableHeaders);

            m_pStatisticsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_pStatisticsTable->setDragDropOverwriteMode(false);
            m_pStatisticsTable->setAlternatingRowColors(true);
            m_pStatisticsTable->setSortingEnabled(true);
            m_pStatisticsTable->setCornerButtonEnabled(false);
            m_pStatisticsTable->setEditTriggers(QTableWidget::NoEditTriggers);
            m_pStatisticsTable->horizontalHeader()->setSortIndicatorShown(true);
            m_pStatisticsTable->horizontalHeader()->setStretchLastSection(true);
            m_pStatisticsTable->verticalHeader()->setVisible(false);

            LPGPU2PPNewQtWidget(&m_pDiffPlot, pMainWidgetSplitter);
            m_pDiffPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            m_pDiffPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            m_pDiffPlot->legend->setVisible(true);

            auto legentFont = font();
            legentFont.setPointSize(9);

            m_pDiffPlot->legend->setFont(legentFont);
            m_pDiffPlot->legend->setBrush(QBrush{});
            m_pDiffPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);
            m_pDiffPlot->xAxis->setLabel(PP_STR_CompareToSession_DiffPlot_XLabel);
            m_pDiffPlot->xAxis->setNumberFormat("gb");
            m_pDiffPlot->yAxis->setLabel(PP_STR_CompareToSession_DiffPlot_YLabel);
            m_pDiffPlot->yAxis->setNumberFormat("gb");

            pMainWidgetLayout->addWidget(pMainWidgetSplitter);            

            pMainLayout->addLayout(pHeaderLayout);
            pMainLayout->addWidget(pMainWidget);            

            // Populate the ComboBox with the existing sessions

            // ppAppController contains the list of all sessions loaded to this moment.            
            auto &appController = ppAppController::instance();
            const auto& sessions = appController.GetSessionsTreeData();

            for (auto *sessionData : sessions)
            {
                AddSessionToComparisonList(sessionData);
            }

            // ProfileApplicationTreeHandler triggers events for session renaming, deletion and addition.
            connect(treeHandler, &ProfileApplicationTreeHandler::SessionRenamed, this, &LPGPU2ppSessionComparisonView::OnSessionRenamed);
            connect(treeHandler, &ProfileApplicationTreeHandler::SessionDeleted, this, &LPGPU2ppSessionComparisonView::OnSessionDeleted);
            connect(treeHandler, &ProfileApplicationTreeHandler::SessionAdded, this, &LPGPU2ppSessionComparisonView::OnSessionAdded);
            
            // Connect the click event from the button to trigger the comparison calculations
            connect(m_pComparePushButton, &QPushButton::clicked, this, &LPGPU2ppSessionComparisonView::OnComparisonTriggered);

            // Connect the selection event from the statistics table to build the event graph
            connect(m_pStatisticsTable, &QTableWidget::itemSelectionChanged, this, &LPGPU2ppSessionComparisonView::OnStatisticsTableSelectionChanged);

            bReturn = PPFnStatus::success;
        }
    }

    return bReturn;
}

/// @brief Cleanup resources used by this class.
/// @return PPFnStatus Always return success.
PPFnStatus LPGPU2ppSessionComparisonView::Shutdown()
{
    if (m_pComparePushButton != nullptr)
    {
        delete m_pComparePushButton;
        m_pComparePushButton = nullptr;        
    }

    if (m_pSessionComboBox != nullptr)
    {
        delete m_pSessionComboBox;
        m_pSessionComboBox = nullptr;
    }

    if (m_pSessionNodeData != nullptr)
    {
        // We don't own this pointer
        m_pSessionNodeData = nullptr;
    }

    if (m_pSessionController != nullptr)
    {
        // We don't own this pointer
        m_pSessionController = nullptr;
    }

    if (m_pStatisticsTable != nullptr)
    {
        delete m_pStatisticsTable;
        m_pStatisticsTable = nullptr;
    }

    return PPFnStatus::success;
}

/// @brief                          Slot called when a session is renamed on the Power Profiling tree.
///                                 This will rename the combo box entry for the renamed session.
/// @param[in] vpRenamedSessionData pointer to the SessionTreeNodeData that was renamed.
/// @param[in] vOldSessionFilePath  The previous file path (database file) that the renamed session was referring to.
/// @param[in] vOldSessionDir       The previous directory that the renamed session was referring to.
void LPGPU2ppSessionComparisonView::OnSessionRenamed(SessionTreeNodeData *vpRenamedSessionData, const osFilePath &vOldSessionFilePath, const osDirectory& vOldSessionDir) const
{
    GT_UNREFERENCED_PARAMETER(vOldSessionDir);
    GT_UNREFERENCED_PARAMETER(vOldSessionFilePath);

    if (vpRenamedSessionData != nullptr && vpRenamedSessionData != m_pSessionNodeData && m_pSessionComboBox)
    {
        const auto renamedSessionIndex = m_pSessionComboBox->findData(QVariant::fromValue(vpRenamedSessionData->m_sessionId));
        if (renamedSessionIndex != -1)
        {
            m_pSessionComboBox->setItemText(renamedSessionIndex, vpRenamedSessionData->m_displayName);
        }
    }
}

/// @brief                       Slot called when a session is deleted from the Power Profiling tree.
///                              This will remove the entry for the deleted session from the combo box.
/// @param[in] vSessionId        The id of the session that was deleted.
/// @param[in] vDeleteType       The type of the delete that occurred. Not used here.
/// @param[in] vbSessionDeleted  Whether or not the session was actually deleted. Not used here.
///                              This is used by other observers so they know if the files are still
///                              available or not.
void LPGPU2ppSessionComparisonView::OnSessionDeleted(const ExplorerSessionId vSessionId, SessionExplorerDeleteType vDeleteType, bool &vbSessionDeleted) const
{
    GT_UNREFERENCED_PARAMETER(vDeleteType);
    GT_UNREFERENCED_PARAMETER(vbSessionDeleted);

    if (vSessionId != m_pSessionNodeData->m_sessionId && m_pSessionComboBox)
    {
        const auto deletedSessionIndex = m_pSessionComboBox->findData(QVariant::fromValue(vSessionId));
        if (deletedSessionIndex != -1)
        {
            m_pSessionComboBox->removeItem(deletedSessionIndex);
        }
    }
}

/// @brief                      Slot called when a new session was added to the tree view.
///                             This will cause the session name to be displayed in the
///                             combo box.
/// @param[in] vpNewSessionData The session that was added while this widget was visible.
void LPGPU2ppSessionComparisonView::OnSessionAdded(SessionTreeNodeData *vpNewSessionData) const
{    
    AddSessionToComparisonList(vpNewSessionData);
}

/// @brief  Slot called when the session comparison was triggered. This will
///         read the selected session from the combo box and will do the session
///         comparison by computing the statistics table.
void LPGPU2ppSessionComparisonView::OnComparisonTriggered()
{
    if (m_pSessionComboBox != nullptr)
    {
        // Get the other session id
        m_otherSessionId = m_pSessionComboBox->currentData().toUInt();

        auto *treeHandler = ProfileApplicationTreeHandler::instance();
        if (treeHandler != nullptr)
        {
            auto *otherSessionTreeItemData = treeHandler->GetSessionNodeItemData(m_otherSessionId);
            if (otherSessionTreeItemData != nullptr)
            {
                if (!otherSessionTreeItemData->m_filePath.isEmpty())
                {
                    // Open both databases for read
                    auto &profilerBL = m_pSessionController->GetProfilerBL();
                    PowerProfilerBL otherProfilerBL;
                    const auto bIsReadOnly = true;
                    if (otherProfilerBL.OpenPowerProfilingDatabaseForRead(otherSessionTreeItemData->m_filePath.asString(), bIsReadOnly))
                    {
                        // Now we have access to both databases

                        // Get the list of counters from both databases
                        gtMap<int, AMDTPwrCounterDesc*> counterDetails;
                        profilerBL.GetAllSessionCountersDescription(counterDetails);

                        gtMap<int, AMDTPwrCounterDesc*> otherCounterDetails;
                        otherProfilerBL.GetAllSessionCountersDescription(otherCounterDetails);

                        const auto counterIdList = GetKeysFromMap(counterDetails);
                        const auto otherCounterIdList = GetKeysFromMap(otherCounterDetails);

                        SamplingTimeRange sessionRange{ 0, 0 };
                        profilerBL.GetSessionTimeRange(sessionRange);

                        SamplingTimeRange otherSessionRange{ 0, 0 };
                        otherProfilerBL.GetSessionTimeRange(otherSessionRange);

                        // Get the sample values from both sessions
                        gtMap<int, gtVector<SampledValue>> sessionSampleValues, otherSampleValues;
                        profilerBL.GetSampledValuesByRange(counterIdList, sessionRange, sessionSampleValues);
                        otherProfilerBL.GetSampledValuesByRange(otherCounterIdList, otherSessionRange, otherSampleValues);

                        // Build the statistics table
                        m_pStatisticsTable->clearContents();
                        m_pStatisticsTable->setRowCount(counterIdList.size());
                        
                        // Need to disable sorting here because we can potentially be changing the order of the items and this can cause strange behaviour.
                        // According to QTableWidget::setItem documentation
                        //
                        // http://doc.qt.io/qt-5/qtablewidget.html#setItem
                        // 
                        // Note that if sorting is enabled (see sortingEnabled) and column is the current sort column, the row will be moved to the sorted 
                        // position determined by item.
                        //
                        // If you want to set several items of a particular row(say, by calling setItem() in a loop), you may want to turn off sorting before doing so, 
                        // and turn it back on afterwards; this will allow you to use the same row argument for all items in the same row(i.e.setItem() will not move the row).
                        m_pStatisticsTable->setSortingEnabled(false);

                        auto currentRow = 0;
                        for (const auto &counterIdByName : counterDetails)
                        {
                            const auto &counterId = counterIdByName.first;
                            gtString counterName;
                            counterName << counterIdByName.second->m_name;

                            const auto &samplesForCounter = sessionSampleValues[counterId];
                            const auto &otherSamplesForCounter = otherSampleValues[counterId];

                            // Compute the actual statistics
                            const auto counterAverage = GetAverageSampleValue(samplesForCounter);
                            const auto counterStdDev = GetStdDevSampleValue(samplesForCounter, counterAverage);

                            const auto otherCounterAverage = GetAverageSampleValue(otherSamplesForCounter);
                            const auto otherCounterStdDev = GetStdDevSampleValue(otherSamplesForCounter, otherCounterAverage);

                            // Calculate the comparison ratio
                            const auto averageRatio = counterAverage / otherCounterAverage;
                            const auto stdDevRatio = counterStdDev / otherCounterStdDev;

                            auto counterIdItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(counterId) } };
                            auto counterNameItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ acGTStringToQString(counterName) } };

                            auto counterAverageItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(counterAverage) } };
                            auto otherCounterAverageItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(otherCounterAverage) } };
                            auto averageRatioItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(averageRatio) } };

                            auto counterStdDevItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(counterStdDev) } };
                            auto otherCounterStdDevItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(otherCounterStdDev) } };
                            auto stdDevRatioItem = std::unique_ptr<acTableWidgetItem>{ new (std::nothrow) acTableWidgetItem{ QString::number(stdDevRatio) } };

                            if (counterIdItem != nullptr &&
                                counterNameItem != nullptr &&
                                counterAverageItem != nullptr &&
                                otherCounterAverageItem != nullptr &&
                                averageRatioItem != nullptr &&
                                counterStdDevItem != nullptr &&
                                otherCounterStdDevItem != nullptr &&
                                stdDevRatioItem != nullptr)
                            {
                                counterIdItem->setData(Qt::UserRole, QVariant::fromValue(counterId));

                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_CounterId, counterIdItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_CounterName, counterNameItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_Average1, counterAverageItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_Average2, otherCounterAverageItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_AverageRatio, averageRatioItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_StdDev1, counterStdDevItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_StdDev2, otherCounterStdDevItem.release());
                                m_pStatisticsTable->setItem(currentRow, kEnumStatsTableColumn_StdDevRatio, stdDevRatioItem.release());
                            }

                            currentRow++;
                        }

                        m_pStatisticsTable->setSortingEnabled(true);

                        // Need to delete AMDTPwrCounterDesc*
                        for(const auto& counterDetailsPair : counterDetails)
                        {
                            if(counterDetailsPair.second != nullptr)
                            {
                                delete counterDetailsPair.second;
                            }                            
                        }

                        for(const auto& counterDetailsPair : otherCounterDetails)
                        {
                            if(counterDetailsPair.second != nullptr)
                            {
                                delete counterDetailsPair.second;
                            }                            
                        }
                    }
                }
            }
        }
    }        
}

/// @brief  Method called when the user selects an entry on the statistics
///         table. This will display the data in a plot with a comparison
///         using the difference operator with this section as the reference.
void LPGPU2ppSessionComparisonView::OnStatisticsTableSelectionChanged()
{
    // TODO: This method can freeze the GUI when there is a lot of data
    // to process. Consider moving the calculations to a background task.

    // Builds the graph displaying both counters and the difference

    // Sanity check
    if (m_pStatisticsTable != nullptr)
    {
        // Get the samples for this counter
        auto& profilerBL = m_pSessionController->GetProfilerBL();
        PowerProfilerBL otherProfilerBL;
        auto *treeHandler = ProfileApplicationTreeHandler::instance();
        if (treeHandler != nullptr)
        {
            auto *otherSessionTreeItemData = treeHandler->GetSessionNodeItemData(m_otherSessionId);
            if (otherSessionTreeItemData != nullptr)
            {
                const auto bIsReadOnly = true;
                if (otherProfilerBL.OpenPowerProfilingDatabaseForRead(otherSessionTreeItemData->m_filePath.asString(), bIsReadOnly))
                {
                    auto selectedItems = m_pStatisticsTable->selectedItems();
                    m_pDiffPlot->clearGraphs();

                    auto counterId = -1;
                    QString counterName;

                    // Extract the counterId and counterName from the table selection
                    for (auto *item : selectedItems)
                    {
                        switch (item->column())
                        {
                            case kEnumStatsTableColumn_CounterId:
                                counterId = item->data(Qt::UserRole).toUInt();
                                break;

                            case kEnumStatsTableColumn_CounterName:
                                counterName = item->text();
                                break;

                            default:
                                break;
                        }

                        if(counterId != -1 && !counterName.isEmpty())
                        {
                            break;
                        }
                    }

                    if(counterId == -1 || counterName.isEmpty())
                    {
                        return;
                    }

                    SamplingTimeRange sessionRange{ 0, 0 };
                    profilerBL.GetSessionTimeRange(sessionRange);

                    SamplingTimeRange otherSessionRange{ 0, 0 };
                    otherProfilerBL.GetSessionTimeRange(otherSessionRange);

                    // Filter the items from the first column.
                    // We don't care about other columns because we are selecting rows.
                    gtVector<QTableWidgetItem*> rowItems;
                    std::copy_if(selectedItems.begin(), selectedItems.end(), std::back_inserter(rowItems), [](QTableWidgetItem *item)
                    {
                        return item->column() == 0;
                    });

                    for (const auto *item : rowItems)
                    {
                        // Get the counter id
                        const int rowCounterId = item->data(Qt::UserRole).toUInt();

                        gtMap<int, gtVector<SampledValue>> sessionSampleValues, otherSessionSampleValues;
                        profilerBL.GetSampledValuesByRange({ rowCounterId }, sessionRange, sessionSampleValues);
                        otherProfilerBL.GetSampledValuesByRange({ rowCounterId }, otherSessionRange, otherSessionSampleValues);

                        const auto& samplesForCounter = sessionSampleValues[rowCounterId];
                        const auto& otherSamplesForCounter = otherSessionSampleValues[rowCounterId];

                        // Data to set on the plot for displaying the actual counter data
                        QVector<double> xValues(samplesForCounter.size());
                        QVector<double> yValues(samplesForCounter.size()), yOtherValues(samplesForCounter.size());

                        // This will hold the different data
                        QVector<double> diffYValues(samplesForCounter.size());

                        // Builds the data vectors to display counter data
                        for (size_t iSampleIndex = 0; iSampleIndex < samplesForCounter.size(); ++iSampleIndex)
                        {
                            xValues[iSampleIndex] = samplesForCounter[iSampleIndex].m_sampleTime;
                            yValues[iSampleIndex] = samplesForCounter[iSampleIndex].m_sampleValue;

                            if (iSampleIndex < otherSamplesForCounter.size())
                            {
                                yOtherValues[iSampleIndex] = otherSamplesForCounter[iSampleIndex].m_sampleValue;
                            }
                            else
                            {
                                yOtherValues[iSampleIndex] = yValues[iSampleIndex];
                            }
                        }

                        // Calculates the difference between the two graphs
                        for (auto iIndex = 0; iIndex < xValues.size(); ++iIndex)
                        {
                            diffYValues[iIndex] = qAbs(yValues[iIndex] - yOtherValues[iIndex]);
                        }

                        // Add the graphs to the plot
                        auto *graph = m_pDiffPlot->addGraph();
                        graph->setName(QString{ PP_STR_CompareToSession_ThisCounterLegend }.arg(counterName));
                        graph->setPen(QPen{ Qt::blue });
                        graph->setData(xValues, yValues);

                        auto *otherGraph = m_pDiffPlot->addGraph();
                        otherGraph->setPen(QPen{ Qt::red });
                        otherGraph->setName(QString{ PP_STR_CompareToSession_IncomingCounterLegend }.arg(counterName));
                        otherGraph->setData(xValues, yOtherValues);

                        auto *diffGraph = m_pDiffPlot->addGraph();
                        diffGraph->setName(PP_STR_CompareToSession_DiffLegend);

                        // Shows the difference as a blue dotted line
                        QPen blueDotPen;
                        blueDotPen.setColor(QColor{ 30, 40, 255, 150 });
                        blueDotPen.setStyle(Qt::DotLine);
                        blueDotPen.setWidth(4);

                        diffGraph->setPen(blueDotPen);
                        diffGraph->setData(xValues, diffYValues);

                        // Rescale the graph to show all the data
                        m_pDiffPlot->rescaleAxes();

                        // Finally updates the plot widget
                        m_pDiffPlot->replot();
                    }
                }
            }
        }
    }
}

/// @brief  Add the session name to the combo box so the user can select it for the comparison.
/// @param[in] vpSessionData The SessionTreeNodeData to be added to the combo box.
void LPGPU2ppSessionComparisonView::AddSessionToComparisonList(SessionTreeNodeData *vpSessionData) const
{
    if (vpSessionData != nullptr && vpSessionData != m_pSessionNodeData && m_pSessionComboBox != nullptr)
    {
        m_pSessionComboBox->addItem(vpSessionData->m_displayName, QVariant::fromValue(vpSessionData->m_sessionId));
    }
}
