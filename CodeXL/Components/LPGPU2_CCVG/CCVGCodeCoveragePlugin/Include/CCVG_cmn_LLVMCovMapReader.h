// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMCovMapReader class.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_CMN_LLVMCOVMAPREADER_H_INCLUDE
#define LPGPU2_CCVG_CMN_LLVMCOVMAPREADER_H_INCLUDE

// Std:
#include <vector>
#include <map>
#include <sstream>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVGClassCmnBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGDataTypes.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionCovMap.h>

namespace ccvg {
namespace fw {

/// @brief    CCVGFWLLVMCovMapReader is a class used to process information
///           from an array of bytes extracted from an LLVM Coverage mapping
///           segment in an executable.
/// @see      CCVGFWCCVGItemDataExecuteable, CCVGFWLLVMFunctionCovMap
/// @date     04/09/2017.
/// @author   Callum Fare.
// clang-format off
class CCVGFWLLVMCovMapReader final
: public CCVGClassCmnBase
{
// Typedefs:
using MapFuncHashToCoverageMap_t    = std::map<gtUInt64, CCVGFWLLVMFunctionCovMap>;
using PairFuncHashAndCoverageMap_t  = std::pair<gtUInt64, CCVGFWLLVMFunctionCovMap>;
using VecFilename_t                 = std::vector<gtString>;

// Methods:
public:
  CCVGFWLLVMCovMapReader();
  status                              ReadHeader(const unsigned char *vpData);
  const MapFuncHashToCoverageMap_t&   GetMapFuncHashToCoverageMap() const;
  const std::wstringstream&           GetInfoStream() const;

// Overridden:
public:
  virtual ~CCVGFWLLVMCovMapReader() override; // From CCVGClassCmnBase

// Constants:
public:
  // Header constants
  static constexpr gtSize_t ms_constHeaderFuncCountOffset           = 0;
  static constexpr gtSize_t ms_constHeaderFileNameDataLengthOffset  = 4;
  static constexpr gtSize_t ms_constHeaderCovMapDataLengthOffset    = 8;
  static constexpr gtSize_t ms_constHeaderVersionOffset             = 12;
  static constexpr gtSize_t ms_constHeaderFuncRecordsOffset         = 16;
  // Function record constants
  static constexpr gtSize_t ms_constFuncRecordLength                = 20;
  static constexpr gtSize_t ms_constFuncRecordHashOffset            = 0;
  static constexpr gtSize_t ms_constFuncRecordDataLengthOffset      = 8;
  static constexpr gtSize_t ms_constFuncRecordStructHashOffset      = 12;
  // Mapping region constants
  static constexpr CCVGuint ms_constMapRegionCounterTagMask         = 0x3;
  static constexpr CCVGuint ms_constMapRegionCounterTagBits         = 2;
  static constexpr CCVGuint ms_constMapRegionCounterExpTagMask      = 0x1;
  static constexpr CCVGuint ms_constMapRegionCounterExpTagBits      = 1;
  static constexpr CCVGuint ms_constMapRegionCounterTagZero         = 0;
  static constexpr CCVGuint ms_constMapRegionCounterTagCounter      = 1;
  static constexpr CCVGuint ms_constMapRegionCounterTagSub          = 2;
  static constexpr CCVGuint ms_constMapRegionCounterTagAdd          = 3;

// Attributes:
protected:
  const unsigned char*          m_pData;
  const unsigned char*          m_pFuncRecData;
  const unsigned char*          m_pEncodedData;
  const unsigned char*          m_pCovMapData;
  CCVGuint                      m_funcCount;
  CCVGuint                      m_fileNameDataLength;
  CCVGuint                      m_covMapDataLength;
  std::wstringstream            m_infoStream;
  VecFilename_t                 m_vecFileNames;
  MapFuncHashToCoverageMap_t    m_mapFuncHashToCoverageMap;

// Methods:
private:
  gtUInt64 DecodeLEB128(const unsigned char *vpLEB128, gtSize_t *vpSize);
  gtUInt64 DecodeUInt64(const unsigned char *vpUInt64);
  gtUInt32 DecodeUInt32(const unsigned char *vpUInt32);
  gtString DecodeCovMapString(const unsigned char *vpLEB128);
  status   ReadFuncCovMaps(CCVGuint vFuncCount);
    
};
// clang-format on

} // namespace fw
} // namespace ccvg

#endif // LPGPU2_CCVG_CMN_LLVMCOVMAPREADER_H_INCLUDE