// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppTimelineItem.h
///
/// @brief Timeline item widget that displays OpenGL API calls read from a
///        CodeXL power profiling database file (.cxldb).
///
/// ppTimelineItem interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


#ifndef _LPGPU2PPTIMELINEITEM_H_
#define _LPGPU2PPTIMELINEITEM_H_

#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>

class LPGPU2ppTimelineItem : public acTimelineItem
{
    Q_OBJECT
public:
    LPGPU2ppTimelineItem(quint64 startTime, quint64 endTime, bool inMillisec);

    virtual ~LPGPU2ppTimelineItem() {}
  
    /// Gets the tooltip items for this timeline item.
    /// \param [out] tooltip reference that gets populated with the tooltip contents for this timeline item.
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const override;
private:
    // Overriding this as input units are milliseconds rather than nanoseconds.
    static QString GetDurationStringMs(quint64 duration);

protected:
    bool    m_inMillisec;
};

#endif // _LPGPU2PPTIMELINEITEM_H_
