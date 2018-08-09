// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGFWTheCurrentStateCurrentPCUser implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Framework:
#include <AMDTOSWrappers/Include/osMachine.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentStateCurrentPCUser.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace fw {

/// @brief Class constructor.
CCVGFWTheCurrentStateCurrentPCUser::CCVGFWTheCurrentStateCurrentPCUser()
{
}

/// @brief Class destructor. Tidy up release resources used by *this instance.
CCVGFWTheCurrentStateCurrentPCUser::~CCVGFWTheCurrentStateCurrentPCUser()
{
}

/// @brief Retrieve the computer's unique ID currently running CodeXL.
/// @return gtString Text ID.
gtString CCVGFWTheCurrentStateCurrentPCUser::GetComputerIDCurrent() const
{
  gtString strReturn;
  gtString localMachineName(CCVGRSRC(kIDS_CCVG_STR_word_unknown_brked));
  const bool bOk = osGetLocalMachineName(strReturn);
  if (bOk)
  {
    localMachineName = strReturn;
  }

  gtString osName;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
  osName = L"Windows";
#else
  osName = L"Linux";
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

  gtString pcId;
  pcId.appendFormattedString(L"PCName_'%ls'_OS_'%ls'", localMachineName.asCharArray(), osName.asCharArray());

  return pcId;
}

/// @brief Retrieve the user's log on ID used to currently run CodeXL.
/// @return gtString Text ID.
gtString CCVGFWTheCurrentStateCurrentPCUser::GetUserIDCurrent() const
{
  gtString strUsrName;
  gtString strUsrDomain;
  gtString currentUserName(CCVGRSRC(kIDS_CCVG_STR_word_unknown_brked));
  gtString currentDomainName(CCVGRSRC(kIDS_CCVG_STR_word_unknown_brked));
  const bool bOk = osGetLocalMachineUserAndDomain(strUsrName, strUsrDomain);
  if (bOk)
  {
    currentUserName = strUsrName;
    currentDomainName = strUsrDomain;
  }

  gtString userId;
  userId.appendFormattedString(L"Domain_'%ls'_UserName_'%ls'", strUsrDomain.asCharArray(), currentUserName.asCharArray());

  return userId;
}

} // namespace fw
} // namespace ccvg
