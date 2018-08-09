// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameAPICallTable.cpp
///
/// @brief Separate tab view to display source code using QScintilla as the
///	       the base component. Will be expanded, this is a proof of concept.
///
/// LPGPU2ppFrameAPICallTable definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppFrameAPICallTable.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppNumericTableWidgetItem.h>

// Framework:
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include <cstring>
#include <sstream>
#include <ios>

using lpgpu2::PPFnStatus;

// These are old style enums because the intention is to use them
// to name the columns of the table this class represents. In order
// to avoid casting this to int we are making this enum like this.
// Moreover, this is a private enum used only by this file.
struct APICallsColumns {
  enum EnumAPICallColumns
  {
      function = 0,
      params,
      retVal,
      drawNumber,
      startTime,
      endTime,
      duration,
      numberOfColumns
  };
};

/// @brief Ctor. No work is done here
///
/// @param pParent            The QWidget which owns this one
/// @param frameId            The number of the frame this widget is displaying
LPGPU2ppFrameAPICallTable::LPGPU2ppFrameAPICallTable(QWidget* pParent, quint64 frameId)
: QWidget(pParent)
, m_frameId(frameId)
{
}

/// @brief Dtor. Normally no work is done here, but because of how Qt works
///        we need to make sure that Shutdown was called anyway.
LPGPU2ppFrameAPICallTable::~LPGPU2ppFrameAPICallTable()
{
  Shutdown();
}

/// @brief Initialise the object
PPFnStatus LPGPU2ppFrameAPICallTable::Initialise()
{
  PPFnStatus bReturn = PPFnStatus::failure;    
  
  LPGPU2PPNewQtWidget(&m_pGroupBox, this);
  LPGPU2PPNewQtWidget(&m_pTableWidget, m_pGroupBox);

  m_pGroupBox->setTitle("API Calls For This Frame");
  m_pGroupBox->setStyleSheet("QGroupBox { font-weight: bold; }");

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

    bReturn = PPFnStatus::success;
  }

  return bReturn;
}

/// @brief Deinitialise the object
PPFnStatus LPGPU2ppFrameAPICallTable::Shutdown()
{
  return PPFnStatus::success;
}

/// @brief Initialise the contained QTableWidget
PPFnStatus LPGPU2ppFrameAPICallTable::InitViewLayout()
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
  
  auto functionHeaderItem  = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto drawNumHeaderItem   = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto startTimeHeaderItem = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto endTimeHeaderItem   = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto durationHeaderItem  = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
  auto paramsHeaderItem = std::unique_ptr<QTableWidgetItem>(
      new (std::nothrow) QTableWidgetItem{});
  auto retValHeaderItem = std::unique_ptr<QTableWidgetItem>(
      new (std::nothrow) QTableWidgetItem{});

  if (functionHeaderItem  != nullptr &&
      drawNumHeaderItem   != nullptr &&
      startTimeHeaderItem != nullptr &&
      endTimeHeaderItem   != nullptr &&
      paramsHeaderItem    != nullptr &&
      retValHeaderItem    != nullptr &&
      durationHeaderItem  != nullptr)
  {
    functionHeaderItem->setText("Function");
    drawNumHeaderItem->setText("Draw Number");
    startTimeHeaderItem->setText("Start Time (ns)");
    endTimeHeaderItem->setText("End Time (ns)");
    durationHeaderItem->setText("Duration (ns)");
    paramsHeaderItem->setText("Parameters");
    retValHeaderItem->setText("Return value");

    m_pTableWidget->setColumnCount(APICallsColumns::numberOfColumns);
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::function, functionHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::drawNumber, drawNumHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::startTime, startTimeHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::endTime, endTimeHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::duration, durationHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::params, paramsHeaderItem.release());
    m_pTableWidget->setHorizontalHeaderItem(
        APICallsColumns::retVal, retValHeaderItem.release());

    bReturn = PPFnStatus::success;
  }

  connect(m_pTableWidget, &QTableWidget::itemSelectionChanged, this,
      &LPGPU2ppFrameAPICallTable::OnTableItemClicked);

  return bReturn;
}

/// @brief Populate the table with API call trace entries
/// @param vecTraces    Vector of traces to populate the table with.
PPFnStatus LPGPU2ppFrameAPICallTable::PopulateTable(
    const gtVector<lpgpu2::db::ProfileTraceInfo>& vecTraces,
    const gtVector<lpgpu2::db::StackTraceParameter>& vecParams)
{
  m_pTableWidget->setRowCount(vecTraces.size());
  size_t traceCount = 0;
 
  size_t i = 0;
  size_t p = 0;
  const gtUInt32 num_params = vecParams.size();
  for (; i < vecTraces.size(); ++i)
  {
    const auto &trace = vecTraces[i];    

    auto functionItem  = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
    auto drawNumItem   = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>(new (std::nothrow) LPGPU2ppNumericTableWidgetItem{});
    auto startTimeItem = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>(new (std::nothrow) LPGPU2ppNumericTableWidgetItem{});
    auto endTimeItem   = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>(new (std::nothrow) LPGPU2ppNumericTableWidgetItem{});
    auto durationItem  = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>(new (std::nothrow) LPGPU2ppNumericTableWidgetItem{});
    auto paramsItem = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>(
        new (std::nothrow) LPGPU2ppNumericTableWidgetItem{});
    auto retValItem = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>(
        new (std::nothrow) LPGPU2ppNumericTableWidgetItem{});

    if (functionItem  != nullptr &&
        drawNumItem   != nullptr &&
        startTimeItem != nullptr &&
        endTimeItem   != nullptr &&
        paramsItem    != nullptr &&
        retValItem    != nullptr &&
        durationItem  != nullptr)
    {

      // Parse params and return type
      gtString paramsStr;
      gtString retValStr;

      if (!vecParams.empty() && p < vecParams.size())
      {
          const auto &params = vecParams[p];

          if ((p < num_params) && (params.m_drawNum == trace.m_drawNum))
          {
              gtStringTokenizer tokenizerParamsNames{ params.m_typeDefDescription, L"," };
              gtStringTokenizer tokenizerParamsSizes{ params.m_paramsSize, L"," };
              gtString paramsNameToken;
              gtString paramsSizeToken;
              size_t binaryParamsOffset = 0;

              // First token in list is always the return type of the function
              tokenizerParamsNames.getNextToken(paramsNameToken);
              retValStr.append(paramsNameToken);

              // Parse the parameters string
              while (tokenizerParamsNames.getNextToken(paramsNameToken) &&
                  tokenizerParamsSizes.getNextToken(paramsSizeToken)) {
                  if (!paramsStr.isEmpty())
                  {
                      paramsStr << L", ";
                  }

                  paramsStr.append(paramsNameToken);

                  gtUInt64 sizeInBytesAsUInt = 0;
                  GT_ASSERT(paramsSizeToken.toUnsignedInt64Number(sizeInBytesAsUInt));

                  // If it is a float and not a pointer
                  if (paramsNameToken.count(L"float") > 0 &&
                      paramsNameToken.count(L"*") == 0)
                  {
                      float value = 0;
                      memcpy(&value, &params.m_binaryParams[binaryParamsOffset],
                          sizeInBytesAsUInt);
                      paramsStr << L": " << value;

                      binaryParamsOffset += sizeInBytesAsUInt;
                      continue;
                  }

                  gtUInt64 value = 0;
                  memcpy(&value, &params.m_binaryParams[binaryParamsOffset],
                      sizeInBytesAsUInt);

                  paramsStr << L": ";

                  // If it is a pointer
                  if (paramsNameToken.count(L"*") > 0)
                  {
                      std::stringstream ss;
                      ss << std::hex << value;
                      std::string stdStrAsHex;
                      ss >> stdStrAsHex;
                      gtString gtStrAsHex;
                      gtStrAsHex << stdStrAsHex.c_str();
                      paramsStr << L"0x" << gtStrAsHex;
                      binaryParamsOffset += sizeInBytesAsUInt;

                      continue;
                  }

                  // Base case
                  paramsStr << static_cast<gtUInt32>(value);
                  binaryParamsOffset += sizeInBytesAsUInt;

              }

              // Parse the return value
              {
                  gtUInt64 retValue = 0;
                  memcpy(&retValue, params.m_binaryReturn.data(),
                      params.m_returnSize);

                  retValStr << L": " << static_cast<gtUInt32>(retValue);
              }

              ++p;
          }
      }      

      functionItem->setText(acGTStringToQString(trace.m_callName));
      drawNumItem->setText(QString::number(trace.m_drawNum));
      startTimeItem->setText(QString::number(trace.m_cpuStart));
      endTimeItem->setText(QString::number(trace.m_cpuEnd));
      durationItem->setText(QString::number(trace.m_cpuEnd - trace.m_cpuStart));
      paramsItem->setText(acGTStringToQString(paramsStr));
      retValItem->setText(acGTStringToQString(retValStr));

      drawNumItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);
      startTimeItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);
      endTimeItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);
      durationItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);

      // QTableWidget takes ownership on the QTableWidgetItems
      m_pTableWidget->setItem(traceCount, APICallsColumns::function, functionItem.release());
      m_pTableWidget->setItem(traceCount, APICallsColumns::drawNumber, drawNumItem.release());
      m_pTableWidget->setItem(traceCount, APICallsColumns::startTime, startTimeItem.release());
      m_pTableWidget->setItem(traceCount, APICallsColumns::endTime, endTimeItem.release());
      m_pTableWidget->setItem(traceCount, APICallsColumns::duration, durationItem.release());
      m_pTableWidget->setItem(traceCount, APICallsColumns::params,
          paramsItem.release());
      m_pTableWidget->setItem(traceCount, APICallsColumns::retVal,
          retValItem.release());
      traceCount++;
    }
  }
  m_pTableWidget->resizeColumnsToContents();
  
  return PPFnStatus::success;
}

/// @brief Slot for when an item in the API call table is clicked. Detects which
///        item was clicked and emits APICallSelected signal.
void LPGPU2ppFrameAPICallTable::OnTableItemClicked()
{
  QModelIndexList selection = m_pTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selection.count(); i++)
  {
    // Use the row to get to the selected item's draw number:
    bool bOk;
    auto item = m_pTableWidget->item(
        selection.at(i).row(), APICallsColumns::drawNumber)->text().toInt(&bOk);
    if (bOk)
    {
      emit APICallSelected(item);
    }
  }
}

/// @brief Changes the selection of the API call table to the given row.
PPFnStatus LPGPU2ppFrameAPICallTable::SelectRow(quint64 drawNum)
{
  m_pTableWidget->selectRow(drawNum);
  return PPFnStatus::success;
}
