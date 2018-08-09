// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdCmdSwitchCCVGMode implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// Framework:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdSwitchCCVGMode.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdSwitchCCVGMode::CreateSelf()
{
  return new (std::nothrow) CCVGCmdSwitchCCVGMode();
}

/// @brief Class constructor. Initialise *this command's mandatory attributes.
CCVGCmdSwitchCCVGMode::CCVGCmdSwitchCCVGMode()
: m_strCCVGExecMode(CCVGRSRC(kIDS_CCVG_STR_execmode_name))
{
  m_cmdID = CmdID::kSwitchCCVGMode;
  m_cmdName = L"CmdSwitchCCVGMode";
  m_pSelfCreatorFn = &CCVGCmdSwitchCCVGMode::CreateSelf;
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdSwitchCCVGMode::~CCVGCmdSwitchCCVGMode()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdSwitchCCVGMode::Initialise()
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
status CCVGCmdSwitchCCVGMode::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  m_bBeenShutdown = true;

  return success;
}

/// @brief  Retrieve whether *this command has parameters to be filled in by the
///         client.
/// @return bool True = Yes command has parameters, false = no parameters.
bool CCVGCmdSwitchCCVGMode::GetHasParams() const
{
  return false;
}

/// @brief  Retrieve *this command's parameters container to be filled in by the
///         client.
/// @return CCVGCmdParams& Reference to container.
CCVGCmdParams& CCVGCmdSwitchCCVGMode::GetParams()
{
  return m_cmdParamsDummy;
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdSwitchCCVGMode::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief  The Execute() is the command work function to carry out work. Part 
///         of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdSwitchCCVGMode::Execute()
{
  m_bFinishedTask = false;
  
  afIExecutionMode *pPrevExecMode = afExecutionModeManager::instance().activeMode();
  m_strPrevExecMode = (pPrevExecMode != nullptr) ? pPrevExecMode->modeName() : gtString();
  apExecutionModeChangedEvent executionModeEvent(m_strCCVGExecMode, 0);
  apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent); 

  m_bFinishedTask = true;

  return success;
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdSwitchCCVGMode::Undo()
{
  m_bFinishedTask = false;
  
  if (!GetIsUndoableFlag())
  {
    m_bFinishedTask = true;
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_not_undoable));
  }

  if ((m_strCCVGExecMode == m_strPrevExecMode) || m_strPrevExecMode.isEmpty())
  {
    m_bFinishedTask = true;
    return success;
  }
  
  apExecutionModeChangedEvent executionModeEvent(m_strPrevExecMode, 0);
  apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
  
  m_bFinishedTask = true;

  return success;
}

/// @brief  Ask the command if it should be undoable, it is able to reverse
///         the task carried out in its Execute() function. It may be the has
///         indicate is cannot undo a previous action because it for example
///         performing a GUI operation i.e. open a dialog.
/// @return bool True = can undo, false = cannot undo.
bool CCVGCmdSwitchCCVGMode::GetIsUndoableFlag() const
{
  return true;
}

} // namespace ccvg 
} // namespace cmd 