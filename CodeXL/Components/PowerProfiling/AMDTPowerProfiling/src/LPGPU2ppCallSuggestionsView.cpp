// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Call Suggestions View widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppCallSuggestionsView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppInfoWidget.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

// Qt
#include <QTabWidget>
#include <QLineEdit>

using lpgpu2::PPFnStatus;

// These are old style enums because the intention is to use them
// to name the columns of the table this class represents. In order
// to avoid casting this to int we are making this enum like this.
// Moreover, this is a private enum used only by this file.
enum EnumColumnNames
{    
    kEnumColumnNames_FrameNumber = 0,
    kEnumColumnNames_CallSuggestion,
    KEnumColumnNames_NumberOfColumns
};

static const QString kLocalTabLabel{"Local"};
static const QString kGlobalTabLabel{"Global"};
static const QString kUserTabLabel{"User"};

/// @brief Class constructor. No work is performed here.
LPGPU2ppCallSuggestionsView::LPGPU2ppCallSuggestionsView(QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
{
}

/// @brief Class destructor. Calls the Shutdown method to cleanup
///        resources used by this method.
LPGPU2ppCallSuggestionsView::~LPGPU2ppCallSuggestionsView()
{
    Shutdown();
}

/// @brief  Initialises the components and the layout of this widget.
/// @return PPFnStatus  success: If the components were initialised,
///                     failure: An error has occurred.
PPFnStatus LPGPU2ppCallSuggestionsView::Initialise()
{
    LPGPU2PPNewQtWidget(&m_pFilterLineEdit, this);
    LPGPU2PPNewQtWidget(&m_pLocalAnnotationsWidget, this);
    LPGPU2PPNewQtWidget(&m_pGlobalAnnotationsWidget, this);
    LPGPU2PPNewQtWidget(&m_pUserAnnotationsWidget, this);

    auto bReturnLocal = InitialiseTableWidgetLayout(m_pLocalAnnotationsWidget);
    auto bReturnGlobal = InitialiseTableWidgetLayout(m_pGlobalAnnotationsWidget);
    auto bReturnUser = InitialiseTableWidgetLayout(m_pUserAnnotationsWidget);

    if (bReturnLocal == PPFnStatus::success &&
        bReturnGlobal == PPFnStatus::success &&
        bReturnUser == PPFnStatus::success)
    {
        QVBoxLayout *pVBoxLayout = nullptr;
        LPGPU2PPNewQtWidget(&pVBoxLayout, this);

        m_pFilterLineEdit->setPlaceholderText(PP_STR_CallSuggestions_FilterPlaceHolder);
        m_pFilterLineEdit->setClearButtonEnabled(true);
        m_pFilterLineEdit->setFrame(true);
        connect(m_pFilterLineEdit, &QLineEdit::textChanged, this, &LPGPU2ppCallSuggestionsView::OnFilterTextChanged);

        pVBoxLayout->addWidget(m_pFilterLineEdit);

        LPGPU2PPNewQtWidget(&m_pAnnotationsTab, this);
        m_pAnnotationsTab->addTab(m_pLocalAnnotationsWidget, kLocalTabLabel);
        m_pAnnotationsTab->addTab(m_pGlobalAnnotationsWidget, kGlobalTabLabel);
        m_pAnnotationsTab->addTab(m_pUserAnnotationsWidget, kUserTabLabel);        

        LPGPU2PPNewQtLPGPU2Widget(&m_pInfoWidget, PP_STR_LPGPU2InfoNoAPISuggestionsTitle, PP_STR_LPGPU2InfoNoAPISuggestionsMsg, this);
        m_pInfoWidget->setVisible(false);

        pVBoxLayout->setMargin(0);
        pVBoxLayout->addWidget(m_pAnnotationsTab);
        pVBoxLayout->addWidget(m_pInfoWidget, 0, Qt::AlignTop);
        return PPFnStatus::success;
    }
    else
    {
        return PPFnStatus::failure;
    }
}

/// @brief Cleanup resources used by this class.
/// @return PPFnStatus always return success.
PPFnStatus LPGPU2ppCallSuggestionsView::Shutdown()
{
    if (m_pInfoWidget != nullptr)
    {
        delete m_pInfoWidget;
        m_pInfoWidget = nullptr;
    }

    if (m_pLocalAnnotationsWidget != nullptr)
    {
        delete m_pLocalAnnotationsWidget;
        m_pLocalAnnotationsWidget = nullptr;
    }

    if (m_pGlobalAnnotationsWidget != nullptr)
    {
        delete m_pGlobalAnnotationsWidget;
        m_pGlobalAnnotationsWidget = nullptr;
    }

    if (m_pFilterLineEdit != nullptr)
    {
        delete m_pFilterLineEdit;
        m_pFilterLineEdit = nullptr;
    }

    if (m_pAnnotationsTab != nullptr)
    {
        delete m_pAnnotationsTab;
        m_pAnnotationsTab = nullptr;
    }   

    return PPFnStatus::success;
}

/// @brief  Display the annotations into this widget.
/// @param[in] vAnnotations The annotations to display.
/// @return    PPFnStatus   success: All the annotations were added to the table.
///                         failure: the table widget was not initialised before calling this function,
///                         there is no annotation to be displayed or something went wrong when
///                         creating the items to displayed.
PPFnStatus LPGPU2ppCallSuggestionsView::SetAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations)
{
    const auto rc = SetAnnotationsForTableWidget(vAnnotations, m_pLocalAnnotationsWidget);

    if (vAnnotations.empty())
    {
        m_isLocalTableVisible = false;

        if (!m_isGlobalTableVisible && !m_isUserTableVisible)
        {
            m_pAnnotationsTab->setVisible(false);
            m_pInfoWidget->setVisible(true);
        }
    }

    return rc;
}


/// @brief  Display the annotations into the given widget.
/// @param[in] vAnnotations The annotations to display.
/// @param[in] pTable .
/// @return    PPFnStatus   success: All the annotations were added to the
///                         table.
///                         failure: the table widget was not initialised before
///                         calling this function,
///                         there is no annotation to be displayed or something
///                         went wrong when
///                         creating the items to displayed.
PPFnStatus LPGPU2ppCallSuggestionsView::SetAnnotationsForTableWidget(const gtVector<lpgpu2::db::Annotation> &vAnnotations, QTableWidget *pTable)
{
    GT_ASSERT(pTable != nullptr);
    auto bReturn = PPFnStatus::success;
    
    if (pTable == nullptr)
    {
        return PPFnStatus::failure;
    }
    
    if (!vAnnotations.empty())
    {
      pTable->clearContents();
      pTable->setRowCount(vAnnotations.size());

      for (gtUInt32 iAnnotationIndex = 0; iAnnotationIndex < vAnnotations.size(); ++iAnnotationIndex)
      {
        const auto& annotation = vAnnotations[iAnnotationIndex];

        auto frameNumberItem = new QTableWidgetItem{};
        frameNumberItem->setText(QString::number(annotation.m_frameNum));
        frameNumberItem->setData(Qt::UserRole, QVariant::fromValue(annotation.m_annotationId));
        frameNumberItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        pTable->setItem(iAnnotationIndex, kEnumColumnNames_FrameNumber, frameNumberItem);

        auto annotationItem = new QTableWidgetItem{};
        annotationItem->setText(acGTStringToQString(annotation.m_text));
        annotationItem->setData(Qt::UserRole, QVariant::fromValue(annotation.m_annotationId));            
        annotationItem->setTextAlignment(Qt::AlignCenter);
        annotationItem->setToolTip(acGTStringToQString(annotation.m_text));
        pTable->setItem(iAnnotationIndex, kEnumColumnNames_CallSuggestion, annotationItem);
      }
    }

    return bReturn;
}

/// @brief  Display the annotations into the global widget.
/// @param[in] vAnnotations The annotations to display.
/// @return    PPFnStatus   success: All the annotations were added to the
///                         table.
///                         failure: the table widget was not initialised before
///                         calling this function,
///                         there is no annotation to be displayed or something
///                         went wrong when
///                         creating the items to displayed.
PPFnStatus LPGPU2ppCallSuggestionsView::SetGlobalAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations)
{
    const auto rc = SetAnnotationsForTableWidget(vAnnotations, m_pGlobalAnnotationsWidget);

    if (vAnnotations.empty())
    {
        m_isGlobalTableVisible = false;

        if (!m_isUserTableVisible && !m_isLocalTableVisible)
        {
            m_pAnnotationsTab->setVisible(false);
            m_pInfoWidget->setVisible(true);
        }
    }

    return rc;
}

/// @brief  Display the annotations into the user widget.
/// @param[in] vAnnotations The annotations to display.
/// @return    PPFnStatus   success: All the annotations were added to the
///                         table.
///                         failure: the table widget was not initialised before
///                         calling this function,
///                         there is no annotation to be displayed or something
///                         went wrong when
///                         creating the items to displayed.
PPFnStatus LPGPU2ppCallSuggestionsView::SetUserAnnotations(const gtVector<lpgpu2::db::Annotation> &vAnnotations)
{
    const auto rc = SetAnnotationsForTableWidget(vAnnotations, m_pUserAnnotationsWidget);

    if (vAnnotations.empty())
    {
        m_isUserTableVisible = false;

        if (!m_isLocalTableVisible && !m_isGlobalTableVisible)
        {
            m_pAnnotationsTab->setVisible(false);
            m_pInfoWidget->setVisible(true);
        }
    }

    return rc;
}

/// @brief  Initialise the table widgets layout and set their behaviour.
/// @return PPFnstauts  success: The table widgets were initialised and the
///                     behaviour configured,
///                     failure: The table widgets were not created before
///                     calling this function or an error has occurred while
///                     initialising the tables headers.
PPFnStatus LPGPU2ppCallSuggestionsView::InitialiseTableWidgetLayout(QTableWidget *pTable)
{
    auto bReturn = PPFnStatus::failure;
    GT_ASSERT(pTable != nullptr);

    if (pTable != nullptr)
    {
        // Sets the table style and behaviour
        pTable->setDragDropOverwriteMode(true);
        pTable->setAlternatingRowColors(true);
        pTable->setSelectionMode(QAbstractItemView::SingleSelection);
        pTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        pTable->setSortingEnabled(true);
        pTable->setCornerButtonEnabled(false);
        pTable->setEditTriggers(QTableWidget::NoEditTriggers);
        pTable->horizontalHeader()->setSortIndicatorShown(true);
        pTable->horizontalHeader()->setStretchLastSection(true);
        pTable->horizontalHeader()->setFixedHeight(30);
        pTable->verticalHeader()->setVisible(false);
        pTable->setStyleSheet(PP_STR_FeedbackViews_TableWidgetStyleSheet);

        QFont headerFont;

        auto frameNumberHeaderItem = new QTableWidgetItem{ PP_STR_CallSuggestionsTable_FrameNumTitle };
        headerFont = frameNumberHeaderItem->font();
        headerFont.setBold(true);
        frameNumberHeaderItem->setFont(headerFont);
        
        auto callSuggestionHeaderItem = new QTableWidgetItem{ PP_STR_CallSuggestionsView_TableTitle };
        callSuggestionHeaderItem->setFont(headerFont);

        pTable->setColumnCount(KEnumColumnNames_NumberOfColumns);
        pTable->setHorizontalHeaderItem(kEnumColumnNames_FrameNumber, frameNumberHeaderItem);
        pTable->setHorizontalHeaderItem(kEnumColumnNames_CallSuggestion, callSuggestionHeaderItem);

        bReturn = PPFnStatus::success;

        connect(pTable, &QTableWidget::itemSelectionChanged, this, [=]
        {
            auto items = pTable->selectedItems();
            if (!items.empty())
            {
                OnTableItemActivated(items.back());
            }
        });
    }

    return bReturn;
}

/// @brief     Slot called when the user clicks on an item in the table. 
///            This will emit signal SuggestionSelected
/// @param[in] vpTableItem  The QTableWidgetItem of the item that was clicked.
void LPGPU2ppCallSuggestionsView::OnTableItemActivated(QTableWidgetItem *vpTableItem)
{
    // Cannot use vRow directly. Need to get information from the item that was clicked
    if (vpTableItem != nullptr)
    {
        auto bIsConversionOk = false;
        const auto annotationId = vpTableItem->data(Qt::UserRole).toUInt(&bIsConversionOk);

        if (bIsConversionOk)
        {
            emit SuggestionSelected(annotationId);
        }        
    }    
}

void LPGPU2ppCallSuggestionsView::OnFilterTextChanged(const QString& vText)
{
    auto FilterTableWidget = [=](QTableWidget* vTableWidget)
    {
        for (int iRow = 0; iRow < vTableWidget->rowCount(); ++iRow)
        {
            auto *item = vTableWidget->item(iRow, kEnumColumnNames_CallSuggestion);
            vTableWidget->setRowHidden(iRow, !item->text().contains(vText, Qt::CaseInsensitive));
        }
    };

    FilterTableWidget(m_pUserAnnotationsWidget);
    FilterTableWidget(m_pGlobalAnnotationsWidget);
    FilterTableWidget(m_pLocalAnnotationsWidget);    
}
