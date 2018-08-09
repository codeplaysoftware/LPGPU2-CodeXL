// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppFrameTimeline.h
///
/// LPGPU2ppFrameTimeline definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved

#include "LPGPU2ppFrameTimeline.h"

LPGPU2ppFrameTimeline::LPGPU2ppFrameTimeline(QWidget* parent, Qt::WindowFlags flags)
    : acTimeline(parent, flags)
{
    m_pHScrollBar = nullptr; // Disable scroll bars for this timeline
}

void LPGPU2ppFrameTimeline::updateGrid()
{
  if (m_pGrid != nullptr)
  {
    m_pGrid->setStartTime(m_nStartTime);
    m_pGrid->setFullRange(m_nFullRange);        
    m_pGrid->setVisibleStartTime(m_nVisibleStartTime);
    m_pGrid->setVisibleRange(m_nVisibleRange);

    double startPivot;
    double endPivot;

    if (m_dEndSelectionPivot != -1.0)
    {
      startPivot = m_dStartSelectionPivot;
      endPivot = m_dEndSelectionPivot;
    }
    else
    {
      startPivot = m_dPivot;
      endPivot = m_dPivot;
    }

    quint64 time = m_nVisibleStartTime + (quint64)(startPivot * m_nVisibleRange);
    m_pGrid->setSelectedTime(time);

    time = m_nVisibleStartTime + (quint64)(endPivot * m_nVisibleRange);
    m_pGrid->setEndSelectedTime(time);

    m_pGrid->update();
  }
}
