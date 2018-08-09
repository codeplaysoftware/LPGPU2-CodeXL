// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppTimelineItem.cpp
///
/// @brief Timeline item widget that displays OpenGL API calls read from a
///        CodeXL power profiling database file (.cxldb).
///
/// ppTimelineItem implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


#include <AMDTPowerProfiling/src/LPGPU2ppTimelineItem.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include <chrono>

#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

LPGPU2ppTimelineItem::LPGPU2ppTimelineItem(quint64 startTime, quint64 endTime, bool inMillisec)
: acTimelineItem(startTime, endTime)
, m_inMillisec(inMillisec)
{
}

void LPGPU2ppTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
  tooltip.add(tr(PP_STR_LPGPU2_TooltipName), m_strText);

  if (m_inMillisec)
  {
    // ++ This block is a copy of the origin tooltipItems, with the units adjusted
    //    to match usage in power profiling mode.
    tooltip.add(tr(PP_STR_LPGPU2_TooltipName), m_strText);

    auto strNum = GetDurationStringMs(m_nStartTime);
    tooltip.add(tr(PP_STR_LPGPU2_TooltipStartTime), strNum);

    strNum = GetDurationStringMs(m_nEndTime);
    tooltip.add(tr(PP_STR_LPGPU2_TooltipEndTime), strNum);

    const auto duration = m_nEndTime - m_nStartTime;
    const auto strDuration = GetDurationStringMs(duration);
    tooltip.add(tr(PP_STR_LPGPU2_TooltipDuration), strDuration);    
    // --     
  }
  else
  {
    using nsDouble = std::chrono::duration<double, std::nano>;
    using msDouble = std::chrono::duration<double, std::milli>;
    const nsDouble startNs{static_cast<double>(m_nStartTime)};
    const nsDouble endNs{static_cast<double>(m_nEndTime)};

    const auto startMs = std::chrono::duration_cast<msDouble>(startNs);
    const auto endMs = std::chrono::duration_cast<msDouble>(endNs);

    auto strNum = QString(tr("%1 millisecond")).arg(
        startMs.count(), 0, 'f', 3);
    tooltip.add(tr("Start Time"), strNum);

    strNum = QString(tr("%1 millisecond")).arg(endMs.count(), 0, 'f', 3);
    tooltip.add(tr("End Time"), strNum);

    const auto duration = endNs - startNs;
    QString strDuration = QString(tr("%1 millisecond")).arg(
        NanosecToTimeString(duration.count(), true, false));
    tooltip.add(tr("Duration"), strDuration);
  }
}

// Converts units to nanoseconds before calling the original getDurationString
QString LPGPU2ppTimelineItem::GetDurationStringMs(quint64 duration)
{
    const std::chrono::milliseconds durationMs{ duration };
    const auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(durationMs);
    return acTimelineItem::getDurationString(durationNs.count());
}
