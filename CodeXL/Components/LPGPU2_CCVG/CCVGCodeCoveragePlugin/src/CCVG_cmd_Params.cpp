// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdParams implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Params.h>

namespace ccvg {
namespace cmd {

/// @brief Class constructor. 
CCVGCmdParams::CCVGCmdParams()
: m_pListCmdParameters(nullptr)
{
}

/// @brief Class constructor. 
CCVGCmdParams::CCVGCmdParams(ListCmdParam_t &vParameters)
: m_pListCmdParameters(&vParameters)
, m_listIterator(vParameters.begin())
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdParams::~CCVGCmdParams()
{
}

/// @brief Retrieve the first parameter from *this container. Call this 
///        function before calling function GetNext as it resets the internal
///        index to the begining of the container.
/// @result CCVGCmdParameter* A command parameter object, NULL if the container
///         does not have any parameters assigned to it.
CCVGCmdParameter* CCVGCmdParams::GetFirst()
{
  if ((m_pListCmdParameters == nullptr) || IsEmpty())
  {
    return nullptr;
  }

  m_listIterator = m_pListCmdParameters->begin();
  return *m_listIterator;
}

/// @brief Retrieve the next parameter from *this container. Call the  
///        GetFirst() function before calling this function as it resets the
///         internal index to the begining of the container.
/// @result CCVGCmdParameter* A command parameter object, NULL if the container
///         does not have any parameters assigned to it or reached the end of
///         parameters.
CCVGCmdParameter* CCVGCmdParams::GetNext()
{
  if (m_pListCmdParameters == nullptr)
  {
    return nullptr;
  }

  if (++m_listIterator != m_pListCmdParameters->end())
  {
    return *m_listIterator;
  }

  return nullptr;
}

/// @brief Get the number of parameters *this container holds.
/// @return CCVGuint Parameter count.
CCVGuint CCVGCmdParams::GetCount() const
{
  if (m_pListCmdParameters != nullptr)
  {
    return static_cast<CCVGuint>(m_pListCmdParameters->size());
  }

  return 0;
}

/// @brief Determine if *this container is empty of any parameters.
/// @return bool True = yes empty, false = has at least one parameter.
bool CCVGCmdParams::IsEmpty() const
{
  return (GetCount() == 0);
}

/// @brief Determine if *this container is empty of any parameters.
/// @result CCVGCmdParameter* A command parameter object, NULL if not
///         found or there are no parameters.
CCVGCmdParameter* CCVGCmdParams::GetParam(CCVGuint vParamID) const
{
  CCVGCmdParameter *pParam = nullptr;

  if (IsEmpty() || (vParamID >= GetCount()))
  {
    return pParam;
  }

  ListCmdParam_t::const_iterator it = m_pListCmdParameters->cbegin();
  while (it != m_pListCmdParameters->cend())
  {
    CCVGCmdParameter *pParameter = *it;
    if (pParameter->parameterID == vParamID)
    {
      return pParameter;
    }
    ++it;
  }

  return nullptr;
}

} // namespace ccvg 
} // namespace cmd 