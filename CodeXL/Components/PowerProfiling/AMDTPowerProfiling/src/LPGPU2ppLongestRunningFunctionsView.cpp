// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Longest running functions widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

//  Local:
#include <AMDTPowerProfiling/src/LPGPU2ppLongestRunningFunctionsView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppNumericTableWidgetItem.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// STL:
// Introduces unique_ptr
#include <memory>

// Declarations
using lpgpu2::PPFnStatus;

// These are old style enums because the intention is to use them
// to name the columns of the table this class represents. In order
// to avoid casting this to int we are making this enum like this.
// Moreover, this is a private enum used only by this file.
enum EnumColumnNames
{
    kEnumColumnNames_FunctionName = 0,
    kEnumColumnNames_NumberOfCalls = 1,
    kEnumColumnNames_PercentageOfTime = 2,
    kEnumColumnNames_NumberOfColumns
};

/// @brief              Class constructor. No work is performed here.
/// @param[in] vpParent The parent of this widget.
LPGPU2ppLongestRunningFunctionsView::LPGPU2ppLongestRunningFunctionsView(QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
{
}

/// @brief Class destructor. Calls the Shutdown method to cleanup
///        resources used by this class.
LPGPU2ppLongestRunningFunctionsView::~LPGPU2ppLongestRunningFunctionsView()
{
    Shutdown();
}

/// @brief  Initialises the components and the layout of this widget.
/// @return PPFnStatus  success: If the components were initialised,
///                     failure: An error has occurred.
PPFnStatus LPGPU2ppLongestRunningFunctionsView::Initialise()
{
    auto bReturn = PPFnStatus::failure;    
    
    LPGPU2PPNewQtWidget(&m_pGroupBox, this);
    LPGPU2PPNewQtWidget(&m_pTableWidget, m_pGroupBox);

    bReturn = InitialiseTableWidgetLayout();

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

/// @brief             Cleanup resources used by this class.
/// @return PPFnStatus Always return success.
PPFnStatus LPGPU2ppLongestRunningFunctionsView::Shutdown()
{
    if (m_pTableWidget != nullptr)
    {
        delete m_pTableWidget;
        m_pTableWidget = nullptr;
    }

    if(m_pGroupBox != nullptr)
    {
        delete m_pGroupBox;
        m_pGroupBox = nullptr;
    }

    return PPFnStatus::success;
}

/// @brief                       Set the call summary data and builds the table of the 
///                              longest running functions.
/// @param[in] vAccumulatedTime  The accumulated time of all the function calls. This is
///                              used to calculate the relative time that one particular
///                              function took to run. This can be obtained from the
///                              database adapter.
/// @param[in] vCallSummaryList  The list of lpgpu2::db::CallSummary objects to display in
///                              the table of longest running functions.
/// @return    PPFnStatus        success: The table was constructed and all function calls
///                              are ready to display,
///                              failure: The table widget was not initialised, vAccumulatedTime is
///                              0 or a memory allocation problem was detected.
/// @see                         LPGPU2DatabaseAdapter::GetAccumulatedCallSummaryTime().
PPFnStatus LPGPU2ppLongestRunningFunctionsView::SetCallSummaryData(const gtUInt64 vAccumulatedTime, const gtVector<lpgpu2::db::CallSummary> &vCallSummaryList) const
{
    auto bReturn = PPFnStatus::failure;

    if (m_pTableWidget != nullptr && vAccumulatedTime != 0)
    {
        m_pTableWidget->clearContents();
        m_pTableWidget->setRowCount(vCallSummaryList.size());
        
        // NOTE: Used to check that the percentage was at least 1%, but calls generally take up a very 
        //       small portion of execution time so this was arbitrarily cutting off too much data.
        for (gtUInt32 iCallSummaryIndex = 0; iCallSummaryIndex < vCallSummaryList.size(); ++iCallSummaryIndex)
        {
            const auto& callSummary = vCallSummaryList[iCallSummaryIndex];
            const auto percentageOfTime = callSummary.m_totalTime / static_cast<double>(vAccumulatedTime);

            auto funcNameItem = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>{ new (std::nothrow) LPGPU2ppNumericTableWidgetItem{} };
            auto numberOfCallsItem = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>{ new (std::nothrow) LPGPU2ppNumericTableWidgetItem{} };
            auto percentageOfTimeItem = std::unique_ptr<LPGPU2ppNumericTableWidgetItem>{ new (std::nothrow) LPGPU2ppNumericTableWidgetItem{} };

            if (funcNameItem != nullptr &&
                numberOfCallsItem != nullptr &&
                percentageOfTimeItem != nullptr)
            {
                funcNameItem->setText(acGTStringToQString(callSummary.m_callName));
                numberOfCallsItem->setText(QString::number(callSummary.m_timesCalled));
                percentageOfTimeItem->setText(QString::number(percentageOfTime * 100, 'f', 2));

                numberOfCallsItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);
                percentageOfTimeItem->setTextAlignment(Qt::AlignTrailing | Qt::AlignVCenter);

                // QTableWidget takes ownership on the QTableWidgetItem's
                m_pTableWidget->setItem(iCallSummaryIndex, kEnumColumnNames_FunctionName, funcNameItem.release());
                m_pTableWidget->setItem(iCallSummaryIndex, kEnumColumnNames_NumberOfCalls, numberOfCallsItem.release());
                m_pTableWidget->setItem(iCallSummaryIndex, kEnumColumnNames_PercentageOfTime, percentageOfTimeItem.release());
            }
            else
            {
                bReturn = PPFnStatus::failure;
                break;
            }
            
            if (bReturn == PPFnStatus::success)
            {
                m_pTableWidget->sortByColumn(kEnumColumnNames_PercentageOfTime, Qt::DescendingOrder);
            }
        }
    }    

    return bReturn;
}

/// @brief  Initialises the table widget layout.
/// @return PPFnStatus  success: The table widget layout was initialised,
///                     failure: An error has occurred when initialising the
///                     table widget layout.
PPFnStatus LPGPU2ppLongestRunningFunctionsView::InitialiseTableWidgetLayout() const
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

        auto funcNameHeaderItem = std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };
        auto numberOfCallsHeaderItem = std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };
        auto percentageOfCallsHeaderItem = std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };

        if (funcNameHeaderItem != nullptr &&
            numberOfCallsHeaderItem != nullptr &&
            percentageOfCallsHeaderItem != nullptr)
        {
            funcNameHeaderItem->setText(PP_STR_LongestRunningFunctionsView_FunctionColumnName);
            numberOfCallsHeaderItem->setText(PP_STR_LongestRunningFunctionsView_NumberOfCallsColumnName);
            percentageOfCallsHeaderItem->setText(PP_STR_LongestRunningFunctionsView_PercentageOfTimeColumnName);

            m_pTableWidget->setColumnCount(kEnumColumnNames_NumberOfColumns);
            m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_FunctionName, funcNameHeaderItem.release());
            m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_NumberOfCalls, numberOfCallsHeaderItem.release());
            m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_PercentageOfTime, percentageOfCallsHeaderItem.release());

            bReturn = PPFnStatus::success;
        }
    }    

    return bReturn;
}
