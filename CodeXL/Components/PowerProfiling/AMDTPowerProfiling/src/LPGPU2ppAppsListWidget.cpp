// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppAppsListWidget.cpp
///
/// @brief Widget used to display the list of available applications. It extends
///        Qt's ListWidget to provide extra functionalities
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Qt:
#include <QtWidgets>

// Local
#include <AMDTPowerProfiling/src/LPGPU2ppAppsListWidget.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

namespace lpgpu2 {

/// @brief Ctor; no work is done here
/// @param pParent The parent of this widget
AppsList::AppsList(QWidget *pParent)
: QListWidget{pParent}, m_pLastSelectedItem{nullptr}
{
}

/// @brief Dtor; calls the Shutdown() method
AppsList::~AppsList()
{
  Shutdown();
}

/// @brief Initialise the object
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus AppsList::Initialise()
{
  connect(this, &QListWidget::itemClicked, this, &AppsList::OnItemClicked);
  connect(this, &QListWidget::currentItemChanged, this,
      &AppsList::OnCurrentItemChanged);

  return PPFnStatus::success;
}

/// @brief Deinitialise the object
/// @return PPFnStatus::success if it succeeded, PPFnStatus::failure if it
///         failed
PPFnStatus AppsList::Shutdown()
{
  disconnect(this, &QListWidget::itemClicked, this, &AppsList::OnItemClicked);
  disconnect(this, &QListWidget::currentItemChanged, this,
      &AppsList::OnCurrentItemChanged);

  return PPFnStatus::success;
}

/// @brief Callback called when an item is clicked. Deselects the current item
///        if it was clicked and it was already active
/// @param item The item which was clicked
void AppsList::OnItemClicked(QListWidgetItem *item)
{
  if (m_pLastSelectedItem == item)
  {
    setCurrentItem(item, QItemSelectionModel::Clear |
        QItemSelectionModel::Deselect |
        QItemSelectionModel::Current);
    m_pLastSelectedItem = nullptr;
  }
  else
  {
    m_pLastSelectedItem = item;
  }
}

/// @brief Callback called when the current item is changed
/// @param current The current item
/// @param previous The previous item
void AppsList::OnCurrentItemChanged(QListWidgetItem *current,
      QListWidgetItem *previous)
{
  (void)previous;

  if (current != nullptr && current->isSelected())
  {
    m_pLastSelectedItem = current;
  }
}

} // namespace lpgpu2
