// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2_ShaderMetaDataPacketParser.h
///
/// @brief Concrete implementation of the BaseDataPacketParser class.
///        Handles SHADER (Power Prorfiling) data packets.
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_SHADERMETADATAPACKETPARSER_H_INCLUDE
#define LPGPU2_SHADERMETADATAPACKETPARSER_H_INCLUDE

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

/// @brief    Concrete packet parser class. Parses Shaders Meta type data
///           packets.
/// @warning  None.
/// @date     16/02/2018.
/// @author   Alberto Taiuti.
class ShaderMetaDataPacketParser final : public BaseDataPacketParser
{
// Methods
public:
  ShaderMetaDataPacketParser(db::LPGPU2DatabaseAdapter *pDataAdapter);

// Rule of 5
// Deleted
public:
  ShaderMetaDataPacketParser() = delete;
  ShaderMetaDataPacketParser(const ShaderMetaDataPacketParser &) = delete;
  ShaderMetaDataPacketParser &operator=(const ShaderMetaDataPacketParser &) = delete;

// Default
public:
  ShaderMetaDataPacketParser(ShaderMetaDataPacketParser &&) = default;
  ShaderMetaDataPacketParser &operator=(ShaderMetaDataPacketParser &&) = default;
  ~ShaderMetaDataPacketParser() = default;

// Overridden
private:
  PPFnStatus ConsumeDataImpl(BytesParser &bp) override;
  PPFnStatus FlushDataImpl() override;

// Attributes
private:
  db::LPGPU2DatabaseAdapter *m_pDataAdapter = nullptr;

}; // class ShaderMetaDataPacketParser

} // namespace lpgpu2
// clang-format on

#endif // LPGPU2_SHADERMETADATAPACKETPARSER_H_INCLUDE
