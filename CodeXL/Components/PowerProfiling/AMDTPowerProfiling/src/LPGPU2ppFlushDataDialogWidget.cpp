// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFlushDataDialogWidget.cpp
///
/// @brief Widget used to display a "please wait" message to the user while CXL
///        flushes data from the remote device
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfiling/src/LPGPU2ppFlushDataDialogWidget.h>
#include <thread>
#include <chrono>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace lpgpu2 {

const QString kWindowTitle{"Flushing data"};
const QString kWindowContent{"The remaining profile data is being flushed\
 from the device. Please wait..."};

/// @brief Ctor; sets up the look of the widget
/// @param p The parent widget
FlushDataDialog::FlushDataDialog(QWidget* p)
: QDialog{ p }
{
  setWindowTitle(kWindowTitle);
  auto *mainLayout = new QVBoxLayout{this};
  mainLayout->addWidget(new QLabel{kWindowContent});
  setLayout(mainLayout);
}

/// @brief Called automatically when the dialog is shown
/// @param The associated event
void FlushDataDialog::showEvent(QShowEvent *)
{
  emit dialogShown();
}

} // namespace lpgpu2
