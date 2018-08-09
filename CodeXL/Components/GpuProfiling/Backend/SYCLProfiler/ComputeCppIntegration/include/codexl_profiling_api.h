// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file SYCLProfileEvents.h
///
/// @brief SYCLProfileEvents contains the different kind of events that the
///        CodeXL SYCL profiler can handle.
///        NOTE that this is header file is required by both CodeXL and
///             ComputeCpp and must be kept consistent.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef COMPUTECPP_PROFILING_CODEXL_INTERFACE
#define COMPUTECPP_PROFILING_CODEXL_INTERFACE

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace SYCLProfiler {

/// Typedef
using TimelineItemInfo_t = std::vector<std::pair<std::string, std::string>>;

/// @brief Enum which determines which category a SYCL Profile event belongs to,
///        which is used to assign items to branches in the Trace View.
enum class EventCategory : int {
  NONE = -1,
  COMMAND_GROUP = 0,
  COMMAND_GROUP_DETAIL,
  DATA_TRANSFER,
  KERNEL_EXECUTION,

  CATEGORY_COUNT  // Keep this item last
};

/// @brief When a SYCLProfileEvent is wrapped in a SYCLProfileEventPart, this
///        indicates whether the associated time is for the start or end of
///        the event's duration.
enum class EventTimingType { START, END };

/// @brief Base class for all Profile events which are passed to the callback
class Event {
 public:
  /// @brief Get the name of this event (ie. the event type)
  virtual const std::string EventName() const = 0;

  /// @brief Does this event belong to a queue?
  virtual bool HasQueue() const { return false; }

  /// @brief Get the Queue ID of this event.
  virtual unsigned int QueueID() const { return m_uiQueueID; }

  /// @brief The category of event that this item belongs to.
  virtual EventCategory Category() const { return m_eCategory; }

  /// @brief Get event information with formatted labels.
  /// @return Vector of pairs of foramtted key/value strings
  virtual TimelineItemInfo_t TimelineInfo() const { return {}; }

  /// @brief Write event data to an output stream in ATP format
  virtual void AsATPText(std::ostream& out) const { out << EventName(); }

  void SetEventDeleteFlag(bool needToDelete) { m_deleteFlag = needToDelete; }

  bool IsEventToBeDeleted() const { return m_deleteFlag; }

  /// @brief
  /// @note We use a pointer here as we have to be able to downcast the pointer
  ///       in overriding functions:
  virtual bool Matches(const Event* const pEvt) const {
    return pEvt->EventName() == EventName();
  }

  /// @brief Construct a new SYCLProfileEvent from ATP string data.
  static std::unique_ptr<Event> CreateFromATPText(std::stringstream& ss);

 protected:
  EventCategory m_eCategory = EventCategory::NONE;
  unsigned int m_uiQueueID = 0;
  bool m_deleteFlag = false;
};

/******************************************************************************
      Create new SYCL Profile Events here:
      (Also add a matching case in SYCLProfileEvent::FromATPFile in
       SYCLProfileEvents.cpp in CodeXL)
*******************************************************************************/

/// @brief Profile Event for when a buffer is created.
class BufferCreateEvent : public Event {
 public:
  const std::string EventName() const override { return "SYCL_BUFFER_CREATE"; }
};

/// @brief Profile Event for when a buffer is destroyed.
class BufferDestroyEvent : public Event {
 public:
  const std::string EventName() const override { return "SYCL_BUFFER_DESTROY"; }
};

/// @brief Profile Event for when a requisite action is run.
class RequisiteActionEvent : public Event {
 public:
  RequisiteActionEvent(unsigned int queueID, unsigned int transactionID)
      : m_uiTransactionID(transactionID) {
    m_eCategory = EventCategory::COMMAND_GROUP_DETAIL;
    m_uiQueueID = queueID;
  };

  const std::string EventName() const override {
    return "SYCL_REQUISITE_ACTION";
  }
  virtual bool HasQueue() const override { return true; }
  virtual void AsATPText(std::ostream& out) const override {
    Event::AsATPText(out);
    out << std::string("\t") << std::to_string(m_uiTransactionID);
    out << std::string("\t") << std::to_string(m_uiQueueID);
  }

  virtual TimelineItemInfo_t TimelineInfo() const override {
    return {{"Transaction ID", std::to_string(m_uiTransactionID)},
            {"Queue ID", std::to_string(m_uiQueueID)}};
  }

 private:
  unsigned int m_uiTransactionID;
};

/// @brief Profile Event for when a command is committed.
class CommandCommitEvent : public Event {
 public:
  CommandCommitEvent(unsigned int queueID, unsigned int transactionID)
      : m_uiTransactionID(transactionID) {
    m_eCategory = EventCategory::COMMAND_GROUP_DETAIL;
    m_uiQueueID = queueID;
  };

  const std::string EventName() const override { return "SYCL_COMMAND_COMMIT"; }
  virtual bool HasQueue() const override { return true; }
  virtual void AsATPText(std::ostream& out) const override {
    Event::AsATPText(out);
    out << std::string("\t") << std::to_string(m_uiTransactionID);
    out << std::string("\t") << std::to_string(m_uiQueueID);
  };

  virtual TimelineItemInfo_t TimelineInfo() const override {
    return {{"Transaction ID", std::to_string(m_uiTransactionID)},
            {"Queue ID", std::to_string(m_uiQueueID)}};
  };

 private:
  unsigned int m_uiTransactionID;
};

/// @brief Profile Event for when a transaction changes state.
class TransactionEvent : public Event {
 public:
  TransactionEvent(unsigned int state, unsigned int queueID,
                   unsigned int transactionID, std::string&& kernelName)
      : m_uiTransactionState(state),
        m_uiTransactionID(transactionID),
        m_kernelName(std::move(kernelName)) {
    std::replace(m_kernelName.begin(), m_kernelName.end(), ' ', '_');
    m_eCategory = EventCategory::COMMAND_GROUP;
    m_uiQueueID = queueID;
  }

  /// @brief Get the event name, which varies depending on the transaction
  /// state.
  const std::string EventName() const override {
    switch (m_uiTransactionState) {
      case 0:
        return "SYCL_CG_CREATED";
      case 1:
        return "SYCL_CG_READY";
      case 2:
        return "SYCL_CG_SCHEDULED";
      case 3:
        return "SYCL_CG_RUNNING";
      case 4:
        return "SYCL_CG_COMPLETE  ";
      case 5:
        return "SYCL_CG_ROLLBACK";
      case 6:
      default:
        return "SYCL_CG_ABORTED";
    }
  }

  void AsATPText(std::ostream& out) const override {
    Event::AsATPText(out);
    out << std::string("\t") << m_kernelName;
    out << std::string("\t") << std::to_string(m_uiTransactionID);
    out << std::string("\t") << std::to_string(m_uiQueueID);
  }

  TimelineItemInfo_t TimelineInfo() const override {
    return {{"Transaction ID", std::to_string(m_uiTransactionID)},
            {"Queue ID", std::to_string(m_uiQueueID)},
            {"Kernel name", m_kernelName}};
  }

  bool HasQueue() const override { return true; }
  unsigned int TransactionID() const { return m_uiTransactionID; }

  bool Matches(const Event* const pEvt) const override {
    if (!Event::Matches(pEvt)) return false;

    // Event name matches so it is safe to cast:
    const auto* const pTransactionEvt =
        dynamic_cast<const TransactionEvent* const>(pEvt);
    return pEvt == nullptr
               ? false
               : pTransactionEvt->TransactionID() == TransactionID();
  }

 private:
  unsigned int m_uiTransactionState = 0;
  unsigned int m_uiTransactionID = 0;
  std::string m_kernelName = "(Unknown kernel)";
};

}  // namespace SYCLProfiler

#endif  // COMPUTECPP_PROFILING_CODEXL_INTERFACE
