// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdInvoker implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Invoker.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_Factory.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBase.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_fw_TheCurrentState.h>
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_MsgDlg.h>

namespace ccvg {
namespace cmd {

/// @brief    Class constructor. No work is done here by this class.
CCVGCmdInvoker::CCVGCmdInvoker()
: m_pCmdFactory(nullptr)
, m_bHaveCmdsToUndo(false)
, m_bCmdUndoDisabled(false)
, m_bCmdUndoOperating(false)
, m_constTxtNoCurrentCmdATM(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_msg_noCurrentCmdATM))
, m_pCmdSubOfAnotherCmd(nullptr)
, m_pCmdCurrent(nullptr)
{
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdInvoker::~CCVGCmdInvoker()
{
  Shutdown();
}

/// @brief  Package initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdInvoker::Initialise()
{
  m_refCount++;
  if (m_bBeenInitialised)
  {
    return success;
  }

  const status status = CCVGCmdFactory::Instance().Initialise();
  if (status == failure)
  {
    const wchar_t *pErr = CCVGCmdFactory::Instance().ErrorGetDescriptionW();
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_string_err_fail_init), pErr));
  }
  m_pCmdFactory = &CCVGCmdFactory::Instance();

  m_bBeenInitialised = true;

  return success;
}

/// @brief  Package shutdown, tear down resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdInvoker::Shutdown()
{
  if (m_bBeenShutdown || (--m_refCount != 0))
  {
    return success;
  }

  status status = success;
  if (m_pCmdFactory != nullptr)
  {
    status = m_pCmdFactory->Shutdown();
    if (status == failure)
    {
      const wchar_t *pErr = m_pCmdFactory->ErrorGetDescriptionW();
      ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_string_err_fail_shtdwn), pErr));
    }
    m_pCmdFactory = nullptr;
  }

  ccvg::fnstatus::status cmdStatus = ClearCmds();
  status = (status == success) ? cmdStatus : failure;

  m_bBeenShutdown = true;

  return status;
}

/// @brief      Create and initialise a specified command object ready to be
///             then executed using the Invoker's Execute() function. Use this
///             when the command does not have any parameters to be filled in
///             prior to it's execution. If the command does have parameters
///             use the Invoker::CmdGetParams() function instead. 
///             CmdGetParams() calls this functions.
/// @param[in]  vkCmdID Command unique identifier.
/// @return     status success = all ok, 
///             failure = error occurred see error description.
status CCVGCmdInvoker::CmdInit(CmdID vkCmdID)
{
  CCVGCmdBase *pCmd = nullptr;
  if (m_pCmdFactory->CmdCreate(vkCmdID, pCmd) == failure)
  {
    const wchar_t *pErr = m_pCmdFactory->ErrorGetDescriptionW();
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err), pErr));
  }

  if (pCmd->Initialise() == failure)
  {
    const wchar_t *pErr = pCmd->ErrorGetDescriptionW();
    const wchar_t *pCmdName = pCmd->GetCmdName().asCharArray();
    return ErrorSet(gtString().appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_init_fail), pCmdName, pErr));
  }

  // Is a command calling another command
  m_pCmdSubOfAnotherCmd = nullptr;
  if (m_pCmdCurrent != nullptr)
  {
    // Not allowed to set up more one commmand at a time
    pCmd->SetParent(*m_pCmdCurrent);
    m_pCmdSubOfAnotherCmd = pCmd;
  }
  
  m_pCmdCurrent = pCmd;

  return success;
}

/// @brief      Retrieve a specified command's parameter container to be filled
///             by the client. The container is the actual command's container.
///             Calling this function also calls the Invoker's CmdInit() so the
///             command ready to be then executed using the Invoker's Execute() 
///             function.
/// @param[in]    vkCmdID Command unique identifier.
/// @param[inout] vpParams Command's parameters.
/// @return       status success = all ok, 
///               failure = error occurred see error description.
status CCVGCmdInvoker::CmdGetParams(CmdID vkCmdID, CCVGCmdParams *&vpParams)
{
  const status status = CmdInit(vkCmdID);
  if ((status == success) && m_pCmdCurrent->GetHasParams())
  {
    vpParams = &m_pCmdCurrent->GetParams();
  }

  return status;
}

/// @brief  Set the command Invoker so that it will not allow command undo
///         functioality to be called. This cannot be set once at least one
///         command has been executed.
/// @return status success = all ok, 
///         failure = error occurred see error description.
status CCVGCmdInvoker::CmdSetDisableUndoFunctionality()
{
  if (m_bCmdUndoOperating)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_undo_already_in_operation));
  }

  m_bCmdUndoDisabled = true;
  return success;
}

/// @brief      Execute a command which has been created, initialised with or
///             without parameters being set beforehand. Call either 
///             Invoker::CmdGetParamsOnce() or Invoker::CmdInit() before 
///             calling this function. Once a command has completed it's task
///             successfully it can optionally be called by its Undo() 
///             function. Should a commmand fail (it reports it has failed)
///             the error description is copied before the command object is
///             deleted. If the command is successful in it's task then it is
///             placed on the stack of commmands ready to be undone.
/// @return     status success = all ok, 
///             failure = error occurred see error description.
status CCVGCmdInvoker::CmdExecute()
{
  if (m_pCmdCurrent == nullptr)
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_not_init_for_use));
  }
  ccvg::fw::CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  bool bOk = true;
  bool bExceptionCaught = false;
  status cmdStatus = success;
  const wchar_t *pCmdName = m_pCmdCurrent->GetCmdName().asCharArray();
  try
  {
    cmdStatus = m_pCmdCurrent->Execute();
  }
  catch (...)
  {
    bExceptionCaught = true;
  }
  if (!bExceptionCaught && (cmdStatus == failure))
  {
    bOk = false;
    gtString msg;
    const wchar_t *pErr = m_pCmdCurrent->ErrorGetDescriptionW();
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_failed), pCmdName, pErr);
    cmdStatus = m_pCmdCurrent->Shutdown();
    if (cmdStatus == failure)
    {
      const wchar_t *pErr2 = m_pCmdCurrent->ErrorGetDescriptionW();
      msg.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_word_new_sentence), pErr2);
    }

    // Pass back to the parent if one
    CCVGCmdBase *pCurrCmd = m_pCmdCurrent;
    CCVGCmdBase *pCmdParent = m_pCmdCurrent->GetParent();
    if (pCmdParent != nullptr)
    {
      m_pCmdCurrent = pCmdParent;
    }
    else
    {
      m_pCmdCurrent = nullptr;
    }

    delete pCurrCmd;
    ErrorSet(msg);
  }
  if (bExceptionCaught)
  {
    bOk = false;
    gtString msg;
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_exceptionCaught), pCmdName);
    cmdStatus = m_pCmdCurrent->Shutdown();
    if (cmdStatus == failure)
    {
      const wchar_t *pErr2 = m_pCmdCurrent->ErrorGetDescriptionW();
      msg.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_word_new_sentence), pErr2);
    }
    
    // Pass back to the parent if one
    CCVGCmdBase *pCurrCmd = m_pCmdCurrent;
    CCVGCmdBase *pCmdParent = m_pCmdCurrent->GetParent();
    if (pCmdParent != nullptr)
    {
      m_pCmdCurrent = pCmdParent;
    }
    else
    {
      m_pCmdCurrent = nullptr;
    }

    delete pCurrCmd;
    ErrorSet(msg);
  }
     
  if (bOk)
  {
    // Pass back to the parent if one
    CCVGCmdBase *pCurrCmd = m_pCmdCurrent;
    CCVGCmdBase *pCmdParent = m_pCmdCurrent->GetParent();
    if (pCmdParent != nullptr)
    {
      m_pCmdCurrent = pCmdParent;
    }
    else
    {
      m_pCmdCurrent = nullptr;
    }

    // For when (if) command is running in it's own thread and it has been terminated early
    if (pCurrCmd->GetCmdFinishedTaskFlag())
    {
      pCurrCmd->SetExecuteDone();
      // If terminated early report terminated early 
    }

    LogCommand(*pCurrCmd);

    if (pCmdParent == nullptr)
    {
      if (m_bCmdUndoDisabled || !pCurrCmd->GetIsUndoableFlag())
      {
        delete pCurrCmd;
      }
      else
      {
        // Record for Undo
        m_stackCmdObjsUndo.push(pCurrCmd);
        m_bHaveCmdsToUndo = true;
        m_bCmdUndoOperating = true;
      }
    }
  }
 
  return bOk ? success : failure;
}

/// @brief  Undo the action or task of the last command executed. The command 
///         is undone and deleted. A command is still
///         deleted should  the command fail in its undo task.
/// @param[in] vSubCmd The command to carry out undo work.
/// @return status success = all ok, 
///         failure = error occurred see error description.
status CCVGCmdInvoker::CmdUndoSubCmd(CCVGCmdBase &vSubCmd)
{
  CCVGCmdBase *pCmdParent = vSubCmd.GetParent();
  const wchar_t *pCmdName = vSubCmd.GetCmdName().asCharArray();
  if (pCmdParent == nullptr)
  {
    gtString msg;
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_subCmdNotSubCmd), pCmdName);
    return ErrorSet(msg);
  }

  return CmdUndoPrivate(vSubCmd); 
}

/// @brief  Undo the action or task of the last command executed. The command 
///         is removed from the command stack and deleted. A command is still
///         deleted should  the command fail in its undo task.
/// @return status success = all ok, 
///         failure = error occurred see error description.
status CCVGCmdInvoker::CmdUndo()
{
  if (m_stackCmdObjsUndo.empty())
  {
    return ErrorSet(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_undo_cmd_stack_empty));
  }

  CCVGCmdBase *pCmd = m_stackCmdObjsUndo.top();
  CCVGCmdBase *pCmdParent = pCmd->GetParent();
  const wchar_t *pCmdName = pCmd->GetCmdName().asCharArray();
  if (pCmdParent != nullptr)
  {
    gtString msg;
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_subCmdFromCmdStack), pCmdName);
    return ErrorSet(msg);
  }

  status ok = CmdUndoPrivate(*pCmd);

  delete pCmd;
  m_stackCmdObjsUndo.pop();
  if (m_stackCmdObjsUndo.empty())
  {
    m_bHaveCmdsToUndo = false;
  }

  return ok ? success : failure;
}

/// @brief  Undo the action or task of the last command executed. 
/// @return status success = all ok, 
///         failure = error occurred see error description.
status CCVGCmdInvoker::CmdUndoPrivate(CCVGCmdBase &vSubCmd)
{
  const wchar_t *pCmdName = vSubCmd.GetCmdName().asCharArray();
  if (!vSubCmd.GetExecuteDone())
  {
    gtString msg;
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_err_called_undo_when_exec_not_finished), pCmdName);
    return ErrorSet(msg);
  }

  ccvg::fw::CCVGFWTheCurrentStateSetIsBusy CCVGIsBusy;

  bool bOk = true;
  bool bExceptionCaught = false;
  status cmdStatus = success;
  try
  {
    cmdStatus = vSubCmd.Undo();
  }
  catch (...)
  {
    bExceptionCaught = true;
  }
  if (!bExceptionCaught && (cmdStatus == failure))
  {
    bOk = false;
    gtString msg;
    const wchar_t *pErr = vSubCmd.ErrorGetDescriptionW();
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_failed), pCmdName, pErr);
    cmdStatus = vSubCmd.Shutdown();
    if (cmdStatus == failure)
    {
      const wchar_t *pErr2 = vSubCmd.ErrorGetDescriptionW();
      msg.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_word_new_sentence), pErr2);
    }
    ErrorSet(msg);
  }
  if (bExceptionCaught)
  {
    bOk = false;
    gtString msg;
    msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_undo_exceptionCaught), pCmdName);
    cmdStatus = vSubCmd.Shutdown();
    if (cmdStatus == failure)
    {
      const wchar_t *pErr2 = vSubCmd.ErrorGetDescriptionW();
      msg.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_word_new_sentence), pErr2);
    }
    ErrorSet(msg);
  }

  if (bOk)
  {
    LogCommand(vSubCmd);

    // For when (if) command is running in it's own thread and it has been terminated early
    if (vSubCmd.GetCmdFinishedTaskFlag())
    {
      // Not implemented
      // Report command terminated early
    }
  }

  return bOk ? success : failure;
}

/// @brief  Retrieve the name of the command is about to be executed.
///         A command becomes current after it is initialised by the Invoker
///         or parameters requested from a command using the Invoker.
/// @return gtString Name of the command or text stating there is no current
///         command.
gtString CCVGCmdInvoker::CmdGetNameAboutToExecute() const
{
  if (m_pCmdCurrent != nullptr)
  {
    return m_pCmdCurrent->GetCmdName();
  }
  else
  {
    return m_constTxtNoCurrentCmdATM;
  }
}

/// @brief  Client (like the GUI) can ask if there are any commands to carry
///         out their undo tasks.
/// @return bool True = Yes 1 or more commands can be undone, false = no 
///         commands.
bool CCVGCmdInvoker::HaveCommandsToUndo() const
{
  return !m_bCmdUndoDisabled && m_bHaveCmdsToUndo;
}

/// @brief  Log information about commands as they are executed or undone.
/// @return status success = all ok, 
///         failure = error occurred see error description.
status CCVGCmdInvoker::LogCommand(const CCVGCmdBase &vCmd)
{
  GT_UNREFERENCED_PARAMETER(vCmd);

  // ToDo: implement
  return success;
}

/// @brief  Clear the commands placed on the command stack. Each command's
///         Shutdown() function is called prior to being deleted. Any errors
///         from a command or appended to the error description.
/// @return status success = all ok, 
///         failure = error occurred see error description.
status CCVGCmdInvoker::ClearCmds()
{
  gtString errMsg;
  bool bHaveError = false;
  CCVGuint count = 0;

  while (!m_stackCmdObjsUndo.empty())
  {
    CCVGCmdBase *pCmd = m_stackCmdObjsUndo.top();
    if (pCmd->Shutdown() == failure)
    {
      bHaveError = true;
      const wchar_t *pErr = m_pCmdCurrent->ErrorGetDescriptionW();
      const wchar_t *pCmdName = m_pCmdCurrent->GetCmdName().asCharArray();
      gtString msg;
      msg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_err_msg), pCmdName, pErr);
      if (count++ > 0)
      {
        errMsg.appendFormattedString(CCVGRSRC(KIDS_CCVG_STR_word_new_sentence), msg.asCharArray());
      }
      else
      {
        errMsg = msg;
      }
    }
    delete pCmd;
    m_stackCmdObjsUndo.pop();
  }

  if (bHaveError) 
  {
    return ErrorSet(errMsg);
  }

  return success;
}

/// @brief Utility function to handle common code for execution of commands. 
///        Called after a command is executed. Forms appropriate error 
///        messages then displays a modal dailog with the error.
/// @param[in] vFnStatus Calling function's status state.
/// @param[in] vbExceptionCaught True = command through exception, false = 
///            command failed in its task.
/// @param[in] vrCmdName Name of the command that failed.
/// @return status Return appropriate function status to shorten caller's
///         code.
status CCVGCmdInvoker::CmdHandleError(status vFnStatus, bool vbExceptionCaught, const gtString &vrCmdName)
{
  if (vFnStatus == success)
  {
    return success;
  }

  const wchar_t *pCmdName = vrCmdName.asCharArray();
  gtString errMsg;
  if (vbExceptionCaught)
  {
    errMsg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_exceptionCaught), pCmdName);
  }
  else
  {
    const wchar_t *pCmdErr = ErrorGetDescription().asCharArray();
    errMsg.appendFormattedString(CCVGRSRC(kIDS_CCVG_STR_pkg_cmd_invoker_err_cmd_execute_failed), pCmdName, pCmdErr);
  }

  CCVGUtilMsgDlg::Show(errMsg);

  return failure;
}

} // namespace cmd
} // namespace ccvg
