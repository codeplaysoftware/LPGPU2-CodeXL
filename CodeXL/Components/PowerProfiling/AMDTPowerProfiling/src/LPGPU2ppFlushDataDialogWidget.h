// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFlushDataDialogWidget.h
///
/// @brief Widget used to display a "please wait" message to the user while CXL
///        flushes data from the remote device
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PPFLUSHDATADIALOGWIDGET_H
#define LPGPU2_PPFLUSHDATADIALOGWIDGET_H

// Qt
#include <QDialog>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

namespace lpgpu2 {

/// @brief Widget to display a "please wait" message to the user
/// @date     01/02/2018
/// @author   Alberto Taiuti
// clang-format off
class PP_API FlushDataDialog : public QDialog
{
  Q_OBJECT

// Methods
public:
  // Ctor
  explicit FlushDataDialog(QWidget* p = nullptr);

// Methods
signals:
  void dialogShown();

// Methods
protected:
  void showEvent(QShowEvent*);

};
// clang-format on

} // namespace lpgpu2

#endif // LPGPU2_PPFLUSHDATADIALOGWIDGET_H
