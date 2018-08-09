// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppHelpTimelineItem.cpp
///
/// ppHelpTimelineItem implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include "LPGPU2ppHelpTimelineItem.h"

LPGPU2ppHelpTimelineItem::LPGPU2ppHelpTimelineItem(quint64 startTime, quint64 endTime, quint64 id, 
																									 const QString &title, float degreeOfInterest, bool inMillisec)
: LPGPU2ppTimelineItem(startTime, endTime, inMillisec)
, m_helpId(id)
, m_helpTitle(title)
, m_degreeOfInterest(degreeOfInterest)
{
}

void LPGPU2ppHelpTimelineItem::tooltipItems(acTimelineItemToolTip& tooltip) const
{
	LPGPU2ppTimelineItem::tooltipItems(tooltip);

  tooltip.add(tr("Degree of Interest"), QString::number(m_degreeOfInterest));
}
