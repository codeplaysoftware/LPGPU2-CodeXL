// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_ShaderMetaDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles SHADER (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_ShaderMetaDataPacketParser.h>
#include <AMDTDbAdapter/inc/AMDTProfileDbAdapter.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfilingMidTier/include/ppLPGPU2TargetCharacteristics.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2ppSPIRVDisassembler.h>

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
ShaderMetaDataPacketParser::ShaderMetaDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter}
{
  GT_ASSERT(m_pDataAdapter);
}

/// @brief Consume the data contained in bp, write data to BD, call
///        visualisation callbacks.
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus ShaderMetaDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  gtUInt64 shaderID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(shaderID);

  gtUInt64 apiID = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(apiID);

  gtUInt64 shaderType = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(shaderType);

  gtUInt64 metaDataType = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(metaDataType);

  // Length of data to retrieve
  gtUInt64 dataLen = 0U;
  READ_BYTES_AND_CHECK_NO_SWAP(dataLen);


  db::ShaderTrace shaderTrace;
  shaderTrace.m_shaderId = shaderID;
  shaderTrace.m_apiId = static_cast<db::EShaderTraceAPIId>(apiID);
  shaderTrace.m_shaderType = shaderType;

  auto shouldSetASMFromBinaryDisassembly = false;

  const auto metaDataTypeEnum =
    static_cast<db::EShaderMetadataType>(metaDataType);
  if (metaDataTypeEnum == db::EShaderMetadataType::kSource||
    metaDataTypeEnum == db::EShaderMetadataType::kAsm)
  {
    gtString text;
    text.fromASCIIString(
        static_cast<const char *>(&bp.data[bp.off]), static_cast<int>(dataLen));

    if (metaDataTypeEnum == db::EShaderMetadataType::kSource)
    {
      shaderTrace.m_srcCode = std::move(text);
    }
    else if (metaDataTypeEnum == db::EShaderMetadataType::kAsm)
    {
      shaderTrace.m_asmCode = std::move(text);
    }
  }
  else if (metaDataTypeEnum == db::EShaderMetadataType::kBinary||
    metaDataTypeEnum == db::EShaderMetadataType::kDebug)
  {
    gtVector<gtUByte> bytes(static_cast<size_t>(dataLen));
    memcpy(bytes.data(), &bp.data[bp.off], static_cast<size_t>(dataLen));

    if (metaDataTypeEnum == db::EShaderMetadataType::kBinary)
    {
      shaderTrace.m_binary = std::move(bytes);
      
      // Run disassembler
      gtString shaderDisassembly;
      const auto status = lpgpu2::SPIRVDisassemble(
        shaderTrace.m_binary, shaderDisassembly);

      if (status == PPFnStatus::success)
      {
        shouldSetASMFromBinaryDisassembly = true;
        shaderTrace.m_asmCode = shaderDisassembly;
      }
    }
    else if (metaDataTypeEnum == db::EShaderMetadataType::kDebug)
    {
      shaderTrace.m_debug = std::move(bytes);
    }
  }

  bp.off += static_cast<size_t>(dataLen);
  
  auto rcDb = m_pDataAdapter->UpdateShaderTrace(metaDataTypeEnum,
      shaderTrace);
  if (!rcDb)
  {
    return PPFnStatus::failure;
  }

  // Also add the disassembly if present
  if (shouldSetASMFromBinaryDisassembly)
  {
    rcDb = m_pDataAdapter->UpdateShaderTrace(db::EShaderMetadataType::kAsm,
        shaderTrace);

    if (!rcDb)
    {
      return PPFnStatus::failure;
    }
  }


  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::success if successful, PPFnStatus::failure if unsuccessful
PPFnStatus ShaderMetaDataPacketParser::FlushDataImpl()
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
