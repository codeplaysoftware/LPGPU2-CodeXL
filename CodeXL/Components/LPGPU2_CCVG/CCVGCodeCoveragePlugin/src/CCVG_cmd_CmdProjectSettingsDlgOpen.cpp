// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdProjectSettingDlgOpen implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjectSettingsDlgOpen.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdProjectSettingDlgOpen::CreateSelf()
{
  return new (std::nothrow) CCVGCmdProjectSettingDlgOpen();
}

/// @brief Class constructor. Initialise *this command's attributes.
CCVGCmdProjectSettingDlgOpen::CCVGCmdProjectSettingDlgOpen()
: m_paramFlagUseDlgToGetSettings(kParamID_UseDlgFlag, true, L"bool", L"Use dialog flag", L"Retrieve settings from the project settings dialog or use parameter container" )
{
  m_cmdID = CmdID::kProjectSettingDlgOpen;
  m_cmdName = L"CmdProjectSettingDlgOpen";
  m_pSelfCreatorFn = &CCVGCmdProjectSettingDlgOpen::CreateSelf;

  m_listCmdParameters.push_back(&m_paramFlagUseDlgToGetSettings);
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdProjectSettingDlgOpen::~CCVGCmdProjectSettingDlgOpen()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdProjectSettingDlgOpen::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Class shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdProjectSettingDlgOpen::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  m_bBeenShutdown = true;

  return success;
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdProjectSettingDlgOpen::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief  The Execute() is the command work function to carry out work. Part 
///         of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdProjectSettingDlgOpen::Execute()
{
  m_bFinishedTask = false;
  
  const bool bUseDlg = m_paramFlagUseDlgToGetSettings.param.Get<bool>();
  if (bUseDlg)
  {
    afApplicationCommands *pApplicationCommands = afApplicationCommands::instance();
    if (nullptr == pApplicationCommands)
    {
      m_bFinishedTask = true;
      return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_fw_command_instance_not_available));
    }
   
    pApplicationCommands->OnProjectSettings(CCVGRSRC(kIDS_CCVG_STR_project_settings_extn_display_name));
  }

  // Set project's settings
  // Do something

  m_bFinishedTask = true;

  return success;
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdProjectSettingDlgOpen::Undo()
{
  m_bFinishedTask = false;
  
  if (!GetIsUndoableFlag())
  {
    m_bFinishedTask = true;
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable));
  }
  
  m_bFinishedTask = true;

  return success;
}

/// @brief  Ask the command if it should be undoable, it is able to reverse
///         the task carried out in its Execute() function. It may be the has
///         indicate is cannot undo a previous action because it for example
///         performing a GUI operation i.e. open a dialog.
/// @return bool True = can undo, false = cannot undo.
bool CCVGCmdProjectSettingDlgOpen::GetIsUndoableFlag() const
{
  return true;
}

} // namespace ccvg 
} // namespace cmd 