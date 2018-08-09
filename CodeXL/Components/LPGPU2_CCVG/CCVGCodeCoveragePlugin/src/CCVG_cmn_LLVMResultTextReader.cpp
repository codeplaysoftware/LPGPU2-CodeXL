// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMResultTextReader implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Std:
#include <string>
#include <sstream>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMResultTextReader.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>


namespace ccvg {
namespace fw {

/// @brief Class constructor. No work done here.
CCVGFWLLVMResultTextReader::CCVGFWLLVMResultTextReader() = default;

/// @brief Class destructor. Explicitly erases the contents of containers in *this object.
CCVGFWLLVMResultTextReader::~CCVGFWLLVMResultTextReader()
{
  m_mapFuncResults.clear();
}

/// @brief Retrieve the map of function hashes to function counter results
const CCVGFWLLVMResultTextReader::MapFuncHashToLLVMResult_t& CCVGFWLLVMResultTextReader::GetMapFuncResults() const
{
  return m_mapFuncResults;
}

/// @brief Read and store the appropriate information
/// @return status  success = the data was parsed from the binary stream,
///                 failure = an error has occurred when parsing the data.
status CCVGFWLLVMResultTextReader::ReadData(const char *vpData, gtSize_t vDataSize)
{
  std::istringstream dataStrStream{ std::string{ vpData, vDataSize } };
  std::vector<std::string> vecResultLines;
  while (dataStrStream)
  {
    std::string line;
    std::getline(dataStrStream, line);
    // Add to vector of lines, ignoring whitespace and comment lines:
    if ((!line.empty()) && (line[0] != '\0') && (line[0] != ms_constProfDataComment))
    {
      vecResultLines.push_back(line);
    }
  }

  const gtSize_t numResultLines = vecResultLines.size();
  for (gtSize_t iLineIndex = 0; iLineIndex < numResultLines;)
  {
    // Check that there are enough lines left to form a function record:
    if (numResultLines - iLineIndex < ms_constProfDataMinLinesPerFunction)
    {
      const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile);
      gtString errMsg(pErrMsg);
      return ErrorSet(errMsg);
    }

    gtString funcName;
    gtUInt64 funcStructHash;
    gtUInt64 funcNumCounters;

    funcName.fromASCIIString(vecResultLines[iLineIndex].c_str());
    iLineIndex++;
    try
    {
      funcStructHash = std::stoull(vecResultLines[iLineIndex]);
      iLineIndex++;
      funcNumCounters = std::stoi(vecResultLines[iLineIndex]);
      iLineIndex++;
    }
    catch (...)
    {
      // Malformed results file
      const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile);
      gtString errMsg(pErrMsg);
      return ErrorSet(errMsg);    
    }

    CCVGFWLLVMFunctionResult result{ funcName, funcStructHash };

    // Check if the result has was calculated successfully
    if (result.Initialise() == failure)
    {
      return ErrorSet(gtString{ CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile) });
    }
    
    // Check that there are enough lines left to contain all the counter values:
    if (numResultLines - iLineIndex < funcNumCounters)
    {
      return ErrorSet({ CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile) });
    }

    try
    {
      for (CCVGuint iCounterIndex = 0; iCounterIndex < funcNumCounters; ++iCounterIndex)
      {
        result.AddCounterValue(std::stoi(vecResultLines[iLineIndex]));
        iLineIndex++;
      }

      m_mapFuncResults.insert({ result.GetHash(), result });
    }
    catch (...)
    {
      // Malformed results file
      const wchar_t *pErrMsg = CCVGRSRC(kIDS_CCVG_STR_cmn_CCVGLLVMResultTextReader_err_readCCVGRFile);
      gtString errMsg(pErrMsg);
      return ErrorSet(errMsg);
    }
  }
  
  return success;
}


} // namespace fw
} // namespace ccvg
