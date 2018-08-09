// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMCovMapReader implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMCovMapReader.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMRegionExprType.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. No work done here.
CCVGFWLLVMCovMapReader::CCVGFWLLVMCovMapReader()
  : m_pData(nullptr)
  , m_pCovMapData(nullptr)
  , m_funcCount(0)
  , m_fileNameDataLength(0)
  , m_covMapDataLength(0)
{
}

/// @brief Class destructor. Explicitly erases the contents of containers in *this object.
CCVGFWLLVMCovMapReader::~CCVGFWLLVMCovMapReader()
{
  m_vecFileNames.clear();
  m_mapFuncHashToCoverageMap.clear();
}

/// @brief Retrieve the map of function hashes to function coverage mappings.
/// @returns CCVGFWLLVMCovMapReader::MapFuncHashToCoverageMap_t The maps of function hashes.
const CCVGFWLLVMCovMapReader::MapFuncHashToCoverageMap_t& CCVGFWLLVMCovMapReader::GetMapFuncHashToCoverageMap() const
{
  return m_mapFuncHashToCoverageMap;
}

/// @brief Retrieve the stringstream that information has been written to.
/// @returns wstringstream The stringstream.
const std::wstringstream& CCVGFWLLVMCovMapReader::GetInfoStream() const
{
  return m_infoStream;
}

/// @brief Read and store the appropriate metadata from the header of the 
///        coverage mapping information. As data is processed, human-readable 
///        data is generated to be displayed below the executable's hex dump.
/// @param[in] vpData Pointer to the start of data
/// @return status success = the data was read successfully,
///                failure = an error has occurred when reading the data.
status CCVGFWLLVMCovMapReader::ReadHeader(const unsigned char *vpData)
{
  m_pData = vpData;
  m_funcCount = DecodeUInt32(m_pData + ms_constHeaderFuncCountOffset);
  m_fileNameDataLength = DecodeUInt32(m_pData + ms_constHeaderFileNameDataLengthOffset);
  m_covMapDataLength = DecodeUInt32(m_pData + ms_constHeaderCovMapDataLengthOffset);

  // Define the beginning of the actual encoded data (after the header and
  // function records), and the actual coverage map data (after the filename data)
  m_pFuncRecData = m_pData + ms_constHeaderFuncRecordsOffset;
  m_pEncodedData = m_pFuncRecData + ms_constFuncRecordLength * m_funcCount;
  m_pCovMapData = m_pEncodedData + m_fileNameDataLength;

  // Process the list of filenames covered by this executable:
  gtSize_t sizeLEB, readBytes = 0;
  gtUInt64 fileNameCount = DecodeLEB128(m_pEncodedData, &sizeLEB);
  for (gtUInt32 i = 0; i < fileNameCount; i++)
  {
    m_vecFileNames.push_back(DecodeCovMapString(m_pEncodedData + sizeLEB + readBytes));
    readBytes += m_vecFileNames[i].length();
  }

  ReadFuncCovMaps(m_funcCount);

  return success;
}

/// @brief Read an LEB128 encoded unsigned integer from the stream of bytes of
///        the given size. (Adapted from method in LLVM: llvm/Support/LEB128.h)
/// @param[in] vpLEB128 Pointer to the start of LEB128 data.
/// @param[out] Number of bytes read.
/// @return The value of the data as an unsigned integer - if the value is too
///         big to fit in UInt64 then 0 is returned.
gtUInt64 CCVGFWLLVMCovMapReader::DecodeLEB128(const unsigned char *vpLEB128, gtSize_t *vpSize)
{
  gtUInt64 value = 0;
  unsigned shift = 0;
  const unsigned char *pDataStart = vpLEB128;
  do
  {
    const gtUInt64 slice = *vpLEB128 & 0x7f;
    if (shift >= 64 || slice << shift >> shift != slice)
    {
      // Too big for UInt64_t
      return 0;
    }
    value += gtUInt64(*vpLEB128 & 0x7f) << shift;
    shift += 7;
  } while (*(vpLEB128++) >= 128); // loop while highest order bit set

  *vpSize = static_cast<gtSize_t>(vpLEB128 - pDataStart);
  return value;
}

/// @brief Read an 64 bit unsigned integer from the given array of bytes.
/// @param[in] vpUInt64 Pointer to the start of data
/// @return The value of the data as an unsigned integer - if the value is too
///         big to fit in UInt64 then it will be truncated.
gtUInt64 CCVGFWLLVMCovMapReader::DecodeUInt64(const unsigned char *vpUInt64)
{
  // Integers are most significant byte first, but zero-padded to the right, so
  // first we must determine the number of non-zero bytes by reading right-to-left
  unsigned padBytes = 0;
  const unsigned char *pPadCounter = vpUInt64 + 7; //end of 64 bit block
  while (*pPadCounter == 0 && padBytes < 8)
  {
    padBytes++;
    pPadCounter--;
  }

  // Zero integer
  if (padBytes == 8) return 0;

  // Read up to the start of the padding bytes 
  gtUInt64 value = 0;
  unsigned shift = (7 - padBytes) * 8;
  while (vpUInt64 != pPadCounter)
  {
    value += gtUInt64(*vpUInt64++) << shift;
    shift -= 8;
  }
  value += gtUInt64(*vpUInt64);

  return value;
}

/// @brief Read an 32 bit unsigned integer from the given array of bytes.
/// @param[in] vpUInt32 Pointer to the start of data
/// @return The value of the data as an unsigned integer - if the value is too
///         big to fit in UInt32 then it will be truncated.
gtUInt32 CCVGFWLLVMCovMapReader::DecodeUInt32(const unsigned char *vpUInt32)
{
  // Integers are most significant byte first, but zero-padded to the right, so
  // first we must determine the number of non-zero bytes by reading right-to-left
  unsigned padBytes = 0;
  const unsigned char *pPadCounter = vpUInt32 + 3; //end of 32 bit block
  while (*pPadCounter == 0 && padBytes < 4)
  {
    padBytes++;
    pPadCounter--;
  }

  // Zero integer
  if (padBytes == 4) return 0;

  // Read up to the start of the padding bytes 
  gtUInt32 value = 0;
  unsigned shift = (4 - padBytes) * 8;
  while (vpUInt32 != pPadCounter)
  {
    value += gtUInt32(*vpUInt32++) << shift;
    shift -= 8;
  }
  value += gtUInt32(*vpUInt32);

  return value;
}

/// @brief Read a string as encoded in the LLVM cov map format, getting the
///        length from an unsigned LEB128 value.
/// @param[in] Pointer to the start of LEB128 data
/// @return gtString string read from byte stream.
gtString CCVGFWLLVMCovMapReader::DecodeCovMapString(const unsigned char *vpLEB128)
{
  gtSize_t startOffset;
  const gtSize_t strLength = static_cast<gtSize_t>(DecodeLEB128(vpLEB128, &startOffset));

  std::wstring strData;
  strData.resize(strLength + 1);
  for (gtSize_t i = 0; i < strLength; i++)
  {
    strData[i] = (vpLEB128 + startOffset)[i];
  }
  strData[strLength] = '\0';

  return gtString{ strData.data() };
}

/// @brief Read the specified number of functions from the coverage map data,
///        and populate the   
/// @param[in] The number of functions present in this coverage map record.
/// @return status Success = all ok, failure = see error description 
status CCVGFWLLVMCovMapReader::ReadFuncCovMaps(CCVGuint vFuncCount)
{
  gtSize_t sizeLEB;
  const unsigned char *pCovMapDataStart = m_pCovMapData;

  for (CCVGuint funcNum = 0; funcNum < vFuncCount; funcNum++)
  {
    m_infoStream << "\n\n" << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_FunctionRecord) << funcNum << std::endl;
    // Info from function records
    const unsigned char *pFuncRecord = m_pFuncRecData + (ms_constFuncRecordLength * funcNum);
    const gtUInt64 funcHash = DecodeUInt64(pFuncRecord + ms_constFuncRecordHashOffset);
    const gtUInt32 covMapDataSize = DecodeUInt32(pFuncRecord + ms_constFuncRecordDataLengthOffset);
    const gtUInt64 funcStructHash = DecodeUInt64(pFuncRecord + ms_constFuncRecordStructHashOffset);

    m_infoStream << std::hex << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_FunctionHash) << funcHash << std::dec << std::endl;
    CCVGFWLLVMFunctionCovMap funcCovMap(funcHash, funcStructHash);

    // Info from cov map data
    const unsigned char *pCovMapData = pCovMapDataStart;
    pCovMapDataStart += covMapDataSize;

    const auto fileNameIndexCount = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
    gtVector<gtUInt32> fileNameIndices(fileNameIndexCount);
    for (gtUInt32 i = 0; i < fileNameIndexCount; i++)
    {
      pCovMapData += sizeLEB;
      fileNameIndices[i] = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
      m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenameIndex) << i << ": " << fileNameIndices[i] << std::endl;
      m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenamePointsTo) << m_vecFileNames[fileNameIndices[i]].asASCIICharArray() << std::endl;
      funcCovMap.AddFileNameID(fileNameIndices[i]);
    }

    // Counter expressions
    pCovMapData += sizeLEB;
    const gtUInt32 countExprCount = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
    m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_NumCounterExpressions) << countExprCount << std::endl;
    gtVector<gtUInt32> countExprs(countExprCount * 2);

    for (gtUInt32 i = 0; i < countExprCount * 2; i += 2)
    {
      // Get expression counters
      pCovMapData += sizeLEB;
      countExprs[i] = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));    // LHS
      pCovMapData += sizeLEB;
      countExprs[i + 1] = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));  // RHS

      // Decode expression counters
      //gtUInt32 counterTagLHS = countExprs[i] & ms_constMapRegionCounterTagMask;
      const gtUInt32 counterDataLHS = countExprs[i] >> ms_constMapRegionCounterTagBits;
      //gtUInt32 counterTagRHS = countExprs[i+1] & ms_constMapRegionCounterTagMask;
      const gtUInt32 counterDataRHS = countExprs[i + 1] >> ms_constMapRegionCounterTagBits;

      // TODO support for nested expressions, tag is assumed to be 1 for now

      countExprs[i] = counterDataLHS;
      countExprs[i + 1] = counterDataRHS;
      m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_CounterExpression) << i / 2 << ": " << counterDataLHS << "|" << counterDataRHS << std::endl;
    }

    // Mapping regions --
    // Currently multiple filename results are treated as one single filename with
    // possible macro expansions, as it is assumed for now that there is only one
    // source file.
    for (gtUInt32 i = 0; i < fileNameIndexCount; i++)
    {
      m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_FilenameIndex) << i << std::endl;
      pCovMapData += sizeLEB;
      const auto mapRegionCount = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
      gtUInt32 lastLine = 0;
      for (gtUInt32 j = 0; j < mapRegionCount; j++)
      {
        // Counter value contains data on the type of counter
        pCovMapData += sizeLEB;
        const auto covMapCounter = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
        pCovMapData += sizeLEB;
        const auto startLine = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
        pCovMapData += sizeLEB;
        const auto startCol = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
        pCovMapData += sizeLEB;
        const auto numLines = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));
        pCovMapData += sizeLEB;
        const auto endCol = static_cast<gtUInt32>(DecodeLEB128(pCovMapData, &sizeLEB));

        const gtUInt32 counterTag = covMapCounter & ms_constMapRegionCounterTagMask;
        const gtUInt32 counterData = covMapCounter >> ms_constMapRegionCounterTagBits;

        m_infoStream << counterTag << "," << counterData << " " << lastLine + startLine << ":"
          << startCol << "," << lastLine + startLine + numLines << ":" << endCol << std::endl;

        CCVGLLVMMappingSpan span(lastLine + startLine, startCol, lastLine + startLine + numLines, endCol);

        // Expansion regions (even if they are the same file) use a different file id index.
        // Counters are mapped as normal to positions in the actual source file.
        if (counterTag == ms_constMapRegionCounterTagZero)
        {
          const gtUInt32 counterExpansionTag = counterData & ms_constMapRegionCounterExpTagMask;
          const gtUInt32 counterExpansionData = counterData >> ms_constMapRegionCounterExpTagBits;
          m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_ExpansionTag) << counterExpansionTag << std::endl;
          m_infoStream << CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMCovMapReader_ExpansionFile) << counterExpansionData << std::endl;

          // Currently assuming that the expansion region is in the same file
          if (counterExpansionTag == 1)
          {
            const bool bIsExpansion = true;
            funcCovMap.AddMapping(CCVGFWLLVMMapping(span, counterExpansionData,
                                                    CCVGFWLLVMRegionExprType::kNotExpression, bIsExpansion));
          }
          else
          {
            // Special 'zero count' region
            funcCovMap.AddMapping(CCVGFWLLVMMapping(span, 0));
          }
        }
        else if (counterTag == ms_constMapRegionCounterTagSub) // Subtraction expression region
        {
          const gtUInt32 exprIndex = counterData * 2;
          const bool bIsExpansion = false;
          const gtUInt32 counter = 0;
          funcCovMap.AddMapping(CCVGFWLLVMMapping{ span, counter, CCVGFWLLVMRegionExprType::kSubExpression,
                                                   bIsExpansion, countExprs[exprIndex], countExprs[exprIndex + 1] });
        }
        else if (counterTag == ms_constMapRegionCounterTagAdd) // Addition expression region
        {
          const gtUInt32 exprIndex = counterData * 2;
          const bool bIsExpansion = false;
          const gtUInt32 counter = 0;
          funcCovMap.AddMapping(CCVGFWLLVMMapping{ span, counter, CCVGFWLLVMRegionExprType::kAddExpression,
                                                   bIsExpansion, countExprs[exprIndex], countExprs[exprIndex + 1] });
        }
        else if (counterTag == ms_constMapRegionCounterTagCounter)   // Standard region
        {
          funcCovMap.AddMapping(CCVGFWLLVMMapping{ span, counterData });
        }
        lastLine += startLine;
      }
    }
    m_mapFuncHashToCoverageMap.insert({ funcHash, funcCovMap });
  }

  return success;
}

} // namespace fw
} // namespace ccvg
