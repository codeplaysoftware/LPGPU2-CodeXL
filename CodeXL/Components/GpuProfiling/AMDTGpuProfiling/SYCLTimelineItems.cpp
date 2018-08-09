// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL profiler GUI is used to show SYCL traces into CodeXL.
///
/// CCVGMenuActionsExecutor interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include "SYCLTimelineItems.h"
#include "TraceTable.h"
#include <chrono>

#include <iostream>

/// @brief Ctor.
	SYCLAPITimelineItem::SYCLAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex, SYCLTimelineItemInfo_t timelineInfo)
	: HostAPITimelineItem(startTime, endTime, apiIndex)
	, m_timelineInfo(timelineInfo)
	{}
	

/// @brief Populate a tooltip with this item's data.
/// @param[out] tooltip		The tooltip to populate.
void SYCLAPITimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
	APITimelineItem::tooltipItems(tooltip);
	for (const auto& tooltipPair : m_timelineInfo)
	{
		tooltip.add(tr(tooltipPair.first.c_str()), tooltipPair.second.c_str());
	}
}