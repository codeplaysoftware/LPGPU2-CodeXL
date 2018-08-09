// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_TraceParamsDataPacketParser.h
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles PARAMS (Trace) (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_TRACEPARAMSDATAPACKETPARSER_H_INCLUDE
#define LPGPU2_TRACEPARAMSDATAPACKETPARSER_H_INCLUDE

// Local
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
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

/// @brief    Concrete packet parser class. Parses Trace Params type data
///           packets.
/// @warning  None.
/// @date     24/10/2017.
/// @author   Alberto Taiuti.
class TraceParamsDataPacketParser final : public BaseDataPacketParser
{
// Methods
public:
  TraceParamsDataPacketParser(
    db::LPGPU2DatabaseAdapter *pDataAdapter,
    const TargetCharacteristics *pTargetCharacteristics);

// Rule of 5
// Deleted
public:
  TraceParamsDataPacketParser() = delete;
  TraceParamsDataPacketParser(const TraceParamsDataPacketParser &) = delete;
  TraceParamsDataPacketParser &operator=(const TraceParamsDataPacketParser &) = delete;

// Default
public:
  TraceParamsDataPacketParser(TraceParamsDataPacketParser &&) = default;
  TraceParamsDataPacketParser &operator=(TraceParamsDataPacketParser &&) = default;
  ~TraceParamsDataPacketParser() = default;

// Overridden
private:
  PPFnStatus ConsumeDataImpl(BytesParser &bp) override;
  PPFnStatus FlushDataImpl() override;

// Attributes
private:
  db::LPGPU2DatabaseAdapter *m_pDataAdapter = nullptr;
  const TargetCharacteristics *m_pTargetCharacteristics = nullptr;

}; // class TraceParamsDataPacketParser

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_TRACEPARAMSDATAPACKETPARSER_H_INCLUDE
