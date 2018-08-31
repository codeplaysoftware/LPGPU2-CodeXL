// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_TraceDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles TRACE (Power Profiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_TraceDataPacketParser.h>
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
TraceDataPacketParser::TraceDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter,
    const TargetCharacteristics *pTargetCharacteristics)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter},
  m_pTargetCharacteristics{pTargetCharacteristics}
{
  GT_ASSERT(m_pTargetCharacteristics);
  GT_ASSERT(m_pDataAdapter);
}

/// @brief Consume the data contained in bp, write data to BD, call
///        visualisation callbacks.
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::Success if successful, PPFnStatus::Failure if unsuccessful
PPFnStatus TraceDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  // Command ID (API call ID)
  gtUInt64 cmdID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(cmdID);

  // Start time
  gtUInt64 startTime = 0U;
  READ_TIME(startTime);

  // End time
  gtUInt64 endTime = 0U;
  READ_TIME(endTime);

  // Frame number
  gtUInt64 frameNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(frameNumber);

  // Draw number
  gtUInt64 drawNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(drawNumber);

  // API ID
  gtUInt64 apiID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(apiID);

  // Offset
  gtUInt64 offset = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(offset);

  // Length
  gtUInt64 length = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(length);

  // Look up the command info in the TargetCharacteristics LUT tables
  //
  // We can safely use [0] here because even though the Commands entry
  // is treated as an array in the TargetCharacteristics file, there will
  // always be only one entry
  const auto &commands = m_pTargetCharacteristics->GetCommands()[0];
  // No need to check as the API calls made by the app are necessarily
  // the ones available and listed in the TargetCharacteristics.xml
  const auto apiInfo = commands.apisLUT.find(apiID);
  if (apiInfo != commands.apisLUT.end())
  {
	  assert(apiInfo != commands.apisLUT.end());
	  const auto cmdInfoPtr = apiInfo->second.cmdsLUT.find(cmdID);

	  if (cmdInfoPtr != apiInfo->second.cmdsLUT.end())
	  {
		  assert(cmdInfoPtr != apiInfo->second.cmdsLUT.end());
		  const auto &cmdInfo = cmdInfoPtr->second;

		  // Save the data in the database
		  db::ProfileTraceInfo trace = {
			  cmdID,
			  cmdInfo.name,
			  startTime,
			  endTime,
			  frameNumber,
			  drawNumber,
			  apiID,
			  offset,
			  length };

		  auto rcDb = m_pDataAdapter->InsertProfileTraceInfo(trace);
		  if (!rcDb)
		  {
			  return PPFnStatus::failure;
		  }
	  }	  
  }   

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::Success if successful, PPFnStatus::Failure if unsuccessful
PPFnStatus TraceDataPacketParser::FlushDataImpl()
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
