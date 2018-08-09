// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_StackDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles STACK (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_StackDataPacketParser.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfilingMidTier/include/ppLPGPU2TargetCharacteristics.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Database
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>

// STL
// Introduces:
// std::move
// assert
#include <utility>
#include <cassert>

namespace lpgpu2 {

/// @brief Ctor; no work is done here.
/// @param pDataAdapter Handle to the database connection object
StackDataPacketParser::StackDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter}
{
  GT_ASSERT(m_pDataAdapter);
}

/// @brief Consume the data contained in bp, write data to BD
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus StackDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  gtUInt64 apiID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(apiID);

  gtUInt64 id = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(id);

  gtUInt64 frameNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(frameNumber);

  gtUInt64 drawNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(drawNumber);

  gtUInt64 startTime = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(startTime);

  gtUInt64 recordCount = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(recordCount);

  for (auto i = 0U; i < recordCount; ++i)
  {
    gtUInt64 sharedObjectBaseAddress = 0U;
    READ_BYTES_AND_CHECK_NO_SWAP(sharedObjectBaseAddress);

    gtUInt64 symbolAddress = 0U;
    READ_BYTES_AND_CHECK_NO_SWAP(symbolAddress);

    gtUInt64 sharedObjectNameStringLength = 0U;
    READ_BYTES_AND_CHECK_NO_SWAP(sharedObjectNameStringLength);

    gtUInt64 symbolNameStringLength = 0U;
    READ_BYTES_AND_CHECK_NO_SWAP(symbolNameStringLength);

    gtString sharedObjectPathStr;
    sharedObjectPathStr.fromASCIIString(
        static_cast<const char *>(&bp.data[bp.off]),
        static_cast<int>(sharedObjectNameStringLength));
    bp.off += static_cast<size_t>(sharedObjectNameStringLength);

    gtString symbolNameStr;
    symbolNameStr.fromASCIIString(
        static_cast<const char *>(&bp.data[bp.off]),
        static_cast<int>(symbolNameStringLength));
    bp.off += static_cast<size_t>(symbolNameStringLength);

    db::StackTraceInfo stackTrace {
      frameNumber,
      drawNumber,
      sharedObjectBaseAddress,
      std::move(sharedObjectPathStr),
      symbolAddress,
      std::move(symbolNameStr)
    };

    auto rcDb = m_pDataAdapter->InsertStackTrace(stackTrace);
    if (!rcDb)
    {
      return PPFnStatus::failure;
    }
  }

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus StackDataPacketParser::FlushDataImpl()
{
// TODO Investigate why this method locks the DB and stops from calling
// read methods
// TODO Uncomment once above is done
  //auto rcDb = m_pDataAdapter->FlushDb();
  //if (!rcDb)
  //{
    //return PPFnStatus::failure;
  //}

  return PPFnStatus::success;
}

} // namespace lpgpu2
