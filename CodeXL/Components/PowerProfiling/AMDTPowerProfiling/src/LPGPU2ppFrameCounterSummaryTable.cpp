// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameCounterSummaryTable.cpp
///
/// @brief Separate tab view to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// LPGPU2ppFrameAPICallTable definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppFrameCounterSummaryTable.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>

// Framework:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Qt:
#include <QSplitter>
#include <QStyleFactory>

using lpgpu2::PPFnStatus;

// These are old style enums because the intention is to use them
// to name the columns of the table this class represents. In order
// to avoid casting this to int we are making this enum like this.
// Moreover, this is a private enum used only by this file.
enum EnumAPICallColumnNames
{
    kEnumColumnNames_Id = 0,
    kEnumColumnNames_Name = 1,
    kEnumColumnNames_Value = 2,
    kEnumColumnNames_Delta = 3,
    kEnumColumnNames_SessionAvg = 4,
    kEnumColumnNames_NumberOfColumns
};

/// @brief Ctor. No work is done here
///
/// @param pParent            The QWidget which owns this one
/// @param frameId            The number of the frame this widget is displaying
/// @param frameStart         The starting time of the given frame
LPGPU2ppFrameCounterSummaryTable::LPGPU2ppFrameCounterSummaryTable(QWidget* pParent, ppSessionController* pSessionController, quint64 frameId, quint64 frameEnd)
: QWidget(pParent)
, m_pSessionController(pSessionController)
, m_frameId(frameId)
, m_frameEndTime(frameEnd)
{
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
///        we need to make sure that Shutdown was called anyway.
LPGPU2ppFrameCounterSummaryTable::~LPGPU2ppFrameCounterSummaryTable()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus LPGPU2ppFrameCounterSummaryTable::Initialise()
{
  PPFnStatus bReturn = PPFnStatus::failure;    
  
  LPGPU2PPNewQtWidget(&m_pGroupBox, this);
  LPGPU2PPNewQtWidget(&m_pTableWidget, m_pGroupBox);

  m_pGroupBox->setTitle(PP_STR_LPGPU2_CounterSummaryTitle);
  m_pGroupBox->setStyleSheet(PP_STR_LPGPU2_CSS_GroupBoxBold);

  bReturn = InitViewLayout();

  if (bReturn == PPFnStatus::success)
  {
    QVBoxLayout *pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this)

    pMainLayout->setMargin(0);
    pMainLayout->addWidget(m_pGroupBox);

    QVBoxLayout *pGroupBoxLayout = nullptr;
    LPGPU2PPNewQtWidget(&pGroupBoxLayout, m_pGroupBox);
    pGroupBoxLayout->addWidget(m_pTableWidget);

    bReturn = PopulateTable();        
  }

  return bReturn;
}

/// @brief Deinitialise the object
PPFnStatus LPGPU2ppFrameCounterSummaryTable::Shutdown()
{
  return PPFnStatus::success;
}

/// @brief Initialise the contained QTableWidget
PPFnStatus LPGPU2ppFrameCounterSummaryTable::InitViewLayout()
{
  PPFnStatus bReturn = PPFnStatus::failure;

  if (m_pTableWidget == nullptr)
  {
      return bReturn;
  }

  // Sets the table style and behavior
  m_pTableWidget->setDragDropOverwriteMode(true);
  m_pTableWidget->setAlternatingRowColors(true);
  m_pTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  m_pTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_pTableWidget->setSortingEnabled(true);
  m_pTableWidget->setCornerButtonEnabled(false);    
  m_pTableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
  m_pTableWidget->horizontalHeader()->setSortIndicatorShown(true);
  m_pTableWidget->horizontalHeader()->setStretchLastSection(true);
  m_pTableWidget->verticalHeader()->setVisible(false);
  
  auto idHeaderItem     = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto nameHeaderItem  = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto valueHeaderItem = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto deltaHeaderItem = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto avgHeaderItem   = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});


  if (idHeaderItem    != nullptr &&
      nameHeaderItem  != nullptr &&
      valueHeaderItem != nullptr &&
      deltaHeaderItem != nullptr &&
      avgHeaderItem   != nullptr)
  {
    idHeaderItem->setText(PP_STR_LPGPU2_CounterSummaryTableID);
    nameHeaderItem->setText(PP_STR_LPGPU2_CounterSummaryTableName);
    valueHeaderItem->setText(PP_STR_LPGPU2_CounterSummaryTableValue);
    deltaHeaderItem->setText(PP_STR_LPGPU2_CounterSummaryTableDelta);
    avgHeaderItem->setText(PP_STR_LPGPU2_CounterSummaryTableAvg);

    m_pTableWidget->setColumnCount(kEnumColumnNames_NumberOfColumns);
    m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_Id, idHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_Name, nameHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_Value, valueHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_Delta, deltaHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_SessionAvg, avgHeaderItem.release());

    bReturn = PPFnStatus::success;
  }
  
  return bReturn;
}

/// @brief Retrieve the approximated counter values from the database and display them
PPFnStatus LPGPU2ppFrameCounterSummaryTable::PopulateTable()
{
  auto& profilerBL = m_pSessionController->GetProfilerBL();
  size_t counterCount = 0; // In case the counter IDs aren't contiguous
  gtMap<AMDTPwrCounterDesc*,double> mapCounterValues;
  
  bool bOk = profilerBL.GetApproxCounterSamplesAtTimePoint(m_frameEndTime, mapCounterValues);
  if (!bOk)
  {
    return PPFnStatus::failure;
  }
  
  m_pTableWidget->setRowCount(mapCounterValues.size());
  
  for (auto counterDescValuePair : mapCounterValues)
  {
    auto idItem    = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
    auto nameItem  = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
    auto valueItem = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
    auto deltaItem = std::unique_ptr<LPGPU2ppTableItemPercentSortable>(new (std::nothrow) LPGPU2ppTableItemPercentSortable{});
    auto avgItem   = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
    
    const auto* counterDesc  = counterDescValuePair.first;
    const auto  counterValue = counterDescValuePair.second;
    const auto  counterId    = counterDesc->m_counterID;
    const auto  counterName  = counterDesc->m_name;

    gtMap<int, gtVector<SampledValue>> sessionSampleValues;
    SamplingTimeRange sessionRange{ 0, 0 };
    profilerBL.GetSessionTimeRange(sessionRange);
    profilerBL.GetSampledValuesByRange({static_cast<int>(counterId)}, sessionRange, sessionSampleValues);
    const auto counterAvg = GetCounterAverage(sessionSampleValues[counterId]);
    const auto counterDelta = (counterValue / counterAvg);
    auto counterDeltaColor = Qt::black;
    if (counterDelta > 1.01)
      counterDeltaColor = Qt::red;
    else if (counterDelta < 0.99)
      counterDeltaColor = Qt::darkGreen;
    
    if (idItem != nullptr && nameItem != nullptr && valueItem != nullptr && deltaItem != nullptr && avgItem != nullptr)
    {
      idItem->setText(QString::number(counterId));
      nameItem->setText(QString(counterName));
      valueItem->setText(QString::number(counterValue));
      deltaItem->setText(QString("%1%").arg(QString::number(counterDelta * 100, 'f', 2)));
      deltaItem->setTextColor(counterDeltaColor);
      avgItem->setText(QString::number(counterAvg));
      
      avgItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);
      
      m_pTableWidget->setItem(counterCount, kEnumColumnNames_Id, idItem.release());
      m_pTableWidget->setItem(counterCount, kEnumColumnNames_Name, nameItem.release());
      m_pTableWidget->setItem(counterCount, kEnumColumnNames_Value, valueItem.release());
      m_pTableWidget->setItem(counterCount, kEnumColumnNames_Delta, deltaItem.release());
      m_pTableWidget->setItem(counterCount, kEnumColumnNames_SessionAvg, avgItem.release());
      
      counterCount++;
    }
  }
  
  m_pTableWidget->resizeColumnsToContents();
  m_pTableWidget->sortItems(kEnumColumnNames_Id);
  
  // Need to delete AMDTPwrCounterDesc*
  for (const auto& counterDetailsPair : mapCounterValues)
  {
    if (counterDetailsPair.first != nullptr)
    {
      delete counterDetailsPair.first;
    }
  }

  return PPFnStatus::success;
}

/// @brief Get the average of a vector of sample values
double LPGPU2ppFrameCounterSummaryTable::GetCounterAverage(const gtVector<SampledValue>& vSampledValues)
{
    auto average = 0.0;    

    if (!vSampledValues.empty())
    {
        // Compute the total sum of all the sampled values
        const double sum = std::accumulate(vSampledValues.begin(), vSampledValues.end(), 0.0, [](double currentValue, const SampledValue& vSampledValue)
        {
            return currentValue + vSampledValue.m_sampleValue;
        });

        // Divide the sum by the amount of samples.
        average = sum / vSampledValues.size();
    }
    
    return average;
}