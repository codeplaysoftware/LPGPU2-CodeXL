// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdCmdBase implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdBase.h>

namespace ccvg {
namespace cmd {

/// @brief    Class constructor. No work is done here by this class.
CCVGCmdBase::CCVGCmdBase()
: m_refCount(0)
, m_bBeenInitialised(false)
, m_bBeenShutdown(false)
, m_cmdID(CmdID::kInvalid)
, m_cmdName(L"<no cmd name>")
, m_pSelfCreatorFn(nullptr)
, m_bIsExecuteDone(false)
, m_bFinishedTask(false)
, m_pCmdParent(nullptr)
{
}

/// @brief    Set *this command's parent. A command has a parent if a command
///           calls another command. Function normally only used by the command
///           Invoker.
/// @param[in] vCmdParent The parent command of *this commmand.
void CCVGCmdBase::SetParent(CCVGCmdBase &vCmdParent)
{
  m_pCmdParent = &vCmdParent;
}

/// @brief  Retreive *this commands' parent command. A command a valid pointer
///         to its parent when it is a sub command that parent (the parent is
///         calling on other commands to help it do its task).
/// @return CCVGCmdBase* Parent command object pointer of NULL pointer.
CCVGCmdBase* CCVGCmdBase::GetParent() const
{
  return m_pCmdParent;
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdBase::~CCVGCmdBase()
{
}

/// @brief  Retrieve the command unique identifier.
/// @return CmdID Command enumeration ID.
CmdID CCVGCmdBase::GetCmdID() const
{
  return m_cmdID;
}

/// @brief  Retrieve the command's name. The command's name is user facing.
/// @return CmdID Command enumeration ID.
const gtString& CCVGCmdBase::GetCmdName() const
{
  return m_cmdName;
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdBase::Undo()
{
  // Overide to implement undo functionality
  // Be sure this to call this base function
  return failure;
}

/// @brief  Retrieve the flag stating whether *this command finished it task
///         either doing Execute() or Undo(). The command Invoker checks for 
///         this so as to issue a warning and not store the command in the 
///         command queue. The command onnly returns true when it has reached
///         it's end. For when this command is operated in its own thread the
///         will know the command completed (success or failure) or was 
///         terminated early.
/// @return bool True = finished 
bool CCVGCmdBase::GetCmdFinishedTaskFlag() const
{
  return m_bFinishedTask;
}

/// @brief  *this command's execute complete acknowledged flag set by the 
///         Invoker. The command's Undo() will not be executed until this is
///         set. Undo() cannot be called before Execute().
void CCVGCmdBase::SetExecuteDone()
{
  m_bIsExecuteDone = true;
}

/// @brief  *this command's execute complete acknowledged flag set by the 
///         Invoker. The command's Undo() will not be executed until this is
///         set. Undo() cannot be called before Execute().
bool CCVGCmdBase::GetExecuteDone() const
{
  return m_bIsExecuteDone;
}

} // namespace ccvg 
} // namespace cmd 
