// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_AnnotationsDataDataPacketParser.cpp
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles ANNOTATIONS(Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local
#include <AMDTRemoteClient/Include/LPGPU2_AndroidPacketGlobals.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_AnnotationsDataPacketParser.h>
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
AnnotationsDataPacketParser::AnnotationsDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter)
: BaseDataPacketParser{}, m_pDataAdapter{pDataAdapter}
{
}

/// @brief Consume the data contained in bp, write data to BD, call
///        visualisation callbacks.
/// @param bp Handle to the bytes parser
/// @return PPFnStatus::Success if successful, PPFnStatus::Failure if unsuccessful
PPFnStatus AnnotationsDataPacketParser::ConsumeDataImpl(
    BytesParser &bp)
{
  db::Annotation decodedData;
  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_annotationId);

  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_frameNum);

  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_drawNum);

  READ_BYTES_AND_CHECK_NO_SWAP(decodedData.m_annotationType);

  READ_TIME(decodedData.m_cpuStartTime);

  READ_TIME(decodedData.m_cpuEndTime);

  // The length of the string to retrieve
  gtUInt64 strLen = 0;
  READ_BYTES_AND_CHECK_NO_SWAP(strLen);

  decodedData.m_text.fromASCIIString(
      static_cast<const char *>(&bp.data[bp.off]),
      static_cast<int>(strLen));
  bp.off += static_cast<size_t>(strLen);

  auto rcDb = m_pDataAdapter->InsertAnnotation(decodedData);
  if (!rcDb)
  {
    return PPFnStatus::failure;
  }

  return PPFnStatus::success;
}

/// @brief Flush the collected data to the DB
/// @return PPFnStatus::Success if successful, PPFnStatus::Failure if unsuccessful
PPFnStatus AnnotationsDataPacketParser::FlushDataImpl()
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
