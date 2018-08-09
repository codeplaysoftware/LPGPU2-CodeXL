// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Represents the base packet parser which is implemented per-packet
///        type.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_BASEDATAPACKETPARSER_H_INCLUDE
#define LPGPU2_BASEDATAPACKETPARSER_H_INCLUDE

// STL
// Introduce:
// std::vector
// std::unique_ptr
// std::function
// assert
// std ints
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <cassert>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// Forward declarations
class amdtProfileDbAdapter;
enum class DataChunkType : uint32_t;

// clang-format off
namespace lpgpu2 {

/// @brief Represents the advancement within a chunk in terms of the data
///        read from it
struct BytesParser
{
// Type aliases
public:
  using DataVec = std::vector<char>;

// Methods
public:
  /// @brief Reads sizeof(T) bytes from the data in the BytesParser and
  ///        updates its internal state
  /// @return Always success
  template <typename T>
  PPFnStatus ReadBytes(T &t)
  {
    memcpy(&t, &data[off], sizeof(t));
    off += sizeof(t);

    return PPFnStatus::success;
  }

// Attributes
public:
  size_t off;
  const DataVec &data;

}; // struct BytesParser

/// @brief    Base abstract class for specific data packet types parsers
/// @warning  None.
/// @date     19/09/2017.
/// @author   Alberto Taiuti.
class BaseDataPacketParser
{
// Type aliases
private:
  using SetTimeHandler = std::function<void(std::uint64_t)>;

// Rule of 5
// Default
public:
  BaseDataPacketParser() = default;
  BaseDataPacketParser &operator=(BaseDataPacketParser &&) = default;
  BaseDataPacketParser(BaseDataPacketParser &&) = default;
  virtual ~BaseDataPacketParser() = default;

// Deleted
public:
  BaseDataPacketParser(const BaseDataPacketParser &) = delete;
  BaseDataPacketParser &operator=(const BaseDataPacketParser &) = delete;

// Methods
public:
  PPFnStatus ConsumeData(BytesParser &bp);
  PPFnStatus FlushData();
  /// @brief Set the handler to call when the first data packet is received as
  ///        to set the reference start time for all the other handlers
  /// @param f The handler
  /// @return Always PPFnStatus::success
  PPFnStatus SetStartTimeSetHandler(SetTimeHandler f)
  {
    m_timeSetHandler = std::move(f);
    return PPFnStatus::success;
  };
  /// @brief Set the start time from which to offset the time of the data 
  ///        packets received
  /// @param startTime The start time from which to offset
  /// @return Always PPFnStatus::success
  PPFnStatus SetStartTime(std::uint64_t startTime)
  {
    m_startTime = startTime;
    m_isSetStartTime = true;
    return PPFnStatus::success;
  }
  PPFnStatus SetStartTimeForAllHandlers(std::uint64_t startTime)
  {
    assert(m_timeSetHandler != nullptr);
    if (!m_isSetStartTime)
    {
      m_timeSetHandler(startTime);
      OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Called time set handler, time: %u", startTime);

    }
    return PPFnStatus::success;
  }

// Methods
private:
  virtual PPFnStatus ConsumeDataImpl(BytesParser &bp) = 0;
  virtual PPFnStatus FlushDataImpl() = 0;

// Attributes
protected:
  SetTimeHandler m_timeSetHandler = nullptr;
  std::uint64_t m_startTime = 0U;
  bool m_isSetStartTime = false;

}; // class BaseDataPacketParser

// Type aliases
using BDPUPtr = std::unique_ptr<BaseDataPacketParser>;

} // namespace lpgpu2

// Macro used to quickly read some bytes from the BytesParser
// and perform error checking
#define READ_BYTES_AND_CHECK_NO_SWAP(var)  \
{                                          \
  auto rc = bp.ReadBytes(var);             \
  assert(rc == PPFnStatus::success);       \
  if (rc == PPFnStatus::failure)           \
  {                                        \
    return PPFnStatus::failure;            \
  }                                        \
}

// Macro used to quickly read some bytes, specifically time data, from the
// BytesParser and perform error checking. It offsets the time read by a
// base time so that the time read is with respect to the first time received
#define READ_TIME(newTime)                         \
{                                                  \
  READ_BYTES_AND_CHECK_NO_SWAP(newTime);           \
  auto rcst = SetStartTimeForAllHandlers(newTime); \
  assert(rcst == PPFnStatus::success);             \
  if (rcst == PPFnStatus::failure)                 \
  {                                                \
    return PPFnStatus::failure;                    \
  }                                                \
  newTime -= m_startTime;                          \
}
// clang-format on

#endif // LPGPU2_DATAPACKETPARSER_H_INCLUDE
