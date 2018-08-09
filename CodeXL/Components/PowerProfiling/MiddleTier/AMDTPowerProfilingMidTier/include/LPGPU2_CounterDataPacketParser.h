// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_CounterDataPacketParser.h
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles COUNTER (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_COUNTERDATAPACKETPARSER_H_INCLUDE
#define LPGPU2_COUNTERDATAPACKETPARSER_H_INCLUDE

// Local
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_BaseDataPacketParser.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>

// Forward declarations
class PowerProfilerCore;
namespace lpgpu2 {
  namespace db {
    class LPGPU2DatabaseAdapter;
  }
}

// clang-format off
namespace lpgpu2 {

/// @brief    Concrete packet parser class. Parses Counter type data packets.
/// @warning  None.
/// @date     19/09/2017.
/// @author   Alberto Taiuti.
class CounterDataPacketParser final : public BaseDataPacketParser
{
// Methods
public:
  CounterDataPacketParser(db::LPGPU2DatabaseAdapter *pDataAdapter,
      PPSamplesDataHandlerFunc dataHandler,
      void *pDataHandlerParams,
      PowerProfilerCore* pPPCore,
      gtUInt32 blobSize = 0U);

// Rule of 5
// Deleted
public:
  CounterDataPacketParser() = delete;
  CounterDataPacketParser(const CounterDataPacketParser &) = delete;
  CounterDataPacketParser &operator=(const CounterDataPacketParser &) = delete;

// Default
public:
  CounterDataPacketParser(CounterDataPacketParser &&) = default;
  CounterDataPacketParser &operator=(CounterDataPacketParser &&) = default;
  ~CounterDataPacketParser() = default;

// Overridden
private:
  PPFnStatus ConsumeDataImpl(BytesParser &bp) override;
  PPFnStatus FlushDataImpl() override;

// Type aliases
private:
  using CountersVec = gtVector<int>;
  using SamplesPtrVec = gtVector<AMDTProfileTimelineSample*>;
  using SamplesVec = gtVector<AMDTProfileTimelineSample>;

// Attributes
private:
  lpgpu2::db::LPGPU2DatabaseAdapter *m_pDataAdapter = nullptr;
  PPSamplesDataHandlerFunc m_dataHandler;
  void *m_pDataHandlerParams = nullptr;
  PowerProfilerCore* m_pPPcore = nullptr;
  CountersVec m_enabledCounters;
  SamplesVec m_samplesBuffer;
  gtUInt32 m_blobSize = 0U;

}; // class CounterDataPacketParser

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_DATAPACKETPARSER_H_INCLUDE
