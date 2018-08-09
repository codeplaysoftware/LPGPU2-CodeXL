// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_GPUTimingDataDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles GPUTIMING(Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_GPUTimingDataPacketParser.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseAdapter.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// STL
// Introduces:
// std::move
// assert
#include <utility>
#include <cassert>

namespace lpgpu2 {

/// @brief Ctor; no work is done here
/// @param pDataAdapter Handle to the database connection object
GPUTimingDataPacketParser::GPUTimingDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter}
{
}

/// @brief Consume the data contained in bp, write data to BD, call
///        visualisation callbacks.
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::Success if successful, PPFnStatus::Failure if unsuccessful
PPFnStatus GPUTimingDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  db::GPUTimer decodedData;
  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_type);

  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_frameNum);

  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_drawNum);

  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_time);

  auto rcDb = m_pDataAdapter->InsertGPUTimer(decodedData);
  if (!rcDb)
  {
    return PPFnStatus::failure;
  }

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::Success if successful, PPFnStatus::Failure if unsuccessful
PPFnStatus GPUTimingDataPacketParser::FlushDataImpl()
{
// TODO Investigate why this method locks the DB and stops from calling
// read methods
// TODO Uncomment once above is done
  //auto rcDb = m_pDataAdapter->FlushDb();
  //if (!rcDb)
  //{
    //return PPFnStatus::Failure;
  //}

  return PPFnStatus::success;
}

} // namespace lpgpu2
