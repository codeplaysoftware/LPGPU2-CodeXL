// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Represents a packet parser which uses the different components to
///        parse the data. Each component represents a different parser which
///        acts on a different data packet type.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfilingMidTier/include/LPGPU2_DataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_BaseDataPacketParser.h>
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTRemoteClient/Include/LPGPU2_RemoteClientUtils.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

using lpgpu2::PPFnStatus;

// STL
// Introduce:
// std::move
// assert
#include <utility>
#include <cassert>

namespace lpgpu2 {

/// @brief Data structure representing a data chunk header
/// @note This is static for this module since it does not need to be used
///       outside of it
struct DataChunkHeader
{
  gtUInt32 magic = 0U;
  DataChunkType type = DataChunkType::NONE;
  DataChunkFlags flags = DataChunkFlags::NORMAL;
  gtUInt32 size = 0U;
};

constexpr size_t kDataChunkHeaderSize = sizeof(DataChunkHeader);

/// @brief Parse a data chunk header
/// @param bp The bytes parser which contains the chunk from which to parse the
///        header
/// @param [out] header Data structure which contains the parsed header
/// @return PPFnStatus::success if success, PPFnStatus::failure if failure
static PPFnStatus ParseDataChunkHeader(BytesParser &bp, DataChunkHeader &header)
{
  struct DataChunkHeaderRaw
  {
    gtUInt32 magic = 0U;
    gtUInt32 type = 0U;
    gtUInt32 flags = 0U;
    gtUInt32 size = 0U;
  };

  DataChunkHeaderRaw rawHeader;

  READ_BYTES_AND_CHECK_NO_SWAP(rawHeader.magic);
  READ_BYTES_AND_CHECK_NO_SWAP(rawHeader.type);
  READ_BYTES_AND_CHECK_NO_SWAP(rawHeader.flags);
  READ_BYTES_AND_CHECK_NO_SWAP(rawHeader.size);

  header.magic = rawHeader.magic;
  header.type = static_cast<DataChunkType>(rawHeader.type);
  header.flags = static_cast<DataChunkFlags>(rawHeader.flags);
  header.size = rawHeader.size;

  return PPFnStatus::success;
}

/// @brief Consume the data in chunk and call the correct handlers
/// @param chunk The data packet to parse
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus DataPacketParser::ConsumeData(
    const SingleChunk &chunk)
{
  bool dataFlushed = false;
  return ConsumeData(chunk, dataFlushed);
}

/// @brief Consume the data in chunk and call the correct handlers
/// @param chunk The data packet to parse
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus DataPacketParser::ConsumeData(
    const SingleChunk &chunk, bool &isDataFlushed)
{
  BytesParser bp{ 0, chunk };
  (void)isDataFlushed;

  struct MegaChunkMetadata
  {
    bool isMegaChunk = false;
    size_t megaChunkSize = 0;
  };

  MegaChunkMetadata megaChunkMetaData;

  while (bp.off < bp.data.size())
  {
    DataChunkHeader chunkHeader;
    if (ParseDataChunkHeader(bp, chunkHeader)
        != PPFnStatus::success)
    {
      return PPFnStatus::failure;
    }

    if (chunkHeader.type == DataChunkType::DATA_STOP)
    {
      isDataFlushed = true;
      return PPFnStatus::success;
    }

    // If the parsed chunk header is the header of a megachunk, simply skip it
    // and start parsing the megachunk's payload, which is the data chunk
    // header of the first data chunk
    if (chunkHeader.type == DataChunkType::MEGACHUNK)
    {
      megaChunkMetaData.isMegaChunk = true;
      megaChunkMetaData.megaChunkSize = chunkHeader.size;

      if (ParseDataChunkHeader(bp, chunkHeader)
          != PPFnStatus::success)
      {
        return PPFnStatus::failure;
      }
    }

    auto val = m_handlerTypeMap.find(chunkHeader.type);
    if (val != m_handlerTypeMap.end())
    {
      if (val->second->ConsumeData(bp) != PPFnStatus::success)
      {
        return PPFnStatus::failure;
      }
    }
    else
    {
      // Advance the bytes parser by the size of the payload of the chunk for
      // which we don't have a parser so that it can parse the next chunks
      bp.off += (chunkHeader.size - kDataChunkHeaderSize);

      OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO,
          L"Unrecognised data chunk packet; type: %u, size: %u. BP offset: %u.",
          chunkHeader.type, chunkHeader.size, bp.off);
    }

    if (bp.off >= megaChunkMetaData.megaChunkSize &&
        megaChunkMetaData.isMegaChunk)
    {
      megaChunkMetaData = MegaChunkMetadata{};
    }
  }

  return PPFnStatus::success;
}

/// @brief Add a handler for a given type of data packet
/// @param t The type of data packet handler p handles
/// @param p The handler. DataPacketParser takes ownership of it.
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus DataPacketParser::AddHandler(DataChunkType t, BDataPackUP p)
{
  try
  {
    m_handlerTypeMap[t] = std::move(p);
    m_handlerTypeMap[t]->SetStartTimeSetHandler([this](gtUInt64 v) {
      SetStartTime(v);
    });
  }
  catch (...)
  {
    return PPFnStatus::failure;
  }

  return PPFnStatus::success;
}

PPFnStatus DataPacketParser::FlushData()
{
  for (auto &p : m_handlerTypeMap)
  {
    auto rc = p.second->FlushData();

    if (rc != PPFnStatus::success)
    {
      return PPFnStatus::failure;
    }
  }

  return PPFnStatus::success;
}

/// @brief Set the start time for all the handlers to offset their timing from.
///        Called by each handler, with the first one setting the time for the
///        others. Subsequent calls are ignored.
/// @param startTime The first timestamp of the first data packet received
void DataPacketParser::SetStartTime(gtUInt64 startTime)
{
  if (!m_setStartTime)
  {
    for (auto &handler : m_handlerTypeMap)
    {
      handler.second->SetStartTime(startTime);
    }

    m_setStartTime = true;
  }
}

} // namespace lpgpu2
