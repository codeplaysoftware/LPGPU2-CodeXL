// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdBaseWithParams implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBaseWithParams.h>

namespace ccvg {
namespace cmd {

/// @brief Class constructor.
CCVGCmdBaseWithParams::CCVGCmdBaseWithParams()
: m_cmdParams(m_listCmdParameters)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdBaseWithParams::~CCVGCmdBaseWithParams()
{
}

/// @brief  Retrieve whether *this command has parameters to be filled in by the
///         client.
/// @return bool True = Yes command has parameters, false = no parameters.
bool CCVGCmdBaseWithParams::GetHasParams() const
{
  return true;
}

/// @brief  Retrieve *this command's parameters container to be filled in by the
///         client.
/// @return CCVGCmdParams& Reference to container.
CCVGCmdParams& CCVGCmdBaseWithParams::GetParams()
{
  return m_cmdParams;
}

} // namespace ccvg 
} // namespace cmd 