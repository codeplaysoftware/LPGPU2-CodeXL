// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameCounterSummary.h
///
/// LPGPU2ppFrameCounterSummary declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPFRAMECOUNTERSUMMARYTABLE_H
#define __LPGPU2PPFRAMECOUNTERSUMMARYTABLE_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppTimelineView.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>


/// @brief    Widget contaning a table summarising the approximate values
///           of each counter for a given frame. Values are estimated 
///           from samples in the database.
/// @see      ppFrameDetailView
/// @author   Callum Fare
class LPGPU2ppFrameCounterSummaryTable final : public QWidget
{
  Q_OBJECT
  
public:
  LPGPU2ppFrameCounterSummaryTable(QWidget *pParent, ppSessionController* pSessionController, quint64 frameId, quint64 frameEnd);
  virtual ~LPGPU2ppFrameCounterSummaryTable();

  lpgpu2::PPFnStatus Initialise();
  lpgpu2::PPFnStatus Shutdown();
  
private:
  lpgpu2::PPFnStatus InitViewLayout();
  lpgpu2::PPFnStatus PopulateTable();
  double GetCounterAverage(const gtVector<SampledValue> &vSampledValues);

private:
  QGroupBox           *m_pGroupBox = nullptr;
  QTableWidget        *m_pTableWidget = nullptr;
  ppSessionController *m_pSessionController = nullptr;
  quint64              m_frameId;
  quint64              m_frameEndTime;
}; // LPGPU2ppFrameCounterSummaryTable


/// @brief Class that acts as a QTableWidgetItem, except overrides operator< to
///        allow proper sorting of items containing string formatted percentages.
class LPGPU2ppTableItemPercentSortable : public QTableWidgetItem
{
public:
  bool operator< (const QTableWidgetItem& other) const override
  {
    if (text().endsWith('%'))
    {
      QString str1 = text();
      QString str2 = other.text();
      str1.chop(2);
      str2.chop(2);
      return str1.toDouble() < str2.toDouble();
    }
    else
    {
      return text() < other.text();
    }
  }
};

#endif //__LPGPU2PPFRAMECOUNTERSUMMARYTABLE_H