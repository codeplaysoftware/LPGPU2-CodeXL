// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppSPIRVDisassembler.cpp
///
/// @brief Utility functions to disassemble SPIR-V binary, using the SPIRV-Tools library
///
/// LPGPU2ppSPIRVDisassembler definitions.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include <AMDTPowerProfilingMidTier/include/LPGPU2ppSPIRVDisassembler.h>

// Local
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

// External
#include <spirv-tools/libspirv.hpp>

using lpgpu2::PPFnStatus;

/// @brief Enum of which SPIR-V environment to use for disassembly:
static constexpr auto gs_spirEnv   = SPV_ENV_UNIVERSAL_1_2;

/// @brief Flags to pass to the disassemble. This value enables indentation:
static constexpr auto gs_spirFlags = spvtools::SpirvTools::kDefaultDisassembleOption | SPV_BINARY_TO_TEXT_OPTION_INDENT;

/// @brief Disassemble SPIR-V binary from a vector of raw words to a human-readable string.
/// @param vecBinary Vector of binary data to disassemble.
/// @param disOut    String to write the disassembly to.
PPFnStatus lpgpu2::SPIRVDisassemble(const std::vector<uint32_t>& vecBinary, gtString& disOut)
{
  spvtools::SpirvTools tools {gs_spirEnv};
  std::string disassembly;
  const auto bOk = tools.Disassemble(vecBinary, &disassembly, gs_spirFlags);
  
  if (bOk)
  {
    disOut.fromASCIIString(disassembly.c_str());
    return PPFnStatus::success;
  }
  else
  {
    return PPFnStatus::failure;
  }
}

/// @brief Disassemble SPIR-V binary from a vector of bytes to a human-readable string.
/// @param vecBinary Vector of binary data to disassemble in unsigned char form.
/// @param disOut    String to write the disassembly to.
PPFnStatus lpgpu2::SPIRVDisassemble(const std::vector<uint8_t>& vecBinary, gtString& disOut)
{
    spvtools::SpirvTools tools {gs_spirEnv};
    std::string disassembly;

  // The SPIR-V disassemble expects an array of uint32_t, as long as we do some sanity checking we
  // can safely reinterpret the uint8_t data and pass it in. The size must be a multiple of 4 bytes:
  if (vecBinary.size() % 4 != 0)
  {
    return PPFnStatus::failure;
  }

  const auto pData = reinterpret_cast<const uint32_t*>(vecBinary.data());
  const auto reinterpretedSize = vecBinary.size() / 4;
  const auto bOk = tools.Disassemble(pData, reinterpretedSize, &disassembly, gs_spirFlags);
  
  if (bOk)
  {
    disOut.fromASCIIString(disassembly.c_str());
    return PPFnStatus::success;
  }
  else
  {
    return PPFnStatus::failure;
  }
}
