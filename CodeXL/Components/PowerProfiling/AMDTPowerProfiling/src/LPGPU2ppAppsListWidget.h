// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppAppsListWidget.h
///
/// @brief Widget used to display the list of available applications. It extends
///        Qt's ListWidget to provide extra functionalities
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PPAPPSLISTWIDGET
#define LPGPU2_PPAPPSLISTWIDGET

// Qt
#include <QListWidget>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Forward declarations
namespace lpgpu2 {
  enum class PPFnStatus;
} // namespace lpgpu2


namespace lpgpu2 {

/// @brief    Source code display widget derived from afSourceCodeView. It acts
///           a wrapper around the base class so that custom functionalities
///           can be added.
/// @date     02/02/2018.
/// @author   Alberto Taiuti.
// clang-format off
class PP_API AppsList final : public QListWidget
{
  Q_OBJECT

// Methods
public:
  explicit AppsList(QWidget* pParent = nullptr);
  ~AppsList();
  // We deliberately do not declare move and copy ctors.

  // Resource handling
  PPFnStatus Initialise();
  PPFnStatus Shutdown();

// Metohds
private:
  QListWidgetItem *m_pLastSelectedItem = nullptr;

// Methods
private slots:
  void OnItemClicked(QListWidgetItem *item);
  void OnCurrentItemChanged(QListWidgetItem *current,
      QListWidgetItem *previous);

};
// clang-format on

} // namespace lpgpu2

#endif // LPGPU2_PPAPPSLISTWIDGET
