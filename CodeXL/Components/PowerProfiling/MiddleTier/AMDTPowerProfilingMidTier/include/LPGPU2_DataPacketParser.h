// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Represents a packet parser which uses the different components to
///        parse the data. Each component represents a different parser which
///        acts on a different data packet type.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_DATAPACKETPARSER_H_INCLUDE
#define LPGPU2_DATAPACKETPARSER_H_INCLUDE

// STL
// Introduce:
// std::vector
// stdints
// std::unordered_map
// std::unique_ptr
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>

// Infra
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// clang-format off
// Forward declarations
namespace lpgpu2 {
  class BaseDataPacketParser;
  enum class PPFnStatus;
} // namespace lpgpu2
class amdtProfileDbAdapter;
enum class DataChunkType : uint32_t;

namespace lpgpu2 {

/// @brief    SAMS Android data packets main parser. Aggregates different
///           handlers to handle different types of data packets.
/// @warning  None.
/// @date     19/09/2017.
/// @author   Alberto Taiuti.
class DataPacketParser
{
// Type aliases
public:
  using SingleChunk = std::vector<gtByte>;

// Type aliases
private:
  using BDataPackUP = std::unique_ptr<BaseDataPacketParser>;
  using HandlerTypeMap = std::unordered_map<
                            DataChunkType,
                            BDataPackUP,
                            enum_hash>;

// Rule of 5
// Default
public:
  DataPacketParser() = default;
  ~DataPacketParser() = default;

// Deleted
public:
  DataPacketParser(const DataPacketParser &) = delete;
  DataPacketParser &operator=(const DataPacketParser &) = delete;
  DataPacketParser(DataPacketParser &&) = delete;
  DataPacketParser &operator=(DataPacketParser &&) = delete;

// Methods
public:
  PPFnStatus ConsumeData(const SingleChunk &chunkData);
  PPFnStatus ConsumeData(const SingleChunk &chunkData,
      bool &isDataFlushed);
  PPFnStatus FlushData();
  PPFnStatus AddHandler(DataChunkType t, BDataPackUP p);
  void SetStartTime(gtUInt64 startTime);

// Attributes
private:
  HandlerTypeMap m_handlerTypeMap;
  bool m_setStartTime = false;

}; // class DataPacketParser

// Used to make life eaiser to clients so that they don't have
// to type the full name of the class
using DPUptr = std::unique_ptr<DataPacketParser>;

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_DATAPACKETPARSER_H_INCLUDE
