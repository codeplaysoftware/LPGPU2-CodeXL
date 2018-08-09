// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief SYCLTraceAgent is the project that defines a Dynamic Linked Library
///        meant to be injected into the target SYCL application process. This
///        will register a callback function to capture the profiled events.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef SYCLAPIINFO_H_INCLUDE
#define SYCLAPIINFO_H_INCLUDE

// Local:
#include "APIInfo.h"

#include <codexl_profiling_api.h>

/// @brief  SYCL API base class
class SYCLAPIInfo : public APIInfo
{
public:
        
    // Ideally SYCLProfileEvent would derive APIInfo, but this can't be done as
    // the header also needs to be present in ComputeCpp, which would require
    // all the usual AMD and CodeXL headers. Instead this class acts as a wrapper
    std::unique_ptr<SYCLProfiler::Event> m_syclProfileEvent;
    
    unsigned int m_nestLevel = 0; /// How nested the event is in the tree structure
    unsigned int m_queueNestLevel = 0; /// How nested the event is in the queue
};

#endif // SYCLAPIINFO_H_INCLUDE