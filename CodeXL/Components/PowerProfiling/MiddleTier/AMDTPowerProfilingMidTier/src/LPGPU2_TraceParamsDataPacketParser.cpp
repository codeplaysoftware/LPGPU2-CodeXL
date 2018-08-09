// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_TraceParamsDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles PARAMS (Trace) (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_TraceParamsDataPacketParser.h>
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
TraceParamsDataPacketParser::TraceParamsDataPacketParser(
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
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus TraceParamsDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  // Read the header of the data packet
  // Frame number
  // Read the time at which the sample was take
  gtUInt64 frameNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(frameNumber);

  // Draw number
  gtUInt64 drawNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(drawNumber);

  // Thread ID
  gtUInt64 threadID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(threadID);

  // Api ID
  gtUInt32 apiID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(apiID);

  // Command ID
  gtUInt32 cmdID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(cmdID);

  // Error
  gtInt32 error = 0;
  READ_BYTES_AND_CHECK_NO_SWAP(error);

  // Does it have a return
  gtUInt32 hasReturn = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(hasReturn);

  // Look up the command info in the TargetCharacteristics LUT tables
  //
  // We can safely use [0] here because even though the Commands entry
  // is treated as an array in the TargetCharacteristics file, there will
  // always be only one entry
  const auto &commands = m_pTargetCharacteristics->GetCommands()[0];
  // No need to check as the API calls made by the app are necessarily
  // the ones available and listed in the TargetCharacteristics.xml
  const auto apiInfo = commands.apisLUT.find(apiID);
  assert(apiInfo != commands.apisLUT.end());
  const auto cmdInfoPtr = apiInfo->second.cmdsLUT.find(cmdID);
  assert(cmdInfoPtr != apiInfo->second.cmdsLUT.end());
  const auto &cmdInfo = cmdInfoPtr->second;

  // Read the binary return payload
  gtVector<gtUByte> binaryReturn;
  if (hasReturn != 0U)
  {
    binaryReturn.resize(cmdInfo.returnSize);
    memcpy(binaryReturn.data(), &bp.data[bp.off], cmdInfo.returnSize);
    bp.off += cmdInfo.returnSize;
  }

  // Read the parameters in binary form
  gtVector<gtUByte> binaryParams;
  gtSizeType binaryParamsSize = 0;
  gtString paramsSizesAsStr;
  for (const auto &param : cmdInfo.params)
  {
    binaryParamsSize += param.size;
    paramsSizesAsStr.appendUnsignedIntNumber(param.size);
    paramsSizesAsStr.append(L",");
  }
  if (binaryParamsSize > 0)
  {
    binaryParams.resize(binaryParamsSize);
    memcpy(binaryParams.data(), &bp.data[bp.off], binaryParamsSize);
    bp.off += binaryParamsSize;
  }

  // Save the data in the database
  db::StackTraceParameter traceParam = {
    frameNumber,
    drawNumber,
    threadID,
    cmdInfo.typeInfo,
    binaryParams,
    paramsSizesAsStr,
    binaryReturn,
    cmdInfo.returnSize,
    error};
  auto rcDb = m_pDataAdapter->InsertStackTraceParameter(traceParam);
  if (!rcDb)
  {
    return PPFnStatus::failure;
  }

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus TraceParamsDataPacketParser::FlushDataImpl()
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
