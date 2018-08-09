// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_GPUTimingDataPacketParser.h
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles GPUTIMING (Power Profiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_GPUTIMINGDATAPACKETPARSER_H_INCLUDE
#define LPGPU2_GPUTIMINGDATAPACKETPARSER_H_INCLUDE

// Local
#include <AMDTPowerProfilingMidTier/include/LPGPU2_BaseDataPacketParser.h>

// Forward declarations
class TargetCharacteristics;
namespace lpgpu2 {
  struct BytesParser;

  namespace db {
    class LPGPU2DatabaseAdapter;
  }
}

// clang-format off
namespace lpgpu2 {

/// @brief    Concrete packet parser class. Parses GPU timing type data
///           packets.
/// @warning  None.
/// @date     08/11/2017.
/// @author   Alberto Taiuti.
class GPUTimingDataPacketParser final : public BaseDataPacketParser
{
// Methods
public:
  GPUTimingDataPacketParser(db::LPGPU2DatabaseAdapter *pDataAdapter);

// Rule of 5
// Deleted
public:
  GPUTimingDataPacketParser() = delete;
  GPUTimingDataPacketParser(const GPUTimingDataPacketParser &) = delete;
  GPUTimingDataPacketParser &operator=(
      const GPUTimingDataPacketParser &) = delete;

// Default
public:
  GPUTimingDataPacketParser(GPUTimingDataPacketParser &&) = default;
  GPUTimingDataPacketParser &operator=(GPUTimingDataPacketParser &&) = default;
  ~GPUTimingDataPacketParser() = default;

// Overridden
private:
  PPFnStatus ConsumeDataImpl(BytesParser &bp) override;
  PPFnStatus FlushDataImpl() override;

// Attributes
private:
  db::LPGPU2DatabaseAdapter *m_pDataAdapter = nullptr;

}; // class GPUTimingDataPacketParser

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_GPUTIMINGDATAPACKETPARSER_H_INCLUDE
