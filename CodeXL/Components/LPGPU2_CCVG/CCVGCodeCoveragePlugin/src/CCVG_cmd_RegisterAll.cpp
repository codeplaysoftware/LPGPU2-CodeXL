// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_RegisterAll.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjectNew.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjectClose.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdSwitchCCVGMode.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjectSettingsDlgOpen.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadAll.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadSource.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadResults.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadDrivers.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFilesReadExecuteable.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjDataSetNoDriverFilesReq.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdWndSrcOpen.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdFileRead.h>

namespace ccvg {
namespace cmd {

template<typename T> static ccvg::fnstatus::status Register();
template<typename T> static ccvg::fnstatus::status RegisterCmd(const ccvg::fnstatus::status &vPrevRegisterStatus);

} // namespace cmd
} //  namespace ccvg

/// @brief         Global template function. CCVG command to Command Factory 
///                registration function.
/// @param         T A command type class.
/// @return status Success = command is registered, false = command failed to 
///                register.
/// @warning  None.
/// @see      ccvg::cmd::RegisterAll, ccvg::cmd::Factory.
/// @author   Illya Rudkin.
template<typename T>
static ccvg::fnstatus::status ccvg::cmd::Register()
{
  static ccvg::cmd::CCVGCmdFactory &rCmdFactory = ccvg::cmd::CCVGCmdFactory::Instance();
  const T cmd;
  const ccvg::cmd::CmdID id = cmd.GetCmdID();
  const gtString &rCmdName(cmd.GetCmdName());
  ccvg::cmd::CCVGCmdFactory::CmdCreatorFnPtr fn = cmd.GetCmdCreatorFn();
  return rCmdFactory.CmdRegister(id, fn, rCmdName);
}

/// @brief Global template function RegisterCmd().
///          
/// Macro like function to reduce code clutter.
///
/// @param     T A command type class.
/// @param[in] vPrevRegisterStatus Registration task status previously.
/// @param[in] vPrevRegisterStatus Registration task status previously.
/// @return    memmgr::fnstatus::status success or failure.
/// @warning   None.
/// @see       ccvg::cmd::Register, ccvg::cmd::Factory.
/// @author    Illya Rudkin.
template<typename T> 
static ccvg::fnstatus::status ccvg::cmd::RegisterCmd(const ccvg::fnstatus::status &vPrevRegisterStatus) {
  return ((vPrevRegisterStatus == ccvg::fnstatus::success) && 
         (ccvg::fnstatus::success == Register<T>())) ? 
         ccvg::fnstatus::success : ccvg::fnstatus::failure;
}

/// @brief         CCVG Register commands with the Command Factory.
/// @return status Success = All commands are registered, failure = one of 
///                more commands failed to register.
ccvg::fnstatus::status ccvg::cmd::RegisterAll()
{
  ccvg::fnstatus::status status = ccvg::fnstatus::success;
  status = RegisterCmd<CCVGCmdProjectNew>(status);
  status = RegisterCmd<CCVGCmdProjectClose>(status);
  status = RegisterCmd<CCVGCmdSwitchCCVGMode>(status);
  status = RegisterCmd<CCVGCmdProjectSettingDlgOpen>(status);
  status = RegisterCmd<CCVGCmdFilesReadAll>(status);
  status = RegisterCmd<CCVGCmdFilesReadSource>(status);
  status = RegisterCmd<CCVGCmdFilesReadResults>(status);
  status = RegisterCmd<CCVGCmdFilesReadDrivers>(status);
  status = RegisterCmd<CCVGCmdFilesReadExecuteable>(status);
  status = RegisterCmd<CCVGCmdProjDataSetNoDriverFilesReq>(status);
  status = RegisterCmd<CCVGCmdWndSrcOpen>(status);
  status = RegisterCmd<CCVGCmdFileRead>(status);

  return status;
}

