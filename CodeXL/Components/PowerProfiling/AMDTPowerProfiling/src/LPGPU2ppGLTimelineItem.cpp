// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppGLTimelineItem.cpp
///
/// @brief Timeline item widget that displays OpenGL API calls read from a
///        CodeXL power profiling database file (.cxldb).
///
/// ppGLTimelineItem implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


#include <AMDTPowerProfiling/src/LPGPU2ppGLTimelineItem.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

LPGPU2ppGLTimelineItem::LPGPU2ppGLTimelineItem(quint64 startTime, quint64 endTime, quint64 frameId, quint64 drawId, bool inMillisec) :
    LPGPU2ppTimelineItem(startTime, endTime, inMillisec), m_frameId(frameId), m_drawId(drawId)
{
}

//
void LPGPU2ppGLTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
  LPGPU2ppTimelineItem::tooltipItems(tooltip);
  
  QString frameString;
  frameString.setNum(m_frameId);
  tooltip.add(tr(PP_STR_LPGPU2_TooltipFrameId), frameString);

  QString drawString;
  drawString.setNum(m_drawId);
  tooltip.add(tr(PP_STR_LPGPU2_TooltipDrawId), drawString);
}