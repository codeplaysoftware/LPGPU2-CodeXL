// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_StackDataPacketParser.h
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles STACK (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_STACKDATAPACKETPARSER_H_INCLUDE
#define LPGPU2_STACKDATAPACKETPARSER_H_INCLUDE

// Local
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>
#include <AMDTPowerProfilingMidTier/include/LPGPU2_BaseDataPacketParser.h>

// Forward declarations
namespace lpgpu2 {
  struct BytesParser;

  namespace db {
    class LPGPU2DatabaseAdapter;
  }
}

// clang-format off
namespace lpgpu2 {

/// @brief    Concrete packet parser class. Parses Stack type data
///           packets.
/// @warning  None.
/// @date     15/10/2018.
/// @author   Alberto Taiuti.
class StackDataPacketParser final : public BaseDataPacketParser
{
// Methods
public:
  StackDataPacketParser(db::LPGPU2DatabaseAdapter *pDataAdapter);

// Rule of 5
// Deleted
public:
  StackDataPacketParser() = delete;
  StackDataPacketParser(const StackDataPacketParser &) = delete;
  StackDataPacketParser &operator=(const StackDataPacketParser &) = delete;

// Default
public:
  StackDataPacketParser(StackDataPacketParser &&) = default;
  StackDataPacketParser &operator=(StackDataPacketParser &&) = default;
  ~StackDataPacketParser() = default;

// Overridden
private:
  PPFnStatus ConsumeDataImpl(BytesParser &bp) override;
  PPFnStatus FlushDataImpl() override;

// Attributes
private:
  db::LPGPU2DatabaseAdapter *m_pDataAdapter = nullptr;

}; // class StackDataPacketParser

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_STACKDATAPACKETPARSER_H_INCLUDE
