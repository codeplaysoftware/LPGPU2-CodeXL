// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppInfoWidget.h
///
/// LPGPU2ppInfoWidget declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPINFOWIDGET_H
#define __LPGPU2PPINFOWIDGET_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

/// @brief    Widget to display information to the user, with title and
///           main message body.
/// @author   Callum Fare
class LPGPU2ppInfoWidget final : public QWidget
{
  Q_OBJECT
// Methods:
public:
  LPGPU2ppInfoWidget(const QString& vTitle, const QString& vMsg, QWidget* vpParent);
  virtual ~LPGPU2ppInfoWidget();

  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  lpgpu2::PPFnStatus InitViewLayout();
  
// Attributes:
private:
  QString m_title;
  QString m_msg;

}; // LPGPU2ppInfoWidget


#endif //__LPGPU2PPINFOWIDGET_H