// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameAPICallTable.h
///
/// LPGPU2ppFrameAPICallTable declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPFRAMEAPICALLTABLE_H
#define __LPGPU2PPFRAMEAPICALLTABLE_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

/// @brief    Widget contaning a table listing each API call made throughout
///           the duration of a given frame.
/// @see      ppFrameDetailView
/// @author   Callum Fare
class LPGPU2ppFrameAPICallTable final : public QWidget
{
  Q_OBJECT
  
public:
  LPGPU2ppFrameAPICallTable(QWidget *pParent, quint64 frameId);
  virtual ~LPGPU2ppFrameAPICallTable();

  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  lpgpu2::PPFnStatus InitViewLayout();
  lpgpu2::PPFnStatus PopulateTable(
      const gtVector<lpgpu2::db::ProfileTraceInfo>& vecTraces,
      const gtVector<lpgpu2::db::StackTraceParameter>& vecParams);
  lpgpu2::PPFnStatus SelectRow(quint64 drawNum);

signals:
  void APICallSelected(quint64 drawNum);

private slots:
  void OnTableItemClicked();


private:
  QGroupBox    *m_pGroupBox = nullptr;
  QTableWidget *m_pTableWidget = nullptr;
  quint64       m_frameId;
}; // LPGPU2ppFrameAPICallTable


#endif //__LPGPU2PPFRAMEAPICALLTABLE_H
