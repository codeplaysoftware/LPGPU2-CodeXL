// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file ppLPGPU2CounterList.h
///
/// @brief A tree widget which displays counter names and provides checkboxes
///        alongside them so that they can be enabled or disabled. Counters
///        can be grouped into categories, allowing multiple counters to be
///        selected at once.
///
/// ppLPGPU2CounterList interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPCOUNTERLIST_H
#define __LPGPU2PPCOUNTERLIST_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

namespace lpgpu2 {

// Container for counter attributes
struct ppCounter
{
    gtString name;
    gtString id;
    bool enabled;
};

// Container for counter category (set) attributes
struct ppCounterCategory
{
  ppCounterCategory();

  gtString name;
  gtString id;
  gtList<ppCounter> counters;
  bool enabled;
  gtUInt32 samplingInterval;
};

class ppCounterList : public QTreeWidget
{
    Q_OBJECT

public:
  explicit ppCounterList(QWidget* parent = nullptr);

  QList<ppCounterCategory> GetAllCategories();

  QTreeWidgetItem* AddCounterCategory(const QString& categoryName,
                                      const QString& categoryId,
                                      const QString &counterTooltip,
                                      gtUInt32 minSamplingInterval);
  QTreeWidgetItem* AddCounterToCategory(QTreeWidgetItem* categoryItem,
                                        const QString& counterName,
                                        const QString& counterId,
                                        const QString &counterTooltip);
  void SetCounterSetCheckedState(int counterSetId, bool isChecked);
  void SetCounterSetSamplingInterval(int counterSetId,
      gtUInt32 samplingInterval);
  void SetCounterCheckedState(int counterId, bool isChecked);
  size_t GetEnabledCountersPerCategory(const QString &categoryName) const;
  void ClearCountersCache();

// Type aliases
private:
  using CounterSetWidgetsVec = gtVector<QTreeWidgetItem *>;
  using CounterWidgetsVec = gtVector<QTreeWidgetItem *>;
  using CounterCategoryMap = QMap<QString, ppCounterCategory>;

// Attributes
private:
  CounterCategoryMap m_categoriesByNameMap;
  CounterSetWidgetsVec m_counterSetWidgetItems;
  CounterWidgetsVec m_counterWidgetItems;
  size_t m_numEnabledCounters;

// Methods
private slots:
  void OnCounterChanged(QTreeWidgetItem* item, int column);
  void OnSpinboxValueChanged(double d);
};

} // namespace lpgpu2

#endif // __LPGPU2PPCOUNTERLIST_H
