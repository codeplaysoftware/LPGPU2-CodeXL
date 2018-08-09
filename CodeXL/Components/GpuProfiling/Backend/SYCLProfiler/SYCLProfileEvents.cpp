// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file SYCLProfileEvents.cpp
///
/// @brief 
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <sstream>

#include <codexl_profiling_api.h>

namespace SYCLProfiler
{

/// @brief Create a SYCL Profile Event from a line given in a string stream
std::unique_ptr<Event> Event::CreateFromATPText(std::stringstream& ss)
{
  // Read in the event name first, which decides which class of Event will be
  // constructed:
  std::string eventName;
  ss >> eventName;
  
  if (eventName == "SYCL_BUFFER_CREATE")
    return std::unique_ptr<Event>(new BufferCreateEvent());
  else if (eventName == "SYCL_BUFFER_DESTROY")
    return std::unique_ptr<Event>(new BufferDestroyEvent());
  else if (eventName == "SYCL_REQUISITE_ACTION")
  {
    unsigned int queueID, transactionID;
    ss >> transactionID;
    ss >> queueID;
    return std::unique_ptr<Event>(new RequisiteActionEvent(queueID, transactionID));
  }
  else if (eventName == "SYCL_COMMAND_COMMIT")
  {
    unsigned int queueID, transactionID;
    ss >> transactionID;
    ss >> queueID;
    return std::unique_ptr<Event>(new CommandCommitEvent(queueID, transactionID));
  }
  else
  {
    unsigned int stateID, queueID, transactionID;
    std::string kernelName;
    ss >> kernelName;
    ss >> transactionID;
    ss >> queueID;
    
    if (eventName == "SYCL_TRANS_CREATED")
      stateID = 0;
    else if (eventName == "SYCL_TRANS_PACKED")
      stateID = 1;
    else if (eventName == "SYCL_TRANS_SCHEDULED")
      stateID = 2;
    else if (eventName == "SYCL_TRANS_RUNNING")
      stateID = 3;
    else if (eventName == "SYCL_TRANS_DONE")
      stateID = 4;
    else if (eventName == "SYCL_TRANS_ROLLBACK")
      stateID = 5;
    else
      stateID = 6;
      
    return std::unique_ptr<Event>(new TransactionEvent(stateID, queueID, transactionID, std::move(kernelName)));
  }
}
} // namespace SYCLProfiler