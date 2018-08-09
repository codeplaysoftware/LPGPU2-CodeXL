// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief SYCLTraceAgent is the project that defines a Dynamic Linked Library
///        meant to be injected into the target SYCL application process. This
///        will register a callback function to capture the profiled events.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef _SYCL_TRACE_AGENT_H
#define _SYCL_TRACE_AGENT_H

#include "codexl_profiling_api.h"

namespace SYCLProfiler
{

void SYCLProfilerInit();
void SYCLProfilerDeinit();

#if defined _WIN32
    #define SYCL_IMPORT __declspec(dllimport)
    #define so_constructor
    #define so_destructor
#else
    #define SYCL_IMPORT
    #define so_constructor __attribute__((constructor))
    #define so_destructor __attribute__((destructor))
#endif

/// @brief Wrapper around a SYCL Profile Event and timing information
class EventPart
{
    // Methods
public:
    // Ctors
    EventPart() = default;
    EventPart(std::unique_ptr<Event> event, EventTimingType timingType, uint64_t timeNs)
    {
        m_event = std::move(event);
        m_timingType = timingType;
        m_timeNs = timeNs;
        m_bNeedToDelete = m_event->IsEventToBeDeleted();
    };

    virtual ~EventPart()
    {
        // There are cases where we no longer have access to the code that
        // handles m_event allocation/deallocation. On Widows, this might happen
        // when events comes from the SYCL runtime. If the flag m_bNeedToDelete is
        // set to false we release the pointer from the unique_ptr control and let
        // the OS handle it.
        if (!m_bNeedToDelete)
        {
            m_event.release();
        }
    }

    // Required since we have a non-default constructor.
    EventPart(EventPart&&) = default;
    EventPart& operator=(EventPart&& rhs) = default;    
        
    /// @brief Return pointer to the event data.
    const std::unique_ptr<Event>& EventPtr() const
    {
        return m_event;
    };
    /// @brief Return whether this event part represents the start or end of an event.
    EventTimingType TimingType() const
    {
        return m_timingType;
    };
    /// @brief Return the timestamp associated with this event part in nanoseconds.
    uint64_t TimeNs() const
    {
        return m_timeNs;
    };
    
    /// Hints for where to place the event on the trace timeline, gets set during
    /// event processing after target program has finished:
    unsigned int  DepthThread() const  { return m_depthThread; }
    void          SetDepthThread(unsigned int depth) { m_depthThread = depth; }
    unsigned int  DepthQueue() const  { return m_depthQueue; }
    void          SetDepthQueue(unsigned int depth) { m_depthQueue = depth; }

private:
    std::unique_ptr<Event>  m_event = nullptr;
    bool                    m_bNeedToDelete = false;
    EventTimingType         m_timingType = EventTimingType::START;
    uint64_t                m_timeNs = 0;
    unsigned int            m_depthThread = 0;
    unsigned int            m_depthQueue = 0;
};

/// @brief Acts as a wrapper for two parts of an event, and the depth/lane
///        to display the event at in the timeline view.
class EventComplete
{
public:
  EventComplete(EventPart&& startPart, EventPart&& endPart, unsigned int depthThread)
  : m_startPart(std::move(startPart))
  , m_endPart(std::move(endPart))
  , m_depthThread(depthThread)
  , m_depthQueue(1)
  , m_bUsesQueue(false)
  {}
  
  EventComplete(EventPart&& startPart, EventPart&& endPart, unsigned int depthThread,
                unsigned int depthQueue)
  : m_startPart(std::move(startPart))
  , m_endPart(std::move(endPart))
  , m_depthThread(depthThread)
  , m_depthQueue(depthQueue)
  , m_bUsesQueue(true)
  {}
    
  EventPart     m_startPart;
  EventPart     m_endPart;
  unsigned int  m_depthThread;
  unsigned int  m_depthQueue;
  bool          m_bUsesQueue;
};

} // SYCLProfiler

#endif // _SYCL_TRACE_AGENT_H