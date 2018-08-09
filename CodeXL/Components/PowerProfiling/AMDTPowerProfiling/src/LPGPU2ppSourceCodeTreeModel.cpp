// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppSourceCodeTreeModel.cpp
///
/// @brief Data model for assembly code associated with performance counters
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/src/afUtils.h>

#include <AMDTPowerProfiling/src/LPGPU2ppSourceCodeTreeModel.h>
#include <AMDTCpuProfiling/inc/SourceViewTreeItem.h>

#include <iostream>
#include <algorithm>

namespace lpgpu2 {

constexpr size_t kSamplePercentPrecision = 2;
const QString kColumnTooltipStr{"Line"};
const QString kAsmTooltipStr{"Lines of assembly code"};
const QString kPercentTooltipStr{"Percentage for a given line of assembly"};
const QString kColumnCaptionStr{"Line"};
const QString kAsmCaptionStr{"ASM Lines"};
const QString kPercentCaptionStr{"% for counter"};

/// @brief Ctor; create the root item and set the headers
/// @param parent The parent widget
ppSourceCodeTreeModel::ppSourceCodeTreeModel(QWidget *parent) :
    QAbstractItemModel{parent}
{
  m_pRootItem = new SourceViewTreeItem(nullptr,
      SOURCE_VIEW_DEFAULT_DEPTH, nullptr);

  // Update the headers:
  UpdateHeaders();

  // Set the gray forground color:
  const auto color = acGetSystemDefaultBackgroundColor();
  m_forgroundColor =
    QColor::fromRgb(color.red() * 8 / 10, color.green() * 8 / 10,
        color.blue() * 8 / 10);
}

/// @brief Dtor; clear the data from the model and delete the root item
ppSourceCodeTreeModel::~ppSourceCodeTreeModel()
{
  Clear();
  if (m_pRootItem)
  {
    delete m_pRootItem;
    m_pRootItem = nullptr;
  }
}

/// @brief Clear the data from the model
void ppSourceCodeTreeModel::Clear()
{
  // Remove all rows:
  removeRows(0, rowCount());
}

int ppSourceCodeTreeModel::rowCount(const QModelIndex& parent) const
{
  auto retVal = 0;
  auto *pParentItem = getItem(parent);
  GT_IF_WITH_ASSERT(pParentItem != nullptr)
  {
    retVal = pParentItem->childCount();
  }
  return retVal;
}

int ppSourceCodeTreeModel::columnCount(const QModelIndex& parent) const
{
  (void)(parent); // Unused

  return SrcViewColumns::enumCount;
}

QVariant ppSourceCodeTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::ForegroundRole)
  {
    auto *pItem = getItem(index);
    GT_IF_WITH_ASSERT(pItem != nullptr)
    {
      return pItem->forground(index.column());
    }
  }

  if ((role != Qt::DisplayRole) && (role != Qt::ToolTipRole))
  {
    return QVariant();
  }

  QString retVal;

  if (index.isValid())
  {
    // Get the string for the item:
    if (role == Qt::DisplayRole)
    {
      auto *pItem = getItem(index);
      GT_IF_WITH_ASSERT(pItem != nullptr)
      {
        retVal = pItem->data(index.column()).toString();
      }
    }
    else if (role == Qt::ToolTipRole)
    {
      auto *pItem = getItem(index);
      GT_IF_WITH_ASSERT(pItem != nullptr)
      {
        retVal = pItem->tooltip(index.column()).toString();
      }
    }
  }

  return retVal;
}

Qt::ItemFlags ppSourceCodeTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return 0;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

SourceViewTreeItem* ppSourceCodeTreeModel::getItem(
    const QModelIndex& index) const
{
    SourceViewTreeItem *pRetVal = nullptr;

    if (index.isValid())
    {
        auto *pItem = static_cast<SourceViewTreeItem*>(index.internalPointer());

        if (pItem != nullptr)
        {
            pRetVal = pItem;
        }
    }
    else
    {
        pRetVal = m_pRootItem;
    }

    return pRetVal;
}

QVariant ppSourceCodeTreeModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            GT_IF_WITH_ASSERT((section >= 0) && (section < m_headerCaptions.size()))
            {
                return m_headerCaptions.at(section);
            }
        }

        if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
        {
            GT_IF_WITH_ASSERT((section >= 0) && (section < m_headerTooltips.size()))
            {
                return m_headerTooltips.at(section);
            }
        }

        return QVariant();
    }

    return QVariant();

}

QModelIndex ppSourceCodeTreeModel::index(int row, int column,
    const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return QModelIndex();
    }

    auto *pParentItem = getItem(parent);

    if (pParentItem != nullptr)
    {
        auto *pChildItem = pParentItem->child(row);

        if (pChildItem)
        {
            return createIndex(row, column, pChildItem);
        }
        else
        {
            return QModelIndex();
        }
    }

    return QModelIndex();

}

bool ppSourceCodeTreeModel::insertColumns(int position, int columns,
    const QModelIndex& parent)
{
    auto retVal = false;

    beginInsertColumns(parent, position, position + columns - 1);
    retVal = m_pRootItem->insertColumns(position, columns);
    endInsertColumns();

    return retVal;
}

bool ppSourceCodeTreeModel::insertRows(int position, int rows,
    const QModelIndex& parent)
{
    auto retVal = false;

    auto *pParentItem = getItem(parent);
    GT_IF_WITH_ASSERT(pParentItem != nullptr)
    {
        beginInsertRows(parent, position, position + rows - 1);
        retVal = pParentItem->insertChildren(position, rows, m_pRootItem->columnCount());
        endInsertRows();
    }

    return retVal;
}

QModelIndex ppSourceCodeTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    auto *pChildItem = getItem(index);
    GT_IF_WITH_ASSERT(pChildItem != nullptr)
    {
        auto *pParentItem = pChildItem->parent();

        if (pParentItem == m_pRootItem)
        {
            return QModelIndex();
        }
        else
        {
            return createIndex(pParentItem->childCount(), 0, pParentItem);
        }
    }

    return QModelIndex();
}

bool ppSourceCodeTreeModel::removeColumns(int position, int columns,
    const QModelIndex& parent)
{

    auto retVal = false;

    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        beginRemoveColumns(parent, position, position + columns - 1);
        retVal = m_pRootItem->removeColumns(position, columns);
        endRemoveColumns();

        if (m_pRootItem->columnCount() == 0)
        {
            removeRows(0, rowCount());
        }
    }

    return retVal;
}

bool ppSourceCodeTreeModel::removeRows(int position, int rows,
    const QModelIndex& parent)
{
    auto *pParentItem = getItem(parent);
    auto retVal = false;
    GT_IF_WITH_ASSERT(pParentItem != nullptr)
    {
        retVal = true;

        if (rows > 0)
        {
            beginRemoveRows(parent, position, position + rows - 1);
            retVal = pParentItem->removeChildren(position, rows);
            endRemoveRows();
        }
    }

    return retVal;
}

bool ppSourceCodeTreeModel::setData(const QModelIndex& index,
    const QVariant& value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    auto *item = getItem(index);
    auto result = item->setData(index.column(), value);

    if (result)
    {
        emit dataChanged(index, index);
    }

    return result;
}

bool ppSourceCodeTreeModel::setHeaderData(int section,
    Qt::Orientation orientation, const QVariant& value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
    {
        return false;
    }

    auto result = m_pRootItem->setData(section, value);

    if (result)
    {
        emit headerDataChanged(orientation, section, section);
    }

    return result;
}

/// @brief Create the data for the model
/// @param asmLines The assembly code listing to parse line by line
/// @param perc The performance data to associate with each line of assembly
///             code
/// @return True if it succeeds, false if it fails
bool ppSourceCodeTreeModel::BuildDisassemblyTree(const gtString &asmLines,
        const gtVector<db::ShaderAsmStat> &perc)
{
  Clear();
  gtStringTokenizer tokenizer{asmLines, L"\n"};
  gtString nextAsmLine;
  size_t i = 1;
  while (tokenizer.getNextToken(nextAsmLine))
  {
    auto *pAsmItem = new SourceViewTreeItem(nullptr, SOURCE_VIEW_ASM_DEPTH,
                                            m_pRootItem);

    pAsmItem->setData(SrcViewColumns::row,
        QString::number(i, 10));
    pAsmItem->setForeground(SrcViewColumns::row, acQGREY_TEXT_COLOUR);
    pAsmItem->setData(SrcViewColumns::asmCol,
        acGTStringToQString(nextAsmLine));
    pAsmItem->setForeground(SrcViewColumns::asmCol, acQGREY_TEXT_COLOUR);

    auto lineStatsIt = std::find_if(perc.begin(), perc.end(),
        [&i] (const db::ShaderAsmStat &el) {
          return i == el.m_asmLine;
        });

    ++i;

    float percentageValue = 0.f;
    if (lineStatsIt != perc.end())
    {
      percentageValue = lineStatsIt->m_percentage;
    }

    pAsmItem->setData(SrcViewColumns::percentage, percentageValue);
    pAsmItem->setForeground(SrcViewColumns::percentage, acRED_NUMBER_COLOUR);
  }

  return true;
}

/// @brief Update the headers of the model
/// @return True if it succeeds, false if it fails
bool ppSourceCodeTreeModel::UpdateHeaders()
{
    auto retVal = true;

    // Build the header captions and header tooltips:
    m_headerCaptions.clear();
    m_headerTooltips.clear();

    // Add the data header captions and tooltips:
    m_headerTooltips << kColumnTooltipStr;
    m_headerTooltips << kAsmTooltipStr;
    m_headerTooltips << kPercentTooltipStr;

    m_headerCaptions << kColumnCaptionStr;
    m_headerCaptions << kAsmCaptionStr;
    m_headerCaptions << kPercentCaptionStr;

    return retVal;
}

int ppSourceCodeTreeModel::topLevelItemCount()
{
    auto retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        retVal = m_pRootItem->childCount();
    }

    return retVal;
}


SourceViewTreeItem* ppSourceCodeTreeModel::topLevelItem(int index)
{
    SourceViewTreeItem *pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pRootItem != nullptr)
    {
        pRetVal = m_pRootItem->child(index);
    }
    return pRetVal;
}


void ppSourceCodeTreeModel::SetDisplayFormat(double  val, bool appendPercent, QVariant& data, const int precision)
{
    if (val > 0)
    {
        const auto strPrecision = QString::number(val, 'f', precision);
        data = QVariant(strPrecision);

        if (appendPercent)
        {
            auto dataStr = data.toString();
            dataStr.append("%");
            data.setValue(dataStr);
        }
    }
    else if (val == 0)
    {
        const auto emptyStr = "";
        data = QVariant(emptyStr);
    }
}

/// @brief Force an update of the data in the model
void ppSourceCodeTreeModel::UpdateModel()
{
  QModelIndex topLeft = index(0, 0);
  QModelIndex bottomRight = index(rowCount() - 1, columnCount() - 1);
  emit dataChanged(topLeft, bottomRight);

  beginResetModel();
  endResetModel();
}

} // namespace lpgpu2
