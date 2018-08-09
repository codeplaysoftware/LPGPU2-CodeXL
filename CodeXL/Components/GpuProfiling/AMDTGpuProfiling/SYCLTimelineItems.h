// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief The SYCL profiler GUI is used to show SYCL traces into CodeXL.
///
/// SYCLAPITimelineItem interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include "APITimelineItems.h"



/// HostAPITimelineItem descendant for OpenSYCL API items
class SYCLAPITimelineItem : public HostAPITimelineItem
{
public:
	// Typedefs
	using SYCLTimelineItemInfo_t = std::vector<std::pair<std::string,std::string>>;
	
	/// Initializes a new instance of the SYCLAPITimelineItem class
	/// \param startTime the start time for this timeline item.
	/// \param endTime the end time for this timeline item.
	/// \param apiIndex the index of this api in the application's call sequence
	SYCLAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex, SYCLTimelineItemInfo_t timelineInfo);
	
	/// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
	/// \param tooltip qcTimelineItemToolTip instance that should get populated with name/value pairs
	virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

private:
	SYCLTimelineItemInfo_t m_timelineInfo;
};