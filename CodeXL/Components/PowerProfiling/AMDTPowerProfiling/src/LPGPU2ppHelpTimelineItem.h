// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppHelpTimelineItem.h
///
/// @brief Timeline item widget that displays Regions of Interest read from a
///        CodeXL power profiling database file (.cxldb).
///
/// ppHelpTimelineItem interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef _PPHELPTIMELINEITEM_H_
#define _PPHELPTIMELINEITEM_H_

#include <AMDTPowerProfiling/src/LPGPU2ppTimelineItem.h>

class LPGPU2ppHelpTimelineItem : public LPGPU2ppTimelineItem
{
	Q_OBJECT
public:

    LPGPU2ppHelpTimelineItem(quint64 startTime, quint64 endTime, quint64 id, const QString &helpTitle, float degreeOfInterest, bool inMillisec);

	virtual ~LPGPU2ppHelpTimelineItem() {}

	quint64 helpId() const { return m_helpId; }
    const QString &helpTitle() const { return m_helpTitle; }
    float helpDegreeOfInterest() const { return m_degreeOfInterest; }

	/// Gets the tooltip items for this timeline item.
	/// \param [out] tooltip reference that gets populated with the tooltip contents for this timeline item.
	virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

protected:
	quint64 m_helpId;
	QString m_helpTitle;
//++CF:LPGPU2 (Note that we used to also store a filename here, but ROIs can span
//						 multiple shaders or possibly none, so this is no longer the case.)
	float		m_degreeOfInterest;
//--CF:LPGPU2

};
//--KB: LPGPU2DEMO_V

#endif // _PPHELPTIMELINEITEM_H_
