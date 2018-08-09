// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMFunctionCovMap implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionCovMap.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor. No work done here.
CCVGFWLLVMFunctionCovMap::CCVGFWLLVMFunctionCovMap(gtUInt64 vHash, gtUInt64 vStructHash)
: m_funcHash(vHash)
, m_structHash(vStructHash)
{
}

/// @brief Class destructor. Explicitly erases the contents of containers in *this object.
CCVGFWLLVMFunctionCovMap::~CCVGFWLLVMFunctionCovMap()
{
  m_vecFileNameIDs.clear();
  m_vecLLVMMappings.clear();
}

/// @brief Retrieve the hash of the represented function
/// @return gtUInt64 The hash of the function represented by this cov map's name
gtUInt64 CCVGFWLLVMFunctionCovMap::GetFuncHash() const
{
  return m_funcHash;
}

/// @brief Retrieve the structural hash of the represented function
/// @return gtUInt64 The structural hash of the function represented by this cov map.
gtUInt64 CCVGFWLLVMFunctionCovMap::GetStructHash() const
{
  return m_structHash;
}

// @brief Retrieve the structural hash of the represented function
// @return The structural hash of the function represented by this cov map.
gtSize_t CCVGFWLLVMFunctionCovMap::GetFileNameIDCount() const
{
  return m_vecFileNameIDs.size();
}

/// @brief Retrieve the number of mappings regions contained in this function 
///        coverage map.
/// @return gtSize_t The size of the mapping vector.
gtSize_t CCVGFWLLVMFunctionCovMap::GetMappingCount() const
{
  return m_vecLLVMMappings.size();
}

/// @brief Retrieve a constant reference to the vector of Filename IDs. The Filename IDs
///        map back to the coverage mapping's list of filenames.
/// @return Reference to the vector of filename IDs used by this LLVM function.
const CCVGFWLLVMFunctionCovMap::VecFilenameID_t& CCVGFWLLVMFunctionCovMap::GetVecFileNameIDs() const
{
  return m_vecFileNameIDs;
}

/// @brief Retrieve a constant reference to the vector of LLVM Mapping Regions.
/// @return Reference to the vector of mapping regions used by this LLVM function.
const CCVGFWLLVMFunctionCovMap::VecLLVMMapping_t& CCVGFWLLVMFunctionCovMap::GetVecLLVMMappings() const
{
  return m_vecLLVMMappings;
}

/// @brief Insert a filename ID into the vector of filename IDs.
/// @param[in] vFileNameID the filename ID.
/// @returns  status  success = the id was inserted,
///                   failure = an error has occurred when trying to insert the id.
status CCVGFWLLVMFunctionCovMap::AddFileNameID(gtUInt32 vFileNameID)
{
  try
  {
    m_vecFileNameIDs.push_back(vFileNameID);
  }
  catch (...)
  {
    gtString errMsg{ CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMFunctionCovMap_err_FailedPushBack) };
    return ErrorSet(errMsg);
  }
  return success;
}

/// @brief Insert a LLVM Mapping Region into the vector of mappings.
/// @param[in] vMapping The LLVM Mapping region to insert.
/// @returns  status  success = the mapping region was inserted
///                   failure = an error has occurred when trying to insert
///                   the mapping region.
status CCVGFWLLVMFunctionCovMap::AddMapping(const CCVGFWLLVMMapping &vMapping)
{
  try
  {
    m_vecLLVMMappings.push_back(vMapping);
  }
  catch (...)
  {
    gtString errMsg{ CCVGRSRC(kIDS_CCVG_STR_cmn_LLVMFunctionCovMap_err_FailedPushBack) };
    return ErrorSet(errMsg);
  }
  return success;
}

} // namespace fw
} // namespace ccvg
