// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppGLTimelineItem.h
///
/// @brief Timeline item widget that displays OpenGL API calls read from a
///        CodeXL power profiling database file (.cxldb).
///
/// ppGLTimelineItem interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


#ifndef _LPGPU2PPGLTIMELINEITEM_H_
#define _LPGPU2PPGLTIMELINEITEM_H_

#include <AMDTPowerProfiling/src/LPGPU2ppTimelineItem.h>

class LPGPU2ppGLTimelineItem : public LPGPU2ppTimelineItem
{
    Q_OBJECT
public:
    LPGPU2ppGLTimelineItem(quint64 startTime, quint64 endTime, quint64 frameId, quint64 drawId, bool inMillisec);

    virtual ~LPGPU2ppGLTimelineItem() {}

    int frameId() const { return m_frameId; }
    int drawId() const { return m_drawId; }    

    /// Gets the tooltip items for this timeline item.
    /// \param [out] tooltip reference that gets populated with the tooltip contents for this timeline item.
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const override;
    // Overriding this as input units are milliseconds rather than nanoseconds.
    static QString getDurationString(quint64 duration);

protected:
    quint64 m_frameId;
    quint64 m_drawId;
};

#endif
