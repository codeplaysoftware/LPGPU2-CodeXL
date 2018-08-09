// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Defines the Shaders in Region view widget.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <AMDTPowerProfiling/src/LPGPU2ppShadersInRegionView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppInfoWidget.h>
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
    kEnumColumnNames_ShadersInRegion = 0,
    KEnumColumnNames_NumberOfColumns
};

/// @brief               Class constructor. No work is performed here 
///                      a part from calling the base constructor.
/// @param[in] vpParent  The parent widget of this widget.
LPGPU2ppShadersInRegionView::LPGPU2ppShadersInRegionView(QWidget *vpParent /* = nullptr */)
    : QWidget{ vpParent }
{
}

/// @brief  Class destructor. Calls the Shutdown() method to
///         cleanup resources used by this class.
LPGPU2ppShadersInRegionView::~LPGPU2ppShadersInRegionView()
{
    Shutdown();
}

/// @brief              Initialises the components and the 
///                     layout of this widget.
/// @return PPFnStatus  success: if the components were initialised,
///                     failure: An error has occurred while building
///                     the layout.
PPFnStatus LPGPU2ppShadersInRegionView::Initialise()
{
    LPGPU2PPNewQtWidget(&m_pTableWidget, this);
    
    auto bReturn = InitialiseTableWidgetLayout();

    if (bReturn == PPFnStatus::success) 
    {
        QVBoxLayout *pVBoxLayout = nullptr;
        LPGPU2PPNewQtWidget(&pVBoxLayout, this);
        
        LPGPU2PPNewQtLPGPU2Widget(&m_pInfoWidget, PP_STR_LPGPU2InfoNoShadersTitle, PP_STR_LPGPU2InfoNoShadersMsg, this);
        m_pInfoWidget->setVisible(false);

        pVBoxLayout->setMargin(0);
        pVBoxLayout->addWidget(m_pTableWidget);
        pVBoxLayout->addWidget(m_pInfoWidget, 0, Qt::AlignTop);
        bReturn = PPFnStatus::success;
    }

    return bReturn;
}

/// @brief              Cleanup resources used by this class.
/// @return PPFnStatus  Always return success.
PPFnStatus LPGPU2ppShadersInRegionView::Shutdown()
{
    if (m_pTableWidget != nullptr)
    {
        delete m_pTableWidget;
        m_pTableWidget = nullptr;
    }

    return PPFnStatus::success;
}

/// @brief                    Set the shader traces list and builds the table to
///                           display the data.
/// @param[in] vShaderTraces  The list of shader traces to be displayed as a table.
/// @return    PPFnStatus     success: the table was constructed and all shader
///                           traces are ready to be displayed,
///                           failure: The table widget was not initialised or an
///                           error happened while building the table with the shader
///                           traces. 
PPFnStatus LPGPU2ppShadersInRegionView::SetShaderTraces(const gtVector<lpgpu2::db::ShaderTrace> &vShaderTraces) const
{
    auto bReturn = PPFnStatus::success;
    
    if (m_pTableWidget == nullptr)
    {
        return PPFnStatus::failure;
    }
    
    if (!vShaderTraces.empty())
    {
      m_pTableWidget->clearContents();
      m_pTableWidget->setRowCount(vShaderTraces.size());

      for (gtUInt32 iShaderTraceIndex = 0; iShaderTraceIndex < vShaderTraces.size(); ++iShaderTraceIndex)
      {
          const auto& shaderTrace = vShaderTraces[iShaderTraceIndex];

          auto annotationItem = std::unique_ptr<QTableWidgetItem>(new (std::nothrow) QTableWidgetItem{});
          if (annotationItem != nullptr)
          {
              annotationItem->setText(QString(PP_STR_LPGPU2ShaderAtFrameDraw).arg(shaderTrace.m_frameNum).arg(
                                                                                             shaderTrace.m_drawNum));
              annotationItem->setData(Qt::UserRole, QVariant::fromValue(QString("%0:%1").arg(shaderTrace.m_frameNum).arg(
                                                                                             shaderTrace.m_drawNum)));
              annotationItem->setTextAlignment(Qt::AlignCenter);

              m_pTableWidget->setItem(iShaderTraceIndex, kEnumColumnNames_ShadersInRegion, annotationItem.release());
          }
          else
          {
              bReturn = PPFnStatus::failure;
              break;
          }
      }    
    }
    else
    {
      m_pTableWidget->setVisible(false);
      m_pInfoWidget->setVisible(true);
    }

    return bReturn;
}

/// @brief              Initialises the layout of the underlying table widget.
/// @return PPFnStatus  success: The layout of the widget was constructed successfully,
///                     failure: The table widget was not initialised or an error has
///                     occurred while building the table header items.
PPFnStatus LPGPU2ppShadersInRegionView::InitialiseTableWidgetLayout() const
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
        m_pTableWidget->setStyleSheet(PP_STR_FeedbackViews_TableWidgetStyleSheet);

        auto callSuggestionHeaderItem = std::unique_ptr<QTableWidgetItem>{ new (std::nothrow) QTableWidgetItem{} };
        if (callSuggestionHeaderItem != nullptr)
        {
            callSuggestionHeaderItem->setText(PP_STR_ShadersInRegion_TableTitle);
            auto headerFont = callSuggestionHeaderItem->font();
            headerFont.setBold(true);
            callSuggestionHeaderItem->setFont(headerFont);

            m_pTableWidget->setColumnCount(KEnumColumnNames_NumberOfColumns);
            m_pTableWidget->setHorizontalHeaderItem(kEnumColumnNames_ShadersInRegion, callSuggestionHeaderItem.release());

            bReturn = PPFnStatus::success;
        }

        connect(m_pTableWidget, &QTableWidget::itemClicked, this, &LPGPU2ppShadersInRegionView::OnTableItemClicked);
    }    

    return bReturn;
}

/// @brief                  Slot called when an item was clicked on the table widget.
///                         This will get the shader annotation id and emits the signal
///                         ShaderSelected with the Id of the shader selected.
/// @param[in] vpTableItem  The table widget item that was clicked.
void LPGPU2ppShadersInRegionView::OnTableItemClicked(QTableWidgetItem *vpTableItem)
{
    // Cannot use vRow directly. Need to get information from the item that was clicked
    if (vpTableItem != nullptr)
    {
        const QString annotationId = vpTableItem->data(Qt::UserRole).toString();
        emit ShaderSelected(acQStringToGTString(annotationId));
    }    
}
