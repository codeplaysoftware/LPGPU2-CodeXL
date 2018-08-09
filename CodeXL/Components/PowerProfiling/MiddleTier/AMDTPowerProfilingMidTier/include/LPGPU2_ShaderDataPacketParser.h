// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_ShaderDataPacketParser.h
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles SHADER (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_SHADERDATAPACKETPARSER_H_INCLUDE
#define LPGPU2_SHADERDATAPACKETPARSER_H_INCLUDE

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

/// @brief    Concrete packet parser class. Parses Shaders type data
///           packets.
/// @warning  None.
/// @date     26/10/2017.
/// @author   Alberto Taiuti.
class ShaderDataPacketParser final : public BaseDataPacketParser
{
// Methods
public:
  ShaderDataPacketParser(db::LPGPU2DatabaseAdapter *pDataAdapter);

// Rule of 5
// Deleted
public:
  ShaderDataPacketParser() = delete;
  ShaderDataPacketParser(const ShaderDataPacketParser &) = delete;
  ShaderDataPacketParser &operator=(const ShaderDataPacketParser &) = delete;

// Default
public:
  ShaderDataPacketParser(ShaderDataPacketParser &&) = default;
  ShaderDataPacketParser &operator=(ShaderDataPacketParser &&) = default;
  ~ShaderDataPacketParser() = default;

// Overridden
private:
  PPFnStatus ConsumeDataImpl(BytesParser &bp) override;
  PPFnStatus FlushDataImpl() override;

// Attributes
private:
  db::LPGPU2DatabaseAdapter *m_pDataAdapter = nullptr;

}; // class ShaderDataPacketParser

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_SHADERDATAPACKETPARSER_H_INCLUDE
