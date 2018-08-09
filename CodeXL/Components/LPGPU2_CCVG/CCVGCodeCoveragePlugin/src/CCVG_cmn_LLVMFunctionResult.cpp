// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWLLVMFunctionResult implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Std:
#include <string>
#include <sstream>

// Qt: (for MD5 hashing function names to look up in LLVM coverage maps)
#include <QCryptographicHash>
#include <QBuffer>
 
// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmn_LLVMFunctionResult.h>

#include <QDebug>

namespace ccvg {
namespace fw {

/// @brief Class constructor. No work is performed here.
/// @param[in] vName Name of the function on the result file.
/// @param[in] vStructHash The struct hash as read from the result file.
CCVGFWLLVMFunctionResult::CCVGFWLLVMFunctionResult(gtString vName, gtUInt64 vStructHash)
: m_name(vName)
, m_hash(0)
, m_structHash(vStructHash)
{  
}

/// @brief Class destructor. Explicitly erases the contents of containers in *this object.
CCVGFWLLVMFunctionResult::~CCVGFWLLVMFunctionResult()
{
  m_vecCounterValues.clear();
}

/// @brief Sets member values and calculates function name hash.
/// @returns  status  success = the hash was successfully calculated,
///                   failure = an error has occurred and the hash is
///                             not valid.
status CCVGFWLLVMFunctionResult::Initialise()
{
  // Generate function hash from name:
  // The hash used by LLVM is the lower 8 bytes of the MD5 hash
  QCryptographicHash md5Hash(QCryptographicHash::Md5);
  md5Hash.addData(m_name.asASCIICharArray(), m_name.length());
  auto hashLowerData = md5Hash.result();
  hashLowerData.truncate(ms_constHashLowerBytes);
  
  status bOk = success;
  gtUInt64 hashNumeric = hashLowerData.toHex().toULongLong(&bOk, 16);

  m_hash = bOk ? hashNumeric : 0;

  return bOk;
}

/// @brief Add a new counter value to the counter vector (value should be added
///        sequentially).
/// @param[in] vValue The counter value.

status CCVGFWLLVMFunctionResult::AddCounterValue(gtUInt32 vValue)
{
  try
  {
    m_vecCounterValues.push_back(vValue);
  }
  catch (...)
  {
    return failure;
  }

  m_counterCount++;
  if (vValue > 0)
  {
    m_nonZeroCounterCount++;
  }
  
  return success;
}

// @brief Get the value of the counter at the given index.
// @return  The retrieved counter value, or 0 if index extends past vector size.
gtUInt32 CCVGFWLLVMFunctionResult::GetCounterValue(gtUInt32 vIndex) const
{
  if (vIndex >= m_vecCounterValues.size())
  {
    return 0;
  }
  
  return m_vecCounterValues.at(vIndex);
}

// @brief Get the hash of the function's name.
// @return  The retrieved hash value as calculated during construction.
gtUInt64 CCVGFWLLVMFunctionResult::GetHash() const
{
  return m_hash;
}

// @brief Get the function's name.
// @return  The retrieved name of the function represented by this result.
gtString CCVGFWLLVMFunctionResult::GetName() const
{
  return m_name;
}

// @brief Get the number of counters in this function
// @return  The number of counters.
gtUInt32 CCVGFWLLVMFunctionResult::GetCounterCount() const
{
  return m_counterCount;
}

// @brief Get the number of counter regions in this function that were executed
//        at least once.
// @return  The number of executed counters
gtUInt32 CCVGFWLLVMFunctionResult::GetNonZeroCounterCount() const
{
  return m_nonZeroCounterCount;
}

} // namespace fw
} // namespace ccvg
