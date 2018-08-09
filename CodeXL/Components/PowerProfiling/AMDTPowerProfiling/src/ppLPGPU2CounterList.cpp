// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppCounterList.cpp
///
/// @brief A tree widget which displays counter names and provides checkboxes
///        alongside them so that they can be enabled or disabled. Counters
///        can be grouped into categories, allowing multiple counters to be
///        selected at once.
///
/// ppCounterList implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include "ppLPGPU2CounterList.h"

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

#include <QHeaderView>

// Import:
// std::find_if()
// std::chrono
#include <algorithm>
#include <chrono>

namespace lpgpu2 {

const double kLPGPU2MinSamplingFreqHz = 1;
const double kLPGPU2MaxSamplingFreqHz = 10;
const double kLPGPU2SingleStepFreqHz = 0.5;
const double kLPGPU2SpinboxDefaultFreqHz = kLPGPU2MinSamplingFreqHz;
const int kLPGPU2SpinboxDecimals = 2;
const QStringList kLPGPU2HeaderLabels {"Counter Set", "Sampling Frequency"};
const QString kItemPropertyName {"index_in_vector"};

static double fromHzToNanoseconds(double hz)
{
  using FpNanoseconds =
    std::chrono::duration<double, std::chrono::nanoseconds::period>;
  using FpSeconds =
    std::chrono::duration<double, std::chrono::seconds::period>;

  // Convert to nanoseconds
  return std::chrono::duration_cast<FpNanoseconds>(FpSeconds{1.0 / hz}).count();
}

static double fromNanosecondsToHz(double ns)
{
  using FpNanoseconds =
    std::chrono::duration<double, std::chrono::nanoseconds::period>;
  using FpSeconds =
    std::chrono::duration<double, std::chrono::seconds::period>;

  // Convert to Hz 
  return (1.0 /
    std::chrono::duration_cast<FpSeconds>(FpNanoseconds{ns}).count());
}

ppCounterCategory::ppCounterCategory()
: name{}, id{}, counters{}, enabled{false},
  samplingInterval{static_cast<gtUInt32>(
      fromHzToNanoseconds(kLPGPU2SpinboxDefaultFreqHz))}
{
}

/// @brief Constructor
/// @param[in]  parent  This widget's parent widget.
ppCounterList::ppCounterList(QWidget* parent)
: QTreeWidget{parent}, m_categoriesByNameMap{}, m_counterSetWidgetItems{},
  m_counterWidgetItems{}, m_numEnabledCounters{0}
{
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnCounterChanged(QTreeWidgetItem*, int)));
    setColumnCount(2);
    setHeaderLabels(kLPGPU2HeaderLabels);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

/// @brief                              Returns a list of all the counter
///                                     categories (sets).
/// @return QList<ppCounterCategory>    Category (set) list.
QList<ppCounterCategory> ppCounterList::GetAllCategories()
{
  return m_categoriesByNameMap.values();
}

/// @brief                          Adds a counter category, which is displayed
///                                 as a top-level tree widget item.
/// @param[in]  categoryName        The name of the category to be displayed.
/// @param[in]  categoryId          The ID of the counter category (from
///                                 target characteristics file).
/// @return     QTreeWidgetItem*    Pointer to newly-added tree widget item.
QTreeWidgetItem* ppCounterList::AddCounterCategory(
    const QString& categoryName,
    const QString& categoryId,
    const QString &counterTooltip,
    gtUInt32 minSamplingInterval)
{
    // Define the new top-level item to be added
    QTreeWidgetItem* newCategoryItem = new QTreeWidgetItem;
    newCategoryItem->setCheckState(0, Qt::Unchecked);
    newCategoryItem->setText(0, categoryName);
    newCategoryItem->setToolTip(0, counterTooltip);
    addTopLevelItem(newCategoryItem);
    auto *pSamplingIntervalSpinbox = new QDoubleSpinBox{};
    pSamplingIntervalSpinbox->setMinimum(kLPGPU2MinSamplingFreqHz);
    pSamplingIntervalSpinbox->setMaximum(
        fromHzToNanoseconds(minSamplingInterval));
    pSamplingIntervalSpinbox->setSingleStep(kLPGPU2SingleStepFreqHz);
    pSamplingIntervalSpinbox->setValue(kLPGPU2SpinboxDefaultFreqHz);
    pSamplingIntervalSpinbox->setDecimals(kLPGPU2SpinboxDecimals);
    pSamplingIntervalSpinbox->setSuffix("Hz");
    setItemWidget(newCategoryItem, 1, pSamplingIntervalSpinbox);
    m_counterSetWidgetItems.push_back(newCategoryItem);
    pSamplingIntervalSpinbox->setProperty(kItemPropertyName.toStdString().c_str(),
        QVariant::fromValue(categoryName));
    connect(pSamplingIntervalSpinbox,
      SIGNAL(valueChanged(double)), this,
      SLOT(OnSpinboxValueChanged(double)));

    // Save the category information in the categories map
    ppCounterCategory category;
    category.name.fromUtf8String(categoryName.toStdString());
    category.id.fromUtf8String(categoryId.toStdString());
    m_categoriesByNameMap.insert(categoryName, category);

    return newCategoryItem;
}

/// @brief                          Adds a counter under the specified counter
///                                 category, which can be retrieved by adding
///                                 a new counter category.
/// @param[in]  categoryItem        The counter category, which will be displayed
///                                 as this counter's parent in the tree.
/// @param[in]  counterName         The name of the counter to be displayed.
/// @param[in]  counterId           The ID of the counter (from target
///                                 characteristics file).
/// @return     QTreeWidgetItem*    Pointer to newly-added tree widget item.
QTreeWidgetItem* ppCounterList::AddCounterToCategory(QTreeWidgetItem* categoryItem,
                                                           const QString& counterName,
                                                           const QString& counterId,
                                          const QString &counterTooltip)
{
    QTreeWidgetItem* newChildItem = new QTreeWidgetItem;
    newChildItem->setCheckState(0, Qt::Unchecked);
    newChildItem->setText(0, counterName);
    newChildItem->setToolTip(0, counterTooltip);
    categoryItem->addChild(newChildItem);
    m_counterWidgetItems.push_back(newChildItem);

    // Save the counter information in the counters map
    ppCounter counter;
    counter.name.fromUtf8String(counterName.toStdString());
    counter.id.fromUtf8String(counterId.toStdString());
    counter.enabled = false;
    // Set a default counter
    if (m_numEnabledCounters == 0)
    {
      ++m_numEnabledCounters;

      counter.enabled = true;
      newChildItem->setCheckState(0, Qt::Checked);
      categoryItem->setCheckState(0, Qt::PartiallyChecked);
    }

    CounterCategoryMap::iterator it = m_categoriesByNameMap.find(categoryItem->text(0));
    GT_IF_WITH_ASSERT_EX(it != m_categoriesByNameMap.end(), L"No matching category found.")
    {
        it->counters.push_back(counter);
    }

    return newChildItem;
}

/// @brief              Called whenever a counter's checkbox changes state
///                     (although it will be called whenever a tree item changes
///                     in any way). Updates the internally-held objects which can
///                     later be read to determine which counters to enable.
/// @param[in]  item    The tree widget item that was changed.
/// @param[in]  column  The column of the tree widget item that changed.
void ppCounterList::OnCounterChanged(QTreeWidgetItem* item, int column)
{
    // Determine whether this counter was enabled or disabled
    const Qt::CheckState categoryCheckState = item->checkState(0);
    bool enabled = (categoryCheckState == Qt::Checked ||
        categoryCheckState == Qt::PartiallyChecked);

    // Check if the item is a counter by checking for any child items
    bool isCounter = (item->childCount() == 0);
    if (isCounter)
    {
      // Update the corresponding counter in the map
      QTreeWidgetItem* parentItem = item->parent();
      QString categoryName = parentItem->text(column);
      CounterCategoryMap::iterator it = m_categoriesByNameMap.find(categoryName);
      GT_IF_WITH_ASSERT_EX(it != m_categoriesByNameMap.end(), L"Failed to find corresponding counter in map.")
      {
        // Search through all counters in the category for the matching counter name
        for (ppCounter& counter : it->counters)
        {
          gtString selectedCounterName;
          selectedCounterName.fromUtf8String(item->text(column).toStdString());

          if (counter.name == selectedCounterName)
          {
            // Enable/disable the selected counter in the map
            counter.enabled = enabled;

            if (enabled)
            {
              ++m_numEnabledCounters;
            }
            else
            {
              --m_numEnabledCounters;

              // To enforce the need to have at least one active counter,
              // re-set the last counter which was unset
              if (m_numEnabledCounters == 0)
              {
                item->setCheckState(column, Qt::Checked);
              }
            }

            const auto enabledCountersPerCategory =
            GetEnabledCountersPerCategory(categoryName);
            if (enabledCountersPerCategory == it->counters.size())
            {
              parentItem->setCheckState(column, Qt::Checked);
            }
            else if (enabledCountersPerCategory > 0)
            {
              parentItem->setCheckState(column, Qt::PartiallyChecked);
            }
            else
            {
              parentItem->setCheckState(column, Qt::Unchecked);
            }
          }
        }
      }
    }
    // The item must be a counter category, as it has no parent
    else
    {
      // Update all counters belonging to the category in the map
      CounterCategoryMap::iterator it = m_categoriesByNameMap.find(item->text(column));
      GT_IF_WITH_ASSERT_EX(it != m_categoriesByNameMap.end(), L"Failed to find corresponding counter in map.")
      {
          // Enable/disable this category in the map
          it->enabled = enabled;

          if (categoryCheckState == Qt::PartiallyChecked)
          {
            return;
          }

          // Enable/disable all counters in this category in the map
          for (ppCounter& counter : it->counters)
          {
              counter.enabled = enabled;
          }

          // Set the check state of all the child items in the tree
          const size_t childCount = item->childCount();
          for (size_t i = 0; i < childCount; i++)
          {
              QTreeWidgetItem* childItem = item->child(i);
              childItem->setCheckState(column, categoryCheckState);
          }
      }

    }
}

/// @brief Called when the value of one of the sampling interval spinboxes is
///        changed
/// @param d The new value of the spinbox
void ppCounterList::OnSpinboxValueChanged(double d)
{
  const auto *pSpinbox = qobject_cast<QDoubleSpinBox *>(QObject::sender());
  if (!pSpinbox)
  {
    return;
  }

  const auto counterSetName = pSpinbox->property(kItemPropertyName.toStdString().c_str()).toString();
  m_categoriesByNameMap[counterSetName].samplingInterval =
    fromHzToNanoseconds(d);
}

/// @brief Check/uncheck a given counterset; it does not set its children
///        counters
/// @param counterSetId The Id (or index) of the counterSet as it is stored
///        in the counterlist
/// @param isChecked Whether to set as checked or unchecked
/// @note We have a void return type to conform with the existing class
///       interface
void ppCounterList::SetCounterSetCheckedState(int counterSetId,
  bool isChecked)
{
  GT_IF_WITH_ASSERT(!m_counterSetWidgetItems.empty())
  {
    // Determine whether this counter was enabled or disabled
    Qt::CheckState categoryCheckState = Qt::Unchecked;
    if (isChecked)
    {
      categoryCheckState = Qt::Checked;
    }
    m_counterSetWidgetItems[counterSetId]->setCheckState(0, categoryCheckState);
  }
}

/// @brief Check/uncheck a given counter
/// @param counterId The Id (or index) of the counterSet as it is stored
///        in the counterlist
/// @param isChecked Whether to set as checked or unchecked
/// @note We have a void return type to conform with the existing class
///       interface
void ppCounterList::SetCounterCheckedState(int counterId, bool isChecked)
{
  GT_IF_WITH_ASSERT(!m_counterSetWidgetItems.empty())
  {
    // Determine whether this counter was enabled or disabled
    Qt::CheckState categoryCheckState = Qt::Unchecked;
    if (isChecked)
    {
      categoryCheckState = Qt::Checked;
    }
    m_counterWidgetItems[counterId]->setCheckState(0, categoryCheckState);
  }
}

/// @brief Clearup the cache used for fast indexing of the tree items
/// @note We have a void return type to conform with the existing class
///       interface
void ppCounterList::ClearCountersCache()
{
  m_counterWidgetItems.clear();
  m_counterSetWidgetItems.clear();
  m_categoriesByNameMap.clear();
  m_numEnabledCounters = 0;
}

/// @brief Set the sampling interval of a given counter set
/// @param counterSetId Id of the counter set to update
/// @param samplingInterval Interval to set
void ppCounterList::SetCounterSetSamplingInterval(int counterSetId,
      gtUInt32 samplingInterval)
{
  if (m_counterSetWidgetItems.empty() || counterSetId < 0 ||
      (static_cast<unsigned int>(
                        counterSetId) > m_counterSetWidgetItems.size() - 1))
  {
    return;
  }

  auto *pSpinBox = qobject_cast<QDoubleSpinBox *>(itemWidget(
        m_counterSetWidgetItems[counterSetId], 1));
  if (!pSpinBox)
  {
    return;
  }

  pSpinBox->setValue(
      fromNanosecondsToHz(static_cast<double>(samplingInterval)));
}

/// @brief Get the number of enabled counters for a given category
/// @param categoryName The name of the category to look up
/// @return Number of enabled counters. Returns zero if name does not have a
/// corresponding category
size_t ppCounterList::GetEnabledCountersPerCategory(
    const QString &categoryName) const
{
  size_t enabledCountersNum = 0;

  const auto categoryItor = m_categoriesByNameMap.find(categoryName);

  if (categoryItor != m_categoriesByNameMap.end())
  {
    for (const auto counter : categoryItor->counters)
    {
      if (counter.enabled)
      {
        ++enabledCountersNum;
      }
    }
  }

  return enabledCountersNum;
}

}
// namespace lpgpu2
