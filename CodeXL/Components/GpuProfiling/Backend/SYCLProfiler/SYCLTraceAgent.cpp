// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file  SYCLTraceAgent.cpp
///
/// @brief SYCLTraceAgent is the project that defines a Dynamic Linked Library
///        meant to be injected into the target SYCL application process. This
///        will register a callback function to capture the profiled events.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <unordered_map>
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <sstream>

#include "APIInfoManagerBase.h"
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

#include <codexl_profiling_api.h>

#include "../Common/FileUtils.h"
#include "../Common/OSUtils.h"

#include "SYCLTraceAgent.h"

namespace SYCLProfiler
{

// Typedefs:
using Depth_t                     = unsigned int;
using CategoryID_t                = std::underlying_type<EventCategory>::type;
using MapCategoryIDToDepth_t      = std::unordered_map<CategoryID_t, Depth_t>;
using VecLaneUse_t                = std::vector<bool>;
using VecEventComplete_t          = std::vector<EventComplete>;
using VecEventPart_t              = std::vector<EventPart>;
using MapCategoryIDToVecLaneUse_t = std::unordered_map<CategoryID_t, VecLaneUse_t>;
using MapThreadIdToVecEvents_t    = std::unordered_map<osThreadId, VecEventPart_t>;
using MapThreadIdToVecCompleteEvents_t = std::unordered_map<osThreadId, VecEventComplete_t>;
using MapThreadIdToMapCatToDepthMax_t  = std::unordered_map<osThreadId, MapCategoryIDToDepth_t>;
using MapQueueIdToMapCatToDepthMax_t   = std::unordered_map<unsigned int, MapCategoryIDToDepth_t>;
using MapThreadToIterator_t            = std::unordered_map<osThreadId, VecEventPart_t::iterator>;
using MapThreadToMapCatToVecLaneUsage_t = std::unordered_map<osThreadId, MapCategoryIDToVecLaneUse_t>;
using MapQueueIdToMapCatToVecLaneUsage_t = std::unordered_map<unsigned int, MapCategoryIDToVecLaneUse_t>;
using Callback_t = std::function<void(std::unique_ptr<Event>, EventTimingType)>;

static MapThreadIdToVecEvents_t mapThreadIdToVecEvents;
static std::mutex mt;
static bool bCallbackRegistered = false;

// The callback function, called at the start and end of every sycl event to
// be profiled
extern "C" void SYCLCallback(std::unique_ptr<Event> pEvent, EventTimingType timingType)
{
    const auto threadId = osGetUniqueCurrentThreadId();
    const auto timeNow = OSUtils::Instance()->GetTimeNanos();

    mt.lock();
    auto mapIt = mapThreadIdToVecEvents.find(threadId);
    if (mapIt == mapThreadIdToVecEvents.end())
    {
        mapIt = mapThreadIdToVecEvents.emplace(std::piecewise_construct,
                                               std::forward_as_tuple(threadId),
                                               std::forward_as_tuple()).first;
    }

    auto &vecEvents = mapIt->second;
    mt.unlock();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // On Windows, the DLL unload order is not guaranteed. We are using move semantics of unique pointers to create
    // events on the SYCL runtime DLL and storing them here. However, when shutting down the profiled application
    // the code that handles the content of the unique ptrs gets freed and we can no longer use it. So, for Windows,
    // we are setting a flag to not delete the stored event when it comes from the SYCL runtime.
    const auto bNeedToDeleteEvent = false;
    pEvent->SetEventDeleteFlag(bNeedToDeleteEvent);
#endif

    vecEvents.emplace_back(std::move(pEvent), timingType, timeNow);
}


/// @brief Output processed events to a temporary file. This file is later
///        processed to create the ATP file.
void SYCLProfilerDumpToFile(const MapThreadIdToVecCompleteEvents_t& mapThreadIdToVecCompleteEvents,
                            MapThreadIdToMapCatToDepthMax_t& mapThreadIdToMapCatToDepthMax,
                            MapQueueIdToMapCatToDepthMax_t& mapQueueIdToMapCatToDepthMax)
{
    std::string tmpOut = FileUtils::GetTempFragFilePath();

    tmpOut += "/sycltrace"  TMP_TIME_STAMP_EXT;
    std::fstream file(tmpOut, std::ios::out);

    // Most ATP info is per-thread, but queue / category depths is global,
    // so output this first:
    file << mapQueueIdToMapCatToDepthMax.size() << "\n"; // Number of queues
    for (const auto& pairQueueIDMapCatToDepthMax : mapQueueIdToMapCatToDepthMax)
    {
      file << pairQueueIDMapCatToDepthMax.first << "\n"; // Queue ID
      const auto& mapCatToDepthMax = pairQueueIDMapCatToDepthMax.second;
      file << mapCatToDepthMax.size() << "\n"; // Number of categories
      for (const auto& pairCategoryDepth : mapCatToDepthMax)
      {
        file << pairCategoryDepth.first << "\t"
             << pairCategoryDepth.second << "\n"; // category & depth
      }
    }

    /**
    * Format :
    * [thread-id]
    * [num Events]
    * [max depth]
    *
    * ... Events ...
    **/
    for (const auto& it : mapThreadIdToVecCompleteEvents)
    {
        file << it.first << "\n"  // Thread ID
            << it.second.size() << "\n"
        // Output data on nesting depth per category:
            << mapThreadIdToMapCatToDepthMax[it.first].size() << "\n";
        for (const auto& catDepth : mapThreadIdToMapCatToDepthMax[it.first])
        {
          file << catDepth.first << "\t" << (catDepth.second) << "\n"; // category & depth
        }

      // Thread depth for every profile event category:
        for (const auto& completeEvent : it.second)
        {
          #define IOS_TAB "\t"
              file << completeEvent.m_startPart.TimeNs()
                  << IOS_TAB << completeEvent.m_endPart.TimeNs()
                  << IOS_TAB << it.first // thread id
                  << IOS_TAB << completeEvent.m_depthThread
                  << IOS_TAB << completeEvent.m_depthQueue + 1
                  << IOS_TAB;
              completeEvent.m_endPart.EventPtr()->AsATPText(file);
          #undef IOS_TAB
              file << "\n";
        }
    }
}

/// @brief Called on initialisation of the library. Loads the specified
///        SYCL runtime library and attempts to register the callback.
so_constructor void SYCLProfilerInit()
{
    std::string SYCLRuntimePath = OSUtils::Instance()->GetEnvVar("SYCL_RUNTIME_LIB");

    // It turn out that on Linux, because of rpath, the actual sycl runtime library
    // being loaded here needs to match with the sycl runtime library that the application
    // to be profiled was linked against, otherwise the GenericLoadLibrary function will
    // load a different version of the sycl runtime leading to a deadlock at the of the
    // program. This is solved by, on Linux, just asking the dlsym function to get the
    // desired address using the special handle RTLD_DEFAULT, which is actually NULL.
    // From the dlsym man pages:

    // RTLD_DEFAULT
    //    Find the first occurrence of the desired symbol using the default shared object search order.
    //    The search will include global symbols in the executable and its dependencies, as well as symbols in shared objects that
    //    were dynamically loaded with the RTLD_GLOBAL flag.

    // This means the OS will look for the first occurrence of the symbol we are looking for.
    // We didn't see this problem on Windows.

    osModuleHandle syclRuntimeModule = nullptr;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    assert(!SYCLRuntimePath.empty() && "the sycl runtime lib path should be specified");
    syclRuntimeModule = OSUtils::Instance()->GenericLoadLibrary(SYCLRuntimePath);
#endif
    osProcedureAddress pCallbackSetupHandler = nullptr;

    if (osGetProcedureAddress(syclRuntimeModule, "SYCLSetupProfilerCallback", pCallbackSetupHandler))
    {
        typedef void(*CallbackSetup_t)(Callback_t);
        auto SYCLSetupProfilerCallback = reinterpret_cast<CallbackSetup_t>(pCallbackSetupHandler);
        SYCLSetupProfilerCallback(SYCLCallback);
        bCallbackRegistered = true;
    }

#ifdef _DEBUG
    std::cout << "Allow debugger to attach if needed (PID "
              << osGetCurrentProcessId() << "), press enter to run: ";
    std::string a;
    std::getline(std::cin, a);
#endif // _DEBUG
}

/// @brief Templated function to allocate a timeline lane number to a particular
///        event. Lanes are allocated for a particular group (i.e. thread) and
///        category.
/// @param[in] groupID  The identifier of the group this event belongs to
/// @param[in] categoryID The category identifier this event has
/// @param[in/out] laneMap Map of group IDs to a map of counter IDs to a vector
///                representing lane usage.
/// @param[in/out] maxMap Map of group IDs to a map of counter IDs to the maximum
///                number of lanes used so far.
/// @param[in] applyDepth Function which processes the assigned lane in some way,
///            i.e. assigns to the EventPart object.
template <typename T>
void AllocateEventLane(T groupID,
                      CategoryID_t categoryID,
                      std::unordered_map<T, MapCategoryIDToVecLaneUse_t>& laneMap,
                      std::unordered_map<T, MapCategoryIDToDepth_t>& maxMap,
                      std::function<void(Depth_t)> applyDepth)
{
  auto& vecLanes = laneMap[groupID][categoryID];
  size_t laneID;
  for (laneID = 0; laneID < vecLanes.size(); laneID++)
  {
    // Unused lane found, use it:
    if (!vecLanes[laneID])
    {
      applyDepth(static_cast<Depth_t>(laneID));
      vecLanes[laneID] = true;
      break;
    }
  }
  // Didn't find a lane, create a new one:
  if (laneID == vecLanes.size())
  {
    applyDepth(static_cast<Depth_t>(vecLanes.size()));
    vecLanes.push_back(true);
    maxMap[groupID][categoryID] = static_cast<Depth_t>(vecLanes.size());
  }
}

/// @brief Process the vectors of SYCLProfileEventParts, adding them to the
//         relevant Event Manager for each thread.
void SYCLProfilerProcessEvents(MapThreadIdToVecCompleteEvents_t& mapThreadIdToVecCompleteEvents,
                               MapThreadIdToMapCatToDepthMax_t& mapThreadIdToMapCatToDepthMax,
                               MapQueueIdToMapCatToDepthMax_t& mapQueueIdToMapCatToDepthMax)
{
  // We need to iterate through *all* the events in order, but they are grouped by thread. To
  // do this, each thread ID is mapped to an iterator over its vector of events. When selecting
  // the next event to process, all iterators are scanned for the event that was chronologically
  // first.
  MapThreadToIterator_t mapThreadIterators;

  // Vector of events that are currently open:
  VecEventPart_t vecOpenEvents;

  // Map of thread IDs to map of categories to vector of timeline lane allocations
  MapThreadToMapCatToVecLaneUsage_t mapThreadIdToMapCatToVecLaneUsage;

  // Map of queue IDs to map of categories to vector of timeline lane allocations
  MapQueueIdToMapCatToVecLaneUsage_t mapQueueIdToMapCatToVecLaneUsage;

  // Dump some info about captured events
#ifdef _DEBUG
  for (const auto& pairThreadIdVecEvents : mapThreadIdToVecEvents)
  {
    std::wstringstream ssCaptureInfo;
    ssCaptureInfo << "Thread " << pairThreadIdVecEvents.first << std::endl;
    ssCaptureInfo << "Captured " << pairThreadIdVecEvents.second.size() << " events." << std::endl;
    ssCaptureInfo << "End Events: " << std::count_if(pairThreadIdVecEvents.second.cbegin(),
                                                     pairThreadIdVecEvents.second.cend(),
                                                     [](const EventPart& e) {
                                                       return e.TimingType() == EventTimingType::END;
                                                     }) << std::endl;
    OS_OUTPUT_DEBUG_LOG(ssCaptureInfo.str().c_str(), OS_DEBUG_LOG_INFO);
  }
#endif // _DEBUG

  // Populate the iterator map, starting at the beginning of each thread's vector:
  for (auto& pairThreadIdVecEvents : mapThreadIdToVecEvents)
  {
    mapThreadIterators.emplace(pairThreadIdVecEvents.first, pairThreadIdVecEvents.second.begin());
  }

  // Iterate through all the thread event vectors, matching event starts and ends
  // to create complete events:
  while (!mapThreadIterators.empty())
  {
    // Find the thread whose current iterator points to the earliest event:
    auto earliestEventIter = std::min_element(mapThreadIterators.begin(), mapThreadIterators.end(),
                                              [](const MapThreadToIterator_t::value_type& a,
                                                 const MapThreadToIterator_t::value_type& b)
                                                 { return a.second->TimeNs() < b.second->TimeNs(); });

    // All remaining threads should have an event, check we found one anyway:
    assert(earliestEventIter != mapThreadIterators.end());

    auto&      selectedEvent = earliestEventIter->second;
    const auto categoryID = static_cast<CategoryID_t>(selectedEvent->EventPtr()->Category());
    const auto threadID = earliestEventIter->first;

    // If the event part is the start of an event, allocate appropriate timeline lanes
    // and add it to the vector of open events:
    if (selectedEvent->TimingType() == EventTimingType::START)
    {
      // Perform timeline lane allocation. With SYCL tracing, unlike regular API call traces,
      // there can be multiple events of the same type and thread and/or queue overlapping. To
      // allow all events to be visible in the timeline display, multiple 'lanes' are allowed
      // per category, and events are assigned to the first free lane found.
      AllocateEventLane(threadID,
                        categoryID,
                        mapThreadIdToMapCatToVecLaneUsage,
                        mapThreadIdToMapCatToDepthMax,
                        [&](Depth_t d){ selectedEvent->SetDepthThread(d);});

      // Lane allocation for queues:
      // (TODO: note that allocation for CREATED events does not work as expected,
      //        but visually this looks acceptable for now)
      if (selectedEvent->EventPtr()->HasQueue())
      {
        AllocateEventLane(selectedEvent->EventPtr()->QueueID(),
                          categoryID,
                          mapQueueIdToMapCatToVecLaneUsage,
                          mapQueueIdToMapCatToDepthMax,
                          [&](Depth_t d){ selectedEvent->SetDepthQueue(d);});
      }

      // Move this event into the vector of open events
      vecOpenEvents.emplace_back(std::move(*selectedEvent));
    }
    else
    {
      // End event, so iterate through the currently open events to find its matching
      // start event:
      const auto matchingStartEvent = std::find_if(vecOpenEvents.begin(), vecOpenEvents.end(),
                                                   [&](EventPart& evt) -> bool
                                                   {
                                                     return evt.EventPtr()->Matches(
                                                       selectedEvent->EventPtr().get());
                                                   });

      // We should have found a matching start event:
      assert(matchingStartEvent != vecOpenEvents.end());

      // Set thread depth as lane number and mark lane as now unused:
      auto threadDepth = matchingStartEvent->DepthThread();
      mapThreadIdToMapCatToVecLaneUsage[threadID][categoryID][threadDepth] = false;

      if (selectedEvent->EventPtr()->HasQueue())
      {
        // Set queue depth as lane number and mark lane as now unused:
        auto queueDepth = matchingStartEvent->DepthQueue();
        auto queueID = matchingStartEvent->EventPtr()->QueueID();
        mapQueueIdToMapCatToVecLaneUsage[queueID][categoryID][queueDepth] = false;
        mapThreadIdToVecCompleteEvents[threadID].emplace_back(std::move(*matchingStartEvent),
                                                              std::move(*selectedEvent),
                                                              threadDepth,
                                                              queueDepth);
      }
      else
      {
        mapThreadIdToVecCompleteEvents[threadID].emplace_back(std::move(*matchingStartEvent),
                                                              std::move(*selectedEvent),
                                                              threadDepth);
      }

      vecOpenEvents.erase(matchingStartEvent);
    }

    // Done with this event, move up the iterator and clear the thread from the map
    // if it has no more events.
    const auto& nextIt = ++(mapThreadIterators[threadID]);

    if (nextIt == mapThreadIdToVecEvents[threadID].end())
    {
      mapThreadIterators.erase(threadID);
    }
  }
}

/// @brief Called when the library is unloaded. This occurs when the target program
///        has finished executing, so the events are then processed and dumped to
///        a temp file.
so_destructor void SYCLProfilerDeinit()
{
    if (bCallbackRegistered)
    {
      // Map of thread IDs to their completed events
      // (They are grouped like this as the ATP format groups events by host thread ID)
      MapThreadIdToVecCompleteEvents_t mapThreadIdToVecCompleteEvents;

      // Map of thread IDs to map of categories to maximum nest depth
      MapThreadIdToMapCatToDepthMax_t mapThreadIdToMapCatToDepthMax;

      // Map of queue IDs to map of categories to maximum nest depth
      MapQueueIdToMapCatToDepthMax_t mapQueueIdToMapCatToDepthMax;

      SYCLProfilerProcessEvents(mapThreadIdToVecCompleteEvents,
                                mapThreadIdToMapCatToDepthMax,
                                mapQueueIdToMapCatToDepthMax);

      SYCLProfilerDumpToFile(mapThreadIdToVecCompleteEvents,
                            mapThreadIdToMapCatToDepthMax,
                            mapQueueIdToMapCatToDepthMax);

      // Delete env variable
      OSUtils::Instance()->UnsetEnvVar("SYCL_RUNTIME_LIB");
    }
}

} // namespace SYCLProfiler
