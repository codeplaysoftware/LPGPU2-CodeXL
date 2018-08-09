// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdFactory implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Factory.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_RegisterAll.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace cmd {

/// @brief    Class constructor. No work is done here by this class.
CCVGCmdFactory::CCVGCmdFactory()
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdFactory::~CCVGCmdFactory()
{
  Shutdown();
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFactory::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  const status status = RegisterAll();
  if (status == failure)
  {
    const wchar_t *pErr = ErrorGetDescriptionW();
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_string_err_fail_init_register_cmds), pErr));
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFactory::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  m_mapCmdIdToCmdCreatorfn.clear();

  m_bBeenShutdown = true;

  return success;
}

/// @brief  Register a command with the command factory so that it is able to
///         create and instance of the command on demand.
/// @param[in] vkCmdID Command's unique ID.
/// @param[in] vCmdCreateFn The command's how to create itself function.
/// @param[in] vrCmdName The command's name.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFactory::CmdRegister(CmdID vkCmdID, CmdCreatorFnPtr vCmdCreateFn, const gtString &vrCmdName)
{
  if (vrCmdName.isEmpty())
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_cmd_name_invalid));
  }
  
  if (!IsValidCmdID(vkCmdID))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid), vrCmdName.asCharArray()));
  }

  if (HaveCmdIDAlreadyRegistered(vkCmdID))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_alreadyRegistered), vrCmdName.asCharArray()));
  }

  if (vCmdCreateFn == nullptr)
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_cmd_selfFnPtr_invalid), vrCmdName.asCharArray()));
  }
  
  MapPairCmdIdToCmdCreatorFn_t pair(vkCmdID, vCmdCreateFn);
  m_mapCmdIdToCmdCreatorfn.insert(pair);

  return success;
}

/// @brief  Create a scommand with the command factory so that it is able to
///         create and instance of the command on demand.
/// @param[in] vkCmdID Command's unique ID.
/// @param[out] vpCmd Pointer to a command object.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdFactory::CmdCreate(CmdID vkCmdID, CCVGCmdBase *&vpCmd)
{
  vpCmd = nullptr;

  if (!IsValidCmdID(vkCmdID))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid_useID), vkCmdID));
  }

  if (!HaveCmdIDAlreadyRegistered(vkCmdID))
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmdID_err_invalid_useID), vkCmdID));
  }

  const MapCmdIdToCmdCreatorFn_t::const_iterator it = m_mapCmdIdToCmdCreatorfn.find(vkCmdID);
  CmdCreatorFnPtr fn = (*it).second;
  vpCmd = fn();
  
  if (vpCmd == nullptr)
  {
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_factory_cmd_err_create_failed), vkCmdID));
  }

  return success;
}

/// @brief     Check the given command ID is a valid one.
/// @param[in] vkCmdID Command's unique ID.
/// @return    bool True = yes valid,  false = not valid.
bool CCVGCmdFactory::IsValidCmdID(CmdID vkCmdID) const
{
  const int cmdID = static_cast<int>(vkCmdID);
  if ((cmdID < 1) || (cmdID >= static_cast<int>(CmdID::kCmdCount)))
  {
    return false;
  }

  return true;
}

/// @brief     Check the given command ID is already registered with *this factory.
/// @param[in] vkCmdID Command's unique ID.
/// @return    bool True = yes registered,  false = not registered.
bool CCVGCmdFactory::HaveCmdIDAlreadyRegistered(CmdID vkCmdID) const
{
  const MapCmdIdToCmdCreatorFn_t::const_iterator it = m_mapCmdIdToCmdCreatorfn.find(vkCmdID);
  return (it != m_mapCmdIdToCmdCreatorfn.cend() ? true : false);
}

} // namespace ccvg 
} // namespace cmd 
