// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_ShaderDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles SHADER (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_ShaderDataPacketParser.h>
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
ShaderDataPacketParser::ShaderDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter}
{
  GT_ASSERT(m_pDataAdapter);
}

/// @brief Consume the data contained in bp, write data to BD, call
///        visualisation callbacks.
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus ShaderDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  // Frame number
  gtUInt64 frameNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(frameNumber);

  // Draw number
  gtUInt64 drawNumber = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(drawNumber);

  // Shader ID
  gtUInt64 shaderID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(shaderID);

  // API ID
  gtUInt64 apiID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(apiID);

  // Shader type
  gtUInt64 shaderType = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(shaderType);

  // Start time
  gtUInt64 cpuTime = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(cpuTime);

  // Length of string to retrieve
  gtUInt64 strLen = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(strLen);

  // String
  gtString text;
  text.fromASCIIString(static_cast<const char *>(&bp.data[bp.off]), static_cast<int>(strLen));
  bp.off += static_cast<size_t>(strLen);

  // Save the data in the database
    /*  db::ShaderTrace shaderTrace = {
    frameNumber,
    drawNumber,
    static_cast<db::EShaderTraceAPIId>(apiID),
    shaderType,
    cpuTime,
    text}; */ 

  db::ShaderTrace shaderTrace;  
  shaderTrace.m_shaderId = shaderID;
  shaderTrace.m_apiId = static_cast<db::EShaderTraceAPIId>(apiID);
  shaderTrace.m_frameNum = frameNumber;
  shaderTrace.m_drawNum = drawNumber;
  shaderTrace.m_shaderType = shaderType;
  shaderTrace.m_cpuTime = cpuTime;
  shaderTrace.m_srcCode = text;

  auto rcDb = m_pDataAdapter->UpdateShaderTrace(
      db::EShaderMetadataType::kSourceFromMainShaderDataPacket, shaderTrace);
  if (!rcDb)
  {
    return PPFnStatus::failure;
  }

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus ShaderDataPacketParser::FlushDataImpl()
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
