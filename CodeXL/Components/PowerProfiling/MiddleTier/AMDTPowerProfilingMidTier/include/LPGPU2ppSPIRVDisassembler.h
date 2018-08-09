// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppSPIRVDisassembler.h
///
/// @brief Utility to disassemble SPIR-V binary, using the SPIRV-Tools library
///
/// LPGPU2ppSPIRVDisassembler declarations.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPSPIRVDISASSEMBLER
#define __LPGPU2PPSPIRVDISASSEMBLER

// Std
#include <string>
#include <vector>

// Infra
#include <AMDTBaseTools/Include/gtString.h>

namespace lpgpu2 {
  // Forward declaration
  enum class PPFnStatus; // This is used as the return status

  PPFnStatus SPIRVDisassemble(const std::vector<uint32_t>& vecBinary, gtString& disOut);
  PPFnStatus SPIRVDisassemble(const std::vector<uint8_t>& vecBinary, gtString& disOut);

} // namespace lpgpu2


#endif // # __LPGPU2PPSPIRVDISASSEMBLER
