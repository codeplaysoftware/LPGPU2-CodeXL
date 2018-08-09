// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines Calls Per Type View widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppCallsPerTypeView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppPieChartWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Qt:
#include <QGroupBox>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QStyleFactory>
#include <QHeaderView>
#include <QSplitter>

// LPGPU2Database:
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// STL
#include <memory>
#include <vector>

using lpgpu2::PPFnStatus;

struct TableColumns
{
  enum EnumColumnNames
  {
      event = 0,
      numberOfCalls = 1,
      colour = 2,
      numberOfColumns
  };
};

static const QString kEventColStrHeaderName{"Event"};
static const QString kNumOfCallsColStrHeaderName{"% of calls"};
static const QString kColourColStrHeaderName{"Colour"};
static constexpr float kPiechartRatio = 0.7f;

/// @brief Class constructor. No work is performed here.
LPGPU2ppCallsPerTypeView::LPGPU2ppCallsPerTypeView(QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
{
}

/// @brief Class destructor. Calls the Shutdown() method to cleanup resources used
///        by this class.
LPGPU2ppCallsPerTypeView::~LPGPU2ppCallsPerTypeView()
{
    Shutdown();
}

/// @brief              Initialises the widget components and layout.
/// @return PPFnStauts  success: All the components were initialised and
///                     are ready to be used,
///                     failure: An error has occurred while initialising
///                     the widgets components.
PPFnStatus LPGPU2ppCallsPerTypeView::Initialise()
{
    QVBoxLayout* pMainLayout = nullptr;
    LPGPU2PPNewQtWidget(&pMainLayout, this);

    LPGPU2PPNewQtWidget(&m_pSplitter, this);
    pMainLayout->addWidget(m_pSplitter);

    LPGPU2PPNewQtWidget(&m_pPieChartWidget, this);
    LPGPU2PPNewQtWidget(&m_pTableWidget, this);
    
    m_pSplitter->setStyle(QStyleFactory::create(PP_STR_SplitterStyleName));
    m_pSplitter ->setHandleWidth(6);
    m_pSplitter->setStyleSheet(PP_STR_SplitterStyleSheet);

    m_pSplitter->addWidget(m_pPieChartWidget);
    m_pSplitter->addWidget(m_pTableWidget);

    auto initRC = InitialiseTableWidgetLayout();
    if (initRC != PPFnStatus::success)
    {
      return PPFnStatus::failure;
    }

    pMainLayout->setMargin(0);    

    connect(m_pPieChartWidget, &LPGPU2ppPieChartWidget::ItemClicked,
        this, &LPGPU2ppCallsPerTypeView::OnPieChartItemClicked);
    connect(m_pTableWidget, &QTableWidget::cellClicked,
        this, &LPGPU2ppCallsPerTypeView::OnTableWidgetCellChanged);

    return PPFnStatus::success;

}

/// @brief Deletes the resources used by this class.
/// @return Always return success.
PPFnStatus LPGPU2ppCallsPerTypeView::Shutdown()
{
    if (m_pPieChartWidget != nullptr)
    {
        delete m_pPieChartWidget;
        m_pPieChartWidget = nullptr;
    }

    if(m_pGroupBox != nullptr)
    {
        delete m_pGroupBox;
        m_pGroupBox = nullptr;
    }

    return PPFnStatus::success;
}

/// @brief Set the CallsPerType object to be displayed in the pie chart. This
///        will set the pie chart data points with pre-defined colours.
/// @param[in] vCallsPerType The CallsPerType object that will be used to build
///                          the pie chart.
/// @return     PPFnStauts   success: All data points were added to the pie chart,
///                          failure: The pie chart was not created or some data
///                          points could not be added to the pie chart.
/// @see LPGPU2ppPieChartWidget::AddDataPoint().
PPFnStatus LPGPU2ppCallsPerTypeView::SetCallsPerType(const lpgpu2::db::CallsPerType &vCallsPerType) const
{
    auto bReturn = PPFnStatus::failure;

    if (m_pPieChartWidget != nullptr)
    {
        std::vector<LPGPU2ppPieChartWidget::DataPoint> dataPoints;
        LPGPU2ppPieChartWidget::DataPoint drawPercentagePoint;
        drawPercentagePoint.m_value = vCallsPerType.m_drawPercentage;
        drawPercentagePoint.m_colour = QColor{ Qt::blue };
        drawPercentagePoint.m_label = PP_STR_CallsPerTypeView_DrawLabel;
        drawPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_DrawToolTip;
        dataPoints.push_back(drawPercentagePoint);

        LPGPU2ppPieChartWidget::DataPoint bindPercentagePoint;
        bindPercentagePoint.m_value = vCallsPerType.m_bindPercentage;
        bindPercentagePoint.m_colour = QColor{ Qt::cyan };
        bindPercentagePoint.m_label = PP_STR_CallsPerTypeView_BindLabel;
        bindPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_BindTooltip;
        dataPoints.push_back(bindPercentagePoint);

        LPGPU2ppPieChartWidget::DataPoint setupPercentagePoint;
        setupPercentagePoint.m_value = vCallsPerType.m_setupPercentage;
        setupPercentagePoint.m_colour = QColor{ Qt::magenta };
        setupPercentagePoint.m_label = PP_STR_CallsPerTypeView_SetupLabel;
        setupPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_SetupTooltip;
        dataPoints.push_back(setupPercentagePoint);

        LPGPU2ppPieChartWidget::DataPoint waitPercentagePoint;
        waitPercentagePoint.m_value = vCallsPerType.m_waitPercentage;
        waitPercentagePoint.m_colour = QColor{ Qt::green };
        waitPercentagePoint.m_label = PP_STR_CallsPerTypeView_WaitLabel;
        waitPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_WaitTooltip;
        dataPoints.push_back(waitPercentagePoint);

        LPGPU2ppPieChartWidget::DataPoint errorPercentagePoint;
        errorPercentagePoint.m_value = vCallsPerType.m_errorPercentage;
        errorPercentagePoint.m_colour = QColor{ Qt::red };
        errorPercentagePoint.m_label = PP_STR_CallsPerTypeView_ErrorLabel;
        errorPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_ErrorTooltip;
        dataPoints.push_back(errorPercentagePoint);

        LPGPU2ppPieChartWidget::DataPoint delimiterPercentagePoint;
        delimiterPercentagePoint.m_value = vCallsPerType.m_delimiterPercentage;
        delimiterPercentagePoint.m_colour = QColor{ Qt::yellow };
        delimiterPercentagePoint.m_label = PP_STR_CallsPerTypeView_DelimitersLabel;
        delimiterPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_DelimitersTooltip;
        dataPoints.push_back(delimiterPercentagePoint);

        LPGPU2ppPieChartWidget::DataPoint otherPercentagePoint;
        otherPercentagePoint.m_value = vCallsPerType.m_otherPercentage;
        otherPercentagePoint.m_colour = QColor{ Qt::darkCyan };
        otherPercentagePoint.m_label = PP_STR_CallsPerTypeView_OtherLabel;
        otherPercentagePoint.m_tooltip = PP_STR_CallsPerTypeView_OtherTooltip;
        dataPoints.push_back(otherPercentagePoint);

        auto bAreAllPointsOk = true;

        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(drawPercentagePoint);
        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(bindPercentagePoint);
        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(setupPercentagePoint);
        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(waitPercentagePoint);
        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(errorPercentagePoint);
        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(delimiterPercentagePoint);
        bAreAllPointsOk &= PPFnStatus::success == m_pPieChartWidget->AddDataPoint(otherPercentagePoint);

        bReturn = bAreAllPointsOk ? PPFnStatus::success : PPFnStatus::failure;

        GT_ASSERT(m_pTableWidget);
        m_pTableWidget->setRowCount(dataPoints.size());
        for(size_t iItemIndex = 0; iItemIndex < dataPoints.size(); ++iItemIndex)
        {
          const auto &dp = dataPoints[iItemIndex];

          auto *pLabelWidgetItem = new QTableWidgetItem{ acGTStringToQString(dp.m_label) };
          pLabelWidgetItem->setData(Qt::UserRole, QVariant::fromValue(iItemIndex));          

          m_pTableWidget->setItem(iItemIndex, TableColumns::event, pLabelWidgetItem);
          m_pTableWidget->setItem(iItemIndex, TableColumns::numberOfCalls,
              new QTableWidgetItem{QString::number(dp.m_value)});

          // Draw an icon:
          auto *pIconPixmap = new QPixmap{20, 20};
          auto *pIconPainter = new QPainter(pIconPixmap);
          pIconPainter->fillRect(0, 0, 20, 20, Qt::black);
          pIconPainter->fillRect(1, 1, 19, 19, dp.m_colour);
          auto *itemColour = new QTableWidgetItem{};
          itemColour->setIcon(QIcon{*pIconPixmap});

          m_pTableWidget->setItem(iItemIndex, TableColumns::colour,
              itemColour);
        }

        m_pTableWidget->resizeColumnsToContents();
    }
    else
    {
        bReturn = PPFnStatus::failure;
    }

    return bReturn;
}

/// @brief Method called when a sector of the pie chart was clicked. This
///        sets the selection behaviour. It will deselects all data points
///        and select the one that was clicked. Is can be defined as "SingleSelection"
///        behaviour of the QTableWidget.
/// @param[in] vItemIndex The index of the data point that was clicked.
void LPGPU2ppCallsPerTypeView::OnPieChartItemClicked(const int vItemIndex) const
{
    if (m_pPieChartWidget != nullptr && m_pTableWidget != nullptr)
    {
        m_pPieChartWidget->ClearSelection();
        
        const auto bIsSelected = true;
        m_pPieChartWidget->SetPointSelection(vItemIndex, bIsSelected);
        // Select the relative entry in the table
        auto *itemToSelect = m_pTableWidget->item(vItemIndex, 0);
        if (itemToSelect != nullptr)
        {
          m_pTableWidget->setCurrentItem(itemToSelect);
        }
    }
}

void LPGPU2ppCallsPerTypeView::OnTableWidgetCellChanged(int row, int column)
{
  GT_UNREFERENCED_PARAMETER(column);

  GT_IF_WITH_ASSERT(m_pPieChartWidget != nullptr && m_pTableWidget != nullptr)
  {
    m_pPieChartWidget->ClearSelection();

    const auto *pTableItem = m_pTableWidget->item(row, 0);
    const auto itemIndex = pTableItem->data(Qt::UserRole).toInt();
    
    const auto bIsSelected = true;
    m_pPieChartWidget->SetPointSelection(itemIndex, bIsSelected);
  }
}

/// @brief  Initialises the table widget layout.
/// @return PPFnStatus  success: The table widget layout was initialised,
///                     failure: An error has occurred when initialising the
///                     table widget layout.
PPFnStatus LPGPU2ppCallsPerTypeView::InitialiseTableWidgetLayout()
{
  auto bReturn = PPFnStatus::failure;

  if (m_pTableWidget != nullptr)
  {
    // Sets the table style and behaviour
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

    // I choose this method of initialising the headers (using QTableWidgetItem's)
    // of the table because it allows changing properties for each header individually.

    auto eventNameHeaderItem =
      std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };
    auto numberOfCallsHeaderItem =
      std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };
    auto colourHeaderItem =
      std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };

    if (eventNameHeaderItem != nullptr &&
        numberOfCallsHeaderItem != nullptr &&
        colourHeaderItem != nullptr)
    {
      eventNameHeaderItem->setText(kEventColStrHeaderName);
      numberOfCallsHeaderItem->setText(kNumOfCallsColStrHeaderName);
      colourHeaderItem->setText(kColourColStrHeaderName);

      m_pTableWidget->setColumnCount(TableColumns::numberOfColumns);
      m_pTableWidget->setHorizontalHeaderItem(TableColumns::event,
          eventNameHeaderItem.release());
      m_pTableWidget->setHorizontalHeaderItem(TableColumns::numberOfCalls,
          numberOfCallsHeaderItem.release());
      m_pTableWidget->setHorizontalHeaderItem(TableColumns::colour,
          colourHeaderItem.release());

      bReturn = PPFnStatus::success;
    }
  }

  return bReturn;
}

void LPGPU2ppCallsPerTypeView::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  if (!m_isInitialised && m_pSplitter != nullptr)
  {
    auto splitterSizes = m_pSplitter->sizes();
    GT_ASSERT(splitterSizes.size() == 2);
    auto totalWidth = static_cast<float>(splitterSizes[0] + splitterSizes[1]);
    decltype(splitterSizes) newSizes;
    newSizes.push_back(static_cast<int>(totalWidth * kPiechartRatio));
    newSizes.push_back(static_cast<int>(totalWidth * (1.f - kPiechartRatio)));
    m_pSplitter->setSizes(newSizes);
    m_isInitialised = true;
  }
}


