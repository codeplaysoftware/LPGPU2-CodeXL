// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGCmdCmdProjectClose implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_cmd_CmdProjectClose.h>
#include <CCVGCodeCoveragePlugin/Include/CCVGResourcesString.h>

namespace ccvg {
namespace cmd {

/// @brief  Retrieve a new instance of *this class.
/// @return CCVGCmdBase* Pointer to new object, NULL on allocation failure.
CCVGCmdBase* CCVGCmdProjectClose::CreateSelf()
{
  return new (std::nothrow) CCVGCmdProjectClose();
}

/// @brief Class constructor. Initialise *this command's mandatory attributes.
CCVGCmdProjectClose::CCVGCmdProjectClose()
{
  m_cmdID = CmdID::kProjectClose;
  m_cmdName = L"CmdProjectClose";
  m_pSelfCreatorFn = &CCVGCmdProjectClose::CreateSelf;
}

/// @brief  Class destructor. Tidy up release resources used by *this instance.
CCVGCmdProjectClose::~CCVGCmdProjectClose()
{
  Shutdown();
}

/// @brief  Class initialise, setup resources or bindings.
/// @return status success = all ok, 
///                failure = error occurred see error description.
status CCVGCmdProjectClose::Initialise()
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
status CCVGCmdProjectClose::Shutdown()
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
bool CCVGCmdProjectClose::GetHasParams() const
{
  return false;
}

/// @brief  Retrieve *this command's parameters container to be filled in by the
///         client.
/// @return CCVGCmdParams& Reference to container.
CCVGCmdParams& CCVGCmdProjectClose::GetParams()
{
  return m_cmdParamsDummy;
}

/// @brief  Retrieve *this command's creation function.
/// @return CCVGCmdFactory::CmdCreatorFnPtr Function pointer.
CCVGCmdFactory::CmdCreatorFnPtr CCVGCmdProjectClose::GetCmdCreatorFn() const
{
  return m_pSelfCreatorFn;
}

/// @brief  The Execute() is the command work function to carry out work. Part 
///         of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdProjectClose::Execute()
{
  m_bFinishedTask = false;
  // Do work
  m_bFinishedTask = true;

  return success;
}

/// @brief  The opposite function to Execute(). Part of the command pattern.
/// @return status Succcess = task completed ok, failure = task failed.
status CCVGCmdProjectClose::Undo()
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
bool CCVGCmdProjectClose::GetIsUndoableFlag() const
{
  return true;
}

} // namespace ccvg 
} // namespace cmd 